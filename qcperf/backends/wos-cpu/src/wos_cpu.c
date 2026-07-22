/*
        Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
        Redistribution and use in source and binary forms, with or without
        modification, are permitted (subject to the limitations in the
        disclaimer below) provided that the following conditions are met:
                * Redistributions of source code must retain the above copyright
                  notice, this list of conditions and the following disclaimer.
                * Redistributions in binary form must reproduce the above
                  copyright notice, this list of conditions and the following
                  disclaimer in the documentation and/or other materials provided
                  with the distribution.
                * Neither the name of Qualcomm Technologies, Inc. nor the names of its
                  contributors may be used to endorse or promote products derived
                  from this software without specific prior written permission.
        NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
        GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
        HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
        WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
        MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
        IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
        ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
        DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
        GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
        INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
        IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
        OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
        IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * @file wos_cpu.c
 * @brief Implementation of the WOS CPU backend for libqcperf
 * @author Vijay Kumbhani (vkumbhan@qti.qualcomm.com)
 *
 * This file implements the WOS (Windows on Snapdragon) CPU backend for
 * the QcPerf library. It provides per-core CPU utilization, frequency,
 * and effective utilization metrics using two telemetry sources:
 *   1. NtQuerySystemInformation + CallNtPowerInformation (preferred)
 *   2. Windows PDH Performance Data Helper (fallback)
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include "cpu_telemetry.h"
#include "qthread.h"
#include "qtime.h"
#include "wos_cpu.h"
#include "wos_cpu_logger.h"
#include "wos_cpu_info.h"

#define NS_TO_MS 1e6

static volatile bool g_is_thread_running                             = false;
static struct QThreadInfo g_thread_info                              = {0};
static QcPerfDataCallback g_data_callback                            = NULL;
static struct QcPerfBackendInfo g_backend_info                       = {0};
static enum eCpuTelemetrySource g_telemetry_source                   = CPU_TELEMETRY_SOURCE_NTQUERY;
static const uint16_t g_streaming_rates[WOS_CPU_STREAMING_RATES_LEN] = {WOS_CPU_STREAMING_RATES};
static const uint16_t g_sampling_rates[WOS_CPU_SAMPLING_RATES_LEN]   = {WOS_CPU_SAMPLING_RATES};

/* ============================================================================
 * Forward Declarations
 * ============================================================================ */

static enum QcPerfReturnCode wos_cpu_alloc(uint32_t num_cores);
static void wos_cpu_free_memory(void);
static enum QcPerfReturnCode wos_cpu_init(void);
static enum QcPerfReturnCode wos_cpu_info(struct QcPerfBackendInfo *backend_info);
static enum QcPerfReturnCode wos_cpu_set_message_callback(QcPerfMessageCallback message_callback);
static enum QcPerfReturnCode wos_cpu_set_data_callback(QcPerfDataCallback data_callback);
static enum QcPerfReturnCode wos_cpu_start(struct QcPerfRequest *request);
static enum QcPerfReturnCode wos_cpu_stop(struct QcPerfRequest *request);
static enum QcPerfReturnCode wos_cpu_deinit(void);
static void *wos_cpu_collect_metrics_thread(void *lpParam);

/* ============================================================================
 * Memory Management
 * ============================================================================ */

/**
 * @brief Allocate memory for backend capabilities and metrics
 *
 * @param[in] num_cores Number of CPU cores detected
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_CALLOC_FAILED if allocation fails
 */
static enum QcPerfReturnCode wos_cpu_alloc(uint32_t num_cores) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_SUCCESS;
    uint32_t total_metric_count       = WOS_CPU_TOTAL_METRICS + (num_cores * WOS_CPU_METRICS_PER_CORE);

    g_backend_info.capabilities_list = (struct QcPerfCapabilityInfo *)calloc(WOS_CPU_CAPABILITIES_LEN, sizeof(struct QcPerfCapabilityInfo));
    if (NULL == g_backend_info.capabilities_list) {
        return_code = QC_PERF_RETURN_CODE_CALLOC_FAILED;
    } else {
        g_backend_info.capabilities_list_length = WOS_CPU_CAPABILITIES_LEN;
        g_backend_info.capabilities_list[WOS_CPU_CAPABILITY_INDEX].metric_ids_list =
            (struct QcPerfMetricInfo *)calloc(total_metric_count, sizeof(struct QcPerfMetricInfo));
        if (NULL == g_backend_info.capabilities_list[WOS_CPU_CAPABILITY_INDEX].metric_ids_list) {
            free(g_backend_info.capabilities_list);
            g_backend_info.capabilities_list        = NULL;
            g_backend_info.capabilities_list_length = 0;
            return_code                             = QC_PERF_RETURN_CODE_CALLOC_FAILED;
        }
    }

    return return_code;
}

