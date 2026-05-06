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
 * @file wos_power_backend.c
 * @brief Implementation of the WOS Power backend for libqcperf
 * @author Sourav Sarkar (sousar@qti.qualcomm.com)
 *
 * This file implements the WOS (Windows on Snapdragon) Power backend for
 * the QcPerf library. It provides functionality for monitoring power metrics
 * on WoS platforms.
 *
 * The implementation follows the QcPerf backend interface pattern, providing
 * functions for initialization, data collection, and cleanup. It interfaces
 * with the WoS Power library to collect Power data and delivers it to
 * the application through the QcPerf callback mechanism.
 */

#include <stdbool.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include "power_telemetry.h"
#include "qthread.h"
#include "qtime.h"
#include "wos_power_backend.h"
#include "wos_power_backend_logger.h"
#include "wos_power_backend_info.h"

#define NS_TO_MS 1e6
#define SAMPLING_RATE_SHIFT 1
#define MIN_SAMPLING_RATE 100
#define MIN_STREAMING_RATE 200
#define MAX_SAMPLING_RATE 1000

enum ePowerTelemetryHandlerSources {
    POWER_TELEMETRY_SOURCE_RAILS,
    MAX_POWER_TELEMETRY_HANDLER_SOURCES,
};

static bool g_is_thread_running                                               = false;
static struct QThreadInfo g_thread_info                                       = {0};
uint8_t g_capability1_name[]                                                  = "wos-power-metrics";
uint8_t g_thread_name[]                                                       = "wos-power-metrics-thread";
static uint8_t g_requested_metrics_length                                     = 0;
static bool g_sources_enablement_mapping[MAX_POWER_TELEMETRY_HANDLER_SOURCES] = {0};
enum ePowerTelemetryRail g_system_rail_index                                  = POWER_TELEMETRY_RAIL_SYS;
static QcPerfDataCallback g_data_callback                                     = NULL;

static struct QcPerfBackendInfo g_backend_info = {0};
#define TOTAL_CAPABILITIES 1
#define CAPABILITY_1_METRIC_COUNT 16

uint32_t get_wos_power_telemetry_metrics_data(void* lpParam);
static void wos_power_backend_free_memory(void);

/**
 * @brief Allocate memory for backend information.
 *
 * This function allocates memory for the backend's capabilities and metrics.
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful allocation,
 *         QC_PERF_RETURN_CODE_CALLOC_FAILED otherwise.
 */
static enum QcPerfReturnCode wos_power_backend_alloc(void) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_SUCCESS;

    g_backend_info.capabilities_list = (struct QcPerfCapabilityInfo*)calloc(TOTAL_CAPABILITIES, sizeof(struct QcPerfCapabilityInfo));

    if (NULL == g_backend_info.capabilities_list) {
        return_code = QC_PERF_RETURN_CODE_CALLOC_FAILED;
    } else {
        g_backend_info.capabilities_list_length                                        = TOTAL_CAPABILITIES;
        g_backend_info.capabilities_list[WOS_POWER_CAPABILITY_1_INDEX].metric_ids_list = (struct QcPerfMetricInfo*)calloc(CAPABILITY_1_METRIC_COUNT, sizeof(struct QcPerfMetricInfo));
        if (NULL == g_backend_info.capabilities_list[WOS_POWER_CAPABILITY_1_INDEX].metric_ids_list) {
            free(g_backend_info.capabilities_list);
            g_backend_info.capabilities_list        = NULL;
            g_backend_info.capabilities_list_length = 0;
            return_code                             = QC_PERF_RETURN_CODE_CALLOC_FAILED;
        } else {
            g_backend_info.capabilities_list[WOS_POWER_CAPABILITY_1_INDEX].metric_ids_list_len = CAPABILITY_1_METRIC_COUNT;
        }
    }
    return return_code;
}

/**
 * @brief Free all allocated memory for backend information.
 */
static void wos_power_backend_free_memory(void) {
    if (NULL != g_backend_info.capabilities_list) {
        if (NULL != g_backend_info.capabilities_list[WOS_POWER_CAPABILITY_1_INDEX].metric_ids_list) {
            free(g_backend_info.capabilities_list[WOS_POWER_CAPABILITY_1_INDEX].metric_ids_list);
            g_backend_info.capabilities_list[WOS_POWER_CAPABILITY_1_INDEX].metric_ids_list = NULL;
        }
        g_backend_info.capabilities_list[WOS_POWER_CAPABILITY_1_INDEX].metric_ids_list_len = 0;
        free(g_backend_info.capabilities_list);
        g_backend_info.capabilities_list = NULL;
    }
    g_backend_info.capabilities_list_length = 0;
}

