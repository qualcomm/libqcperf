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
 * @file wos_thermal_lib.c
 * @brief Implementation of the WOS Thermal library
 * @author Skand Gupta (skangupt@qti.qualcomm.com)
 *
 * This file implements the public API functions defined in wos_thermal_lib.h,
 * providing functionality for monitoring thermal zones and passive cooling on
 * Windows platforms. It uses the CPU-thermal-etw library to collect temperature
 * and passive cooling data from the system.
 *
 * The implementation includes functions for initializing the library, starting
 * and stopping data collection, retrieving thermal zone information, and cleaning
 * up resources. It uses a separate thread for continuous data collection at
 * specified sampling and streaming rates.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wos_thermal_lib.h"
#include "wos_thermal_logger.h"
#include "cpu_thermal_etw_library.h"
#include "passive_cooling.h"
#include "thermal_common.h"
#include "qthread.h"
#include "qtime.h"

// Static variables
static volatile bool g_is_initialized                         = false;
static struct ThermalCommonZoneNameMap g_zone_name_map        = {0};
static struct ThermalInfoQuery g_temperature_query            = {0};
static struct PassiveCoolingInfoQuery g_passive_cooling_query = {0};

enum WosThermalLibReturnCode wos_thermal_lib_init(void) {
    enum WosThermalLibReturnCode return_code  = WOS_THERMAL_LIB_SUCCESS;
    enum CpuThermalETWReturnCode thermal_ret  = RETURN_CODE_CPU_THERMAL_SUCCESS;
    enum PassiveCoolingReturnCode cooling_ret = RETURN_CODE_PASSIVE_COOLING_SUCCESS;
    enum ThermalCommonReturnCode common_ret   = RETURN_CODE_THERMAL_COMMON_SUCCESS;

    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_DEBUG, "Initializing WOS Thermal library");

    if (true == g_is_initialized) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_WARNING, "WOS Thermal library is already initialized");
        return_code = WOS_THERMAL_LIB_ERROR_ALREADY_INITIALIZED;
    } else {
        // Initialize thermal common
        common_ret = thermal_common_init();
        if (RETURN_CODE_THERMAL_COMMON_SUCCESS != common_ret) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to initialize thermal common, error: %d", common_ret);
            return_code = WOS_THERMAL_LIB_ERROR_THERMAL_COMMON_INIT_FAILED;
        } else {
            // Get zone names
            common_ret = thermal_common_getzone_names(&g_zone_name_map);
            if (RETURN_CODE_THERMAL_COMMON_SUCCESS != common_ret) {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to get thermal zone names, error: %d", common_ret);
                thermal_common_cleanup();
                return_code = WOS_THERMAL_LIB_ERROR_GET_ZONE_NAMES_FAILED;
            } else {
                // Debug: Print all available thermal zones
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_DEBUG, "Found %d thermal zones:", g_zone_name_map.zone_names_ids_length);
                // Initialize thermal sensor
                thermal_ret = thermal_sensor_init(&g_zone_name_map);
                if (RETURN_CODE_CPU_THERMAL_SUCCESS != thermal_ret) {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to initialize thermal sensor, error: %d", thermal_ret);
                    thermal_common_cleanup();
                    return_code = WOS_THERMAL_LIB_ERROR_THERMAL_SENSOR_INIT_FAILED;
                } else {
                    // Initialize passive cooling
                    cooling_ret = passive_cooling_init(&g_zone_name_map);
                    if (RETURN_CODE_PASSIVE_COOLING_SUCCESS != cooling_ret) {
                        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to initialize passive cooling, error: %d", cooling_ret);
                        thermal_sensor_cleanup();
                        thermal_common_cleanup();
                        return_code = WOS_THERMAL_LIB_ERROR_PASSIVE_COOLING_INIT_FAILED;
                    } else {
                        // Allocate memory for temperature and passive cooling queries
                        g_temperature_query.temperatures_length = MAX_THERMAL_ZONES;
                        g_temperature_query.temperatures        = (double*)calloc(g_temperature_query.temperatures_length, sizeof(double));
                        if (NULL == g_temperature_query.temperatures) {
                            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to allocate memory for temperature query");
                            passive_cooling_cleanup();
                            thermal_sensor_cleanup();
                            thermal_common_cleanup();
                            return_code = WOS_THERMAL_LIB_ERROR_MEMORY_ALLOCATION;
                        } else {
                            g_passive_cooling_query.passive_cooling_length = MAX_THERMAL_ZONES;
                            g_passive_cooling_query.passive_cooling        = (double*)calloc(g_passive_cooling_query.passive_cooling_length, sizeof(double));
                            if (NULL == g_passive_cooling_query.passive_cooling) {
                                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to allocate memory for passive cooling query");
                                free(g_temperature_query.temperatures);
                                g_temperature_query.temperatures = NULL;
                                passive_cooling_cleanup();
                                thermal_sensor_cleanup();
                                thermal_common_cleanup();
                                return_code = WOS_THERMAL_LIB_ERROR_MEMORY_ALLOCATION;
                            } else {
                                g_is_initialized = true;
                                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_DEBUG, "WOS Thermal library initialized successfully");
                            }
                        }
                    }
                }
            }
        }
    }

    return return_code;
}