/**
 * @brief Free all allocated memory for backend information
 */
static void wos_cpu_free_memory(void) {
    if (NULL != g_backend_info.capabilities_list) {
        if (NULL != g_backend_info.capabilities_list[WOS_CPU_CAPABILITY_INDEX].metric_ids_list) {
            free(g_backend_info.capabilities_list[WOS_CPU_CAPABILITY_INDEX].metric_ids_list);
            g_backend_info.capabilities_list[WOS_CPU_CAPABILITY_INDEX].metric_ids_list = NULL;
        }
        g_backend_info.capabilities_list[WOS_CPU_CAPABILITY_INDEX].metric_ids_list_len = 0;
        free(g_backend_info.capabilities_list);
        g_backend_info.capabilities_list = NULL;
    }
    g_backend_info.capabilities_list_length = 0;
}

/* ============================================================================
 * Backend Interface Implementations
 * ============================================================================ */

/**
 * @brief Initialize the wos-cpu backend
 *
 * Detects CPU core count, allocates metric structures, and populates
 * capability information including per-core metrics.
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_FAILED if core count cannot be read
 * @return QC_PERF_RETURN_CODE_CALLOC_FAILED if memory allocation fails
 */
static enum QcPerfReturnCode wos_cpu_init(void) {
    enum QcPerfReturnCode return_code                    = QC_PERF_RETURN_CODE_SUCCESS;
    enum eCpuTelemetryReturnCode ntq_ret          = RETURN_CODE_CPU_TELEMETRY_SUCCESS;
    enum eCpuTelemetryReturnCode tel_ret                 = RETURN_CODE_CPU_TELEMETRY_SUCCESS;
    uint32_t num_cores                                   = 0;
    uint32_t metric_index                                = 0;
    uint32_t core_id                                     = 0;
    uint8_t i                                            = 0;
    struct QcPerfMetricInfo *metric                      = NULL;

    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Initializing wos-cpu backend");

