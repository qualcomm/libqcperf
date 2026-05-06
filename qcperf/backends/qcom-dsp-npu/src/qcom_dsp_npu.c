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
 * @file qcom_dsp_npu.c
 * @brief DSP NPU backend implementation for libqcperf
 * @author Snehal Lalage (slalage@qti.qualcomm.com)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#ifdef __linux__
#include <unistd.h>
#else
#include <windows.h>
#endif

#include "qcom_dsp_npu.h"
#include "qcperf_backend_interface.h"
#include "qcom_dsp.h"
#include "qthread.h"
#include "qtime.h"

static QcPerfMessageCallback g_message_callback = NULL;
static QcPerfDataCallback g_data_callback       = NULL;

static struct QcPerfBackendInfo g_backend_info = {0};

// metric data array
struct QcPerfMetricInfo g_capability_0_metrics_data[DSP_NPU_CAPABILITY_0_METRIC_COUNT] = {0};

static volatile bool g_is_thread_running = false;
static struct QThreadInfo g_thread_info  = {0};
static bool g_is_dsp_initialized         = false;

static inline void send_message(enum QcPerfMessageLevel level, const char* fmt, ...);
static enum QcPerfReturnCode dsp_npu_alloc(void);
static void dsp_npu_free(void);
static void* get_dsp_npu_data(void* param);
static enum QcPerfReturnCode validate_request(struct QcPerfRequest* request);

#define MESSAGE_MAX_LEN 256

/**
 * @brief Macro to conditionally log messages if message callback is set
 */
#define SEND_MESSAGE(level, ...)          \
    if (g_message_callback != NULL) {     \
        send_message(level, __VA_ARGS__); \
    }

static const uint16_t streaming_rates[DSP_NPU_STREAMING_RATES_LEN] = {DSP_NPU_STREAMING_RATES};
static const uint16_t sampling_rates[DSP_NPU_SAMPLING_RATES_LEN] = {DSP_NPU_SAMPLING_RATES};

/**
 * @brief Set the message callback function
 */
static enum QcPerfReturnCode dsp_npu_set_message_callback(QcPerfMessageCallback message_callback) {
    g_message_callback = message_callback;
    return QC_PERF_RETURN_CODE_SUCCESS;
}

/**
 * @brief Initialize the DSP NPU backend
 */
static enum QcPerfReturnCode dsp_npu_init(void) {
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_SUCCESS;

    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Initializing DSP NPU backend");

    g_backend_info.backend_id = QC_PERF_BACKEND_DSP_NPU;

    // Initialize DSP library
    enum DspReturnCode dsp_ret = qcom_dsp_init(DSP_NPU0);
    if (dsp_ret != RETURN_CODE_DSP_LIB_SUCCESS) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to initialize DSP library: error code %d", dsp_ret);
        ret = QC_PERF_RETURN_CODE_FAILED;
    } else {
        g_is_dsp_initialized = true;

        ret = dsp_npu_alloc();

        if (QC_PERF_RETURN_CODE_SUCCESS == ret) {
            dsp_npu_capability_0_init_metrics(g_capability_0_metrics_data);

            // capability_0
            g_backend_info.capabilities_list[0].capability_id = DSP_NPU_CAPABILITY_0_ID;
            snprintf(g_backend_info.capabilities_list[0].capability_name, CAPABILITY_NAME_MAX_LEN, "%s", DSP_NPU_CAPABILITY_0);
            g_backend_info.capabilities_list[0].capability_name_len = strlen(g_backend_info.capabilities_list[0].capability_name);
            g_backend_info.capabilities_list[0].metric_ids_list_len = DSP_NPU_CAPABILITY_0_METRIC_COUNT;

            for (uint8_t strate_idx = 0; strate_idx < DSP_NPU_STREAMING_RATES_LEN && strate_idx < MAX_SAMPLING_STREAMING_RATES_LEN; strate_idx++) {
                g_backend_info.capabilities_list[0].streaming_rate[strate_idx] = streaming_rates[strate_idx];
                g_backend_info.capabilities_list[0].streaming_rate_len++;
            }

            for (uint8_t sprate_idx = 0; sprate_idx < DSP_NPU_SAMPLING_RATES_LEN && sprate_idx < MAX_SAMPLING_STREAMING_RATES_LEN; sprate_idx++) {
                g_backend_info.capabilities_list[0].sampling_rate[sprate_idx] = sampling_rates[sprate_idx];
                g_backend_info.capabilities_list[0].sampling_rate_len++;
            }

            g_backend_info.capabilities_list[0].metric_ids_list = (struct QcPerfMetricInfo*)&g_capability_0_metrics_data;

            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "DSP NPU backend initialized successfully");
        } else {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to initialize DSP NPU backend: memory allocation failed");
            dsp_npu_free();
            if (g_is_dsp_initialized) {
                qcom_dsp_deinit(DSP_NPU0);
                g_is_dsp_initialized = false;
            }
        }
    }

    return ret;
}

