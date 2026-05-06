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
 * @file qcperf_core_test.c
 * @brief Test application for the QcPerf library core functionality
 * @author Skand Gupta (skangupt@qti.qualcomm.com)
 *
 * This file implements a comprehensive test application that exercises the core
 * functionality of the QcPerf library. It validates the complete workflow from
 * initialization to cleanup, including backend connection, capability discovery,
 * data collection, and proper resource management.
 *
 * The test application performs the following operations:
 * 1. Library initialization with qcperf_init()
 * 2. Backend connection with qcperf_connect_backend()
 * 3. Capability discovery with qcperf_get_capabilities_info()
 * 4. Deep copying of backend information for verbose printing
 * 5. Callback registration for messages and data
 * 6. Performance monitoring with qcperf_start() and qcperf_stop()
 * 7. Backend disconnection with qcperf_disconnect_backend()
 * 8. Library deinitialization with qcperf_deinit()
 * 9. Proper cleanup of all allocated resources
 *
 * The test uses the dummy backend to verify the library's functionality
 * without requiring actual hardware performance monitoring. This ensures
 * that the core API functions correctly in a controlled environment.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qcperf.h"
#include "qcperf_common.h"

#ifdef _WIN32
#include <windows.h>  // This includes winnt.h internally
#else
#include <unistd.h>
#endif

/**
 * @def RETURN_SUCCESS
 * @brief Return code indicating successful test execution
 *
 * This value is returned by main() when all test operations complete successfully.
 */
#define RETURN_SUCCESS 0

/**
 * @def RETURN_FAILED
 * @brief Return code indicating test failure
 *
 * This value is returned by main() when any test operation fails.
 */
#define RETURN_FAILED -1

/**
 * @brief Flag to control verbose printing of metric values
 *
 * When set to true, additional information about metrics will be printed,
 * including metric names, descriptions, and units. This requires creating
 * a deep copy of the backend information structure.
 *
 * When set to false, only essential information will be printed (metric IDs
 * and values), which is more efficient but less informative.
 */
volatile bool g_is_verbose_print = false;

/**
 * @brief Flag to control verbose printing of error information
 *
 * When set to true, detailed error information will be retrieved using
 * qcperf_get_error_info() and printed, including error codes and descriptive messages.
 *
 * When set to false, only a newline will be printed after error messages,
 * resulting in more concise output but less diagnostic information.
 */
bool g_is_error_verbose = true;

volatile struct QcPerfBackendInfo* g_backend_info = NULL;

/**
 * @brief Print a metric value based on its data type
 *
 * This function prints a metric value from a QcPerfGenericType structure
 * to stdout, formatting the output based on the data type. It handles all
 * supported data types (boolean, integer, floating-point, and string values)
 * with appropriate formatting for each type.
 *
 * @param[in] metric_value Pointer to the QcPerfGenericType structure containing the value
 *
 * @note If metric_value is NULL, "NULL" will be printed.
 * @note For string values, if the string pointer is NULL, "(null)" will be printed.
 * @note For unknown data types, "Unknown type X" will be printed, where X is the data type value.
 */
void print_metric_value(const struct QcPerfGenericType* metric_value) {
    if (NULL == metric_value) {
        printf("NULL\n");
    } else {
        switch (metric_value->data_type) {
        case QC_PERF_DATA_TYPE_BOOL:
            if (metric_value->bool_value) {
                printf("true");
            } else {
                printf("false");
            }
            break;
        case QC_PERF_DATA_TYPE_UINT64:
            printf("%llu", (unsigned long long)metric_value->uint64_value);
            break;
        case QC_PERF_DATA_TYPE_INT64:
            printf("%lld", (long long)metric_value->int64_value);
            break;
        case QC_PERF_DATA_TYPE_DOUBLE:
            printf("%f", metric_value->double_value);
            break;
        case QC_PERF_DATA_TYPE_STRING:
            if (NULL != metric_value->string_value) {
                printf("%.*s", (int)metric_value->string_value_len, (char*)metric_value->string_value);
            } else {
                printf("(null)");
            }
            break;
        default:
            printf("Unknown type %d", metric_value->data_type);
            break;
        }
    }
}