    /* Try NtQuery first for core count, fall back to PDH */
    ntq_ret = cpuTelemetryNtQuery_getNumCores(&num_cores);
    if (RETURN_CODE_CPU_TELEMETRY_SUCCESS != ntq_ret) {
        tel_ret = cpuTelemetryNtQuery_getNumCores(&num_cores);
        if (RETURN_CODE_CPU_TELEMETRY_SUCCESS != tel_ret) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to get CPU core count from both NtQuery and PDH");
            return_code = QC_PERF_RETURN_CODE_FAILED;
        }
    }

    if (QC_PERF_RETURN_CODE_SUCCESS == return_code) {
        if (num_cores > WOS_CPU_MAX_CORES) {
            num_cores = WOS_CPU_MAX_CORES;
        }

        return_code = wos_cpu_alloc(num_cores);
        if (QC_PERF_RETURN_CODE_SUCCESS != return_code) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to allocate memory for backend");
        } else {
            g_backend_info.backend_id = QC_PERF_BACKEND_WOS_CPU;

            /* Configure capability */
            g_backend_info.capabilities_list[WOS_CPU_CAPABILITY_INDEX].capability_id = WOS_CPU_CAPABILITY_ID;
            g_backend_info.capabilities_list[WOS_CPU_CAPABILITY_INDEX].capability_name_len =
                (uint8_t)snprintf(g_backend_info.capabilities_list[WOS_CPU_CAPABILITY_INDEX].capability_name,
                                  CAPABILITY_NAME_MAX_LEN, "%s", WOS_CPU_CAPABILITY_NAME);

            /* Set streaming rates */
            for (i = 0; i < WOS_CPU_STREAMING_RATES_LEN && i < MAX_SAMPLING_STREAMING_RATES_LEN; i++) {
                g_backend_info.capabilities_list[WOS_CPU_CAPABILITY_INDEX].streaming_rate[i] = g_streaming_rates[i];
                g_backend_info.capabilities_list[WOS_CPU_CAPABILITY_INDEX].streaming_rate_len++;
            }

            /* Set sampling rates */
            for (i = 0; i < WOS_CPU_SAMPLING_RATES_LEN && i < MAX_SAMPLING_STREAMING_RATES_LEN; i++) {
                g_backend_info.capabilities_list[WOS_CPU_CAPABILITY_INDEX].sampling_rate[i] = g_sampling_rates[i];
                g_backend_info.capabilities_list[WOS_CPU_CAPABILITY_INDEX].sampling_rate_len++;
            }

            /* Populate total CPU utilization metric (ID 0) */
            metric                         = &g_backend_info.capabilities_list[WOS_CPU_CAPABILITY_INDEX].metric_ids_list[metric_index];
            metric->metric_id              = WOS_CPU_TOTAL_UTIL_ID;
            metric->metric_name_len        = (uint8_t)snprintf(metric->metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_CPU_METRIC_TOTAL_UTIL_NAME);
            metric->metric_description_len = (uint8_t)snprintf(metric->metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_CPU_METRIC_TOTAL_UTIL_DESCRIPTION);
            metric->metric_unit_len        = (uint8_t)snprintf(metric->metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_CPU_METRIC_TOTAL_UTIL_UNIT);
            metric_index++;

            /* Populate total CPU frequency metric (ID 1) */
            metric                         = &g_backend_info.capabilities_list[WOS_CPU_CAPABILITY_INDEX].metric_ids_list[metric_index];
            metric->metric_id              = WOS_CPU_TOTAL_FREQ_ID;
            metric->metric_name_len        = (uint8_t)snprintf(metric->metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_CPU_METRIC_TOTAL_FREQ_NAME);
            metric->metric_description_len = (uint8_t)snprintf(metric->metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_CPU_METRIC_TOTAL_FREQ_DESCRIPTION);
            metric->metric_unit_len        = (uint8_t)snprintf(metric->metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_CPU_METRIC_TOTAL_FREQ_UNIT);
            metric_index++;

            /* Populate per-core metrics */
            for (core_id = 0; core_id < num_cores; core_id++) {
                /* Per-core utilization */
                metric                         = &g_backend_info.capabilities_list[WOS_CPU_CAPABILITY_INDEX].metric_ids_list[metric_index];
                metric->metric_id              = WOS_CPU_CORE_BASE_ID(core_id) + WOS_CPU_UTIL_OFFSET;
                metric->metric_name_len        = (uint8_t)snprintf(metric->metric_name, METRIC_NAME_MAX_LEN, WOS_CPU_CORE_UTIL_NAME_FMT, core_id);
                metric->metric_description_len = (uint8_t)snprintf(metric->metric_description, MAX_METRIC_DESCRIPTION_LEN, WOS_CPU_CORE_UTIL_DESC_FMT, core_id);
                metric->metric_unit_len        = (uint8_t)snprintf(metric->metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_CPU_CORE_UTIL_UNIT);
                metric_index++;

                /* Per-core current frequency */
                metric                         = &g_backend_info.capabilities_list[WOS_CPU_CAPABILITY_INDEX].metric_ids_list[metric_index];
                metric->metric_id              = WOS_CPU_CORE_BASE_ID(core_id) + WOS_CPU_FREQ_OFFSET;
                metric->metric_name_len        = (uint8_t)snprintf(metric->metric_name, METRIC_NAME_MAX_LEN, WOS_CPU_CORE_FREQ_NAME_FMT, core_id);
                metric->metric_description_len = (uint8_t)snprintf(metric->metric_description, MAX_METRIC_DESCRIPTION_LEN, WOS_CPU_CORE_FREQ_DESC_FMT, core_id);
                metric->metric_unit_len        = (uint8_t)snprintf(metric->metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_CPU_CORE_FREQ_UNIT);
                metric_index++;

                /* Per-core max frequency */
                metric                         = &g_backend_info.capabilities_list[WOS_CPU_CAPABILITY_INDEX].metric_ids_list[metric_index];
                metric->metric_id              = WOS_CPU_CORE_BASE_ID(core_id) + WOS_CPU_MAX_FREQ_OFFSET;
                metric->metric_name_len        = (uint8_t)snprintf(metric->metric_name, METRIC_NAME_MAX_LEN, WOS_CPU_CORE_MAX_FREQ_NAME_FMT, core_id);
                metric->metric_description_len = (uint8_t)snprintf(metric->metric_description, MAX_METRIC_DESCRIPTION_LEN, WOS_CPU_CORE_MAX_FREQ_DESC_FMT, core_id);
                metric->metric_unit_len        = (uint8_t)snprintf(metric->metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_CPU_CORE_MAX_FREQ_UNIT);
                metric_index++;
            }

            g_backend_info.capabilities_list[WOS_CPU_CAPABILITY_INDEX].metric_ids_list_len = (uint8_t)metric_index;
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "wos-cpu backend initialized: %u cores, %u metrics", num_cores, metric_index);
        }
    }

    if (QC_PERF_RETURN_CODE_SUCCESS != return_code) {
        wos_cpu_free_memory();
    }

    return return_code;
}