/**
 * @brief Initialize the backend and populate its capabilities.
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success, an error code otherwise.
 */
static enum QcPerfReturnCode wos_power_backend_init() {
    enum QcPerfReturnCode return_code                          = QC_PERF_RETURN_CODE_SUCCESS;
    enum QcPerfReturnCode return_code_init_capabilities        = QC_PERF_RETURN_CODE_SUCCESS;
    enum ePowerTelemetryReturnCode power_telemetry_return_code = RETURN_CODE_POWER_TELEMETRY_SUCCESS;
    uint32_t rail_index                                        = 0;
    uint32_t capability_metrics_index                          = 0;
    uint32_t capability_index_rail                             = 0;
    struct RailsSupported rails_supported                      = {0};
    struct QcPerfMetricInfo* metric_info_curr                  = NULL;
    uint32_t streaming_rate_min                                = 1000;
    uint32_t streaming_rate_max                                = 2000;
    uint32_t sampling_rate_min                                 = 1000;
    uint32_t sampling_rate_max                                 = 2000;

    g_backend_info.backend_id = QC_PERF_BACKEND_POWER;
    return_code               = wos_power_backend_alloc();
    if (QC_PERF_RETURN_CODE_SUCCESS != return_code) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Initializtion failed. Unable to allocate memory");
        wos_power_backend_free_memory();
        return_code_init_capabilities = QC_PERF_RETURN_CODE_CALLOC_FAILED;
    } else {
        streaming_rate_max = streaming_rate_min;
        streaming_rate_min = MIN_STREAMING_RATE;
        sampling_rate_min  = MIN_SAMPLING_RATE;
        sampling_rate_max  = MAX_SAMPLING_RATE;

        g_backend_info.capabilities_list[WOS_POWER_CAPABILITY_1_INDEX].streaming_rate[0]  = streaming_rate_min;
        g_backend_info.capabilities_list[WOS_POWER_CAPABILITY_1_INDEX].streaming_rate[1]  = streaming_rate_max;
        g_backend_info.capabilities_list[WOS_POWER_CAPABILITY_1_INDEX].streaming_rate_len = 2;

        g_backend_info.capabilities_list[WOS_POWER_CAPABILITY_1_INDEX].sampling_rate[0]  = sampling_rate_min;
        g_backend_info.capabilities_list[WOS_POWER_CAPABILITY_1_INDEX].sampling_rate[1]  = sampling_rate_max;
        g_backend_info.capabilities_list[WOS_POWER_CAPABILITY_1_INDEX].sampling_rate_len = 2;

        power_telemetry_return_code = powerTelemetry_railsGetSupported(&rails_supported, false);
        if (RETURN_CODE_POWER_TELEMETRY_SUCCESS != power_telemetry_return_code) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "powerTelemetry_railsGetSupported returned error=%u", power_telemetry_return_code);
            return_code_init_capabilities = QC_PERF_RETURN_CODE_FAILED;
        } else {
            g_backend_info.capabilities_list[WOS_POWER_CAPABILITY_1_INDEX].capability_id = WOS_POWER_CAPABILITY_1_ID;
            g_backend_info.capabilities_list[WOS_POWER_CAPABILITY_1_INDEX].capability_name_len =
                snprintf(g_backend_info.capabilities_list[WOS_POWER_CAPABILITY_1_INDEX].capability_name, CAPABILITY_NAME_MAX_LEN, "%s", g_capability1_name);

            while (rail_index < MAX_POWER_TELEMETRY_RAILS && capability_metrics_index < CAPABILITY_1_METRIC_COUNT) {
                metric_info_curr = &g_backend_info.capabilities_list[WOS_POWER_CAPABILITY_1_INDEX].metric_ids_list[capability_metrics_index];
                if (true == rails_supported.supportedRailsMap[rail_index]) {
                    switch ((enum ePowerTelemetryRail)rail_index) {
                    case POWER_TELEMETRY_RAIL_CPU_CLUSTER_0:
                        metric_info_curr->metric_id              = WOS_POWER_METRIC_0_ID;
                        metric_info_curr->metric_name_len        = snprintf(metric_info_curr->metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_POWER_METRIC_0_NAME);
                        metric_info_curr->metric_description_len = snprintf(metric_info_curr->metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_POWER_METRIC_0_DESCRIPTION);
                        metric_info_curr->metric_unit_len        = snprintf(metric_info_curr->metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_POWER_METRIC_0_UNIT);
                        capability_metrics_index++;
                        break;
                    case POWER_TELEMETRY_RAIL_CPU_CLUSTER_1:
                        metric_info_curr->metric_id              = WOS_POWER_METRIC_1_ID;
                        metric_info_curr->metric_name_len        = snprintf(metric_info_curr->metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_POWER_METRIC_1_NAME);
                        metric_info_curr->metric_description_len = snprintf(metric_info_curr->metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_POWER_METRIC_1_DESCRIPTION);
                        metric_info_curr->metric_unit_len        = snprintf(metric_info_curr->metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_POWER_METRIC_1_UNIT);
                        capability_metrics_index++;
                        break;
                    case POWER_TELEMETRY_RAIL_CPU_CLUSTER_2:
                        metric_info_curr->metric_id              = WOS_POWER_METRIC_2_ID;
                        metric_info_curr->metric_name_len        = snprintf(metric_info_curr->metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_POWER_METRIC_2_NAME);
                        metric_info_curr->metric_description_len = snprintf(metric_info_curr->metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_POWER_METRIC_2_DESCRIPTION);
                        metric_info_curr->metric_unit_len        = snprintf(metric_info_curr->metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_POWER_METRIC_2_UNIT);
                        capability_metrics_index++;
                        break;
                    case POWER_TELEMETRY_RAIL_GPU:
                        metric_info_curr->metric_id              = WOS_POWER_METRIC_3_ID;
                        metric_info_curr->metric_name_len        = snprintf(metric_info_curr->metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_POWER_METRIC_3_NAME);
                        metric_info_curr->metric_description_len = snprintf(metric_info_curr->metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_POWER_METRIC_3_DESCRIPTION);
                        metric_info_curr->metric_unit_len        = snprintf(metric_info_curr->metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_POWER_METRIC_3_UNIT);
                        capability_metrics_index++;
                        break;
                    case POWER_TELEMETRY_RAIL_PSU_USB:
                        metric_info_curr->metric_id              = WOS_POWER_METRIC_4_ID;
                        metric_info_curr->metric_name_len        = snprintf(metric_info_curr->metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_POWER_METRIC_4_NAME);
                        metric_info_curr->metric_description_len = snprintf(metric_info_curr->metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_POWER_METRIC_4_DESCRIPTION);
                        metric_info_curr->metric_unit_len        = snprintf(metric_info_curr->metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_POWER_METRIC_4_UNIT);
                        capability_metrics_index++;
                        break;
                    case POWER_TELEMETRY_RAIL_SYS:
                        metric_info_curr->metric_id              = WOS_POWER_METRIC_5_ID;
                        metric_info_curr->metric_name_len        = snprintf(metric_info_curr->metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_POWER_METRIC_5_NAME);
                        metric_info_curr->metric_description_len = snprintf(metric_info_curr->metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_POWER_METRIC_5_DESCRIPTION);
                        metric_info_curr->metric_unit_len        = snprintf(metric_info_curr->metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_POWER_METRIC_5_UNIT);
                        capability_metrics_index++;
                        g_system_rail_index = POWER_TELEMETRY_RAIL_SYS;
                        break;
                    case POWER_TELEMETRY_RAIL_SYSTEM:
                        metric_info_curr->metric_id              = WOS_POWER_METRIC_5_ID;
                        metric_info_curr->metric_name_len        = snprintf(metric_info_curr->metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_POWER_METRIC_5_NAME);
                        metric_info_curr->metric_description_len = snprintf(metric_info_curr->metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_POWER_METRIC_5_DESCRIPTION);
                        metric_info_curr->metric_unit_len        = snprintf(metric_info_curr->metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_POWER_METRIC_5_UNIT);
                        capability_metrics_index++;
                        g_system_rail_index = POWER_TELEMETRY_RAIL_SYSTEM;
                        break;
                    case POWER_TELEMETRY_RAIL_USBC_TOTAL:
                        metric_info_curr->metric_id              = WOS_POWER_METRIC_6_ID;
                        metric_info_curr->metric_name_len        = snprintf(metric_info_curr->metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_POWER_METRIC_6_NAME);
                        metric_info_curr->metric_description_len = snprintf(metric_info_curr->metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_POWER_METRIC_6_DESCRIPTION);
                        metric_info_curr->metric_unit_len        = snprintf(metric_info_curr->metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_POWER_METRIC_6_UNIT);
                        capability_metrics_index++;
                        break;
                    case POWER_TELEMETRY_RAIL_ROP:
                        metric_info_curr->metric_id              = WOS_POWER_METRIC_7_ID;
                        metric_info_curr->metric_name_len        = snprintf(metric_info_curr->metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_POWER_METRIC_7_NAME);
                        metric_info_curr->metric_description_len = snprintf(metric_info_curr->metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_POWER_METRIC_7_DESCRIPTION);
                        metric_info_curr->metric_unit_len        = snprintf(metric_info_curr->metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_POWER_METRIC_7_UNIT);
                        capability_metrics_index++;
                        break;
                    case POWER_TELEMETRY_RAIL_NSP:
                        metric_info_curr->metric_id              = WOS_POWER_METRIC_8_ID;
                        metric_info_curr->metric_name_len        = snprintf(metric_info_curr->metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_POWER_METRIC_8_NAME);
                        metric_info_curr->metric_description_len = snprintf(metric_info_curr->metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_POWER_METRIC_8_DESCRIPTION);
                        metric_info_curr->metric_unit_len        = snprintf(metric_info_curr->metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_POWER_METRIC_8_UNIT);
                        capability_metrics_index++;
                        break;
                    case POWER_TELEMETRY_RAIL_MULTIMEDIA:
                        metric_info_curr->metric_id              = WOS_POWER_METRIC_9_ID;
                        metric_info_curr->metric_name_len        = snprintf(metric_info_curr->metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_POWER_METRIC_9_NAME);
                        metric_info_curr->metric_description_len = snprintf(metric_info_curr->metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_POWER_METRIC_9_DESCRIPTION);
                        metric_info_curr->metric_unit_len        = snprintf(metric_info_curr->metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_POWER_METRIC_9_UNIT);
                        capability_metrics_index++;
                        break;
                    case POWER_TELEMETRY_RAIL_INFRA:
                        metric_info_curr->metric_id              = WOS_POWER_METRIC_10_ID;
                        metric_info_curr->metric_name_len        = snprintf(metric_info_curr->metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_POWER_METRIC_10_NAME);
                        metric_info_curr->metric_description_len = snprintf(metric_info_curr->metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_POWER_METRIC_10_DESCRIPTION);
                        metric_info_curr->metric_unit_len        = snprintf(metric_info_curr->metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_POWER_METRIC_10_UNIT);
                        capability_metrics_index++;
                        break;
                    case POWER_TELEMETRY_RAIL_DRAM_DDR:
                        metric_info_curr->metric_id              = WOS_POWER_METRIC_11_ID;
                        metric_info_curr->metric_name_len        = snprintf(metric_info_curr->metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_POWER_METRIC_11_NAME);
                        metric_info_curr->metric_description_len = snprintf(metric_info_curr->metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_POWER_METRIC_11_DESCRIPTION);
                        metric_info_curr->metric_unit_len        = snprintf(metric_info_curr->metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_POWER_METRIC_11_UNIT);
                        capability_metrics_index++;
                        break;
                    case POWER_TELEMETRY_RAIL_SOC:
                        metric_info_curr->metric_id              = WOS_POWER_METRIC_12_ID;
                        metric_info_curr->metric_name_len        = snprintf(metric_info_curr->metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_POWER_METRIC_12_NAME);
                        metric_info_curr->metric_description_len = snprintf(metric_info_curr->metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_POWER_METRIC_12_DESCRIPTION);
                        metric_info_curr->metric_unit_len        = snprintf(metric_info_curr->metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_POWER_METRIC_12_UNIT);
                        capability_metrics_index++;
                        break;
                    default:
                        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Unknown metric received: %u", rail_index);
                        break;
                    }
                }
                rail_index++;
            }
            g_backend_info.capabilities_list[WOS_POWER_CAPABILITY_1_INDEX].metric_ids_list_len = capability_metrics_index;
        }
    }

    return return_code_init_capabilities;
}

