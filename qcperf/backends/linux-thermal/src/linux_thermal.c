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
 * @file linux_thermal.c
 * @brief Linux Thermal backend implementation for libqcperf
 * @author Vijay Kumbhani (vkumbhan@qti.qualcomm.com)
 *
 * This file implements the Linux Thermal backend, which monitors temperature
 * and cooling-device state on Linux platforms via the kernel thermal framework
 * (/sys/class/thermal). It builds on the linux_thermal_lib API to enumerate and
 * sample the thermal zones and cooling devices, and reports one value per logical
 * domain: the hottest temperature (deg C) across the domain's zones, and the
 * highest throttling level (percent of max_state) across the domain's cooling
 * devices.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "linux_thermal.h"
#include "linux_thermal_info.h"
#include "linux_thermal_lib.h"
#include "linux_thermal_logger.h"
#include "qcperf_backend_enum.h"
#include "qcperf_backend_interface.h"
#include "qthread.h"
#include "qtime.h"

/** Number of logical temperature / cooling domains reported by the backend */
#define LINUX_THERMAL_TEMP_DOMAIN_COUNT 16
#define LINUX_THERMAL_COOL_DOMAIN_COUNT 12

/** Conversion factor from milli-degrees Celsius to degrees Celsius */
#define LINUX_THERMAL_MILLIDEG_PER_DEG 1000.0

/** Conversion factor from milliseconds to microseconds (for usleep) */
#define LINUX_THERMAL_MS_TO_US 1000U

static QcPerfDataCallback g_data_callback     = NULL;
static struct QcPerfBackendInfo g_backend_info = {0};

static volatile bool g_is_thread_running = false;
static struct QThreadInfo g_thread_info  = {0};

static const uint16_t g_streaming_rates[LINUX_THERMAL_STREAMING_RATES_LEN] = {LINUX_THERMAL_STREAMING_RATES};
static const uint16_t g_sampling_rates[LINUX_THERMAL_SAMPLING_RATES_LEN]   = {LINUX_THERMAL_SAMPLING_RATES};

/* ============================================================================
 * Forward declarations
 * ============================================================================ */
static enum QcPerfReturnCode linux_thermal_set_message_callback(QcPerfMessageCallback message_callback);
static enum QcPerfReturnCode linux_thermal_init(void);
static enum QcPerfReturnCode linux_thermal_backend_info(struct QcPerfBackendInfo* backend_info);
static enum QcPerfReturnCode linux_thermal_set_data_callback(QcPerfDataCallback data_callback);
static enum QcPerfReturnCode linux_thermal_start(struct QcPerfRequest* request);
static enum QcPerfReturnCode linux_thermal_stop(struct QcPerfRequest* request);
static enum QcPerfReturnCode linux_thermal_deinit(void);
static void* linux_thermal_collect_data(void* param);

/**
 * @brief Free the backend capability/metric allocations
 */
static void linux_thermal_free_backend_info(void) {
    if (NULL != g_backend_info.capabilities_list) {
        if (NULL != g_backend_info.capabilities_list[0].metric_ids_list) {
            free(g_backend_info.capabilities_list[0].metric_ids_list);
            g_backend_info.capabilities_list[0].metric_ids_list = NULL;
        }
        free(g_backend_info.capabilities_list);
        g_backend_info.capabilities_list = NULL;
    }
    g_backend_info.capabilities_list_length = 0;
}

/**
 * @brief Validate a performance monitoring request against the backend capability
 *
 * @param[in] request Request to validate
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS if capability id and rates are supported
 * @return QC_PERF_RETURN_CODE_CAPABILITY_NOT_FOUND if the capability id is unknown
 * @return QC_PERF_RETURN_CODE_INVALID_ARGUMENTS if the sampling/streaming rate is unsupported
 */
static enum QcPerfReturnCode validate_request(struct QcPerfRequest* request) {
    enum QcPerfReturnCode ret                    = QC_PERF_RETURN_CODE_CAPABILITY_NOT_FOUND;
    struct QcPerfCapabilityInfo* capability_info = NULL;
    bool sampling_rate_valid                     = false;
    bool streaming_rate_valid                    = false;
    uint8_t i                                    = 0;

    for (i = 0; i < g_backend_info.capabilities_list_length; i++) {
        if (g_backend_info.capabilities_list[i].capability_id == request->capability_id) {
            capability_info = &g_backend_info.capabilities_list[i];
            ret             = QC_PERF_RETURN_CODE_SUCCESS;
            break;
        }
    }

