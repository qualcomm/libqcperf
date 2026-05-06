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
 * @file wos_thermal.c
 * @brief Implementation of the WOS Thermal backend for libqcperf
 * @author Skand Gupta (skangupt@qti.qualcomm.com)
 *
 * This file implements the WOS (Windows on Snapdragon) Thermal backend for
 * the QcPerf library. It provides functionality for monitoring temperature
 * and passive cooling metrics for various thermal zones on Windows platforms.
 *
 * The implementation follows the QcPerf backend interface pattern, providing
 * functions for initialization, data collection, and cleanup. It interfaces
 * with the WOS Thermal library to collect thermal data and delivers it to
 * the application through the QcPerf callback mechanism.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "wos_thermal.h"
#include "wos_thermal_lib.h"
#include "wos_thermal_logger.h"
#include "qcperf_backend_interface.h"
#include "qcperf_backend_enum.h"
#include "qthread.h"
#include "qtime.h"

#define METRIC_PER_ZONE 2

// Static variables
static bool g_is_initialized                                                       = false;
static volatile bool g_is_running                                                  = false;
static QcPerfDataCallback g_data_callback                                          = NULL;
static struct QcPerfBackendInfo g_backend_info                                     = {0};
static struct QThreadInfo g_thread_info                                            = {0};
static struct QcPerfCapabilityInfo g_capability_info                               = {0};
static struct QcPerfMetricInfo g_metrics_data[WOS_THERMAL_CAPABILITY_METRIC_COUNT] = {0};
static struct QcPerfRequest g_current_request                                      = {0};

// Streaming and sampling rates
static const uint16_t g_streaming_rates[WOS_THERMAL_STREAMING_RATES_LEN] = {WOS_THERMAL_STREAMING_RATES};
static const uint16_t g_sampling_rates[WOS_THERMAL_SAMPLING_RATES_LEN]   = {WOS_THERMAL_SAMPLING_RATES};

/**
 * @brief Thread function for collecting thermal data
 *
 * This function runs in a separate thread and continuously collects thermal data
 * at the specified sampling rate and delivers updates via the callback at the
 * specified streaming rate. It allocates memory for the QcPerfData structure and
 * metric responses, collects thermal data at regular intervals, and sends batched
 * updates to the registered callback.
 *
 * The function properly manages memory by freeing all allocated resources before
 * exiting, and implements bounds checking to prevent buffer overflows. It also
 * resets the metric index after each callback to ensure proper data tracking.
 *
 * @param[in] param Thread parameters (pointer to QcPerfRequest)
 *
 * @return NULL
 */
