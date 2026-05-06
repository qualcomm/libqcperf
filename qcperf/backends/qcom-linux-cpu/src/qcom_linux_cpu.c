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
 * @file qcom_linux_cpu.c
 * @brief Qualcomm Linux CPU backend implementation for libqcperf
 * @author Himanshu Keshri (hkeshri@qti.qualcomm.com)
 *
 * This file implements the qcom-linux-cpu backend which monitors CPU performance
 * metrics on Qualcomm Linux platforms. It reads CPU statistics from /proc/stat
 * and sysfs nodes to provide per-core and total CPU metrics including utilization,
 * frequency, steal time, effective utilization, and DCVS frequency limits.
 */

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "qcom_linux_cpu.h"
#include "qcom_linux_cpu_utils.h"
#include "qcperf_backend_enum.h"
#include "qcperf_backend_interface.h"
#include "qthread.h"
#include "qtime.h"

/** Maximum length for log messages */
#define QCOM_LINUX_CPU_MESSAGE_MAX_LEN 256

/**
 * @struct QcomLinuxCpuSample
 * @brief Stores computed per-core CPU metrics for one sampling interval
 */
struct QcomLinuxCpuSample {
    double frequency;       /**< Current CPU frequency in MHz */
    double utilization;     /**< CPU utilization percentage */
    double steal_time;      /**< CPU steal time percentage */
    double eff_utilization; /**< Effective CPU utilization percentage (excl. steal) */
    uint32_t dcvs_limit;    /**< DCVS frequency limit in Hz */
};

static QcPerfMessageCallback g_message_callback = NULL;
static QcPerfDataCallback g_data_callback       = NULL;

static struct QcPerfBackendInfo g_backend_info = {0};

static volatile bool g_is_thread_running = false;
static struct QThreadInfo g_thread_info  = {0};

static const uint16_t g_streaming_rates[QCOM_LINUX_CPU_STREAMING_RATES_LEN] = {QCOM_LINUX_CPU_STREAMING_RATES};
static const uint16_t g_sampling_rates[QCOM_LINUX_CPU_SAMPLING_RATES_LEN]   = {QCOM_LINUX_CPU_SAMPLING_RATES};

/**
 * @brief Conditionally send a log message if the message callback is registered
 */
#define SEND_MESSAGE(level, ...)          \
    if (NULL != g_message_callback) {     \
        send_message(level, __VA_ARGS__); \
    }

/* ============================================================================
 * Forward Declarations
 * ============================================================================ */

static inline void send_message(enum QcPerfMessageLevel level, const char *fmt, ...);
static enum QcPerfReturnCode qcom_linux_cpu_alloc(uint32_t num_cores);
static void qcom_linux_cpu_free(void);
static enum QcPerfReturnCode validate_request(struct QcPerfRequest *request);
static enum QcPerfReturnCode qcom_linux_cpu_set_message_callback(QcPerfMessageCallback message_callback);
static enum QcPerfReturnCode qcom_linux_cpu_init(void);
static enum QcPerfReturnCode qcom_linux_cpu_backend_info(struct QcPerfBackendInfo *backend_info);
static enum QcPerfReturnCode qcom_linux_cpu_set_data_callback(QcPerfDataCallback data_callback);
static enum QcPerfReturnCode qcom_linux_cpu_start(struct QcPerfRequest *request);
static enum QcPerfReturnCode qcom_linux_cpu_stop(struct QcPerfRequest *request);
static enum QcPerfReturnCode qcom_linux_cpu_deinit(void);
static void *qcom_linux_cpu_collect_data(void *param);

/**
 * @brief Format and deliver a log message via the registered message callback
 */
static inline void send_message(enum QcPerfMessageLevel level, const char *fmt, ...) {
    char msg[QCOM_LINUX_CPU_MESSAGE_MAX_LEN];
    struct QcPerfMessage msg_struct = {0};
    va_list args;
    int len = 0;

    va_start(args, fmt);
    len = vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    if (len > 0) {
        msg_struct.message        = msg;
        msg_struct.message_length = (size_t)len;
        msg_struct.message_level  = level;
        g_message_callback(&msg_struct);
    }
}