    if (QC_PERF_RETURN_CODE_SUCCESS != ret) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Capability ID %u not found", request->capability_id);
    } else {
        for (i = 0; i < capability_info->sampling_rate_len; i++) {
            if (capability_info->sampling_rate[i] == request->sampling_rate) {
                sampling_rate_valid = true;
                break;
            }
        }
        if (false == sampling_rate_valid) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Invalid sampling rate %u", request->sampling_rate);
            ret = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
        } else {
            for (i = 0; i < capability_info->streaming_rate_len; i++) {
                if (capability_info->streaming_rate[i] == request->streaming_rate) {
                    streaming_rate_valid = true;
                    break;
                }
            }
            if (false == streaming_rate_valid) {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Invalid streaming rate %u", request->streaming_rate);
                ret = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
            }
        }
    }

    return ret;
}

/* ============================================================================
 * Backend interface implementations
 * ============================================================================ */

/**
 * @brief Register the message callback for this backend
 *
 * @param[in] message_callback Function pointer to the message callback
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 */
static enum QcPerfReturnCode linux_thermal_set_message_callback(QcPerfMessageCallback message_callback) {
    linux_thermal_logger_set_message_callback(message_callback);
    return QC_PERF_RETURN_CODE_SUCCESS;
}

/**
 * @brief Flag which metric ids belong to subsystems that are present
 *
 * present[] is indexed by metric id: temperature domains occupy ids
 * [CPU_TEMP_ID .. +TEMP_DOMAIN_COUNT) and cooling domains follow at
 * [CPU_COOLING_ID .. +COOL_DOMAIN_COUNT). A domain counts as present when its
 * block exists and has at least one sensor/device (len > 0).
 *
 * @param[in]  temp_metrics Discovered temperature metrics (may be NULL)
 * @param[in]  cool_metrics Discovered cooling metrics (may be NULL)
 * @param[out] present      Flags array of LINUX_THERMAL_CAPABILITY_METRIC_COUNT entries
 */
static void linux_thermal_mark_present(const struct LinuxThermalTempMetrics* temp_metrics,
                                       const struct LinuxThermalCoolingMetrics* cool_metrics,
                                       bool* present) {
    uint8_t d = 0;

    for (d = 0; d < LINUX_THERMAL_CAPABILITY_METRIC_COUNT; d++) {
        present[d] = false;
    }

    if (NULL != temp_metrics) {
        const struct LinuxThermalTempData* td[LINUX_THERMAL_TEMP_DOMAIN_COUNT] = {
            temp_metrics->cpu,        temp_metrics->gpu,    temp_metrics->npu,      temp_metrics->ddr,
            temp_metrics->modem,      temp_metrics->qmx,    temp_metrics->video,    temp_metrics->camera,
            temp_metrics->wireless,   temp_metrics->usb,    temp_metrics->rf_sdr,   temp_metrics->multimedia,
            temp_metrics->pmic,       temp_metrics->system, temp_metrics->battery,  temp_metrics->soc,
        };
        for (d = 0; d < LINUX_THERMAL_TEMP_DOMAIN_COUNT; d++) {
            present[LINUX_THERMAL_METRIC_CPU_TEMP_ID + d] = (NULL != td[d] && td[d]->len > 0);
        }
    }

    if (NULL != cool_metrics) {
        const struct LinuxThermalCoolingData* cd[LINUX_THERMAL_COOL_DOMAIN_COUNT] = {
            cool_metrics->cpu,        cool_metrics->ddr,        cool_metrics->gpu,     cool_metrics->npu,
            cool_metrics->display,    cool_metrics->storage,    cool_metrics->battery, cool_metrics->modem,
            cool_metrics->rf_sdr,     cool_metrics->multimedia, cool_metrics->dsds,    cool_metrics->thermal_fw,
        };
        for (d = 0; d < LINUX_THERMAL_COOL_DOMAIN_COUNT; d++) {
            present[LINUX_THERMAL_METRIC_CPU_COOLING_ID + d] = (NULL != cd[d] && cd[d]->len > 0);
        }
    }
}

/**
 * @brief Initialize the Linux Thermal backend
 *
 * Initializes the thermal library (enumerate/classify/allocate), discovers the
 * zones and cooling devices, then allocates and populates the backend capability
 * and metric metadata.
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_FAILED if the thermal library cannot be initialized
 * @return QC_PERF_RETURN_CODE_CALLOC_FAILED if metric allocation fails
 */
