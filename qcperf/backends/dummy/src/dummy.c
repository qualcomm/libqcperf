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
 * @file dummy.c
 * @brief Dummy backend implementation for libqcperf
 * @author Himanshu Keshri (hkeshri@qti.qualcomm.com)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdarg.h>
#ifdef __linux__
#include <unistd.h>
#else
#include <windows.h>
#endif

#include "dummy.h"
#include "qcperf_backend_interface.h"
#include "qthread.h"
#include "qtime.h"

static QcPerfMessageCallback g_message_callback = NULL;
static QcPerfDataCallback g_data_callback       = NULL;

static struct QcPerfBackendInfo g_backend_info = {0};

// metric data arrays
struct QcPerfMetricInfo g_capability_1_metrics_data[CAPABILITY_0_METRIC_COUNT] = {0};
struct QcPerfMetricInfo g_capability_2_metrics_data[CAPABILITY_1_METRIC_COUNT] = {0};

static volatile bool g_is_thread_running = false;
static struct QThreadInfo g_thread_info  = {0};

static inline void send_message(enum QcPerfMessageLevel level, const char* fmt, ...);
static enum QcPerfReturnCode dummy_alloc(void);
static void dummy_free(void);
static void* get_dummy_data(void* param);
static enum QcPerfReturnCode validate_request(struct QcPerfRequest* request);

#define MESSAGE_MAX_LEN 256
#define MAX_STRING_SIZE 512

#define METRIC_TYPE_LEN 5
#define METRIC_TYPE_BOOL_INDEX 0
#define METRIC_TYPE_UINT64_INDEX 1
#define METRIC_TYPE_INT64_INDEX 2
#define METRIC_TYPE_DOUBLE_INDEX 3
#define METRIC_TYPE_STRING_INDEX 4

/**
 * @brief Macro to conditionally log messages if message callback is set
 *
 * This macro checks if the message callback is set before calling send_message.
 * If the callback is not set, the send_message call is skipped entirely.
 *
 * Usage: SEND_MESSAGE(level, format, ...)
 * Example: SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Initializing backend");
 */
#define SEND_MESSAGE(level, ...)          \
    if (g_message_callback != NULL) {     \
        send_message(level, __VA_ARGS__); \
    }

static const uint16_t streaming_rates[DUMMY_STREAMING_RATES_LEN] = {DUMMY_STREAMING_RATES};

static const uint16_t sampling_rates[DUMMY_SAMPLING_RATES_LEN] = {DUMMY_SAMPLING_RATES};

/**
 * @brief Set the message callback function
 *
 * This function registers a callback function that will be invoked to deliver
 * informational, warning, or error messages from the dummy backend to the
 * application.
 *
 * @param[in] message_callback Function pointer to the message callback
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful callback registration
 */
static enum QcPerfReturnCode dummy_set_message_callback(QcPerfMessageCallback message_callback) {
    g_message_callback = message_callback;
    return QC_PERF_RETURN_CODE_SUCCESS;
}

/**
 * @brief Initialize the dummy backend
 *
 * This function initializes the dummy backend. It allocates memory for
 * backend information including capabilities and metrics, and populates
 * them with the backend's configuration.
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful initialization
 * @return QC_PERF_RETURN_CODE_CALLOC_FAILED if memory allocation fails
 */
static enum QcPerfReturnCode dummy_init(void) {
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_SUCCESS;

    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Initializing dummy backend");

    g_backend_info.backend_id = QC_PERF_BACKEND_DUMMY;

    ret = dummy_alloc();