/**
 * @brief Print detailed error information for a return code
 *
 * This function retrieves and prints detailed error information for a given
 * return code using qcperf_get_error_info(). The verbosity of the output
 * is controlled by the g_is_error_verbose flag.
 *
 * @param[in] return_code The return code to get information for
 *
 * @note If g_is_error_verbose is true, detailed error information will be printed.
 * @note If g_is_error_verbose is false, only a newline will be printed.
 *
 * @see qcperf_get_error_info()
 * @see g_is_error_verbose
 */
void print_error_info(enum QcPerfReturnCode return_code) {
    struct QcPerfReturnCodeInfo info = {0};
    if (g_is_error_verbose) {
        qcperf_get_error_info(return_code, &info);
        printf("[ERROR] %s (code: %d)\n", info.info_str, return_code);
    } else {
        printf("\n");
    }
}

/**
 * @brief Message callback function for receiving messages from the backend
 *
 * This function is registered with the QcPerf library using qcperf_set_message_callback()
 * to receive informational, warning, and error messages from the backend.
 * It validates the message pointer and prints the message level and content to stdout.
 *
 * @param[in] message Pointer to the QcPerfMessage structure containing:
 *                    - message: The message text
 *                    - message_length: Length of the message in bytes
 *                    - message_level: Severity level (debug, info, warning, error)
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS if the message was processed successfully
 * @return QC_PERF_RETURN_CODE_FAILED if the message pointer is NULL or the message text is NULL
 *
 * @see qcperf_set_message_callback()
 * @see struct QcPerfMessage
 */
enum QcPerfReturnCode message_callback(struct QcPerfMessage* message) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_FAILED;
    const char* level_str             = "UNKNOWN";

    if (NULL == message || NULL == message->message) {
        printf("[ERROR] Message callback received NULL message\n");
        return_code = QC_PERF_RETURN_CODE_FAILED;
    } else {
        // Convert message level to human-readable string
        switch (message->message_level) {
        case QC_PERF_MESSAGE_LEVEL_DEBUG:
            level_str = "DEBUG";
            break;
        case QC_PERF_MESSAGE_LEVEL_INFO:
            level_str = "INFO";
            break;
        case QC_PERF_MESSAGE_LEVEL_WARNING:
            level_str = "WARNING";
            break;
        case QC_PERF_MESSAGE_LEVEL_ERROR:
            level_str = "ERROR";
            break;
        default:
            level_str = "UNKNOWN";
            break;
        }
        if (message->message_level != QC_PERF_MESSAGE_LEVEL_DEBUG) {
            printf("[%s] Backend message: %s\n", level_str, message->message);
        }

        return_code = QC_PERF_RETURN_CODE_SUCCESS;
    }
    return return_code;
}

/**
 * @brief Callback function for receiving performance data from the backend
 *
 * This function is registered with the QcPerf library using qcperf_set_data_callback()
 * to receive performance metric data from the backend. It validates the data pointer
 * and prints the capability ID, number of metrics, and metric values to stdout.
 *
 * The function supports two output modes:
 * 1. Verbose mode: Prints metric names, descriptions, and units (requires g_backend_info)
 * 2. Basic mode: Prints only metric IDs and values
 *
 * @param[in] data Pointer to the QcPerfData structure containing:
 *                 - capabilityId: The capability that generated the data
 *                 - metric_response: Array of metric responses
 *                 - metric_response_len: Number of metrics in the array
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS if the data was processed successfully
 * @return QC_PERF_RETURN_CODE_FAILED if the data pointer is NULL or invalid
 *
 * @see qcperf_set_data_callback()
 * @see struct QcPerfData
 * @see g_is_verbose_print
 * @see print_metric_value()
 */