static enum QcPerfReturnCode linux_thermal_init(void) {
    enum QcPerfReturnCode ret                     = QC_PERF_RETURN_CODE_SUCCESS;
    struct LinuxThermalTempMetrics* temp_metrics  = NULL;
    struct LinuxThermalCoolingMetrics* cool_metrics = NULL;
    bool present[LINUX_THERMAL_CAPABILITY_METRIC_COUNT] = {false};
    uint8_t i                                     = 0;

    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Initializing linux-thermal backend");

    ret = linux_thermal_lib_init();
    if (QC_PERF_RETURN_CODE_SUCCESS != ret) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to initialize thermal library, error: %d", ret);
    } else {
        // Populate zone/device ids and names once so the collection thread can
        // refresh values by id on every sample.
        (void)linux_thermal_lib_discover_thermal_zones(&temp_metrics);
        (void)linux_thermal_lib_discover_cooling_devices(&cool_metrics);

        // Advertise metrics only for domains that actually have sensors/devices.
        linux_thermal_mark_present(temp_metrics, cool_metrics, present);

        g_backend_info.capabilities_list = (struct QcPerfCapabilityInfo*)calloc(LINUX_THERMAL_CAPABILITIES_LEN, sizeof(struct QcPerfCapabilityInfo));
        if (NULL == g_backend_info.capabilities_list) {
            ret = QC_PERF_RETURN_CODE_CALLOC_FAILED;
        } else {
            g_backend_info.capabilities_list[0].metric_ids_list =
                (struct QcPerfMetricInfo*)calloc(LINUX_THERMAL_CAPABILITY_METRIC_COUNT, sizeof(struct QcPerfMetricInfo));
            if (NULL == g_backend_info.capabilities_list[0].metric_ids_list) {
                ret = QC_PERF_RETURN_CODE_CALLOC_FAILED;
            } else {
                ret = linux_thermal_init_available_metrics(g_backend_info.capabilities_list[0].metric_ids_list,
                                                           &g_backend_info.capabilities_list[0].metric_ids_list_len,
                                                           present);
                if (QC_PERF_RETURN_CODE_SUCCESS != ret) {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to initialize metrics");
                } else {
                    g_backend_info.backend_id               = QC_PERF_BACKEND_LINUX_THERMAL;
                    g_backend_info.capabilities_list_length = LINUX_THERMAL_CAPABILITIES_LEN;

                    g_backend_info.capabilities_list[0].capability_id = LINUX_THERMAL_CAPABILITY_ID;
                    snprintf(g_backend_info.capabilities_list[0].capability_name, CAPABILITY_NAME_MAX_LEN, "%s", LINUX_THERMAL_CAPABILITY);
                    g_backend_info.capabilities_list[0].capability_name_len = strlen(g_backend_info.capabilities_list[0].capability_name);

                    for (i = 0; i < LINUX_THERMAL_STREAMING_RATES_LEN && i < MAX_SAMPLING_STREAMING_RATES_LEN; i++) {
                        g_backend_info.capabilities_list[0].streaming_rate[i] = g_streaming_rates[i];
                        g_backend_info.capabilities_list[0].streaming_rate_len++;
                    }
                    for (i = 0; i < LINUX_THERMAL_SAMPLING_RATES_LEN && i < MAX_SAMPLING_STREAMING_RATES_LEN; i++) {
                        g_backend_info.capabilities_list[0].sampling_rate[i] = g_sampling_rates[i];
                        g_backend_info.capabilities_list[0].sampling_rate_len++;
                    }

                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "linux-thermal backend initialized: %u metrics",
                                 g_backend_info.capabilities_list[0].metric_ids_list_len);
                }
            }
        }
    }

    if (QC_PERF_RETURN_CODE_SUCCESS != ret) {
        linux_thermal_free_backend_info();
        (void)linux_thermal_lib_cleanup();
    }

    return ret;
}

/**
 * @brief Retrieve backend information including capabilities and metrics
 *
 * @param[out] backend_info Structure to be populated with backend info
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if backend_info is NULL
 */
static enum QcPerfReturnCode linux_thermal_backend_info(struct QcPerfBackendInfo* backend_info) {
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == backend_info) {
        ret = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        *backend_info = g_backend_info;
    }

    return ret;
}