/**
 * @brief Allocate memory for backend capabilities and metrics
 *
 * @param[in] num_cores Number of CPU cores detected on the device
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_CALLOC_FAILED if any allocation fails
 */
static enum QcPerfReturnCode qcom_linux_cpu_alloc(uint32_t num_cores) {
    enum QcPerfReturnCode ret   = QC_PERF_RETURN_CODE_SUCCESS;
    uint32_t total_metric_count = 0;

    total_metric_count = QCOM_LINUX_CPU_TOTAL_METRICS + (num_cores * QCOM_LINUX_CPU_METRICS_PER_CORE);

    g_backend_info.capabilities_list = (struct QcPerfCapabilityInfo *)calloc(QCOM_LINUX_CPU_CAPABILITIES_LEN, sizeof(struct QcPerfCapabilityInfo));
    if (NULL == g_backend_info.capabilities_list) {
        ret = QC_PERF_RETURN_CODE_CALLOC_FAILED;
    } else {
        g_backend_info.capabilities_list_length             = QCOM_LINUX_CPU_CAPABILITIES_LEN;
        g_backend_info.capabilities_list[0].metric_ids_list = (struct QcPerfMetricInfo *)calloc(total_metric_count, sizeof(struct QcPerfMetricInfo));
        if (NULL == g_backend_info.capabilities_list[0].metric_ids_list) {
            free(g_backend_info.capabilities_list);
            g_backend_info.capabilities_list        = NULL;
            g_backend_info.capabilities_list_length = 0;
            ret                                     = QC_PERF_RETURN_CODE_CALLOC_FAILED;
        }
    }

    return ret;
}

/**
 * @brief Free all memory allocated for backend capabilities and metrics
 */
static void qcom_linux_cpu_free(void) {
    if (NULL != g_backend_info.capabilities_list) {
        if (NULL != g_backend_info.capabilities_list[0].metric_ids_list) {
            free(g_backend_info.capabilities_list[0].metric_ids_list);
            g_backend_info.capabilities_list[0].metric_ids_list = NULL;
        }
        g_backend_info.capabilities_list[0].metric_ids_list_len = 0;
        free(g_backend_info.capabilities_list);
        g_backend_info.capabilities_list = NULL;
    }
    g_backend_info.capabilities_list_length = 0;
}

/**
 * @brief Validate a performance monitoring request
 *
 * Checks that the capability ID, sampling rate, and streaming rate in the
 * request are all supported by this backend.
 *
 * @param[in] request Pointer to the request to validate
 * @return QC_PERF_RETURN_CODE_SUCCESS if all parameters are valid
 * @return QC_PERF_RETURN_CODE_CAPABILITY_NOT_FOUND if capability ID is unknown
 * @return QC_PERF_RETURN_CODE_INVALID_ARGUMENTS if rates are unsupported
 */
static enum QcPerfReturnCode validate_request(struct QcPerfRequest *request) {
    enum QcPerfReturnCode ret                    = QC_PERF_RETURN_CODE_CAPABILITY_NOT_FOUND;
    struct QcPerfCapabilityInfo *capability_info = NULL;
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
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Invalid sampling rate %u for capability ID %u", request->sampling_rate, request->capability_id);
            ret = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
        } else {
            for (i = 0; i < capability_info->streaming_rate_len; i++) {
                if (capability_info->streaming_rate[i] == request->streaming_rate) {
                    streaming_rate_valid = true;
                    break;
                }
            }

            if (false == streaming_rate_valid) {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Invalid streaming rate %u for capability ID %u", request->streaming_rate, request->capability_id);
                ret = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
            }
        }
    }

    return ret;
}

/* ============================================================================
 * Backend Interface Implementations
 * ============================================================================ */

/**
 * @brief Register the message callback for this backend
 *
 * Also forwards the callback to the utils module so utility functions
 * can emit log messages through the same channel.
 */
static enum QcPerfReturnCode qcom_linux_cpu_set_message_callback(QcPerfMessageCallback message_callback) {
    g_message_callback = message_callback;
    return QC_PERF_RETURN_CODE_SUCCESS;
}