/**
 * @brief Retrieve backend information including capabilities and metrics
 *
 * @param[out] backend_info Pointer to structure to be populated
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if backend_info is NULL
 */
static enum QcPerfReturnCode wos_cpu_info(struct QcPerfBackendInfo *backend_info) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == backend_info) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "backend_info is NULL");
        return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        *backend_info = g_backend_info;
    }

    return return_code;
}

/**
 * @brief Register the message callback for this backend
 */
static enum QcPerfReturnCode wos_cpu_set_message_callback(QcPerfMessageCallback message_callback) {
    wos_cpu_logger_set_message_callback(message_callback);
    return QC_PERF_RETURN_CODE_SUCCESS;
}

/**
 * @brief Register the data callback for delivering collected metrics
 */
static enum QcPerfReturnCode wos_cpu_set_data_callback(QcPerfDataCallback data_callback) {
    g_data_callback = data_callback;
    return QC_PERF_RETURN_CODE_SUCCESS;
}

/**
 * @brief Start CPU performance data collection
 *
 * Initializes the preferred telemetry source (NtQuery with PDH fallback),
 * and spawns the data collection thread.
 *
 * @param[in] request Pointer to the performance request structure
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful start
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if request is NULL
 * @return QC_PERF_RETURN_CODE_FAILED if telemetry init or thread creation fails
 */
static enum QcPerfReturnCode wos_cpu_start(struct QcPerfRequest *request) {
    enum QcPerfReturnCode return_code                    = QC_PERF_RETURN_CODE_SUCCESS;
    enum eCpuTelemetryReturnCode tel_ret                 = RETURN_CODE_CPU_TELEMETRY_SUCCESS;
    struct QThreadAttributes thread_attrs                = {0};
    enum QThreadReturnCode thread_return_code            = RET_QTHREAD_CREATE_SUCCESS;

    if (NULL == request) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "QcPerfRequest NULL pointer");
        return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else if (NULL == g_data_callback) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Data callback is not set");
        return_code = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
    } else if (true == g_is_thread_running) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Already running capability=%u", request->capability_id);
        return_code = QC_PERF_RETURN_CODE_ALREADY_INITIALIZED;
    } else {
        /* Try NtQuery first, fall back to PDH if it fails */
        tel_ret = cpuTelemetryNtQuery_init();
        if (RETURN_CODE_CPU_TELEMETRY_SUCCESS == tel_ret) {
            g_telemetry_source = CPU_TELEMETRY_SOURCE_NTQUERY;
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "CPU telemetry using NtQuerySystemInformation + CallNtPowerInformation");
        } else {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_WARNING, "NtQuery init failed (error=%u), trying PDH fallback", tel_ret);
            tel_ret = cpuTelemetryPdh_init();
            if (RETURN_CODE_CPU_TELEMETRY_SUCCESS == tel_ret) {
                g_telemetry_source = CPU_TELEMETRY_SOURCE_PDH;
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "CPU telemetry using Windows PDH (Performance Data Helper)");
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_WARNING, "PDH does not provide per-core max frequency; max frequency metrics will report 0");
            } else {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Both NtQuery and PDH telemetry init failed");
                return_code = QC_PERF_RETURN_CODE_FAILED;
            }
        }

        if (QC_PERF_RETURN_CODE_SUCCESS == return_code) {
            g_is_thread_running = true;

            memset(&thread_attrs, 0, sizeof(struct QThreadAttributes));
            snprintf((char *)thread_attrs.thread_name, THREAD_NAME_SIZE, "wos-cpu-metrics-thread");
            thread_attrs.thread_name_len = (uint8_t)strlen((char *)thread_attrs.thread_name);
            thread_attrs.stack_size      = 0;
            thread_attrs.thread_params   = request;
            thread_attrs.thread_fn       = wos_cpu_collect_metrics_thread;

            thread_return_code = thread_create(&thread_attrs, &g_thread_info);
            if (RET_QTHREAD_CREATE_SUCCESS != thread_return_code) {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to start CPU collection thread");
                g_is_thread_running = false;
                if (CPU_TELEMETRY_SOURCE_NTQUERY == g_telemetry_source) {
                    cpuTelemetryNtQuery_destroy();
                } else {
                    cpuTelemetryPdh_destroy();
                }
                return_code = QC_PERF_RETURN_CODE_FAILED;
            } else {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "CPU collection thread started successfully");
            }
        }
    }

    return return_code;
}