/**
 * @brief Register the data callback for delivering collected metrics
 *
 * @param[in] data_callback Function pointer to the data callback
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_INVALID_ARGUMENTS if data_callback is NULL
 * @return QC_PERF_RETURN_CODE_CALLBACK_ALREADY_SET if a callback is already registered
 */
static enum QcPerfReturnCode linux_thermal_set_data_callback(QcPerfDataCallback data_callback) {
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == data_callback) {
        ret = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
    } else if (NULL != g_data_callback) {
        ret = QC_PERF_RETURN_CODE_CALLBACK_ALREADY_SET;
    } else {
        g_data_callback = data_callback;
    }

    return ret;
}

/**
 * @brief Start thermal data collection
 *
 * Validates the request, checks the data callback, and creates the collection thread.
 *
 * @param[in] request Pointer to the performance request structure
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if request is NULL
 * @return QC_PERF_RETURN_CODE_FAILED if the callback is not set or thread creation fails
 * @return QC_PERF_RETURN_CODE_ALREADY_INITIALIZED if collection is already running
 */
static enum QcPerfReturnCode linux_thermal_start(struct QcPerfRequest* request) {
    enum QcPerfReturnCode ret             = QC_PERF_RETURN_CODE_SUCCESS;
    enum QThreadReturnCode thread_ret     = RET_QTHREAD_CREATE_FAILED;
    struct QThreadAttributes thread_attrs = {0};

    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Starting linux-thermal data collection");

    if (NULL == request) {
        ret = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        ret = validate_request(request);
        if (QC_PERF_RETURN_CODE_SUCCESS != ret) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Request validation failed");
        } else if (NULL == g_data_callback) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Data callback is not set");
            ret = QC_PERF_RETURN_CODE_FAILED;
        } else if (g_is_thread_running) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_WARNING, "Collection thread is already running");
            ret = QC_PERF_RETURN_CODE_ALREADY_INITIALIZED;
        } else {
            thread_attrs.stack_size    = 0;
            thread_attrs.thread_params = request;
            thread_attrs.thread_fn     = linux_thermal_collect_data;
            snprintf((char*)thread_attrs.thread_name, THREAD_NAME_SIZE, "qcperf_thermal");
            thread_attrs.thread_name_len = (uint8_t)strlen((char*)thread_attrs.thread_name);

            thread_ret = thread_create(&thread_attrs, &g_thread_info);
            if (RET_QTHREAD_CREATE_SUCCESS != thread_ret) {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to create thermal collection thread");
                ret = QC_PERF_RETURN_CODE_FAILED;
            } else {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Thermal collection thread started successfully");
            }
        }
    }

    return ret;
}

/**
 * @brief Stop thermal data collection
 *
 * Signals the collection thread to stop, joins it, and releases the handle.
 *
 * @param[in] request Pointer to the performance request structure
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if request is NULL
 * @return QC_PERF_RETURN_CODE_FAILED if joining or destroying the thread fails
 */
static enum QcPerfReturnCode linux_thermal_stop(struct QcPerfRequest* request) {
    enum QcPerfReturnCode ret         = QC_PERF_RETURN_CODE_SUCCESS;
    enum QThreadReturnCode thread_ret = RET_QTHREAD_JOIN_FAILED;

    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Stopping linux-thermal data collection");

    if (NULL == request) {
        ret = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else if (g_is_thread_running) {
        g_is_thread_running = false;

        thread_ret = thread_join(&g_thread_info);
        if (RET_QTHREAD_JOIN_SUCCESS == thread_ret) {
            thread_ret = thread_destroy(&g_thread_info);
            if (RET_QTHREAD_DESTROY_SUCCESS != thread_ret) {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to destroy thermal collection thread handle");
                ret = QC_PERF_RETURN_CODE_FAILED;
            } else {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Thermal collection thread stopped successfully");
            }
        } else {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to join thermal collection thread");
            ret = QC_PERF_RETURN_CODE_FAILED;
        }
    } else {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Thermal collection thread is not running");
    }

    return ret;
}

/**
 * @brief Deinitialize the Linux Thermal backend
 *
 * Frees backend metadata and cleans up the thermal library.
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 */
static enum QcPerfReturnCode linux_thermal_deinit(void) {
    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Deinitializing linux-thermal backend");
    linux_thermal_free_backend_info();
    (void)linux_thermal_lib_cleanup();
    g_data_callback = NULL;
    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "linux-thermal backend deinitialized successfully");
    return QC_PERF_RETURN_CODE_SUCCESS;
}