enum WosThermalLibReturnCode wos_thermal_lib_cleanup(void) {
    enum WosThermalLibReturnCode return_code  = WOS_THERMAL_LIB_SUCCESS;
    enum CpuThermalETWReturnCode thermal_ret  = RETURN_CODE_CPU_THERMAL_SUCCESS;
    enum PassiveCoolingReturnCode cooling_ret = RETURN_CODE_PASSIVE_COOLING_SUCCESS;
    enum ThermalCommonReturnCode common_ret   = RETURN_CODE_THERMAL_COMMON_SUCCESS;

    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_DEBUG, "Cleaning up WOS Thermal library");

    if (false == g_is_initialized) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_WARNING, "WOS Thermal library is not initialized");
        return_code = WOS_THERMAL_LIB_ERROR_NOT_INITIALIZED;
    } else {
        // Free allocated memory
        if (NULL != g_temperature_query.temperatures) {
            free(g_temperature_query.temperatures);
            g_temperature_query.temperatures = NULL;
        }

        if (NULL != g_passive_cooling_query.passive_cooling) {
            free(g_passive_cooling_query.passive_cooling);
            g_passive_cooling_query.passive_cooling = NULL;
        }

        // Clean up passive cooling
        cooling_ret = passive_cooling_cleanup();
        if (RETURN_CODE_PASSIVE_COOLING_SUCCESS != cooling_ret) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to clean up passive cooling, error: %d", cooling_ret);
            return_code = WOS_THERMAL_LIB_ERROR_PASSIVE_COOLING_CLEANUP_FAILED;
            // Continue with cleanup anyway
        }

        // Clean up thermal sensor
        thermal_ret = thermal_sensor_cleanup();
        if (RETURN_CODE_CPU_THERMAL_SUCCESS != thermal_ret) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to clean up thermal sensor, error: %d", thermal_ret);
            return_code = WOS_THERMAL_LIB_ERROR_THERMAL_SENSOR_CLEANUP_FAILED;
            // Continue with cleanup anyway
        }

        // Clean up thermal common
        common_ret = thermal_common_cleanup();
        if (RETURN_CODE_THERMAL_COMMON_SUCCESS != common_ret) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to clean up thermal common, error: %d", common_ret);
            return_code = WOS_THERMAL_LIB_ERROR_THERMAL_COMMON_CLEANUP_FAILED;
            // Continue with cleanup anyway
        }

        g_is_initialized = false;
    }
    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_DEBUG, "WOS Thermal library cleaned up successfully");

    return return_code;
}

enum WosThermalLibReturnCode wos_thermal_lib_get_zone_count(uint8_t* zone_count) {
    enum WosThermalLibReturnCode return_code = WOS_THERMAL_LIB_ERROR_FAILED;