    if (QC_PERF_RETURN_CODE_SUCCESS == ret) {
        dummy_capability_0_init_metrics(g_capability_1_metrics_data);
        dummy_capability_1_init_metrics(g_capability_2_metrics_data);

        // capability_1
        g_backend_info.capabilities_list[0].capability_id = DUMMY_CAPABILITY_0_ID;
        snprintf(g_backend_info.capabilities_list[0].capability_name, CAPABILITY_NAME_MAX_LEN, "%s", CAPABILITY_0);
        g_backend_info.capabilities_list[0].capability_name_len = strlen(g_backend_info.capabilities_list[0].capability_name);
        g_backend_info.capabilities_list[0].metric_ids_list_len = CAPABILITY_0_METRIC_COUNT;

        for (uint8_t i = 0; i < DUMMY_STREAMING_RATES_LEN && i < MAX_SAMPLING_STREAMING_RATES_LEN; i++) {
            g_backend_info.capabilities_list[0].streaming_rate[i] = streaming_rates[i];
            g_backend_info.capabilities_list[0].streaming_rate_len++;
        }

        for (uint8_t i = 0; i < DUMMY_SAMPLING_RATES_LEN && i < MAX_SAMPLING_STREAMING_RATES_LEN; i++) {
            g_backend_info.capabilities_list[0].sampling_rate[i] = sampling_rates[i];
            g_backend_info.capabilities_list[0].sampling_rate_len++;
        }

        g_backend_info.capabilities_list[0].metric_ids_list = (struct QcPerfMetricInfo*)&g_capability_1_metrics_data;

        // capability_2
        g_backend_info.capabilities_list[1].capability_id = DUMMY_CAPABILITY_0_ID;
        snprintf(g_backend_info.capabilities_list[1].capability_name, CAPABILITY_NAME_MAX_LEN, "%s", CAPABILITY_1);
        g_backend_info.capabilities_list[1].capability_name_len = strlen(g_backend_info.capabilities_list[1].capability_name);
        g_backend_info.capabilities_list[1].metric_ids_list_len = CAPABILITY_1_METRIC_COUNT;

        for (uint8_t i = 0; i < DUMMY_STREAMING_RATES_LEN && i < MAX_SAMPLING_STREAMING_RATES_LEN; i++) {
            g_backend_info.capabilities_list[1].streaming_rate[i] = streaming_rates[i];
            g_backend_info.capabilities_list[1].streaming_rate_len++;
        }

        for (uint8_t i = 0; i < DUMMY_SAMPLING_RATES_LEN && i < MAX_SAMPLING_STREAMING_RATES_LEN; i++) {
            g_backend_info.capabilities_list[1].sampling_rate[i] = sampling_rates[i];
            g_backend_info.capabilities_list[1].sampling_rate_len++;
        }

        g_backend_info.capabilities_list[1].metric_ids_list = (struct QcPerfMetricInfo*)&g_capability_2_metrics_data;

        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Dummy backend initialized successfully");
    } else {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to initialize dummy backend: memory allocation failed");
        dummy_free();
    }

    return ret;
}

/**
 * @brief Get backend information including capabilities and metrics
 *
 * This function provides access to the dummy backend's information by
 * copying the pre-allocated backend information structure that was
 * initialized during backend initialization.
 *
 * @param[in,out] backend_info Pointer to structure to be filled with backend information
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful information retrieval
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if backend_info pointer is NULL
 */
static enum QcPerfReturnCode dummy_backend_info(struct QcPerfBackendInfo* backend_info) {
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == backend_info) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Backend info is NULL");
        ret = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Retrieving dummy backend capabilities information");
        *backend_info = g_backend_info;
    }

    return ret;
}

/**
 * @brief Set the result callback function
 *
 * This function registers a callback function that will be invoked to deliver
 * performance metric results from the dummy backend to the application.
 *
 * @param[in] data_callback Function pointer to the result callback
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful callback registration
 */
static enum QcPerfReturnCode dummy_set_data_callback(QcPerfDataCallback data_callback) {
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
 *
 * This function validates that the request parameters are valid:
 * - Capability ID must match one of the available capabilities
 * - Sampling rate must match one of the supported sampling rates for that capability
 * - Streaming rate must match one of the supported streaming rates for that capability
 *
 * @param[in] request Pointer to the performance request structure
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS if validation passes
 * @return QC_PERF_RETURN_CODE_CAPABILITY_NOT_FOUND if capability ID is not found
 * @return QC_PERF_RETURN_CODE_INVALID_ARGUMENTS if sampling or streaming rate is invalid
 */
static enum QcPerfReturnCode validate_request(struct QcPerfRequest* request) {
    enum QcPerfReturnCode ret                    = QC_PERF_RETURN_CODE_CAPABILITY_NOT_FOUND;
    struct QcPerfCapabilityInfo* capability_info = NULL;
    bool sampling_rate_valid                     = false;
    bool streaming_rate_valid                    = false;