/**
 * @brief Get backend information including capabilities and metrics
 */
static enum QcPerfReturnCode dsp_npu_backend_info(struct QcPerfBackendInfo* backend_info) {
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == backend_info) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Backend info is NULL");
        ret = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Retrieving DSP NPU backend capabilities information");
        *backend_info = g_backend_info;
    }

    return ret;
}

/**
 * @brief Set the result callback function
 */
static enum QcPerfReturnCode dsp_npu_set_data_callback(QcPerfDataCallback data_callback) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_FAILED;
    if (NULL == data_callback) {
        return_code = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
    } else if (NULL != g_data_callback) {
        return_code = QC_PERF_RETURN_CODE_CALLBACK_ALREADY_SET;
    } else {
        g_data_callback = data_callback;
        return_code     = QC_PERF_RETURN_CODE_SUCCESS;
    }
    return return_code;
}

/**
 * @brief Validate the performance monitoring request
 */
static enum QcPerfReturnCode validate_request(struct QcPerfRequest* request) {
    enum QcPerfReturnCode ret                    = QC_PERF_RETURN_CODE_CAPABILITY_NOT_FOUND;
    struct QcPerfCapabilityInfo* capability_info = NULL;
    bool sampling_rate_valid                     = false;
    bool streaming_rate_valid                    = false;

    // Find the capability
    for (uint8_t cap_idx = 0; cap_idx < g_backend_info.capabilities_list_length; cap_idx++) {
        if (g_backend_info.capabilities_list[cap_idx].capability_id == request->capability_id) {
            capability_info = &g_backend_info.capabilities_list[cap_idx];
            ret             = QC_PERF_RETURN_CODE_SUCCESS;
            break;
        }
    }

    if (QC_PERF_RETURN_CODE_SUCCESS != ret) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Capability ID %u not found", request->capability_id);
    } else {
        for (uint8_t sprate_idx = 0; sprate_idx < capability_info->sampling_rate_len; sprate_idx++) {
            if (capability_info->sampling_rate[sprate_idx] == request->sampling_rate) {
                sampling_rate_valid = true;
                break;
            }
        }

        if (false == sampling_rate_valid) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Invalid sampling rate %u for capability ID %u", request->sampling_rate, request->capability_id);
            ret = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
        } else {
            for (uint8_t strate_idx = 0; strate_idx < capability_info->streaming_rate_len; strate_idx++) {
                if (capability_info->streaming_rate[strate_idx] == request->streaming_rate) {
                    streaming_rate_valid = true;
                    break;
                }
            }

            if (false == streaming_rate_valid) {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Invalid streaming rate %u for capability ID %u", request->streaming_rate, request->capability_id);
                ret = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
            } else {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Request validation successful for capability ID %u", request->capability_id);
                ret = QC_PERF_RETURN_CODE_SUCCESS;
            }
        }
    }

    return ret;
}

/**
 * @brief Start performance data collection
 */
static enum QcPerfReturnCode dsp_npu_start(struct QcPerfRequest* request) {
    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Starting profiling");
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == request) {
        ret = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        ret = validate_request(request);
        if (QC_PERF_RETURN_CODE_SUCCESS != ret) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Request validation failed");
        } else if (NULL == g_data_callback) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Data Callback is not set");
            ret = QC_PERF_RETURN_CODE_FAILED;
        } else if (g_is_thread_running) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_WARNING, "Performance monitoring thread is already running");
            ret = QC_PERF_RETURN_CODE_ALREADY_INITIALIZED;
        } else {
            struct QThreadAttributes thread_attrs = {0};
            enum QThreadReturnCode thread_ret     = RET_QTHREAD_CREATE_FAILED;

            thread_attrs.stack_size    = 0;
            thread_attrs.thread_params = request;
            thread_attrs.thread_fn     = get_dsp_npu_data;
            snprintf((char*)thread_attrs.thread_name, THREAD_NAME_SIZE, "qcperf_dsp_npu_thread");
            thread_attrs.thread_name_len = (uint8_t)strlen((char*)thread_attrs.thread_name);

            thread_ret = thread_create(&thread_attrs, &g_thread_info);

            if (RET_QTHREAD_CREATE_SUCCESS != thread_ret) {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to create performance monitoring thread");
                ret = QC_PERF_RETURN_CODE_FAILED;
            } else {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Performance monitoring thread started successfully");
                ret = QC_PERF_RETURN_CODE_SUCCESS;
            }
        }
    }

    return ret;
}