enum QcPerfReturnCode result_callback(struct QcPerfData* data) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_FAILED;
    bool metric_found                 = false;
    const char* metric_name           = NULL;
    const char* metric_unit           = NULL;
    const char* metric_desc           = NULL;
    if (NULL == data) {
        printf("[ERROR] Data callback received NULL data\n");
        return_code = QC_PERF_RETURN_CODE_FAILED;
    } else {
        printf("[DATA] Capability ID: %d, Metrics count: %d\n", data->capabilityId, data->metric_response_len);

        if (NULL != data->metric_response) {
            for (uint32_t metric_index = 0; metric_index < data->metric_response_len; metric_index++) {
                // Check if we can use verbose printing mode
                if (g_is_verbose_print && NULL != g_backend_info && NULL != g_backend_info->capabilities_list && data->capabilityId < g_backend_info->capabilities_list_length &&
                    NULL != g_backend_info->capabilities_list[data->capabilityId].metric_ids_list) {
                    // Search for the metric by ID (don't use metric_id as array index)
                    for (uint32_t i = 0; i < g_backend_info->capabilities_list[data->capabilityId].metric_ids_list_len; i++) {
                        if (g_backend_info->capabilities_list[data->capabilityId].metric_ids_list[i].metric_id == data->metric_response[metric_index].metric_id) {
                            metric_name  = g_backend_info->capabilities_list[data->capabilityId].metric_ids_list[i].metric_name;
                            metric_unit  = g_backend_info->capabilities_list[data->capabilityId].metric_ids_list[i].metric_unit;
                            metric_desc  = g_backend_info->capabilities_list[data->capabilityId].metric_ids_list[i].metric_description;
                            metric_found = true;
                            break;
                        }
                    }

                    if (true == metric_found) {
                        // Verbose mode - print metric name, value, unit and description
                        printf("  [%llu] %s: ", (unsigned long long)data->metric_response[metric_index].timestamp, metric_name);
                        print_metric_value(&data->metric_response[metric_index].metric_value);
                        printf(" %s (%s)\n", metric_unit, metric_desc);
                    } else {
                        // Metric not found in list - print only metric ID and value
                        printf("  [%llu] Metric ID %d: ", (unsigned long long)data->metric_response[metric_index].timestamp, data->metric_response[metric_index].metric_id);
                        print_metric_value(&data->metric_response[metric_index].metric_value);
                        printf("\n");
                    }
                } else {
                    // Basic mode - print only metric ID and value
                    printf("  [%llu] Metric ID %d: ", (unsigned long long)data->metric_response[metric_index].timestamp, data->metric_response[metric_index].metric_id);
                    print_metric_value(&data->metric_response[metric_index].metric_value);
                    printf("\n");
                }
            }
            return_code = QC_PERF_RETURN_CODE_SUCCESS;
        }
    }
    return return_code;
}

/**
 * @brief Main entry point for the QcPerf core test application
 *
 * This function performs a comprehensive test of the QcPerf library's core functionality,
 * exercising the complete workflow from initialization to cleanup. It tests:
 *
 * 1. Library initialization with qcperf_init()
 * 2. Backend connection with qcperf_connect_backend()
 * 3. Capability discovery with qcperf_get_capabilities_info()
 * 4. Deep copying of backend information (when verbose printing is enabled)
 * 5. Message callback registration with qcperf_set_message_callback()
 * 6. Data callback registration with qcperf_set_data_callback()
 * 7. Performance monitoring with qcperf_start() and qcperf_stop()
 * 8. Backend disconnection with qcperf_disconnect_backend()
 * 9. Library deinitialization with qcperf_deinit()
 * 10. Proper cleanup of all allocated resources
 *
 * The function includes comprehensive error handling and resource management,
 * ensuring that all allocated memory is properly freed even in error cases.
 *
 * @param[in] argc Number of command line arguments (unused)
 * @param[in] argv Array of command line argument strings (unused)
 *
 * @return RETURN_SUCCESS (0) if all tests pass
 * @return RETURN_FAILED (-1) if any test fails
 *
 * @see qcperf_init()
 * @see qcperf_connect_backend()
 * @see qcperf_get_capabilities_info()
 * @see qcperf_set_message_callback()
 * @see qcperf_set_data_callback()
 * @see qcperf_start()
 * @see qcperf_stop()
 * @see qcperf_disconnect_backend()
 * @see qcperf_deinit()
 */