static void* thermal_data_collection_thread(void* param) {
    enum QcPerfReturnCode return_code                = QC_PERF_RETURN_CODE_SUCCESS;
    enum WosThermalLibReturnCode thermal_return_code = WOS_THERMAL_LIB_ERROR_FAILED;
    uint32_t samples_per_stream                      = 0;
    uint32_t sample_count                            = 0;
    uint64_t current_time                            = 0;
    struct QcPerfRequest* request                    = (struct QcPerfRequest*)param;
    struct QcPerfData* data                          = NULL;
    struct WosThermalData* thermal_data              = NULL;
    uint32_t metric_count                            = 0;
    uint32_t metric_index                            = 0;
    uint8_t zone_count                               = 0;
    uint64_t last_stream_time                        = 0;
    // Calculate how many samples to collect before streaming
    // Convert ms to ns for consistent calculations
    uint64_t sampling_rate_ns  = (uint64_t)request->sampling_rate * 1000000ULL;
    uint64_t streaming_rate_ns = (uint64_t)request->streaming_rate * 1000000ULL;
    uint64_t elapsed_ns        = 0;
    uint16_t temp_metric_id    = UINT16_MAX;
    uint16_t cooling_metric_id = UINT16_MAX;

    g_is_running = true;
    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_DEBUG, "Thermal data collection thread started");
    samples_per_stream  = streaming_rate_ns / sampling_rate_ns;
    thermal_return_code = wos_thermal_lib_get_zone_count(&zone_count);  // Get number of thermal zones from lib
    if (thermal_return_code != WOS_THERMAL_LIB_SUCCESS) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to get thermal zone count from lib");  // Log error
        g_is_running = false;
    } else {
        thermal_data = (struct WosThermalData*)calloc(1, sizeof(struct WosThermalData));
        if (NULL == thermal_data) {
            return_code = QC_PERF_RETURN_CODE_CALLOC_FAILED;
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to allocate memory for WosThermalData");
        } else {
            data = (struct QcPerfData*)calloc(1, sizeof(struct QcPerfData));
            if (NULL == data) {
                return_code = QC_PERF_RETURN_CODE_CALLOC_FAILED;
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to allocate memory for QcPerfData");
            } else {
                // Track time for streaming rate
                metric_count          = zone_count * METRIC_PER_ZONE * samples_per_stream;
                data->metric_response = (struct QcPerfMetricResponse*)calloc(metric_count, sizeof(struct QcPerfMetricResponse));
                if (NULL == data->metric_response) {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to allocate memory for thermal metric responses");
                    g_is_running = false;
                } else {
                    thermal_return_code = get_time_ns(&current_time);
                    last_stream_time    = current_time;
                    while (true == g_is_running) {
                        // Fill in data structure
                        data->backend_id   = QC_PERF_BACKEND_THERMAL;
                        data->capabilityId = WOS_THERMAL_CAPABILITY_ID;
                        // Get current timestamp in nanoseconds
                        thermal_return_code = get_time_ns(&current_time);

                        // Get thermal data
                        thermal_return_code = wos_thermal_lib_get_zone_info(thermal_data);
                        if (thermal_return_code != WOS_THERMAL_LIB_SUCCESS) {
                            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to get thermal zone info");
                        } else {
                            // Update the thermal data structure with collected values
                            // Fill in metric responses
                            for (uint8_t i = 0; i < thermal_data->zone_count && metric_index < metric_count - 1; i++) {
                                uint8_t zone_id = thermal_data->zones[i].zone_id;

                                // Get metric indices for temperature and cooling
                                wos_thermal_get_metric_index(zone_id, false, &temp_metric_id);
                                wos_thermal_get_metric_index(zone_id, true, &cooling_metric_id);

                                // Skip zones that don't have metrics defined
                                if (temp_metric_id == 0xFFFF || cooling_metric_id == 0xFFFF) {
                                    continue;
                                }

                                // Check if we have space for both metrics
                                if (metric_index + 1 >= metric_count) {
                                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Metric index exceeds allocated size");
                                    break;
                                }

                                // Temperature metric
                                data->metric_response[metric_index].timestamp                 = thermal_data->timestamp;
                                data->metric_response[metric_index].metric_id                 = temp_metric_id;
                                data->metric_response[metric_index].metric_value.data_type    = QC_PERF_DATA_TYPE_DOUBLE;
                                data->metric_response[metric_index].metric_value.double_value = thermal_data->zones[i].temperature;
                                metric_index++;

                                // Passive cooling metric
                                data->metric_response[metric_index].timestamp                 = thermal_data->timestamp;
                                data->metric_response[metric_index].metric_id                 = cooling_metric_id;
                                data->metric_response[metric_index].metric_value.data_type    = QC_PERF_DATA_TYPE_DOUBLE;
                                data->metric_response[metric_index].metric_value.double_value = thermal_data->zones[i].passive_cooling;
                                metric_index++;
                            }

                            data->metric_response_len = metric_index;
                            // Increment sample count
                            sample_count = sample_count + 1;
                        }

                        // Check if it's time to stream data (either by sample count or elapsed time)
                        elapsed_ns = current_time - last_stream_time;
                        if (sample_count >= samples_per_stream || elapsed_ns >= streaming_rate_ns) {
                            // Call the callback with the thermal data
                            if (NULL != g_data_callback) {
                                // Call data callback
                                g_data_callback(data);  // Pass data directly, not &data
                                memset(data->metric_response, 0, metric_count * sizeof(struct QcPerfMetricResponse));
                            }

                            // Reset counters
                            sample_count     = 0;
                            metric_index     = 0;  // Reset metric index after each callback
                            last_stream_time = current_time;
                        }

                        // Sleep for the sampling rate
                        if (g_is_running) {
                            Sleep(request->sampling_rate);
                        }
                    }
                }
            }
        }
    }
    // free WosThermalData
    if (NULL != thermal_data) {
        free(thermal_data);
        thermal_data = NULL;
    }
    // free QcPerfData
    if (NULL != data) {
        if (NULL != data->metric_response) {
            free(data->metric_response);  // Free allocated memory for metric responses
            data->metric_response     = NULL;
            data->metric_response_len = 0;
        }
        free(data);
        data = NULL;
    }
    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_DEBUG, "Thermal data collection thread stopped");
    return NULL;
}