/**
 * @brief Get backend information.
 *
 * @param[out] backend_info Pointer to be filled with backend information.
 * @return QC_PERF_RETURN_CODE_SUCCESS on success, an error code otherwise.
 */
static enum QcPerfReturnCode wos_power_backend_info(struct QcPerfBackendInfo* backend_info) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_SUCCESS;
    if (NULL == backend_info) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "backend_info is NULL");
        return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        *backend_info = g_backend_info;
    }
    return return_code;
}

static enum QcPerfReturnCode wos_power_backend_set_message_callback(QcPerfMessageCallback message_callback) {
    wos_power_backend_logger_set_message_callback(message_callback);
    return QC_PERF_RETURN_CODE_SUCCESS;
}

static enum QcPerfReturnCode wos_power_backend_set_data_callback(QcPerfDataCallback data_callback) {
    g_data_callback = data_callback;
    return QC_PERF_RETURN_CODE_SUCCESS;
}

/**
 * @brief Start the backend profiling thread.
 *
 * @param[in] request The performance request.
 * @return QC_PERF_RETURN_CODE_SUCCESS on success, an error code otherwise.
 */
static enum QcPerfReturnCode wos_power_backend_start(struct QcPerfRequest* request) {
    enum QcPerfReturnCode return_code                = QC_PERF_RETURN_CODE_SUCCESS;
    enum ePowerTelemetryReturnCode power_return_code = RETURN_CODE_POWER_TELEMETRY_SUCCESS;
    struct QThreadAttributes thread_attrs            = {0};
    enum QThreadReturnCode thread_return_code        = RET_QTHREAD_CREATE_SUCCESS;
    struct RailsInfoRequest* rails_request           = NULL;
    uint8_t original_rails_length                    = 0;
    struct QcPerfCapabilityInfo* capability_info     = NULL;