/**
 * @brief Create a deep copy of backend info for verbose printing
 *
 * This function creates a deep copy of the backend information structure,
 * including all nested capability and metric information. If any allocation
 * fails during the copy process, all previously allocated memory is properly
 * cleaned up before returning an error code.
 *
 * @param[in] backend_info Source backend info structure to copy
 * @return QC_PERF_RETURN_CODE_SUCCESS if successful
 * @return QC_PERF_RETURN_CODE_INVALID_ARGUMENTS if backend_info is NULL
 * @return QC_PERF_RETURN_CODE_CALLOC_FAILED if memory allocation fails
 *
 * @note On failure, g_backend_info is set to NULL and g_is_verbose_print is set to false
 * @note All allocated memory is freed on failure to prevent memory leaks
 */
static enum QcPerfReturnCode create_backend_info_copy(const struct QcPerfBackendInfo* backend_info) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_SUCCESS;
    uint8_t capability_itr            = 0;
    uint8_t metric_id_itr             = 0;

    if (NULL == backend_info) {
        return_code = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
    } else {
        // Allocate memory for global backend info
        g_backend_info = (struct QcPerfBackendInfo*)calloc(1, sizeof(struct QcPerfBackendInfo));
        if (NULL == g_backend_info) {
            printf("Failed to allocate memory for global backend info, disabling verbose print\n");
            g_is_verbose_print = false;
            return_code        = QC_PERF_RETURN_CODE_CALLOC_FAILED;
        } else {
            // Copy basic fields
            g_backend_info->backend_id               = backend_info->backend_id;
            g_backend_info->capabilities_list_length = backend_info->capabilities_list_length;

            // Allocate memory for capabilities list
            g_backend_info->capabilities_list = (struct QcPerfCapabilityInfo*)calloc(backend_info->capabilities_list_length, sizeof(struct QcPerfCapabilityInfo));

            if (NULL == g_backend_info->capabilities_list) {
                printf("Failed to allocate memory for capabilities list, disabling verbose print\n");
                g_is_verbose_print = false;
                free((void*)g_backend_info);
                g_backend_info = NULL;
                return_code    = QC_PERF_RETURN_CODE_CALLOC_FAILED;
            } else {
                // Copy each capability
                for (capability_itr = 0; capability_itr < backend_info->capabilities_list_length; capability_itr++) {
                    // Copy capability structure
                    memcpy(&g_backend_info->capabilities_list[capability_itr], &backend_info->capabilities_list[capability_itr], sizeof(struct QcPerfCapabilityInfo));

                    // Allocate memory for metric_ids_list
                    g_backend_info->capabilities_list[capability_itr].metric_ids_list =
                        (struct QcPerfMetricInfo*)calloc(backend_info->capabilities_list[capability_itr].metric_ids_list_len, sizeof(struct QcPerfMetricInfo));

                    if (NULL == g_backend_info->capabilities_list[capability_itr].metric_ids_list) {
                        printf("Failed to allocate memory for metric IDs list, disabling verbose print\n");
                        g_is_verbose_print = false;
                        return_code        = QC_PERF_RETURN_CODE_CALLOC_FAILED;
                        // Clean up previously allocated metric_ids_list arrays
                        for (uint8_t cleanup_itr = 0; cleanup_itr < capability_itr; cleanup_itr++) {
                            if (NULL != g_backend_info->capabilities_list[cleanup_itr].metric_ids_list) {
                                free(g_backend_info->capabilities_list[cleanup_itr].metric_ids_list);
                                g_backend_info->capabilities_list[cleanup_itr].metric_ids_list = NULL;
                            }
                        }
                        free(g_backend_info->capabilities_list);
                        g_backend_info->capabilities_list = NULL;
                        free((void*)g_backend_info);
                        g_backend_info = NULL;
                    } else {
                        // Copy each metric
                        for (metric_id_itr = 0; metric_id_itr < g_backend_info->capabilities_list[capability_itr].metric_ids_list_len; metric_id_itr++) {
                            memcpy(&g_backend_info->capabilities_list[capability_itr].metric_ids_list[metric_id_itr], &backend_info->capabilities_list[capability_itr].metric_ids_list[metric_id_itr],
                                   sizeof(struct QcPerfMetricInfo));
                        }
                    }
                }
            }
        }
    }
    return return_code;
}

/**
 * @brief Clean up the global backend info structure
 */