/**
 * @brief Set the message callback function
 *
 * This function registers a callback function that will be invoked to deliver
 * informational, warning, or error messages from the WOS Thermal backend to the
 * application.
 *
 * @param[in] message_callback Function pointer to the message callback
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful callback registration
 */
static enum QcPerfReturnCode wos_thermal_set_message_callback(QcPerfMessageCallback message_callback) {
    wos_thermal_logger_set_message_callback(message_callback);
    return QC_PERF_RETURN_CODE_SUCCESS;
}

/**
 * @brief Set the result callback function
 *
 * This function registers a callback function that will be invoked to deliver
 * performance metric results from the WOS Thermal backend to the application.
 *
 * @param[in] data_callback Function pointer to the result callback
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful callback registration
 * @return QC_PERF_RETURN_CODE_INVALID_ARGUMENTS if data_callback is NULL
 * @return QC_PERF_RETURN_CODE_CALLBACK_ALREADY_SET if callback is already set
 */
static enum QcPerfReturnCode wos_thermal_set_data_callback(QcPerfDataCallback data_callback) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_SUCCESS;

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
 * @brief Initialize the WOS Thermal backend
 *
 * This function initializes the WOS Thermal backend. It allocates memory for
 * backend information including capabilities and metrics, and populates
 * them with the backend's configuration.
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful initialization
 * @return QC_PERF_RETURN_CODE_ALREADY_INITIALIZED if already initialized
 * @return QC_PERF_RETURN_CODE_FAILED if initialization fails
 */
static enum QcPerfReturnCode wos_thermal_init(void) {
    enum QcPerfReturnCode return_code    = QC_PERF_RETURN_CODE_SUCCESS;
    enum WosThermalLibReturnCode lib_ret = WOS_THERMAL_LIB_SUCCESS;
    uint8_t available_metric_count       = 0;

    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_DEBUG, "Initializing WOS Thermal backend");

    if (g_is_initialized) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_WARNING, "WOS Thermal backend is already initialized");
        return_code = QC_PERF_RETURN_CODE_ALREADY_INITIALIZED;
    } else {
        // Initialize thermal library first to get available zones
        lib_ret = wos_thermal_lib_init();
        if (WOS_THERMAL_LIB_SUCCESS != lib_ret) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to initialize WOS Thermal library, error: %d", lib_ret);
            return_code = QC_PERF_RETURN_CODE_FAILED;
        } else {
            // Initialize backend info
            g_backend_info.backend_id               = QC_PERF_BACKEND_THERMAL;
            g_backend_info.capabilities_list        = &g_capability_info;
            g_backend_info.capabilities_list_length = WOS_THERMAL_CAPABILITIES_LEN;

            // Initialize capability info
            g_capability_info.capability_id       = WOS_THERMAL_CAPABILITY_ID;
            g_capability_info.capability_name_len = snprintf(g_capability_info.capability_name, CAPABILITY_NAME_MAX_LEN, "%s", WOS_THERMAL_CAPABILITY);
            g_capability_info.metric_ids_list     = g_metrics_data;

            // Dynamically initialize metrics only for available zones
            wos_thermal_capability_init_available_metrics(g_metrics_data, &available_metric_count);
            if (available_metric_count > 0) {
                g_capability_info.metric_ids_list_len = available_metric_count;
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Initialized %d metrics for available thermal zones", available_metric_count);
            } else {
                // Fallback to static initialization if dynamic fails
                wos_thermal_capability_init_metrics(g_metrics_data);
                g_capability_info.metric_ids_list_len = WOS_THERMAL_CAPABILITY_METRIC_COUNT;
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_WARNING, "Failed to dynamically detect thermal zones, using static configuration");
            }

            // Set streaming rates
            for (uint8_t i = 0; i < WOS_THERMAL_STREAMING_RATES_LEN && i < MAX_SAMPLING_STREAMING_RATES_LEN; i++) {
                g_capability_info.streaming_rate[i] = g_streaming_rates[i];
                g_capability_info.streaming_rate_len++;
            }

            // Set sampling rates
            for (uint8_t i = 0; i < WOS_THERMAL_SAMPLING_RATES_LEN && i < MAX_SAMPLING_STREAMING_RATES_LEN; i++) {
                g_capability_info.sampling_rate[i] = g_sampling_rates[i];
                g_capability_info.sampling_rate_len++;
            }

            g_is_initialized = true;
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_DEBUG, "WOS Thermal backend initialized successfully");
        }
    }
    return return_code;
}