    if (NULL == request) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "QcPerfRequest NULL pointer");
        return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else if (NULL == g_data_callback) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Result Callback is not set");
        return_code = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
    } else if (g_is_thread_running) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Already running capability=%u", request->capability_id);
        return_code = QC_PERF_RETURN_CODE_ALREADY_INITIALIZED;
    } else {
        g_is_thread_running        = true;
        g_requested_metrics_length = 0;
        rails_request              = (struct RailsInfoRequest*)calloc(1, sizeof(struct RailsInfoRequest));
        if (NULL == rails_request) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Memory allocation failed");
            return_code = QC_PERF_RETURN_CODE_CALLOC_FAILED;
        } else {
            capability_info = &g_backend_info.capabilities_list[WOS_POWER_CAPABILITY_1_INDEX];
            for (uint8_t metric_id_index = 0; metric_id_index < capability_info->metric_ids_list_len; metric_id_index++) {
                switch (capability_info->metric_ids_list[metric_id_index].metric_id) {
                case WOS_POWER_METRIC_0_ID:
                    rails_request->rails[metric_id_index] = POWER_TELEMETRY_RAIL_CPU_CLUSTER_0;
                    rails_request->railsLength++;
                    break;
                case WOS_POWER_METRIC_1_ID:
                    rails_request->rails[metric_id_index] = POWER_TELEMETRY_RAIL_CPU_CLUSTER_1;
                    rails_request->railsLength++;
                    break;
                case WOS_POWER_METRIC_2_ID:
                    rails_request->rails[metric_id_index] = POWER_TELEMETRY_RAIL_CPU_CLUSTER_2;
                    rails_request->railsLength++;
                    break;
                case WOS_POWER_METRIC_3_ID:
                    rails_request->rails[metric_id_index] = POWER_TELEMETRY_RAIL_GPU;
                    rails_request->railsLength++;
                    break;
                case WOS_POWER_METRIC_4_ID:
                    rails_request->rails[metric_id_index] = POWER_TELEMETRY_RAIL_PSU_USB;
                    rails_request->railsLength++;
                    break;
                case WOS_POWER_METRIC_5_ID:
                    rails_request->rails[metric_id_index] = g_system_rail_index;
                    rails_request->railsLength++;
                    break;
                case WOS_POWER_METRIC_6_ID:
                    rails_request->rails[metric_id_index] = POWER_TELEMETRY_RAIL_USBC_TOTAL;
                    rails_request->railsLength++;
                    break;
                case WOS_POWER_METRIC_7_ID:
                    rails_request->rails[metric_id_index] = POWER_TELEMETRY_RAIL_ROP;
                    rails_request->railsLength++;
                    break;
                case WOS_POWER_METRIC_8_ID:
                    rails_request->rails[metric_id_index] = POWER_TELEMETRY_RAIL_NSP;
                    rails_request->railsLength++;
                    break;
                case WOS_POWER_METRIC_9_ID:
                    rails_request->rails[metric_id_index] = POWER_TELEMETRY_RAIL_MULTIMEDIA;
                    rails_request->railsLength++;
                    break;
                case WOS_POWER_METRIC_10_ID:
                    rails_request->rails[metric_id_index] = POWER_TELEMETRY_RAIL_INFRA;
                    rails_request->railsLength++;
                    break;
                case WOS_POWER_METRIC_11_ID:
                    rails_request->rails[metric_id_index] = POWER_TELEMETRY_RAIL_DRAM_DDR;
                    rails_request->railsLength++;
                    break;
                case WOS_POWER_METRIC_12_ID:
                    rails_request->rails[metric_id_index] = POWER_TELEMETRY_RAIL_SOC;
                    rails_request->railsLength++;
                    break;
                default:
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Unknown metric received: %u", capability_info->metric_ids_list[metric_id_index].metric_id);
                    break;
                }
            }

            if (0 != rails_request->railsLength) {
                original_rails_length = rails_request->railsLength;
                power_return_code     = powerTelemetry_railsInit(rails_request);
                if (RETURN_CODE_POWER_TELEMETRY_SUCCESS != power_return_code) {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "powerTelemetry_init failed with ePowerTelemetryReturnCode=%u", power_return_code);
                    return_code = QC_PERF_RETURN_CODE_FAILED;
                } else {
                    g_requested_metrics_length += rails_request->railsLength;
                    g_sources_enablement_mapping[POWER_TELEMETRY_SOURCE_RAILS] = true;
                    if (original_rails_length > rails_request->railsLength) {
                        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_WARNING, "Internal library failed to add all metrics %s are missing", original_rails_length - rails_request->railsLength);
                    }
                    return_code = QC_PERF_RETURN_CODE_SUCCESS;
                }
            }

            if (QC_PERF_RETURN_CODE_SUCCESS == return_code) {
                memset(&thread_attrs, 0, sizeof(struct QThreadAttributes));
                thread_attrs.thread_name_len = snprintf((char*)thread_attrs.thread_name, (size_t)strlen((char*)g_thread_name), (char*)g_thread_name);
                thread_attrs.stack_size      = 0;
                thread_attrs.thread_params   = request;
                thread_attrs.thread_fn       = get_wos_power_telemetry_metrics_data;
                thread_return_code           = thread_create(&thread_attrs, &g_thread_info);
                if (RET_QTHREAD_CREATE_SUCCESS != thread_return_code) {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to start thread for capability=%u", request->capability_id);
                    return_code = QC_PERF_RETURN_CODE_FAILED;
                } else {
                    return_code = QC_PERF_RETURN_CODE_SUCCESS;
                }
            }
        }
        if (QC_PERF_RETURN_CODE_SUCCESS != return_code && true == g_is_thread_running) {
            g_is_thread_running = false;
        }
    }

    if (rails_request != NULL) {
        free(rails_request);
        rails_request = NULL;
    }

    return return_code;
}