    // Find the capability
    for (uint8_t i = 0; i < g_backend_info.capabilities_list_length; i++) {
        if (g_backend_info.capabilities_list[i].capability_id == request->capability_id) {
            capability_info = &g_backend_info.capabilities_list[i];
            ret             = QC_PERF_RETURN_CODE_SUCCESS;
            break;
        }
    }

    if (QC_PERF_RETURN_CODE_SUCCESS != ret) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Capability ID %u not found", request->capability_id);
    } else {
        for (uint8_t i = 0; i < capability_info->sampling_rate_len; i++) {
            if (capability_info->sampling_rate[i] == request->sampling_rate) {
                sampling_rate_valid = true;
                break;
            }
        }

        if (false == sampling_rate_valid) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Invalid sampling rate %u for capability ID %u", request->sampling_rate, request->capability_id);
            ret = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
        } else {
            for (uint8_t i = 0; i < capability_info->streaming_rate_len; i++) {
                if (capability_info->streaming_rate[i] == request->streaming_rate) {
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
 *
 * This function initiates performance data collection for the specified
 * capability with the given sampling and streaming rates. It validates
 * the request and starts the data collection process by creating a thread
 * that generates dummy metric responses.
 *
 * @param[in] request Pointer to the performance request structure
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful start
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if request pointer is NULL
 * @return QC_PERF_RETURN_CODE_FAILED if callbacks are not set or thread creation fails
 * @return QC_PERF_RETURN_CODE_ALREADY_INITIALIZED if thread is already running
 * @return QC_PERF_RETURN_CODE_CAPABILITY_NOT_FOUND if capability ID is not found
 * @return QC_PERF_RETURN_CODE_INVALID_ARGUMENTS if sampling or streaming rate is invalid
 */
static enum QcPerfReturnCode dummy_start(struct QcPerfRequest* request) {
    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Starting profiling");
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == request) {
        ret = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        ret = validate_request(request);
        if (QC_PERF_RETURN_CODE_SUCCESS != ret) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Request validation failed");
        } else if (NULL == g_data_callback) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Result or Message Callback is not set");
            ret = QC_PERF_RETURN_CODE_FAILED;
        } else if (g_is_thread_running) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_WARNING, "Performance monitoring thread is already running");
            ret = QC_PERF_RETURN_CODE_ALREADY_INITIALIZED;
        } else {
            struct QThreadAttributes thread_attrs = {0};
            enum QThreadReturnCode thread_ret     = RET_QTHREAD_CREATE_FAILED;

            thread_attrs.stack_size    = 0;
            thread_attrs.thread_params = request;
            thread_attrs.thread_fn     = get_dummy_data;
            snprintf((char*)thread_attrs.thread_name, THREAD_NAME_SIZE, "qcperf_dummy_thread");
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
 *
 * This function stops performance data collection for the specified
 * capability. It signals the thread to stop, waits for it to complete,
 * and cleans up thread resources.
 *
 * @param[in] request Pointer to the performance request structure
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful stop
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if request pointer is NULL
 * @return QC_PERF_RETURN_CODE_FAILED if thread join or destroy fails
 */
static enum QcPerfReturnCode dummy_stop(struct QcPerfRequest* request) {
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
 * @brief Deinitialize the dummy backend
 *
 * This function deinitializes the dummy backend. It frees all allocated
 * memory for backend information including capabilities and metrics.
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful deinitialization
 */
static enum QcPerfReturnCode dummy_deinit(void) {
    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Deinitializing dummy backend");

    dummy_free();

    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Dummy backend deinitialized successfully");

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
 *
 * This function allocates memory for the backend's capabilities.
 * Metric strings are now fixed-size arrays in the structures, so no
 * additional allocation is needed for them.
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful allocation
 * @return QC_PERF_RETURN_CODE_CALLOC_FAILED if any memory allocation fails
 */
static enum QcPerfReturnCode dummy_alloc(void) {
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_SUCCESS;

    g_backend_info.capabilities_list = (struct QcPerfCapabilityInfo*)calloc(DUMMY_CAPABILITIES_LEN, sizeof(struct QcPerfCapabilityInfo));

    if (NULL == g_backend_info.capabilities_list) {
        ret = QC_PERF_RETURN_CODE_CALLOC_FAILED;
    } else {
        g_backend_info.capabilities_list_length = DUMMY_CAPABILITIES_LEN;
    }

    return ret;
}

/**
 * @brief Free all allocated memory for backend information
 *
 * This function frees all memory allocated for the backend's capabilities.
 * Metric strings are now fixed-size arrays, so no freeing is needed for them.
 */
static void dummy_free(void) {
    if (NULL != g_backend_info.capabilities_list) {
        for (int i = 0; i < g_backend_info.capabilities_list_length; i++) {
            // Note: metric_ids_list points to static data, so we don't free it
            g_backend_info.capabilities_list[i].metric_ids_list     = NULL;
            g_backend_info.capabilities_list[i].metric_ids_list_len = 0;
        }

        free(g_backend_info.capabilities_list);
        g_backend_info.capabilities_list = NULL;
    }

    g_backend_info.capabilities_list_length = 0;
}

/**
 * @brief Thread function for generating dummy metric data
 *
 * This function runs in a separate thread and continuously generates dummy
 * metric responses based on the capability configuration. It sends the data
 * via the result callback at the specified streaming rate.
 *
 * Memory is allocated once at the start and freed when the thread stops.
 * Total metrics = ceil(streaming_rate / sampling_rate) * metric_count
 * Data is collected at sampling_rate intervals and sent at streaming_rate.
 *
 * @param[in] param Pointer to QcPerfRequest structure
 *
 * @return 0 on successful completion, 1 on error
 */
static void* get_dummy_data(void* param) {
    struct QcPerfRequest* request                = (struct QcPerfRequest*)param;
    struct QcPerfCapabilityInfo* capability_info = NULL;
    uint32_t samples_per_stream                  = 0;
    uint32_t total_metrics                       = 0;
    uint64_t uint64_counter                      = 1;
    int64_t int64_counter                        = 1;
    double double_counter                        = 0.1;
    uint32_t string_counter                      = 1;
    struct QcPerfData* data                      = NULL;
    uint64_t current_time                        = 0;
    uint64_t last_stream_time                    = 0;
    uint64_t elapsed_ns                          = 0;
    char dummy_string[64]                        = {0};
    uint32_t sample_count = 0;

    if (NULL != request) {
        g_is_thread_running = true;

        for (int i = 0; i < g_backend_info.capabilities_list_length; i++) {
            if (g_backend_info.capabilities_list[i].capability_id == request->capability_id) {
                capability_info = &g_backend_info.capabilities_list[i];
                break;
            }
        }

        if (NULL != capability_info) {
            // Calculate how many samples to collect before streaming
            // Convert ms to ns for consistent calculations
            uint64_t streaming_rate_ns = (uint64_t)request->streaming_rate * 1000000ULL;
            samples_per_stream         = (uint32_t)((request->streaming_rate + request->sampling_rate - 1) / request->sampling_rate);

            total_metrics = samples_per_stream * capability_info->metric_ids_list_len;

            data = (struct QcPerfData*)calloc(1, sizeof(struct QcPerfData));
            if (NULL != data) {
                data->metric_response = (struct QcPerfMetricResponse*)calloc(total_metrics, sizeof(struct QcPerfMetricResponse));
                if (NULL != data->metric_response) {
                    data->capabilityId        = request->capability_id;
                    data->backend_id          = QC_PERF_BACKEND_DUMMY;
                    data->metric_response_len = 0;

                    for (uint32_t i = 0; i < total_metrics; i++) {
                        if (METRIC_TYPE_STRING_INDEX == (i % METRIC_TYPE_LEN)) {
                            data->metric_response[i].metric_value.string_value = (char*)calloc(MAX_STRING_SIZE, sizeof(char));
                            if (NULL == data->metric_response[i].metric_value.string_value) {
                                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to allocate memory for string buffer");
                                g_is_thread_running = false;
                                break;
                            }
                        }
                    }

                    get_time_ns(&current_time);
                    last_stream_time = current_time;

                    while (g_is_thread_running) {
                        get_time_ns(&current_time);

                        for (uint32_t i = 0; i < capability_info->metric_ids_list_len; i++) {
                            uint32_t metric_index = data->metric_response_len;
                            if (metric_index >= total_metrics) {
                                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Metric index exceeds allocated size");
                                break;
                            }

                            data->metric_response[metric_index].metric_id = capability_info->metric_ids_list[i].metric_id;
                            data->metric_response[metric_index].timestamp = current_time;

                            if (METRIC_TYPE_BOOL_INDEX == (i % METRIC_TYPE_LEN)) {
                                data->metric_response[metric_index].metric_value.data_type  = QC_PERF_DATA_TYPE_BOOL;
                                data->metric_response[metric_index].metric_value.bool_value = true;
                            } else if (METRIC_TYPE_UINT64_INDEX == (i % METRIC_TYPE_LEN)) {
                                data->metric_response[metric_index].metric_value.data_type    = QC_PERF_DATA_TYPE_UINT64;
                                data->metric_response[metric_index].metric_value.uint64_value = uint64_counter;
                                uint64_counter++;
                            } else if (METRIC_TYPE_INT64_INDEX == (i % METRIC_TYPE_LEN)) {
                                data->metric_response[metric_index].metric_value.data_type   = QC_PERF_DATA_TYPE_INT64;
                                data->metric_response[metric_index].metric_value.int64_value = int64_counter;
                                int64_counter++;
                            } else if (METRIC_TYPE_DOUBLE_INDEX == (i % METRIC_TYPE_LEN)) {
                                data->metric_response[metric_index].metric_value.data_type    = QC_PERF_DATA_TYPE_DOUBLE;
                                data->metric_response[metric_index].metric_value.double_value = round(double_counter * 1000.0) / 1000.0;
                                double_counter += 0.1;
                            } else {
                                data->metric_response[metric_index].metric_value.data_type = QC_PERF_DATA_TYPE_STRING;
                                snprintf(dummy_string, sizeof(dummy_string), "dummy_string_%u", string_counter);
                                if (NULL != data->metric_response[metric_index].metric_value.string_value) {
                                    snprintf(data->metric_response[metric_index].metric_value.string_value, MAX_STRING_SIZE, "%s", dummy_string);
                                    data->metric_response[metric_index].metric_value.string_value_len = strlen(data->metric_response[metric_index].metric_value.string_value);
                                } else {
                                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "String buffer is NULL");
                                }
                                string_counter++;
                            }
                            data->metric_response_len++;
                        }

                        // Increment sample count
                        sample_count = sample_count + 1;

                        // Check if it's time to stream data (either by sample count or elapsed time)
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
#ifdef __linux__
                            usleep((useconds_t)(request->streaming_rate * 1000));
#elif _WIN32
                            Sleep(request->streaming_rate);
#endif
                        }
                    }

                    // Free string buffers at stop
                    for (uint32_t i = 0; i < total_metrics; i++) {
                        if (NULL != data->metric_response[i].metric_value.string_value) {
                            free(data->metric_response[i].metric_value.string_value);
                            data->metric_response[i].metric_value.string_value = NULL;
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
 * @brief Create and configure the dummy backend
 *
 * This function sets up the dummy backend by assigning function pointers
 * to the backend interface structure. It configures all required callbacks
 * for backend lifecycle management and performance monitoring operations.
 *
 * @param[in] backend Pointer to the backend private structure to be configured
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful creation
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if backend pointer is NULL
 */
enum QcPerfReturnCode qcperf_dummy_create(struct QcPerfBackendPrivate* backend) {
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == backend) {
        ret = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        backend->qcperf_backend_init   = dummy_init;
        backend->qcperf_backend_start  = dummy_start;
        backend->qcperf_backend_stop   = dummy_stop;
        backend->qcperf_backend_deinit = dummy_deinit;
        backend->qcperf_backend_info   = dummy_backend_info;
        backend->set_message_callback  = dummy_set_message_callback;
        backend->set_data_callback     = dummy_set_data_callback;
    }

    return ret;
}