/**
 * @brief Initialize the qcom-linux-cpu backend
 *
 * Reads the CPU core count, capacity, and DCVS availability from sysfs,
 * allocates metric and capability structures, and populates them with the
 * backend configuration.
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful initialization
 * @return QC_PERF_RETURN_CODE_FAILED if core count or capacity cannot be read
 * @return QC_PERF_RETURN_CODE_CALLOC_FAILED if memory allocation fails
 */
static enum QcPerfReturnCode qcom_linux_cpu_init(void) {
    enum QcPerfReturnCode ret   = QC_PERF_RETURN_CODE_SUCCESS;
    uint32_t num_cores          = 0;
    uint32_t total_capacity     = 0;
    uint32_t total_metric_count = 0;
    uint8_t i                   = 0;

    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Initializing qcom-linux-cpu backend");

    ret = qcom_linux_cpu_util_get_num_cores(&num_cores);
    if (QC_PERF_RETURN_CODE_SUCCESS != ret) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to read CPU core count");
    } else if (num_cores > QCOM_LINUX_CPU_MAX_CORES) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "CPU core count %u exceeds maximum supported %u", num_cores, QCOM_LINUX_CPU_MAX_CORES);
        ret = QC_PERF_RETURN_CODE_FAILED;
    } else {
        ret = qcom_linux_cpu_util_get_total_capacity(&total_capacity);
        if (QC_PERF_RETURN_CODE_SUCCESS != ret) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to read CPU capacity");
        } else {
            ret = qcom_linux_cpu_alloc(num_cores);
            if (QC_PERF_RETURN_CODE_SUCCESS != ret) {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to allocate backend memory");
            } else {
                ret = qcom_linux_cpu_init_metrics(g_backend_info.capabilities_list[0].metric_ids_list, &g_backend_info.capabilities_list[0].metric_ids_list_len);
                if (QC_PERF_RETURN_CODE_SUCCESS != ret) {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to initialize metrics");
                } else {
                    total_metric_count        = (uint32_t)g_backend_info.capabilities_list[0].metric_ids_list_len;
                    g_backend_info.backend_id = QC_PERF_BACKEND_QCOM_LINUX_CPU;

                    g_backend_info.capabilities_list[0].capability_id = QCOM_LINUX_CPU_CAPABILITY_ID;
                    snprintf(g_backend_info.capabilities_list[0].capability_name, CAPABILITY_NAME_MAX_LEN, "%s", QCOM_LINUX_CPU_CAPABILITY);
                    g_backend_info.capabilities_list[0].capability_name_len = strlen(g_backend_info.capabilities_list[0].capability_name);

                    for (i = 0; i < QCOM_LINUX_CPU_STREAMING_RATES_LEN && i < MAX_SAMPLING_STREAMING_RATES_LEN; i++) {
                        g_backend_info.capabilities_list[0].streaming_rate[i] = g_streaming_rates[i];
                        g_backend_info.capabilities_list[0].streaming_rate_len++;
                    }

                    for (i = 0; i < QCOM_LINUX_CPU_SAMPLING_RATES_LEN && i < MAX_SAMPLING_STREAMING_RATES_LEN; i++) {
                        g_backend_info.capabilities_list[0].sampling_rate[i] = g_sampling_rates[i];
                        g_backend_info.capabilities_list[0].sampling_rate_len++;
                    }

                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "qcom-linux-cpu backend initialized: %u cores, %u metrics", num_cores, total_metric_count);
                }
            }
        }
    }

    if (QC_PERF_RETURN_CODE_SUCCESS != ret) {
        qcom_linux_cpu_free();
    }

    return ret;
}

/**
 * @brief Retrieve backend information including capabilities and metrics
 *
 * @param[out] backend_info Pointer to structure to be populated
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if backend_info is NULL
 */
static enum QcPerfReturnCode qcom_linux_cpu_backend_info(struct QcPerfBackendInfo *backend_info) {
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
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_INVALID_ARGUMENTS if data_callback is NULL
 * @return QC_PERF_RETURN_CODE_CALLBACK_ALREADY_SET if a callback is already registered
 */
static enum QcPerfReturnCode qcom_linux_cpu_set_data_callback(QcPerfDataCallback data_callback) {
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_FAILED;