/**
 * @brief Stop performance data collection
 */
static enum QcPerfReturnCode dsp_npu_stop(struct QcPerfRequest* request) {
    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Stopping profiling");
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == request) {
        ret = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        if (g_is_thread_running) {
            enum QThreadReturnCode thread_ret = RET_QTHREAD_JOIN_FAILED;

            g_is_thread_running = false;

            thread_ret = thread_join(&g_thread_info);

            if (RET_QTHREAD_JOIN_SUCCESS == thread_ret) {
                thread_ret = thread_destroy(&g_thread_info);

                if (RET_QTHREAD_DESTROY_SUCCESS == thread_ret) {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Performance monitoring thread stopped successfully");
                    ret = QC_PERF_RETURN_CODE_SUCCESS;
                } else {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to destroy thread handle");
                    ret = QC_PERF_RETURN_CODE_FAILED;
                }
            } else {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to join thread");
                ret = QC_PERF_RETURN_CODE_FAILED;
            }
        } else {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Performance monitoring thread is not running");
            ret = QC_PERF_RETURN_CODE_SUCCESS;
        }
    }

    return ret;
}

/**
 * @brief Deinitialize the DSP NPU backend
 */
static enum QcPerfReturnCode dsp_npu_deinit(void) {
    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Deinitializing DSP NPU backend");

    dsp_npu_free();

    if (g_is_dsp_initialized) {
        enum DspReturnCode dsp_ret = qcom_dsp_deinit(DSP_NPU0);
        if (dsp_ret != RETURN_CODE_DSP_LIB_SUCCESS) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to deinitialize DSP library: error code %d", dsp_ret);
        }
        g_is_dsp_initialized = false;
    }

    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "DSP NPU backend deinitialized successfully");

    return QC_PERF_RETURN_CODE_SUCCESS;
}

static inline void send_message(enum QcPerfMessageLevel level, const char* fmt, ...) {
    char msg[MESSAGE_MAX_LEN];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    if (len > 0) {
        if (len >= MESSAGE_MAX_LEN) {
            len = MESSAGE_MAX_LEN - 1;
        }
        struct QcPerfMessage msgStruct = {.message = msg, .message_length = (size_t)len, .message_level = level};

        g_message_callback(&msgStruct);
    }
}

/**
 * @brief Allocate memory for backend information
 */
static enum QcPerfReturnCode dsp_npu_alloc(void) {
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_SUCCESS;

    g_backend_info.capabilities_list = (struct QcPerfCapabilityInfo*)calloc(DSP_NPU_CAPABILITIES_LEN, sizeof(struct QcPerfCapabilityInfo));

    if (NULL == g_backend_info.capabilities_list) {
        ret = QC_PERF_RETURN_CODE_CALLOC_FAILED;
    } else {
        g_backend_info.capabilities_list_length = DSP_NPU_CAPABILITIES_LEN;
    }

    return ret;
}

/**
 * @brief Free all allocated memory for backend information
 */
static void dsp_npu_free(void) {
    if (NULL != g_backend_info.capabilities_list) {
        for (int cap_idx = 0; cap_idx < g_backend_info.capabilities_list_length; cap_idx++) {
            g_backend_info.capabilities_list[cap_idx].metric_ids_list     = NULL;
            g_backend_info.capabilities_list[cap_idx].metric_ids_list_len = 0;
        }

        free(g_backend_info.capabilities_list);
        g_backend_info.capabilities_list = NULL;
    }

    g_backend_info.capabilities_list_length = 0;
}

/**
 * @brief Thread function for collecting DSP NPU metric data
 */