/* ============================================================================
 * Data collection thread
 * ============================================================================ */

/**
 * @brief Thread function for collecting thermal metrics
 *
 * Each sample refreshes the temperature and cooling values via the thermal
 * library and emits one metric per populated domain: the hottest temperature
 * (deg C) for a temperature domain and the highest throttling level (percent of
 * max_state) for a cooling domain. Data is delivered to the callback at the
 * requested streaming rate.
 *
 * @param[in] param Pointer to QcPerfRequest structure
 *
 * @return NULL on completion
 */
static void* linux_thermal_collect_data(void* param) {
    struct QcPerfRequest* request                = (struct QcPerfRequest*)param;
    struct QcPerfData* data                      = NULL;
    struct LinuxThermalTempMetrics* temp_metrics = NULL;
    struct LinuxThermalCoolingMetrics* cool_metrics = NULL;

    uint32_t total_metric_count = 0;
    uint32_t samples_per_stream = 0;
    uint32_t total_responses    = 0;
    uint32_t sample_count       = 0;
    uint32_t position           = 0;
    uint64_t current_time       = 0;
    uint64_t last_stream_time   = 0;
    uint64_t streaming_rate_ns  = 0;
    uint64_t elapsed_ns         = 0;
    uint32_t max_milli          = 0;
    double max_pct              = 0.0;
    double pct                  = 0.0;
    uint16_t metric_id          = 0;
    uint8_t d                   = 0;
    uint8_t i                   = 0;
    bool supported[LINUX_THERMAL_CAPABILITY_METRIC_COUNT] = {false};

    g_is_thread_running = true;

    if (NULL == request) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Request parameter is NULL in collection thread");
        g_is_thread_running = false;
    } else {
        total_metric_count = (uint32_t)g_backend_info.capabilities_list[0].metric_ids_list_len;

        // Only metrics advertised to the client (populated at init from the
        // subsystems actually present) may be sent as data. Flag their ids so
        // the emission loops below can never report an unsupported metric.
        for (i = 0; i < g_backend_info.capabilities_list[0].metric_ids_list_len; i++) {
            metric_id = g_backend_info.capabilities_list[0].metric_ids_list[i].metric_id;
            if (metric_id < LINUX_THERMAL_CAPABILITY_METRIC_COUNT) {
                supported[metric_id] = true;
            }
        }

        streaming_rate_ns  = (uint64_t)request->streaming_rate * 1000000ULL;
        samples_per_stream = ((uint32_t)request->streaming_rate + (uint32_t)request->sampling_rate - 1U) / (uint32_t)request->sampling_rate;
        total_responses    = samples_per_stream * total_metric_count;

        data = (struct QcPerfData*)calloc(1, sizeof(struct QcPerfData));
        if (NULL == data) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to allocate QcPerfData structure");
            g_is_thread_running = false;
        } else {
            data->metric_response = (struct QcPerfMetricResponse*)calloc(total_responses, sizeof(struct QcPerfMetricResponse));
            if (NULL == data->metric_response) {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to allocate metric response array");
                g_is_thread_running = false;
            } else {
                data->backend_id   = QC_PERF_BACKEND_LINUX_THERMAL;
                data->capabilityId = request->capability_id;

                (void)get_time_ns(&last_stream_time);

                while (g_is_thread_running) {
                    // Refresh the live values from sysfs
                    (void)linux_thermal_lib_thermal_zones_info(&temp_metrics);
                    (void)linux_thermal_lib_cooling_devices_info(&cool_metrics);
                    (void)get_time_ns(&current_time);

                    // Temperature domains: report the hottest zone in each domain (deg C)
                    if (NULL != temp_metrics) {
                        struct LinuxThermalTempData* td[LINUX_THERMAL_TEMP_DOMAIN_COUNT] = {
                            temp_metrics->cpu,        temp_metrics->gpu,    temp_metrics->npu,      temp_metrics->ddr,
                            temp_metrics->modem,      temp_metrics->qmx,    temp_metrics->video,    temp_metrics->camera,
                            temp_metrics->wireless,   temp_metrics->usb,    temp_metrics->rf_sdr,   temp_metrics->multimedia,
                            temp_metrics->pmic,       temp_metrics->system, temp_metrics->battery,  temp_metrics->soc,
                        };
                        for (d = 0; d < LINUX_THERMAL_TEMP_DOMAIN_COUNT && position < total_responses; d++) {
                            max_milli = 0;
                            metric_id = (uint16_t)(LINUX_THERMAL_METRIC_CPU_TEMP_ID + d);
                            if (false == supported[metric_id] || NULL == td[d] || NULL == td[d]->temp || 0 == td[d]->len) {
                                continue;
                            }
                            for (i = 0; i < td[d]->len; i++) {
                                if (td[d]->temp[i] > max_milli) {
                                    max_milli = td[d]->temp[i];
                                }
                            }
                            data->metric_response[position].metric_id                 = metric_id;
                            data->metric_response[position].timestamp                 = current_time;
                            data->metric_response[position].metric_value.data_type    = QC_PERF_DATA_TYPE_DOUBLE;
                            data->metric_response[position].metric_value.double_value = (double)max_milli / LINUX_THERMAL_MILLIDEG_PER_DEG;
                            position++;
                        }
                    }

                    // Cooling domains: report the highest throttle level in each domain (%)
                    if (NULL != cool_metrics) {
                        struct LinuxThermalCoolingData* cd[LINUX_THERMAL_COOL_DOMAIN_COUNT] = {
                            cool_metrics->cpu,        cool_metrics->ddr,        cool_metrics->gpu,     cool_metrics->npu,
                            cool_metrics->display,    cool_metrics->storage,    cool_metrics->battery, cool_metrics->modem,
                            cool_metrics->rf_sdr,     cool_metrics->multimedia, cool_metrics->dsds,    cool_metrics->thermal_fw,
                        };
                        for (d = 0; d < LINUX_THERMAL_COOL_DOMAIN_COUNT && position < total_responses; d++) {
                            max_pct = 0.0;
                            metric_id = (uint16_t)(LINUX_THERMAL_METRIC_CPU_COOLING_ID + d);
                            if (false == supported[metric_id] || NULL == cd[d] || NULL == cd[d]->cur_state || NULL == cd[d]->max_state || 0 == cd[d]->len) {
                                continue;
                            }
                            for (i = 0; i < cd[d]->len; i++) {
                                if (cd[d]->max_state[i] > 0) {
                                    pct = ((double)cd[d]->cur_state[i] / (double)cd[d]->max_state[i]) * 100.0;
                                    if (pct > max_pct) {
                                        max_pct = pct;
                                    }
                                }
                            }
                            data->metric_response[position].metric_id                 = metric_id;
                            data->metric_response[position].timestamp                 = current_time;
                            data->metric_response[position].metric_value.data_type    = QC_PERF_DATA_TYPE_DOUBLE;
                            data->metric_response[position].metric_value.double_value = max_pct;
                            position++;
                        }
                    }

                    sample_count++;

                    // Deliver to the callback at the streaming rate
                    elapsed_ns = current_time - last_stream_time;
                    if ((sample_count >= samples_per_stream) || (elapsed_ns >= streaming_rate_ns)) {
                        if ((NULL != g_data_callback) && (position > 0U)) {
                            data->metric_response_len = position;
                            g_data_callback(data);
                        }
                        position         = 0;
                        sample_count     = 0;
                        last_stream_time = current_time;
                    }

                    if (g_is_thread_running) {
                        usleep((unsigned int)((uint32_t)request->sampling_rate * LINUX_THERMAL_MS_TO_US));
                    }
                }

                free(data->metric_response);
                data->metric_response = NULL;
            }
            free(data);
            data = NULL;
        }
    }

    g_is_thread_running = false;
    return NULL;
}

/* ============================================================================
 * Backend creation entry point
 * ============================================================================ */

enum QcPerfReturnCode qcperf_linux_thermal_backend_create(struct QcPerfBackendPrivate* backend) {
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == backend) {
        ret = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        backend->set_message_callback  = linux_thermal_set_message_callback;
        backend->qcperf_backend_init   = linux_thermal_init;
        backend->qcperf_backend_info   = linux_thermal_backend_info;
        backend->set_data_callback     = linux_thermal_set_data_callback;
        backend->qcperf_backend_start  = linux_thermal_start;
        backend->qcperf_backend_stop   = linux_thermal_stop;
        backend->qcperf_backend_deinit = linux_thermal_deinit;
    }

    return ret;
}