static enum QcPerfReturnCode wos_power_backend_stop(struct QcPerfRequest* request) {
    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_DEBUG, "Stopping profiling");
    enum QcPerfReturnCode return_code                = QC_PERF_RETURN_CODE_SUCCESS;
    enum ePowerTelemetryReturnCode power_return_code = RETURN_CODE_POWER_TELEMETRY_SUCCESS;
    enum QThreadReturnCode thread_return_code        = RET_QTHREAD_CREATE_SUCCESS;
    if (NULL == request) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "QcPerfRequest NULL pointer");
        return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        if (false == g_is_thread_running) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Already stopped capability=%u", request->capability_id);
            return_code = QC_PERF_RETURN_CODE_SUCCESS;
        } else {
            g_is_thread_running = false;
            thread_return_code  = thread_join(&g_thread_info);
            if (RET_QTHREAD_JOIN_SUCCESS != thread_return_code) {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to stop capability=%u", request->capability_id);
                return_code = QC_PERF_RETURN_CODE_FAILED;
            } else {
                thread_return_code = thread_destroy(&g_thread_info);
                if (thread_return_code != RET_QTHREAD_DESTROY_SUCCESS) {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to destroy thread handle");
                    return_code = QC_PERF_RETURN_CODE_FAILED;
                } else {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Performance monitoring thread stopped successfully");
                    return_code = QC_PERF_RETURN_CODE_SUCCESS;
                }
            }
            power_return_code = powerTelemetry_railsDestroy();
            if (RETURN_CODE_POWER_TELEMETRY_SUCCESS != power_return_code) {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "powerTelemetry_railsDestroy failed with ePowerTelemetryReturnCode=%u", power_return_code);
            }

            g_requested_metrics_length = 0;
            memset(g_sources_enablement_mapping, 0, MAX_POWER_TELEMETRY_HANDLER_SOURCES * sizeof(bool));
        }
    }
    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_DEBUG, "wos_power_backend profiling stopped.");
    return return_code;
}