    if (false == g_is_initialized) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "WOS Thermal library is not initialized");
        return_code = WOS_THERMAL_LIB_ERROR_NOT_INITIALIZED;
    } else if (NULL == zone_count) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Zone count pointer is NULL");
        return_code = WOS_THERMAL_LIB_ERROR_NULL_POINTER;
    } else {
        *zone_count = g_zone_name_map.zone_names_ids_length;
        return_code = WOS_THERMAL_LIB_SUCCESS;
    }
    return return_code;
}

/**
 * @brief Get thermal zone information
 *
 * This function returns information about all thermal zones, including
 * temperature and passive cooling data. It populates the provided thermal_data
 * structure with the current thermal zone information, including zone IDs,
 * names, temperatures, and passive cooling percentages.
 *
 * The function uses the global zone name map that was populated during
 * initialization to identify thermal zones and retrieve their current state.
 *
 * @param[out] thermal_data Pointer to store thermal zone data
 *
 * @return WOS_THERMAL_LIB_SUCCESS on success
 * @return WOS_THERMAL_LIB_ERROR_NOT_INITIALIZED if library is not initialized
 * @return WOS_THERMAL_LIB_ERROR_NULL_POINTER if thermal_data is NULL
 * @return WOS_THERMAL_LIB_ERROR_GET_THERMAL_INFO_FAILED if getting thermal info fails
 * @return WOS_THERMAL_LIB_ERROR_GET_PASSIVE_COOLING_INFO_FAILED if getting passive cooling info fails
 */
enum WosThermalLibReturnCode wos_thermal_lib_get_zone_info(struct WosThermalData* thermal_data) {
    enum WosThermalLibReturnCode return_code  = WOS_THERMAL_LIB_ERROR_FAILED;
    enum CpuThermalETWReturnCode thermal_ret  = RETURN_CODE_CPU_THERMAL_SUCCESS;
    enum PassiveCoolingReturnCode cooling_ret = RETURN_CODE_PASSIVE_COOLING_SUCCESS;

    uint64_t current_time_ns = 0;
    if (false == g_is_initialized) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "WOS Thermal library is not initialized");
        return_code = WOS_THERMAL_LIB_ERROR_NOT_INITIALIZED;
    } else if (NULL == thermal_data) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Data pointer is NULL");
        return_code = WOS_THERMAL_LIB_ERROR_NULL_POINTER;
    } else {
        return_code             = get_time_ns(&current_time_ns);
        thermal_data->timestamp = current_time_ns;
        // Get thermal data
        thermal_ret = thermal_sensor_get_info(&g_temperature_query);
        if (RETURN_CODE_CPU_THERMAL_SUCCESS != thermal_ret) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to get thermal sensor info, error: %d", thermal_ret);
            return_code = WOS_THERMAL_LIB_ERROR_GET_THERMAL_INFO_FAILED;
        } else {
            // Get passive cooling data
            cooling_ret = passive_cooling_get_info(&g_passive_cooling_query);
            if (RETURN_CODE_PASSIVE_COOLING_SUCCESS != cooling_ret) {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to get passive cooling info, error: %d", cooling_ret);
                return_code = WOS_THERMAL_LIB_ERROR_GET_PASSIVE_COOLING_INFO_FAILED;
            } else {
                // Update thermal data structure
                thermal_data->zone_count = g_zone_name_map.zone_names_ids_length;

                for (uint8_t i = 0; i < g_zone_name_map.zone_names_ids_length && i < MAX_THERMAL_ZONES; i++) {
                    thermal_data->zones[i].zone_id          = g_zone_name_map.zone_ids[i];
                    thermal_data->zones[i].zone_name_length = strncpy(thermal_data->zones[i].zone_name, (const char*)g_zone_name_map.zone_names[i].string, 64);
                    thermal_data->zones[i].temperature      = g_temperature_query.temperatures[i];
                    thermal_data->zones[i].passive_cooling  = g_passive_cooling_query.passive_cooling[i];
                }
                return_code = WOS_THERMAL_LIB_SUCCESS;
            }
        }
    }

    return return_code;
}