/**
 * @brief Get backend information including capabilities and metrics
 *
 * This function provides access to the WOS Thermal backend's information by
 * copying the pre-allocated backend information structure that was
 * initialized during backend initialization.
 *
 * @param[in,out] backend_info Pointer to structure to be filled with backend information
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful information retrieval
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if backend_info pointer is NULL
 * @return QC_PERF_RETURN_CODE_NOT_INITIALIZED if backend is not initialized
 */
static enum QcPerfReturnCode wos_thermal_backend_info(struct QcPerfBackendInfo* backend_info) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == backend_info) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Backend info pointer is NULL");
        return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else if (false == g_is_initialized) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "WOS Thermal backend is not initialized");
        return_code = QC_PERF_RETURN_CODE_NOT_INITIALIZED;
    } else {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Retrieving WOS Thermal backend capabilities information");
        // Copy backend info
        *backend_info = g_backend_info;
    }
    return return_code;
}

/**
 * @brief Validate the performance monitoring request
 *
 * This function validates that the request parameters are valid:
 * - Capability ID must match the WOS Thermal capability
 * - Sampling rate must match one of the supported sampling rates
 * - Streaming rate must match one of the supported streaming rates
 *
 * @param[in] request Pointer to the performance request structure
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS if validation passes
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if request is NULL
 * @return QC_PERF_RETURN_CODE_CAPABILITY_NOT_FOUND if capability ID is not found
 * @return QC_PERF_RETURN_CODE_INVALID_ARGUMENTS if sampling or streaming rate is invalid
 */
static enum QcPerfReturnCode validate_request(struct QcPerfRequest* request) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_SUCCESS;
    bool sampling_rate_valid          = false;
    bool streaming_rate_valid         = false;

    if (NULL == request) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Request pointer is NULL");
        return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        // Check capability ID
        if (request->capability_id != WOS_THERMAL_CAPABILITY_ID) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Invalid capability ID: %u", request->capability_id);
            return_code = QC_PERF_RETURN_CODE_CAPABILITY_NOT_FOUND;
        } else {
            // Check sampling rate
            if (g_capability_info.sampling_rate_len == 2) {
                if (g_capability_info.sampling_rate[0] <= request->sampling_rate && request->sampling_rate <= g_capability_info.sampling_rate[1]) {
                    sampling_rate_valid = true;
                }
            } else {
                for (uint8_t i = 0; i < g_capability_info.sampling_rate_len; i++) {
                    if (g_capability_info.sampling_rate[i] == request->sampling_rate) {
                        sampling_rate_valid = true;
                        break;
                    }
                }
            }
            if (false == sampling_rate_valid) {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Invalid sampling rate: %u", request->sampling_rate);
                return_code = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
            } else {
                // Check streaming rate
                if (g_capability_info.streaming_rate_len == 2) {
                    if (g_capability_info.streaming_rate[0] <= request->streaming_rate && request->streaming_rate <= g_capability_info.streaming_rate[1]) {
                        streaming_rate_valid = true;
                    }
                } else {
                    for (uint8_t i = 0; i < g_capability_info.streaming_rate_len; i++) {
                        if (g_capability_info.streaming_rate[i] == request->streaming_rate) {
                            streaming_rate_valid = true;
                            break;
                        }
                    }
                }
                if (false == streaming_rate_valid) {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Invalid streaming rate: %u", request->streaming_rate);
                    return_code = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
                }
            }
        }
    }
    return return_code;
}