static enum QcPerfReturnCode wos_power_backend_deinit(void) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_SUCCESS;
    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Deinitializing dummy backend");
    wos_power_backend_free_memory();
    return return_code;
}

uint32_t get_wos_power_telemetry_metrics_data(void* lpParam) {
    struct QcPerfRequest* request                    = (struct QcPerfRequest*)lpParam;
    struct QcPerfCapabilityInfo* capability_info     = NULL;
    uint32_t return_code                             = QC_PERF_RETURN_CODE_SUCCESS;
    uint32_t sample_count                            = 1;
    uint64_t elapsed_ms                              = 0;
    uint64_t start_time                              = 0;
    uint64_t end_time                                = 0;
    uint32_t metric_response_index                   = 0;
    uint32_t metric_response_allocated               = 0;
    enum ePowerTelemetryReturnCode power_return_code = RETURN_CODE_POWER_TELEMETRY_SUCCESS;
    struct RailsInfoResponse* rails_response         = NULL;
    uint16_t current_sample                          = 0;
    time_t current_time                              = 0;
    struct QcPerfData* data                          = NULL;

    if (NULL == request) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "request NULL pointer");
        return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        rails_response = (struct RailsInfoResponse*)calloc(1, sizeof(struct RailsInfoResponse));
        data           = (struct QcPerfData*)calloc(1, sizeof(struct QcPerfData));
        if (NULL == rails_response || NULL == data) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Memory allocation failed for a fixed-size structure");
            return_code = QC_PERF_RETURN_CODE_CALLOC_FAILED;
        } else {
            capability_info = &g_backend_info.capabilities_list[WOS_POWER_CAPABILITY_1_ID];
            if (NULL == capability_info) {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "capability_info is null");
                return_code = QC_PERF_RETURN_CODE_FAILED;
            } else {
                data->capabilityId = WOS_POWER_CAPABILITY_1_ID;
                // Calculate number of samples using streaming rate
                if (request->sampling_rate == 0 || request->sampling_rate > request->streaming_rate) {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_WARNING,
                                 "Sampling rate (%u ms) exceeds streaming rate (%u ms). Sampling rate adjusted to match streaming rate and Newer sampling rate is ( %u ms)", request->sampling_rate,
                                 request->streaming_rate, request->streaming_rate);
                    request->sampling_rate = request->streaming_rate;
                } else {
                    sample_count = request->streaming_rate / request->sampling_rate;
                }
                metric_response_allocated = sample_count * capability_info->metric_ids_list_len;
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Expecting sample_count=%u metrics=%u total=%u", sample_count, capability_info->metric_ids_list_len, metric_response_allocated);
                data->metric_response = (struct QcPerfMetricResponse*)calloc((size_t)(metric_response_allocated), sizeof(struct QcPerfMetricResponse));
                if (NULL == data->metric_response) {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Memory allocation for QcPerfMetricResponse failed");
                    return_code = QC_PERF_RETURN_CODE_CALLOC_FAILED;
                } else {
                    (void)get_time_ns(&start_time);
                    // Need to ignore the first packets
                    if (true == g_sources_enablement_mapping[POWER_TELEMETRY_SOURCE_RAILS]) {
                        power_return_code = powerTelemetry_railsGetInfo(rails_response);
                        // Still need time between first and second call for valid data
#ifdef __linux__
                        usleep(request->sampling_rate * 1000);
#elif _WIN32
                        Sleep(request->sampling_rate);
#endif
                    }

                    metric_response_index = 0;
                    while (g_is_thread_running) {
                        (void)get_time_ns(&current_time);

                        if (true == g_sources_enablement_mapping[POWER_TELEMETRY_SOURCE_RAILS]) {
                            power_return_code = powerTelemetry_railsGetInfo(rails_response);
                            if (RETURN_CODE_POWER_TELEMETRY_SUCCESS != power_return_code) {
                                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "powerTelemetry_railsGetInfo failed with ePowerTelemetryReturnCode=%u", power_return_code);
                            } else {
                                for (uint8_t i = 0; i < rails_response->railsAndValuesLength && metric_response_index < metric_response_allocated; i++) {
                                    switch (rails_response->rails[i]) {
                                    case POWER_TELEMETRY_RAIL_CPU_CLUSTER_0:
                                        data->metric_response[metric_response_index].metric_id = WOS_POWER_METRIC_0_ID;
                                        break;
                                    case POWER_TELEMETRY_RAIL_CPU_CLUSTER_1:
                                        data->metric_response[metric_response_index].metric_id = WOS_POWER_METRIC_1_ID;
                                        break;
                                    case POWER_TELEMETRY_RAIL_CPU_CLUSTER_2:
                                        data->metric_response[metric_response_index].metric_id = WOS_POWER_METRIC_2_ID;
                                        break;
                                    case POWER_TELEMETRY_RAIL_GPU:
                                        data->metric_response[metric_response_index].metric_id = WOS_POWER_METRIC_3_ID;
                                        break;
                                    case POWER_TELEMETRY_RAIL_PSU_USB:
                                        data->metric_response[metric_response_index].metric_id = WOS_POWER_METRIC_4_ID;
                                        break;
                                    case POWER_TELEMETRY_RAIL_SYS:
                                        data->metric_response[metric_response_index].metric_id = WOS_POWER_METRIC_5_ID;
                                        break;
                                    case POWER_TELEMETRY_RAIL_SYSTEM:
                                        data->metric_response[metric_response_index].metric_id = WOS_POWER_METRIC_5_ID;
                                        break;
                                    case POWER_TELEMETRY_RAIL_USBC_TOTAL:
                                        data->metric_response[metric_response_index].metric_id = WOS_POWER_METRIC_6_ID;
                                        break;
                                    case POWER_TELEMETRY_RAIL_ROP:
                                        data->metric_response[metric_response_index].metric_id = WOS_POWER_METRIC_7_ID;
                                        break;
                                    case POWER_TELEMETRY_RAIL_NSP:
                                        data->metric_response[metric_response_index].metric_id = WOS_POWER_METRIC_8_ID;
                                        break;
                                    case POWER_TELEMETRY_RAIL_MULTIMEDIA:
                                        data->metric_response[metric_response_index].metric_id = WOS_POWER_METRIC_9_ID;
                                        break;
                                    case POWER_TELEMETRY_RAIL_INFRA:
                                        data->metric_response[metric_response_index].metric_id = WOS_POWER_METRIC_10_ID;
                                        break;
                                    case POWER_TELEMETRY_RAIL_DRAM_DDR:
                                        data->metric_response[metric_response_index].metric_id = WOS_POWER_METRIC_11_ID;
                                        break;
                                    case POWER_TELEMETRY_RAIL_SOC:
                                        data->metric_response[metric_response_index].metric_id = WOS_POWER_METRIC_12_ID;
                                        break;
                                    default:
                                        log_message(QC_PERF_MESSAGE_LEVEL_INFO, "Unknown metric received: %u", data->metric_response[metric_response_index].metric_id);
                                        break;
                                    }
                                    data->metric_response[metric_response_index].timestamp                 = current_time;
                                    data->metric_response[metric_response_index].metric_value.data_type    = QC_PERF_DATA_TYPE_DOUBLE;
                                    data->metric_response[metric_response_index].metric_value.double_value = rails_response->values[i];
                                    metric_response_index                                                  = metric_response_index + 1;
                                }
                            }
                        }

                        current_sample++;
                        (void)get_time_ns(&end_time);
                        elapsed_ms = (uint64_t)((end_time - start_time) / NS_TO_MS);
                        if (current_sample >= sample_count || metric_response_index >= metric_response_allocated || request->streaming_rate < elapsed_ms) {
                            if (0 == metric_response_index) {
                                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "No profiling data is present for capability");
                                return_code = QC_PERF_RETURN_CODE_FAILED;
                            } else {
                                data->metric_response_len = metric_response_index;

                                if (NULL != g_data_callback) {
                                    g_data_callback(data);
                                } else {
                                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Result callback is NULL, cannot send metric data");
                                }

                                metric_response_index = 0;
                                current_sample        = 0;
                            }
                            (void)get_time_ns(&start_time);
                        }

#ifdef __linux__
                        usleep(request->sampling_rate * 1000);
#elif _WIN32
                        Sleep(request->sampling_rate);
#endif
                    }
                }
            }
        }
    }

    if (NULL != rails_response) {
        free(rails_response);
        rails_response = NULL;
    }

    if (data != NULL && data->metric_response != NULL) {
        free(data->metric_response);
        data->metric_response = NULL;
    }

    if (data != NULL) {
        free(data);
        data = NULL;
    }

    return return_code;
}

enum QcPerfReturnCode wos_power_backend_create(struct QcPerfBackendPrivate* backend) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == backend) {
        return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        backend->qcperf_backend_init   = wos_power_backend_init;
        backend->qcperf_backend_start  = wos_power_backend_start;
        backend->qcperf_backend_stop   = wos_power_backend_stop;
        backend->qcperf_backend_deinit = wos_power_backend_deinit;
        backend->qcperf_backend_info   = wos_power_backend_info;
        backend->set_message_callback  = wos_power_backend_set_message_callback;
        backend->set_data_callback     = wos_power_backend_set_data_callback;
    }

    return return_code;
}