/**
 * @brief Stop CPU performance data collection
 *
 * @param[in] request Pointer to the performance request structure
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful stop
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if request is NULL
 * @return QC_PERF_RETURN_CODE_FAILED if thread join or destroy fails
 */
static enum QcPerfReturnCode wos_cpu_stop(struct QcPerfRequest *request) {
    enum QcPerfReturnCode return_code        = QC_PERF_RETURN_CODE_SUCCESS;
    enum QThreadReturnCode thread_return_code = RET_QTHREAD_CREATE_SUCCESS;

    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_DEBUG, "Stopping wos-cpu profiling");

    if (NULL == request) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "QcPerfRequest NULL pointer");
        return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        if (false == g_is_thread_running) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Already stopped capability=%u", request->capability_id);
        } else {
            g_is_thread_running = false;
            thread_return_code  = thread_join(&g_thread_info);
            if (RET_QTHREAD_JOIN_SUCCESS != thread_return_code) {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to join CPU collection thread");
                return_code = QC_PERF_RETURN_CODE_FAILED;
            } else {
                thread_return_code = thread_destroy(&g_thread_info);
                if (RET_QTHREAD_DESTROY_SUCCESS != thread_return_code) {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to destroy thread handle");
                    return_code = QC_PERF_RETURN_CODE_FAILED;
                } else {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "CPU collection thread stopped successfully");
                }
            }

            if (CPU_TELEMETRY_SOURCE_NTQUERY == g_telemetry_source) {
                cpuTelemetryNtQuery_destroy();
            } else {
                cpuTelemetryPdh_destroy();
            }
        }
    }

    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_DEBUG, "wos-cpu profiling stopped");
    return return_code;
}

/**
 * @brief Deinitialize the wos-cpu backend and free all resources
 */
static enum QcPerfReturnCode wos_cpu_deinit(void) {
    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Deinitializing wos-cpu backend");
    wos_cpu_free_memory();
    g_data_callback = NULL;
    return QC_PERF_RETURN_CODE_SUCCESS;
}

/* ============================================================================
 * Data Collection Thread
 * ============================================================================ */

/**
 * @brief Thread function for collecting CPU performance metrics
 *
 * @param[in] lpParam Pointer to QcPerfRequest structure
 * @return NULL on completion
 */