/**
 * @brief Start performance data collection
 *
 * This function initiates performance data collection for the specified
 * capability with the given sampling and streaming rates. It validates
 * the request and starts the data collection process.
 *
 * @param[in] request Pointer to the performance request structure
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful start
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if request pointer is NULL
 * @return QC_PERF_RETURN_CODE_NOT_INITIALIZED if backend is not initialized
 * @return QC_PERF_RETURN_CODE_ALREADY_INITIALIZED if data collection is already running
 * @return QC_PERF_RETURN_CODE_CAPABILITY_NOT_FOUND if capability ID is not found
 * @return QC_PERF_RETURN_CODE_INVALID_ARGUMENTS if sampling or streaming rate is invalid
 * @return QC_PERF_RETURN_CODE_FAILED if data collection fails to start
 */
static enum QcPerfReturnCode wos_thermal_start(struct QcPerfRequest* request) {
    enum QcPerfReturnCode return_code     = QC_PERF_RETURN_CODE_SUCCESS;
    enum QThreadReturnCode thread_ret     = RET_QTHREAD_CREATE_SUCCESS;
    struct QThreadAttributes thread_attrs = {0};

    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_DEBUG, "Starting WOS Thermal data collection");

    if (false == g_is_initialized) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "WOS Thermal backend is not initialized");
        return_code = QC_PERF_RETURN_CODE_NOT_INITIALIZED;
    } else if (true == g_is_running) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_WARNING, "WOS Thermal data collection is already running");
        return_code = QC_PERF_RETURN_CODE_ALREADY_INITIALIZED;
    } else if (NULL == g_data_callback) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Data callback is not set");
        return_code = QC_PERF_RETURN_CODE_FAILED;
    } else {
        // Validate request
        return_code = validate_request(request);
        if (QC_PERF_RETURN_CODE_SUCCESS == return_code) {
            // Create thread
            thread_attrs.stack_size    = 0;
            thread_attrs.thread_params = request;
            thread_attrs.thread_fn     = thermal_data_collection_thread;
            snprintf((char*)thread_attrs.thread_name, THREAD_NAME_SIZE, "wos_thermal_thread");
            thread_attrs.thread_name_len = (uint8_t)strlen((char*)thread_attrs.thread_name);

            thread_ret = thread_create(&thread_attrs, &g_thread_info);
            if (RET_QTHREAD_CREATE_SUCCESS != thread_ret) {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to create thermal data collection thread, error: %d", thread_ret);
                return_code = WOS_THERMAL_LIB_ERROR_THREAD_CREATE_FAILED;
            } else {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Thermal data collection started successfully");
                // Store current request
                g_current_request = *request;

                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_DEBUG, "WOS Thermal data collection started successfully");
            }
        }
    }
    return return_code;
}

/**
 * @brief Stop performance data collection
 *
 * This function stops performance data collection for the specified
 * capability.
 *
 * @param[in] request Pointer to the performance request structure
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful stop
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if request pointer is NULL
 * @return QC_PERF_RETURN_CODE_NOT_INITIALIZED if backend is not initialized
 * @return QC_PERF_RETURN_CODE_FAILED if data collection fails to stop
 */