static void cleanup_backend_info(void) {
    uint8_t capability_itr = 0;

    if (NULL == g_backend_info) {
        return;
    }

    if (NULL != g_backend_info->capabilities_list) {
        for (capability_itr = 0; capability_itr < g_backend_info->capabilities_list_length; capability_itr++) {
            if (NULL != g_backend_info->capabilities_list[capability_itr].metric_ids_list) {
                free(g_backend_info->capabilities_list[capability_itr].metric_ids_list);
                g_backend_info->capabilities_list[capability_itr].metric_ids_list = NULL;
            }
        }
        free(g_backend_info->capabilities_list);
        g_backend_info->capabilities_list = NULL;
    }

    free((void*)g_backend_info);
    g_backend_info = NULL;
}

/**
 * @brief Test a specific capability
 *
 * @param[in] test_backend Backend ID to test
 * @param[in] capability_info Pointer to capability information
 * @param[in] request Pointer to request structure to use
 * @return QC_PERF_RETURN_CODE_SUCCESS if successful, error code otherwise
 */
static enum QcPerfReturnCode test_capability(enum QcPerfBackendId test_backend, const struct QcPerfCapabilityInfo* capability_info, struct QcPerfRequest* request) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == capability_info || NULL == request) {
        printf("[ERROR] Invalid parameters for capability test\n");
        return_code = QC_PERF_RETURN_CODE_FAILED;
    } else {
        printf("\n========================================================================\n");
        printf("[TEST] Testing capability ID: %d", capability_info->capability_id);

        if (g_is_verbose_print) {
            printf(" (%s)", capability_info->capability_name);
        }
        printf("\n");

        // Configure request with capability's recommended settings
        request->capability_id = capability_info->capability_id;

        printf("[INFO] Using sampling rate: %d ms, streaming rate: %d ms\n", request->sampling_rate, request->streaming_rate);

        // Start profiling
        printf("[INFO] Starting profiling...\n");
        return_code = qcperf_start(test_backend, request);
        if (QC_PERF_RETURN_CODE_SUCCESS != return_code) {
            printf("[ERROR] Failed to start profiling");
            print_error_info(return_code);
        } else {
            printf("[INFO] Profiling started successfully\n");

            // Wait for data collection
#ifdef _WIN32
            printf("[INFO] Collecting data for 10 seconds...\n");
            Sleep(10000);  // Wait for 10 seconds to allow profiling to run
#else
            printf("[INFO] Collecting data for 10 seconds...\n");
            sleep(10);  // Wait for 10 seconds to allow profiling to run
#endif

            // Stop profiling
            printf("[INFO] Stopping profiling...\n");
            return_code = qcperf_stop(test_backend, request);
            if (QC_PERF_RETURN_CODE_SUCCESS != return_code) {
                printf("[ERROR] Failed to stop profiling");
                print_error_info(return_code);
            } else {
                printf("[INFO] Profiling stopped successfully\n");
            }
        }

        printf("========================================================================\n");
    }
    return return_code;
}

/**
 * @brief Main entry point for the QcPerf core test application
 *
 * @param[in] argc Number of command line arguments (unused)
 * @param[in] argv Array of command line argument strings (unused)
 * @return RETURN_SUCCESS if all tests pass, RETURN_FAILED if any test fails
 */