static void* get_dsp_npu_data(void* param) {
    struct QcPerfRequest* request                = (struct QcPerfRequest*)param;
    struct QcPerfCapabilityInfo* capability_info = NULL;
    uint32_t samples_per_stream                  = 0;
    uint32_t total_metrics                       = 0;
    struct QcPerfData* data                      = NULL;
    uint64_t current_time                        = 0;
    uint64_t last_stream_time                    = 0;
    uint64_t elapsed_ns                          = 0;
    uint32_t sample_count                        = 0;
    int no_metrics                               = 0;

    if (NULL != request) {
        g_is_thread_running = true;

        for (int cap_idx = 0; cap_idx < g_backend_info.capabilities_list_length; cap_idx++) {
            if (g_backend_info.capabilities_list[cap_idx].capability_id == request->capability_id) {
                capability_info = &g_backend_info.capabilities_list[cap_idx];
                break;
            }
        }

        if (NULL != capability_info) {
            // Calculate how many samples to collect before streaming
            uint64_t streaming_rate_ns = (uint64_t)request->streaming_rate * 1000000ULL;
            samples_per_stream         = (uint32_t)((request->streaming_rate + request->sampling_rate - 1) / request->sampling_rate);

            total_metrics = samples_per_stream * capability_info->metric_ids_list_len;

            data = (struct QcPerfData*)calloc(1, sizeof(struct QcPerfData));
            if (NULL != data) {
                data->metric_response = (struct QcPerfMetricResponse*)calloc(total_metrics, sizeof(struct QcPerfMetricResponse));
                if (NULL != data->metric_response) {
                    data->capabilityId        = request->capability_id;
                    data->backend_id          = QC_PERF_BACKEND_DSP_NPU;
                    data->metric_response_len = 0;

                    get_time_ns(&current_time);
                    last_stream_time = current_time;

                    while (g_is_thread_running) {
                        get_time_ns(&current_time);

                        // Get data from DSP library
                        struct sysmon_query_prof_data* prof_data = qcom_dsp_get_prof_data(DSP_NPU0, &no_metrics);

                        if (prof_data != NULL && no_metrics> 0) {
                            // Populate metrics from DSP data
                            for (uint32_t metric_idx = 0; metric_idx < capability_info->metric_ids_list_len; metric_idx++) {
                                uint32_t metric_index = data->metric_response_len;
                                if (metric_index >= total_metrics) {
                                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Metric index exceeds allocated size");
                                    break;
                                }

                                data->metric_response[metric_index].metric_id = capability_info->metric_ids_list[metric_idx].metric_id;
                                data->metric_response[metric_index].timestamp = current_time;
                                data->metric_response[metric_index].metric_value.data_type = QC_PERF_DATA_TYPE_DOUBLE;

                                switch (metric_idx) {
                                    case DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_UTILIZATION:
                                        data->metric_response[metric_index].metric_value.double_value = (double)prof_data->q6_utilization;
                                        break;
                                    case DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_CLOCK:
                                        data->metric_response[metric_index].metric_value.double_value = (double)prof_data->q6_clock;
                                        break;
                                    case DSP_NPU_CAPABILITY_0_METRIC_INDEX_HVX_UTILIZATION:
                                        data->metric_response[metric_index].metric_value.double_value = (double)prof_data->hvx_utilization;
                                        break;
                                    case DSP_NPU_CAPABILITY_0_METRIC_INDEX_HMX_UTILIZATION:
                                        data->metric_response[metric_index].metric_value.double_value = (double)prof_data->hmx_utilization;
                                        break;
                                    default:
                                        break;
                                }
                                data->metric_response_len++;
                            }
                        } else {
                            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_WARNING, "Failed to get profiling data from DSP");
                        }

                        // Increment sample count
                        sample_count = sample_count + 1;

                        // Check if it's time to stream data
                        elapsed_ns = current_time - last_stream_time;
                        if (sample_count >= samples_per_stream || elapsed_ns >= streaming_rate_ns) {
                            // Call the callback with the data
                            if (NULL != g_data_callback && data->metric_response_len > 0) {
                                g_data_callback(data);
                                data->metric_response_len = 0;
                            } else if (NULL == g_data_callback) {
                                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Result callback is NULL, cannot send metric data");
                            }

                            // Reset counters
                            sample_count     = 0;
                            last_stream_time = current_time;
                        }

                        // Sleep for the sampling rate
                        if (g_is_thread_running) {
                            usleep((useconds_t)(request->sampling_rate * 1000));
                        }
                    }

                    free(data->metric_response);
                } else {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to allocate memory for metric responses");
                }
                free(data);
            } else {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to allocate memory for performance data");
            }
        } else {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Capability info not found for requested capability ID");
            g_is_thread_running = false;
        }
    } else {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Request parameter is NULL in thread function");
    }
    return NULL;
}

/**
 * @brief Create and configure the DSP NPU backend
 */
enum QcPerfReturnCode qcperf_dsp_npu_create(struct QcPerfBackendPrivate* backend) {
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == backend) {
        ret = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        backend->qcperf_backend_init   = dsp_npu_init;
        backend->qcperf_backend_start  = dsp_npu_start;
        backend->qcperf_backend_stop   = dsp_npu_stop;
        backend->qcperf_backend_deinit = dsp_npu_deinit;
        backend->qcperf_backend_info   = dsp_npu_backend_info;
        backend->set_message_callback  = dsp_npu_set_message_callback;
        backend->set_data_callback     = dsp_npu_set_data_callback;
    }

    return ret;
}