static enum QcPerfReturnCode wos_thermal_stop(struct QcPerfRequest* request) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_SUCCESS;
    enum QThreadReturnCode thread_ret = RET_QTHREAD_JOIN_FAILED;

    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_DEBUG, "Stopping WOS Thermal data collection");

    if (NULL == request) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Request pointer is NULL");
        return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else if (false == g_is_initialized) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "WOS Thermal backend is not initialized");
        return_code = QC_PERF_RETURN_CODE_NOT_INITIALIZED;
    } else if (false == g_is_running) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_WARNING, "WOS Thermal data collection is not running");
        return_code = QC_PERF_RETURN_CODE_SUCCESS;
    } else {
        // Signal thread to stop
        g_is_running = false;
        // Wait for thread to finish
        thread_ret = thread_join(&g_thread_info);
        if (RET_QTHREAD_JOIN_SUCCESS != thread_ret) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to join thermal data collection thread, error: %d", thread_ret);
            return_code = WOS_THERMAL_LIB_ERROR_THREAD_JOIN_FAILED;
        }
        // Destroy thread
        thread_ret = thread_destroy(&g_thread_info);
        if (RET_QTHREAD_DESTROY_SUCCESS != thread_ret) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to destroy thermal data collection thread, error: %d", thread_ret);
            return_code = WOS_THERMAL_LIB_ERROR_THREAD_DESTROY_FAILED;
        } else {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_DEBUG, "WOS Thermal data collection stopped successfully");
        }
    }
    return return_code;
}

/**
 * @brief Deinitialize the WOS Thermal backend
 *
 * This function deinitializes the WOS Thermal backend. It stops data collection
 * if it is running and cleans up resources.
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful deinitialization
 * @return QC_PERF_RETURN_CODE_NOT_INITIALIZED if backend is not initialized
 * @return QC_PERF_RETURN_CODE_FAILED if deinitialization fails
 */
static enum QcPerfReturnCode wos_thermal_deinit(void) {
    enum QcPerfReturnCode return_code    = QC_PERF_RETURN_CODE_SUCCESS;
    enum WosThermalLibReturnCode lib_ret = WOS_THERMAL_LIB_SUCCESS;

    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_DEBUG, "Deinitializing WOS Thermal backend");

    if (false == g_is_initialized) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_WARNING, "WOS Thermal backend is not initialized");
        return_code = QC_PERF_RETURN_CODE_NOT_INITIALIZED;
    } else {
        // Stop data collection if running
        if (g_is_running) {
            return_code = wos_thermal_stop(&g_current_request);
            if (QC_PERF_RETURN_CODE_SUCCESS != return_code) {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to stop WOS Thermal data collection during deinitialization");
                // Continue with deinitialization anyway
            }
        }

        // Clean up thermal library
        lib_ret = wos_thermal_lib_cleanup();
        if (WOS_THERMAL_LIB_SUCCESS != lib_ret) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to clean up WOS Thermal library, error: %d", lib_ret);
            return_code = QC_PERF_RETURN_CODE_FAILED;
        } else {
            g_is_initialized = false;
            g_data_callback  = NULL;
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_DEBUG, "WOS Thermal backend deinitialized successfully");
        }
    }
    return return_code;
}

/**
 * @brief Create and configure the WOS Thermal backend
 *
 * This function sets up the WOS Thermal backend by assigning function pointers
 * to the backend interface structure. It configures all required callbacks
 * for backend lifecycle management and performance monitoring operations.
 *
 * @param[in] backend Pointer to the backend private structure to be configured
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful creation
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if backend pointer is NULL
 */
enum QcPerfReturnCode qcperf_wos_thermal_backend_create(struct QcPerfBackendPrivate* backend) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == backend) {
        return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        backend->qcperf_backend_init   = wos_thermal_init;
        backend->qcperf_backend_start  = wos_thermal_start;
        backend->qcperf_backend_stop   = wos_thermal_stop;
        backend->qcperf_backend_deinit = wos_thermal_deinit;
        backend->qcperf_backend_info   = wos_thermal_backend_info;
        backend->set_message_callback  = wos_thermal_set_message_callback;
        backend->set_data_callback     = wos_thermal_set_data_callback;
    }
    return return_code;
}