    if (NULL == data_callback) {
        ret = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
    } else if (NULL != g_data_callback) {
        ret = QC_PERF_RETURN_CODE_CALLBACK_ALREADY_SET;
    } else {
        g_data_callback = data_callback;
        ret             = QC_PERF_RETURN_CODE_SUCCESS;
    }

    return ret;
}

/**
 * @brief Start CPU performance data collection
 *
 * Validates the request, checks that the data callback is registered, and
 * creates the data collection thread.
 *
 * @param[in] request Pointer to the performance request structure
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful start
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if request is NULL
 * @return QC_PERF_RETURN_CODE_FAILED if callback not set or thread creation fails
 * @return QC_PERF_RETURN_CODE_ALREADY_INITIALIZED if thread is already running
 */
static enum QcPerfReturnCode qcom_linux_cpu_start(struct QcPerfRequest *request) {
    enum QcPerfReturnCode ret             = QC_PERF_RETURN_CODE_SUCCESS;
    enum QThreadReturnCode thread_ret     = RET_QTHREAD_CREATE_FAILED;
    struct QThreadAttributes thread_attrs = {0};

    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Starting qcom-linux-cpu data collection");

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
            thread_attrs.thread_fn     = qcom_linux_cpu_collect_data;
            snprintf((char *)thread_attrs.thread_name, THREAD_NAME_SIZE, "qcperf_cpu_thread");
            thread_attrs.thread_name_len = (uint8_t)strlen((char *)thread_attrs.thread_name);

            thread_ret = thread_create(&thread_attrs, &g_thread_info);
            if (RET_QTHREAD_CREATE_SUCCESS != thread_ret) {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to create CPU collection thread");
                ret = QC_PERF_RETURN_CODE_FAILED;
            } else {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "CPU collection thread started successfully");
            }
        }
    }

    return ret;
}

/**
 * @brief Stop CPU performance data collection
 *
 * Signals the collection thread to stop, waits for it to finish, and
 * releases the thread handle.
 *
 * @param[in] request Pointer to the performance request structure
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful stop
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if request is NULL
 * @return QC_PERF_RETURN_CODE_FAILED if thread join or destroy fails
 */
static enum QcPerfReturnCode qcom_linux_cpu_stop(struct QcPerfRequest *request) {
    enum QcPerfReturnCode ret         = QC_PERF_RETURN_CODE_SUCCESS;
    enum QThreadReturnCode thread_ret = RET_QTHREAD_JOIN_FAILED;

    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Stopping qcom-linux-cpu data collection");

    if (NULL == request) {
        ret = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        if (g_is_thread_running) {
            g_is_thread_running = false;

            thread_ret = thread_join(&g_thread_info);
            if (RET_QTHREAD_JOIN_SUCCESS == thread_ret) {
                thread_ret = thread_destroy(&g_thread_info);
                if (RET_QTHREAD_DESTROY_SUCCESS == thread_ret) {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "CPU collection thread stopped successfully");
                } else {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to destroy CPU collection thread handle");
                    ret = QC_PERF_RETURN_CODE_FAILED;
                }
            } else {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to join CPU collection thread");
                ret = QC_PERF_RETURN_CODE_FAILED;
            }
        } else {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "CPU collection thread is not running");
        }
    }

    return ret;
}

/**
 * @brief Deinitialize the qcom-linux-cpu backend
 *
 * Frees all allocated memory and resets global state.
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS always
 */
static enum QcPerfReturnCode qcom_linux_cpu_deinit(void) {
    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Deinitializing qcom-linux-cpu backend");
    qcom_linux_cpu_free();
    g_data_callback = NULL;
    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "qcom-linux-cpu backend deinitialized successfully");
    return QC_PERF_RETURN_CODE_SUCCESS;
}

/* ============================================================================
 * Data Collection Thread
 * ============================================================================ */

/**
 * @brief Thread function for collecting CPU performance metrics
 * @param[in] param Pointer to QcPerfRequest structure
 * @return NULL on completion
 */