static void *wos_cpu_collect_metrics_thread(void *lpParam) {
    struct QcPerfRequest *request                         = (struct QcPerfRequest *)lpParam;
    uint32_t sample_count                                = 1;
    uint64_t elapsed_ms                                  = 0;
    uint64_t start_time                                  = 0;
    uint64_t end_time                                    = 0;
    uint32_t metric_response_index                       = 0;
    uint32_t metric_response_allocated                   = 0;
    uint16_t current_sample                              = 0;
    uint64_t current_time                                = 0;
    struct QcPerfData *data                              = NULL;
    uint32_t core_id                                     = 0;
    uint32_t total_metric_count                          = 0;
    struct CpuMetrics cpu_info                            = {0};
    enum eCpuTelemetryReturnCode tel_ret                  = RETURN_CODE_CPU_TELEMETRY_SUCCESS;

    if (NULL == request) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "request NULL pointer");
        return NULL;
    }

    total_metric_count = (uint32_t)g_backend_info.capabilities_list[WOS_CPU_CAPABILITY_INDEX].metric_ids_list_len;

    data = (struct QcPerfData *)calloc(1, sizeof(struct QcPerfData));
    if (NULL == data) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Memory allocation failed for QcPerfData");
        return NULL;
    }

    if (0 == request->sampling_rate || request->sampling_rate > request->streaming_rate) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_WARNING,
                     "Sampling rate (%u ms) invalid or exceeds streaming rate (%u ms). Adjusted.",
                     request->sampling_rate, request->streaming_rate);
        request->sampling_rate = request->streaming_rate;
    }
    sample_count = request->streaming_rate / request->sampling_rate;

    metric_response_allocated = sample_count * total_metric_count;
    data->metric_response = (struct QcPerfMetricResponse *)calloc((size_t)metric_response_allocated, sizeof(struct QcPerfMetricResponse));
    if (NULL == data->metric_response) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Memory allocation for metric responses failed");
        free(data);
        data = NULL;
        return NULL;
    }

    data->capabilityId = WOS_CPU_CAPABILITY_ID;
    data->backend_id   = QC_PERF_BACKEND_WOS_CPU;

    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Collection thread: sample_count=%u metrics=%u total=%u",
                 sample_count, total_metric_count, metric_response_allocated);

    (void)get_time_ns(&start_time);

    /* Prime the counters with initial sleep */
    Sleep(request->sampling_rate);

    metric_response_index = 0;
    while (g_is_thread_running) {
        (void)get_time_ns(&current_time);

        if (CPU_TELEMETRY_SOURCE_NTQUERY == g_telemetry_source) {
            tel_ret = cpuTelemetryNtQuery_collect(&cpu_info);
        } else {
            tel_ret = cpuTelemetryPdh_collect(&cpu_info);
        }
        if (RETURN_CODE_CPU_TELEMETRY_SUCCESS != tel_ret) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "cpuTelemetry collect failed with error=%u", tel_ret);
        } else {
            /* Clamp total utilization */
            if (cpu_info.total_utilization < 0.0) {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_WARNING, "%s: Total utilization below 0%% (%.2f), clamping to 0",
                             (CPU_TELEMETRY_SOURCE_NTQUERY == g_telemetry_source) ? "NtQuery" : "PDH", cpu_info.total_utilization);
                cpu_info.total_utilization = 0.0;
            } else if (cpu_info.total_utilization > 100.0) {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_WARNING, "%s: Total utilization above 100%% (%.2f), clamping to 100",
                             (CPU_TELEMETRY_SOURCE_NTQUERY == g_telemetry_source) ? "NtQuery" : "PDH", cpu_info.total_utilization);
                cpu_info.total_utilization = 100.0;
            }

            /* Clamp per-core utilization */
            for (core_id = 0; core_id < cpu_info.num_cores; core_id++) {
                if (cpu_info.cores[core_id].utilization < 0.0) {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_WARNING, "%s: Core %u utilization below 0%% (%.2f), clamping to 0",
                                 (CPU_TELEMETRY_SOURCE_NTQUERY == g_telemetry_source) ? "NtQuery" : "PDH", core_id, cpu_info.cores[core_id].utilization);
                    cpu_info.cores[core_id].utilization = 0.0;
                } else if (cpu_info.cores[core_id].utilization > 100.0) {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_WARNING, "%s: Core %u utilization above 100%% (%.2f), clamping to 100",
                                 (CPU_TELEMETRY_SOURCE_NTQUERY == g_telemetry_source) ? "NtQuery" : "PDH", core_id, cpu_info.cores[core_id].utilization);
                    cpu_info.cores[core_id].utilization = 100.0;
                }
            }

            /* Total CPU utilization */
            if (metric_response_index < metric_response_allocated) {
                data->metric_response[metric_response_index].metric_id                 = WOS_CPU_TOTAL_UTIL_ID;
                data->metric_response[metric_response_index].timestamp                 = current_time;
                data->metric_response[metric_response_index].metric_value.data_type    = QC_PERF_DATA_TYPE_DOUBLE;
                data->metric_response[metric_response_index].metric_value.double_value = cpu_info.total_utilization;
                metric_response_index++;
            }

            /* Total (average) CPU frequency */
            if (metric_response_index < metric_response_allocated) {
                data->metric_response[metric_response_index].metric_id                 = WOS_CPU_TOTAL_FREQ_ID;
                data->metric_response[metric_response_index].timestamp                 = current_time;
                data->metric_response[metric_response_index].metric_value.data_type    = QC_PERF_DATA_TYPE_DOUBLE;
                data->metric_response[metric_response_index].metric_value.double_value = cpu_info.total_frequency;
                metric_response_index++;
            }

            /* Per-core metrics */
            for (core_id = 0; core_id < cpu_info.num_cores && metric_response_index < metric_response_allocated; core_id++) {
                data->metric_response[metric_response_index].metric_id                 = WOS_CPU_CORE_BASE_ID(core_id) + WOS_CPU_UTIL_OFFSET;
                data->metric_response[metric_response_index].timestamp                 = current_time;
                data->metric_response[metric_response_index].metric_value.data_type    = QC_PERF_DATA_TYPE_DOUBLE;
                data->metric_response[metric_response_index].metric_value.double_value = cpu_info.cores[core_id].utilization;
                metric_response_index++;

                if (metric_response_index < metric_response_allocated) {
                    data->metric_response[metric_response_index].metric_id                 = WOS_CPU_CORE_BASE_ID(core_id) + WOS_CPU_FREQ_OFFSET;
                    data->metric_response[metric_response_index].timestamp                 = current_time;
                    data->metric_response[metric_response_index].metric_value.data_type    = QC_PERF_DATA_TYPE_DOUBLE;
                    data->metric_response[metric_response_index].metric_value.double_value = (double)cpu_info.cores[core_id].frequency;
                    metric_response_index++;
                }

                if (metric_response_index < metric_response_allocated) {
                    data->metric_response[metric_response_index].metric_id                 = WOS_CPU_CORE_BASE_ID(core_id) + WOS_CPU_MAX_FREQ_OFFSET;
                    data->metric_response[metric_response_index].timestamp                 = current_time;
                    data->metric_response[metric_response_index].metric_value.data_type    = QC_PERF_DATA_TYPE_DOUBLE;
                    data->metric_response[metric_response_index].metric_value.double_value = (double)cpu_info.cores[core_id].maximum_frequency;
                    metric_response_index++;
                }
            }
        }

        current_sample++;
        (void)get_time_ns(&end_time);
        elapsed_ms = (uint64_t)((end_time - start_time) / NS_TO_MS);

        if (current_sample >= sample_count || metric_response_index >= metric_response_allocated || request->streaming_rate < elapsed_ms) {
            if (0 == metric_response_index) {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "No profiling data collected");
            } else {
                data->metric_response_len = metric_response_index;

                if (NULL != g_data_callback) {
                    g_data_callback(data);
                } else {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Data callback is NULL");
                }

                metric_response_index = 0;
                current_sample        = 0;
            }
            (void)get_time_ns(&start_time);
        }

        Sleep(request->sampling_rate);
    }

    if (NULL != data->metric_response) {
        free(data->metric_response);
        data->metric_response = NULL;
    }
    if (NULL != data) {
        free(data);
        data = NULL;
    }

    return NULL;
}

/* ============================================================================
 * Backend Creation Entry Point
 * ============================================================================ */

/**
 * @brief Create and configure the wos-cpu backend
 *
 * Populates the backend private structure with function pointers for all
 * required backend operations.
 *
 * @param[in,out] backend Pointer to the backend private structure to configure
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if backend is NULL
 */
enum QcPerfReturnCode wos_cpu_create(struct QcPerfBackendPrivate *backend) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == backend) {
        return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        backend->qcperf_backend_init   = wos_cpu_init;
        backend->qcperf_backend_start  = wos_cpu_start;
        backend->qcperf_backend_stop   = wos_cpu_stop;
        backend->qcperf_backend_deinit = wos_cpu_deinit;
        backend->qcperf_backend_info   = wos_cpu_info;
        backend->set_message_callback  = wos_cpu_set_message_callback;
        backend->set_data_callback     = wos_cpu_set_data_callback;
    }

    return return_code;
}