int main(int argc, char** argv) {
    int main_return                        = RETURN_SUCCESS;
    enum QcPerfReturnCode return_code      = QC_PERF_RETURN_CODE_FAILED;
    enum QcPerfBackendId test_backend      = QC_PERF_BACKEND_DUMMY;
    struct QcPerfBackendInfo* backend_info = NULL;
    struct QcPerfRequest* request          = NULL;
    uint8_t capability_index               = 0;
    uint16_t streaming_rate                = 0;
    uint16_t sampling_rate                 = 0;
    struct QcPerfVersionInfo version_info  = {0};

    printf("\n========================================================================\n");
    printf("                         QCPerf Core Test                                \n");
    printf("========================================================================\n\n");

    // Display version information
    printf("[INFO] Getting QcPerf version information...\n");
    return_code = qcperf_version(&version_info);
    if (QC_PERF_RETURN_CODE_SUCCESS == return_code) {
        printf("[INFO] QcPerf Library Version: %d.%d.%d.%d\n", version_info.build, version_info.major, version_info.minor, version_info.patch);

    } else {
        printf("[ERROR] Failed to get version information\n");
    }
    printf("\n");
    if (argc == 5) {
        test_backend   = atoi(argv[1]);
        sampling_rate  = (uint16_t)atoi(argv[2]);
        streaming_rate = (uint16_t)atoi(argv[3]);

        // Set verbose mode based on 4th argument (0 = false, any other value = true)
        if (argv[4][0] == '0') {
            g_is_verbose_print = false;
            printf("[INFO] Verbose mode disabled\n");
        } else {
            g_is_verbose_print = true;
            printf("[INFO] Verbose mode enabled\n");
        }

        printf("[INFO] Using backend ID: %d, sampling rate: %d ms, streaming rate: %d ms\n", test_backend, sampling_rate, streaming_rate);
        // ========================================================================
        // Step 1: Initialize QCPerf
        // ========================================================================
        printf("[STEP 1] Initializing QCPerf library...\n");
        return_code = qcperf_init();
        if (QC_PERF_RETURN_CODE_SUCCESS != return_code) {
            printf("[ERROR] Failed to initialize QCPerf");
            print_error_info(return_code);
            main_return = RETURN_FAILED;
        } else {
            printf("[INFO] QCPerf library initialized successfully\n\n");

            // ========================================================================
            // Step 2: Connect to backend
            // ========================================================================
            // Get backend name based on backend ID
            const char* backend_name = "unknown";
            switch (test_backend) {
#if defined(QCPERF_ENABLED_DUMMY)
            case QC_PERF_BACKEND_DUMMY:
                backend_name = "dummy";
                break;
#endif
#if defined(QCPERF_ENABLED_QCOM_LINUX_CPU)
            case QC_PERF_BACKEND_QCOM_LINUX_CPU:
                backend_name = "cpu";
                break;
#endif
#if defined(QCPERF_ENABLED_QCOM_LINUX_NPU)
            case QC_PERF_BACKEND_DSP_NPU:
                backend_name = "npu";
                break;
#endif
#if defined(QCPERF_ENABLED_WOS_POWER)
            case QC_PERF_BACKEND_POWER:
                backend_name = "power";
                break;
#endif
#if defined(QCPERF_ENABLED_WOS_THERMAL)
            case QC_PERF_BACKEND_THERMAL:
                backend_name = "thermal";
                break;
#endif
            default:
                backend_name = "unknown";
                break;
            }

            printf("[STEP 2] Connecting to %s backend...\n", backend_name);
            return_code = qcperf_connect_backend(test_backend, &message_callback);
            if (QC_PERF_RETURN_CODE_SUCCESS != return_code) {
                printf("[ERROR] Failed to connect to backend");
                print_error_info(return_code);
                main_return = RETURN_FAILED;
            } else {
                printf("[INFO] Backend connected successfully\n\n");

                // ========================================================================
                // Step 3: Get backend capabilities
                // ========================================================================
                printf("[STEP 3] Retrieving backend capabilities...\n");

                // Allocate memory for backend info
                backend_info = (struct QcPerfBackendInfo*)calloc(1, sizeof(struct QcPerfBackendInfo));
                if (NULL == backend_info) {
                    printf("[ERROR] Failed to allocate memory for backend info\n");
                    main_return = RETURN_FAILED;
                } else {
                    // Get capabilities info
                    return_code = qcperf_get_capabilities_info(test_backend, backend_info);
                    if (QC_PERF_RETURN_CODE_SUCCESS != return_code || 0 == backend_info->capabilities_list_length || NULL == backend_info->capabilities_list) {
                        printf("[ERROR] Failed to get backend capabilities");
                        print_error_info(return_code);
                        main_return = RETURN_FAILED;
                    } else {
                        printf("[INFO] Retrieved %d capabilities from backend\n\n", backend_info->capabilities_list_length);

                        // ========================================================================
                        // Step 4: Create deep copy of backend info (if verbose mode enabled)
                        // ========================================================================
                        if (true == g_is_verbose_print) {
                            printf("[STEP 4] Creating deep copy of backend info for verbose printing...\n");
                            return_code = create_backend_info_copy(backend_info);
                            if (QC_PERF_RETURN_CODE_SUCCESS != return_code) {
                                printf("[WARNING] Failed to create deep copy of backend info\n");
                                main_return = RETURN_FAILED;
                            } else {
                                printf("[INFO] Backend info copied successfully\n\n");
                            }
                        }

                        // ========================================================================
                        // Step 5: Register data callback
                        // ========================================================================
                        printf("[STEP 5] Registering data callback...\n");
                        return_code = qcperf_set_data_callback(test_backend, &result_callback);
                        if (QC_PERF_RETURN_CODE_SUCCESS != return_code) {
                            printf("[ERROR] Failed to set data callback");
                            print_error_info(return_code);
                            main_return = RETURN_FAILED;
                        } else {
                            printf("[INFO] Data callback registered successfully\n\n");

                            // ========================================================================
                            // Step 6: Test each capability
                            // ========================================================================
                            printf("[STEP 6] Testing %d capabilities...\n", backend_info->capabilities_list_length);

                            // Allocate memory for request
                            request = (struct QcPerfRequest*)calloc(1, sizeof(struct QcPerfRequest));
                            if (NULL == request) {
                                printf("[ERROR] Failed to allocate memory for request\n");
                                main_return = RETURN_FAILED;
                            } else {
                                // Test each capability
                                for (capability_index = 0; capability_index < backend_info->capabilities_list_length; capability_index++) {
                                    // Set sampling and streaming rates
                                    if (sampling_rate != 0) {
                                        request->sampling_rate = sampling_rate;
                                    } else {
                                        request->sampling_rate = backend_info->capabilities_list[capability_index].sampling_rate[0];
                                    }

                                    if (streaming_rate != 0) {
                                        request->streaming_rate = streaming_rate;
                                    } else {
                                        request->streaming_rate = backend_info->capabilities_list[capability_index].streaming_rate[0];
                                    }
                                    return_code = test_capability(test_backend, &backend_info->capabilities_list[capability_index], request);

                                    if (QC_PERF_RETURN_CODE_SUCCESS != return_code) {
                                        main_return = RETURN_FAILED;
                                    }
                                }
                                printf("[INFO] Capability testing completed\n\n");
                            }
                        }
                    }
                }

                // ========================================================================
                // Step 7: Disconnect backend
                // ========================================================================
                printf("[STEP 7] Disconnecting backend...\n");
                return_code = qcperf_disconnect_backend(test_backend);
                if (QC_PERF_RETURN_CODE_SUCCESS != return_code) {
                    printf("[ERROR] Failed to disconnect backend");
                    print_error_info(return_code);
                    main_return = RETURN_FAILED;
                } else {
                    printf("[INFO] Backend disconnected successfully\n\n");
                }
            }

            // ========================================================================
            // Step 8: Deinitialize QCPerf
            // ========================================================================
            printf("[STEP 8] Deinitializing QCPerf library...\n");
            return_code = qcperf_deinit();
            if (QC_PERF_RETURN_CODE_SUCCESS != return_code) {
                printf("[ERROR] Failed to deinitialize QCPerf");
                print_error_info(return_code);
                main_return = RETURN_FAILED;
            } else {
                printf("[INFO] QCPerf library deinitialized successfully\n\n");
            }
        }
        // ========================================================================
        // Step 9: Clean up resources
        // ========================================================================
        printf("[STEP 9] Cleaning up resources...\n");

        if (NULL != request) {
            free(request);
            request = NULL;
        }

        if (NULL != backend_info) {
            free(backend_info);
            backend_info = NULL;
        }

        cleanup_backend_info();
        printf("[INFO] Resource cleanup completed\n\n");

        // ========================================================================
        // Print final result
        // ========================================================================
        printf("========================================================================\n");
        if (RETURN_SUCCESS == main_return) {
            printf("[SUCCESS] Core test completed successfully\n");
        } else {
            printf("[FAILURE] Core test failed\n");
        }
    } else {
        printf("[WARNING] Invalid arguments. Usage: %s <backend_id> <sampling_rate> <streaming_rate> <verbose_mode>\n", argv[0]);
    }
    printf("========================================================================\n\n");

    return main_return;
}