static void *qcom_linux_cpu_collect_data(void *param) {
    struct QcPerfRequest *request          = (struct QcPerfRequest *)param;
    struct QcPerfData *data                = NULL;
    struct QcomLinuxCpuLoadInfo *load_prev = NULL;
    struct QcomLinuxCpuLoadInfo *load_curr = NULL;
    struct QcomLinuxCpuSample *samples     = NULL;

    /* System topology — read once at thread start */
    uint32_t num_cores                              = 0;
    uint32_t cpu_capacity[QCOM_LINUX_CPU_MAX_CORES] = {0};
    uint32_t total_capacity                         = 0;

    uint32_t total_metric_count    = 0;
    uint32_t samples_per_stream    = 0;
    uint32_t total_responses       = 0;
    uint32_t sample_count          = 0;
    uint32_t position              = 0;
    uint32_t core_id               = 0;
    uint64_t current_time          = 0;
    uint64_t last_stream_time      = 0;
    uint64_t streaming_rate_ns     = 0;
    uint64_t elapsed_ns            = 0;
    double delta_total             = 0.0;
    double delta_idle              = 0.0;
    double delta_steal             = 0.0;
    double total_utilization       = 0.0;
    double total_eff_utilization   = 0.0;
    double capacity_factor         = 0.0;
    enum QcPerfReturnCode stat_ret = QC_PERF_RETURN_CODE_FAILED;
    enum QcPerfReturnCode dcvs_ret = QC_PERF_RETURN_CODE_FAILED;

    g_is_thread_running = true;

    if (NULL == request) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Request parameter is NULL in collection thread");
        g_is_thread_running = false;
    } else {
        /* Read stable system topology once at thread start */
        if (QC_PERF_RETURN_CODE_SUCCESS != qcom_linux_cpu_util_get_num_cores(&num_cores)) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to read CPU core count in collection thread");
            g_is_thread_running = false;
        } else if (num_cores > QCOM_LINUX_CPU_MAX_CORES) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "CPU core count %u exceeds maximum in collection thread", num_cores);
            g_is_thread_running = false;
        } else if (QC_PERF_RETURN_CODE_SUCCESS != qcom_linux_cpu_util_get_total_capacity(&total_capacity)) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to read CPU capacity in collection thread");
            g_is_thread_running = false;
        } else {
            /* Populate per-core capacity array using qcom_linux_cpu_util_get_core_capacity */
            for (core_id = 0; core_id < num_cores; core_id++) {
                (void)qcom_linux_cpu_util_get_core_capacity(core_id, &cpu_capacity[core_id]);
            }

            total_metric_count = (uint32_t)g_backend_info.capabilities_list[0].metric_ids_list_len;
            streaming_rate_ns  = (uint64_t)request->streaming_rate * 1000000ULL;
            samples_per_stream = (request->streaming_rate + request->sampling_rate - 1U) / request->sampling_rate;
            total_responses    = samples_per_stream * total_metric_count;

            load_prev = (struct QcomLinuxCpuLoadInfo *)calloc(num_cores, sizeof(struct QcomLinuxCpuLoadInfo));
            load_curr = (struct QcomLinuxCpuLoadInfo *)calloc(num_cores, sizeof(struct QcomLinuxCpuLoadInfo));
            samples   = (struct QcomLinuxCpuSample *)calloc(num_cores, sizeof(struct QcomLinuxCpuSample));

            if (NULL == load_prev || NULL == load_curr || NULL == samples) {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to allocate CPU load info buffers");
                g_is_thread_running = false;
            } else {
                data = (struct QcPerfData *)calloc(1, sizeof(struct QcPerfData));
                if (NULL == data) {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to allocate QcPerfData structure");
                    g_is_thread_running = false;
                } else {
                    data->metric_response = (struct QcPerfMetricResponse *)calloc(total_responses, sizeof(struct QcPerfMetricResponse));
                    if (NULL == data->metric_response) {
                        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to allocate metric response array");
                        g_is_thread_running = false;
                    } else {
                        data->backend_id          = QC_PERF_BACKEND_QCOM_LINUX_CPU;
                        data->capabilityId        = request->capability_id;
                        data->metric_response_len = 0;

                        (void)get_time_ns(&last_stream_time);

                        while (g_is_thread_running) {
                            /* First /proc/stat snapshot */
                            stat_ret = qcom_linux_cpu_util_get_proc_stat(load_prev, num_cores);
                            if (QC_PERF_RETURN_CODE_SUCCESS != stat_ret) {
                                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to read /proc/stat (first snapshot)");
                            }

                            /* Sleep for the sampling interval */
                            usleep((unsigned int)((uint32_t)request->sampling_rate * (uint32_t)QCOM_LINUX_CPU_MS_TO_US));

                            if (!g_is_thread_running) {
                                break;
                            }

                            /* Second /proc/stat snapshot */
                            stat_ret = qcom_linux_cpu_util_get_proc_stat(load_curr, num_cores);
                            if (QC_PERF_RETURN_CODE_SUCCESS != stat_ret) {
                                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to read /proc/stat (second snapshot)");
                            }

                            (void)get_time_ns(&current_time);

                            total_utilization     = 0.0;
                            total_eff_utilization = 0.0;

                            /* Compute per-core metrics and accumulate capacity-weighted totals */
                            for (core_id = 0; core_id < num_cores; core_id++) {
                                delta_total = load_curr[core_id].total_time - load_prev[core_id].total_time;
                                delta_idle  = load_curr[core_id].idle - load_prev[core_id].idle;
                                delta_steal = load_curr[core_id].steal_time - load_prev[core_id].steal_time;

                                if (delta_total > 0.0) {
                                    samples[core_id].utilization     = fabs(((delta_total - delta_idle) / delta_total) * 100.0);
                                    samples[core_id].steal_time      = fabs((delta_steal / delta_total) * 100.0);
                                    samples[core_id].eff_utilization = fabs(((delta_total - delta_idle - delta_steal) / delta_total) * 100.0);
                                } else {
                                    samples[core_id].utilization     = 0.0;
                                    samples[core_id].steal_time      = 0.0;
                                    samples[core_id].eff_utilization = 0.0;
                                }

                                (void)qcom_linux_cpu_util_get_core_frequency(core_id, &samples[core_id].frequency);

                                if (0U != total_capacity) {
                                    capacity_factor = (double)cpu_capacity[core_id] / (double)total_capacity;
                                    total_utilization += samples[core_id].utilization * capacity_factor;
                                    total_eff_utilization += samples[core_id].eff_utilization * capacity_factor;
                                }
                            }

                            /* -------------------------------------------------- */
                            /* Fill total metric responses first (IDs 0-1)         */
                            /* -------------------------------------------------- */

                            /* Total CPU Load */
                            if (position < total_responses) {
                                data->metric_response[position].metric_id                 = QCOM_LINUX_CPU_TOTAL_LOAD_ID;
                                data->metric_response[position].timestamp                 = current_time;
                                data->metric_response[position].metric_value.data_type    = QC_PERF_DATA_TYPE_DOUBLE;
                                data->metric_response[position].metric_value.double_value = total_utilization;
                                position++;
                            }

                            /* Total CPU Effective Utilization */
                            if (position < total_responses) {
                                data->metric_response[position].metric_id                 = QCOM_LINUX_CPU_TOTAL_EFF_UTIL_ID;
                                data->metric_response[position].timestamp                 = current_time;
                                data->metric_response[position].metric_value.data_type    = QC_PERF_DATA_TYPE_DOUBLE;
                                data->metric_response[position].metric_value.double_value = total_eff_utilization;
                                position++;
                            }

                            /* -------------------------------------------------- */
                            /* Fill per-core metric responses                      */
                            /* -------------------------------------------------- */

                            for (core_id = 0; (core_id < num_cores) && (position < total_responses); core_id++) {
                                /* CPU Core Load */
                                data->metric_response[position].metric_id                 = QCOM_LINUX_CPU_CORE_BASE_ID(core_id);
                                data->metric_response[position].timestamp                 = current_time;
                                data->metric_response[position].metric_value.data_type    = QC_PERF_DATA_TYPE_DOUBLE;
                                data->metric_response[position].metric_value.double_value = samples[core_id].utilization;
                                position++;

                                /* CPU Core Frequency */
                                if (position < total_responses) {
                                    data->metric_response[position].metric_id                 = QCOM_LINUX_CPU_CORE_BASE_ID(core_id) + QCOM_LINUX_CPU_FREQ_OFFSET;
                                    data->metric_response[position].timestamp                 = current_time;
                                    data->metric_response[position].metric_value.data_type    = QC_PERF_DATA_TYPE_DOUBLE;
                                    data->metric_response[position].metric_value.double_value = samples[core_id].frequency;
                                    position++;
                                }

                                /* CPU Core Steal Time */
                                if (position < total_responses) {
                                    data->metric_response[position].metric_id                 = QCOM_LINUX_CPU_CORE_BASE_ID(core_id) + QCOM_LINUX_CPU_STEAL_TIME_OFFSET;
                                    data->metric_response[position].timestamp                 = current_time;
                                    data->metric_response[position].metric_value.data_type    = QC_PERF_DATA_TYPE_DOUBLE;
                                    data->metric_response[position].metric_value.double_value = samples[core_id].steal_time;
                                    position++;
                                }

                                /* CPU Core Effective Utilization */
                                if (position < total_responses) {
                                    data->metric_response[position].metric_id                 = QCOM_LINUX_CPU_CORE_BASE_ID(core_id) + QCOM_LINUX_CPU_EFF_UTIL_OFFSET;
                                    data->metric_response[position].timestamp                 = current_time;
                                    data->metric_response[position].metric_value.data_type    = QC_PERF_DATA_TYPE_DOUBLE;
                                    data->metric_response[position].metric_value.double_value = samples[core_id].eff_utilization;
                                    position++;
                                }

                                dcvs_ret = qcom_linux_cpu_util_get_dcvs_limit(core_id, &samples[core_id].dcvs_limit);
                                if (dcvs_ret == QC_PERF_RETURN_CODE_SUCCESS && (position < total_responses)) {
                                    data->metric_response[position].metric_id                 = QCOM_LINUX_CPU_CORE_BASE_ID(core_id) + QCOM_LINUX_CPU_DCVS_LIMIT_OFFSET;
                                    data->metric_response[position].timestamp                 = current_time;
                                    data->metric_response[position].metric_value.data_type    = QC_PERF_DATA_TYPE_UINT64;
                                    data->metric_response[position].metric_value.uint64_value = (uint64_t)samples[core_id].dcvs_limit;
                                    position++;
                                }
                            }

                            sample_count++;

                            /* Check if it is time to deliver data to the callback */
                            elapsed_ns = current_time - last_stream_time;
                            if ((sample_count >= samples_per_stream) || (elapsed_ns >= streaming_rate_ns)) {
                                if ((NULL != g_data_callback) && (position > 0U)) {
                                    data->metric_response_len = position;
                                    g_data_callback(data);
                                }
                                /* Reset for next streaming window */
                                position         = 0;
                                sample_count     = 0;
                                last_stream_time = current_time;
                            }
                        }

                        free(data->metric_response);
                        data->metric_response = NULL;
                    }
                    free(data);
                    data = NULL;
                }
            }

            if (NULL != load_prev) {
                free(load_prev);
                load_prev = NULL;
            }
            if (NULL != load_curr) {
                free(load_curr);
                load_curr = NULL;
            }
            if (NULL != samples) {
                free(samples);
                samples = NULL;
            }
        }
    }

    g_is_thread_running = false;
    return NULL;
}

/* ============================================================================
 * Backend Creation Entry Point
 * ============================================================================ */

/**
 * @brief Create and configure the qcom-linux-cpu backend
 *
 * Populates the backend private structure with function pointers for all
 * required backend operations.
 *
 * @param[in,out] backend Pointer to the backend private structure to configure
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if backend is NULL
 */
enum QcPerfReturnCode qcperf_qcom_linux_cpu_create(struct QcPerfBackendPrivate *backend) {
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == backend) {
        ret = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        backend->set_message_callback  = qcom_linux_cpu_set_message_callback;
        backend->qcperf_backend_init   = qcom_linux_cpu_init;
        backend->qcperf_backend_info   = qcom_linux_cpu_backend_info;
        backend->set_data_callback     = qcom_linux_cpu_set_data_callback;
        backend->qcperf_backend_start  = qcom_linux_cpu_start;
        backend->qcperf_backend_stop   = qcom_linux_cpu_stop;
        backend->qcperf_backend_deinit = qcom_linux_cpu_deinit;
    }

    return ret;
}
