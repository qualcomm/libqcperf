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
 * @file wos_thermal_info.h
 * @brief Static metric definitions and initialization functions for WOS Thermal backend
 * @author Skand Gupta (skangupt@qti.qualcomm.com)
 *
 * This header defines all metric IDs, names, descriptions, and units for the
 * WOS Thermal backend's capabilities. It includes definitions for temperature
 * and passive cooling metrics for various thermal zones (excluding QCLimitsPolicy
 * zones), and provides initialization functions to populate metric information
 * structures with these static definitions.
 */

#ifndef WOS_THERMAL_INFO_H
#define WOS_THERMAL_INFO_H

#include "qcperf_common.h"

#define WOS_THERMAL_CAPABILITY_ID 0
#define WOS_THERMAL_CAPABILITIES_LEN 1

#define WOS_THERMAL_STREAMING_RATES_LEN 2
#define WOS_THERMAL_STREAMING_RATES 200, 1000

#define WOS_THERMAL_SAMPLING_RATES_LEN 2
#define WOS_THERMAL_SAMPLING_RATES 50, 1000

#define WOS_THERMAL_CAPABILITY "thermal"
#define WOS_THERMAL_CAPABILITY_METRIC_COUNT 24  // 12 thermal zones * 2 metrics (temperature and passive cooling)

/**
 * @enum WosThermalMetricIndex
 * @brief Array indices for WOS Thermal metrics
 * @author Skand Gupta (skangupt@qti.qualcomm.com)
 * This enum defines the array indices for accessing metrics in the
 * metrics_data array, providing better code readability and maintainability.
 * For each thermal zone, index 2n is for temperature and index 2n+1 is for
 * passive cooling.
 */
enum WosThermalMetricIndex {
    // CPU Cluster 0 Thermal Zone
    WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_0_TEMP = 0,
    WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_0_COOLING,

    // CPU Cluster 1 Thermal Zone
    WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_1_TEMP,
    WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_1_COOLING,

    // CPU Cluster 2 Thermal Zone
    WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_2_TEMP,
    WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_2_COOLING,

    // QMX0 Thermal Zone
    WOS_THERMAL_METRIC_INDEX_QMX0_TEMP,
    WOS_THERMAL_METRIC_INDEX_QMX0_COOLING,

    // QMX1 Thermal Zone
    WOS_THERMAL_METRIC_INDEX_QMX1_TEMP,
    WOS_THERMAL_METRIC_INDEX_QMX1_COOLING,

    // QMX2 Thermal Zone
    WOS_THERMAL_METRIC_INDEX_QMX2_TEMP,
    WOS_THERMAL_METRIC_INDEX_QMX2_COOLING,

    // GPU Thermal Zone
    WOS_THERMAL_METRIC_INDEX_GPU_TEMP,
    WOS_THERMAL_METRIC_INDEX_GPU_COOLING,

    // NSP Thermal Zone
    WOS_THERMAL_METRIC_INDEX_NSP_TEMP,
    WOS_THERMAL_METRIC_INDEX_NSP_COOLING,

    // Critical Thermal Zones for All Internal TSENS
    WOS_THERMAL_METRIC_INDEX_CRITICAL_THERMAL_ZONES_TEMP,
    WOS_THERMAL_METRIC_INDEX_CRITICAL_THERMAL_ZONES_COOLING,

    // EC thermistor 1
    WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_1_TEMP,
    WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_1_COOLING,

    // EC thermistor 2
    WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_2_TEMP,
    WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_2_COOLING,

    // EC thermistor 3
    WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_3_TEMP,
    WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_3_COOLING
};

// CPU Cluster 0 Thermal Zone
#define WOS_THERMAL_METRIC_CPU_CLUSTER_0_ZONE_NAME "CPU Cluster 0 Thermal Zone"
#define WOS_THERMAL_METRIC_CPU_CLUSTER_0_TEMP_ID 0
#define WOS_THERMAL_METRIC_CPU_CLUSTER_0_TEMP_NAME "Zone id : 0 CPU Cluster 0 Thermal Zone Temperature"
#define WOS_THERMAL_METRIC_CPU_CLUSTER_0_TEMP_DESCRIPTION "Temperature of CPU Cluster 0 Thermal Zone"
#define WOS_THERMAL_METRIC_CPU_CLUSTER_0_TEMP_UNIT "deg C"

#define WOS_THERMAL_METRIC_CPU_CLUSTER_0_COOLING_ID 1
#define WOS_THERMAL_METRIC_CPU_CLUSTER_0_COOLING_NAME "Zone id : 0 CPU Cluster 0 Thermal Zone Passive Cooling"
#define WOS_THERMAL_METRIC_CPU_CLUSTER_0_COOLING_DESCRIPTION "Passive Cooling of CPU Cluster 0 Thermal Zone"
#define WOS_THERMAL_METRIC_CPU_CLUSTER_0_COOLING_UNIT "%"

// CPU Cluster 1 Thermal Zone
#define WOS_THERMAL_METRIC_CPU_CLUSTER_1_ZONE_NAME "CPU Cluster 1 Thermal Zone"
#define WOS_THERMAL_METRIC_CPU_CLUSTER_1_TEMP_ID 2
#define WOS_THERMAL_METRIC_CPU_CLUSTER_1_TEMP_NAME "Zone id : 1 CPU Cluster 1 Thermal Zone Temperature"
#define WOS_THERMAL_METRIC_CPU_CLUSTER_1_TEMP_DESCRIPTION "Temperature of CPU Cluster 1 Thermal Zone"
#define WOS_THERMAL_METRIC_CPU_CLUSTER_1_TEMP_UNIT "deg C"

#define WOS_THERMAL_METRIC_CPU_CLUSTER_1_COOLING_ID 3
#define WOS_THERMAL_METRIC_CPU_CLUSTER_1_COOLING_NAME "Zone id : 1 CPU Cluster 1 Thermal Zone Passive Cooling"
#define WOS_THERMAL_METRIC_CPU_CLUSTER_1_COOLING_DESCRIPTION "Passive Cooling of CPU Cluster 1 Thermal Zone"
#define WOS_THERMAL_METRIC_CPU_CLUSTER_1_COOLING_UNIT "%"

// CPU Cluster 2 Thermal Zone
#define WOS_THERMAL_METRIC_CPU_CLUSTER_2_ZONE_NAME "CPU Cluster 2 Thermal Zone"
#define WOS_THERMAL_METRIC_CPU_CLUSTER_2_TEMP_ID 4
#define WOS_THERMAL_METRIC_CPU_CLUSTER_2_TEMP_NAME "Zone id : 2 CPU Cluster 2 Thermal Zone Temperature"
#define WOS_THERMAL_METRIC_CPU_CLUSTER_2_TEMP_DESCRIPTION "Temperature of CPU Cluster 2 Thermal Zone"
#define WOS_THERMAL_METRIC_CPU_CLUSTER_2_TEMP_UNIT "deg C"

#define WOS_THERMAL_METRIC_CPU_CLUSTER_2_COOLING_ID 5
#define WOS_THERMAL_METRIC_CPU_CLUSTER_2_COOLING_NAME "Zone id : 2 CPU Cluster 2 Thermal Zone Passive Cooling"
#define WOS_THERMAL_METRIC_CPU_CLUSTER_2_COOLING_DESCRIPTION "Passive Cooling of CPU Cluster 2 Thermal Zone"
#define WOS_THERMAL_METRIC_CPU_CLUSTER_2_COOLING_UNIT "%"

// QMX0 Thermal Zone
#define WOS_THERMAL_METRIC_QMX0_ZONE_NAME "QMX0 Thermal Zone"
#define WOS_THERMAL_METRIC_QMX0_TEMP_ID 6
#define WOS_THERMAL_METRIC_QMX0_TEMP_NAME "Zone id : 3 QMX0 Thermal Zone Temperature"
#define WOS_THERMAL_METRIC_QMX0_TEMP_DESCRIPTION "Temperature of QMX0 Thermal Zone"
#define WOS_THERMAL_METRIC_QMX0_TEMP_UNIT "deg C"

#define WOS_THERMAL_METRIC_QMX0_COOLING_ID 7
#define WOS_THERMAL_METRIC_QMX0_COOLING_NAME "Zone id : 3 QMX0 Thermal Zone Passive Cooling"
#define WOS_THERMAL_METRIC_QMX0_COOLING_DESCRIPTION "Passive Cooling of QMX0 Thermal Zone"
#define WOS_THERMAL_METRIC_QMX0_COOLING_UNIT "%"

// QMX1 Thermal Zone
#define WOS_THERMAL_METRIC_QMX1_ZONE_NAME "QMX1 Thermal Zone"
#define WOS_THERMAL_METRIC_QMX1_TEMP_ID 8
#define WOS_THERMAL_METRIC_QMX1_TEMP_NAME "Zone id : 4 QMX1 Thermal Zone Temperature"
#define WOS_THERMAL_METRIC_QMX1_TEMP_DESCRIPTION "Temperature of QMX1 Thermal Zone"
#define WOS_THERMAL_METRIC_QMX1_TEMP_UNIT "deg C"

#define WOS_THERMAL_METRIC_QMX1_COOLING_ID 9
#define WOS_THERMAL_METRIC_QMX1_COOLING_NAME "Zone id : 4 QMX1 Thermal Zone Passive Cooling"
#define WOS_THERMAL_METRIC_QMX1_COOLING_DESCRIPTION "Passive Cooling of QMX1 Thermal Zone"
#define WOS_THERMAL_METRIC_QMX1_COOLING_UNIT "%"

// QMX2 Thermal Zone
#define WOS_THERMAL_METRIC_QMX2_ZONE_NAME "QMX2 Thermal Zone"
#define WOS_THERMAL_METRIC_QMX2_TEMP_ID 10
#define WOS_THERMAL_METRIC_QMX2_TEMP_NAME "Zone id : 5 QMX2 Thermal Zone Temperature"
#define WOS_THERMAL_METRIC_QMX2_TEMP_DESCRIPTION "Temperature of QMX2 Thermal Zone"
#define WOS_THERMAL_METRIC_QMX2_TEMP_UNIT "deg C"

#define WOS_THERMAL_METRIC_QMX2_COOLING_ID 11
#define WOS_THERMAL_METRIC_QMX2_COOLING_NAME "Zone id : 5 QMX2 Thermal Zone Passive Cooling"
#define WOS_THERMAL_METRIC_QMX2_COOLING_DESCRIPTION "Passive Cooling of QMX2 Thermal Zone"
#define WOS_THERMAL_METRIC_QMX2_COOLING_UNIT "%"

// GPU Thermal Zone
#define WOS_THERMAL_METRIC_GPU_ZONE_NAME "GPU Thermal Zone"
#define WOS_THERMAL_METRIC_GPU_TEMP_ID 12
#define WOS_THERMAL_METRIC_GPU_TEMP_NAME "Zone id : 6 GPU Thermal Zone Temperature"
#define WOS_THERMAL_METRIC_GPU_TEMP_DESCRIPTION "Temperature of GPU Thermal Zone"
#define WOS_THERMAL_METRIC_GPU_TEMP_UNIT "deg C"

#define WOS_THERMAL_METRIC_GPU_COOLING_ID 13
#define WOS_THERMAL_METRIC_GPU_COOLING_NAME "Zone id : 6 GPU Thermal Zone Passive Cooling"
#define WOS_THERMAL_METRIC_GPU_COOLING_DESCRIPTION "Passive Cooling of GPU Thermal Zone"
#define WOS_THERMAL_METRIC_GPU_COOLING_UNIT "%"

// NSP Thermal Zone
#define WOS_THERMAL_METRIC_NSP_ZONE_NAME "NSP Thermal Zone"
#define WOS_THERMAL_METRIC_NSP_TEMP_ID 14
#define WOS_THERMAL_METRIC_NSP_TEMP_NAME "Zone id : 11 NSP Thermal Zone Temperature"
#define WOS_THERMAL_METRIC_NSP_TEMP_DESCRIPTION "Temperature of NSP Thermal Zone"
#define WOS_THERMAL_METRIC_NSP_TEMP_UNIT "deg C"

#define WOS_THERMAL_METRIC_NSP_COOLING_ID 15
#define WOS_THERMAL_METRIC_NSP_COOLING_NAME "Zone id : 11 NSP Thermal Zone Passive Cooling"
#define WOS_THERMAL_METRIC_NSP_COOLING_DESCRIPTION "Passive Cooling of NSP Thermal Zone"
#define WOS_THERMAL_METRIC_NSP_COOLING_UNIT "%"

// Critical Thermal Zones for All Internal TSENS
#define WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_ZONE_NAME "Critical Thermal Zones"
#define WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_TEMP_ID 16
#define WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_TEMP_NAME "Zone id : 99 Critical Thermal Zones for All Internal TSENS Temperature"
#define WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_TEMP_DESCRIPTION "Temperature of Critical Thermal Zones for All Internal TSENS"
#define WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_TEMP_UNIT "deg C"

#define WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_COOLING_ID 17
#define WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_COOLING_NAME "Zone id : 99 Critical Thermal Zones for All Internal TSENS Passive Cooling"
#define WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_COOLING_DESCRIPTION "Passive Cooling of Critical Thermal Zones for All Internal TSENS"
#define WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_COOLING_UNIT "%"

// EC thermistor 1
#define WOS_THERMAL_METRIC_EC_THERMISTOR_1_ZONE_NAME "EC thermistor 1"
#define WOS_THERMAL_METRIC_EC_THERMISTOR_1_TEMP_ID 18
#define WOS_THERMAL_METRIC_EC_THERMISTOR_1_TEMP_NAME "Zone id : 31 EC thermistor 1 Temperature"
#define WOS_THERMAL_METRIC_EC_THERMISTOR_1_TEMP_DESCRIPTION "Temperature of EC thermistor 1"
#define WOS_THERMAL_METRIC_EC_THERMISTOR_1_TEMP_UNIT "deg C"

#define WOS_THERMAL_METRIC_EC_THERMISTOR_1_COOLING_ID 19
#define WOS_THERMAL_METRIC_EC_THERMISTOR_1_COOLING_NAME "Zone id : 31 EC thermistor 1 Passive Cooling"
#define WOS_THERMAL_METRIC_EC_THERMISTOR_1_COOLING_DESCRIPTION "Passive Cooling of EC thermistor 1"
#define WOS_THERMAL_METRIC_EC_THERMISTOR_1_COOLING_UNIT "%"

// EC thermistor 2
#define WOS_THERMAL_METRIC_EC_THERMISTOR_2_ZONE_NAME "EC thermistor 2"
#define WOS_THERMAL_METRIC_EC_THERMISTOR_2_TEMP_ID 20
#define WOS_THERMAL_METRIC_EC_THERMISTOR_2_TEMP_NAME "Zone id : 32 EC thermistor 2 Temperature"
#define WOS_THERMAL_METRIC_EC_THERMISTOR_2_TEMP_DESCRIPTION "Temperature of EC thermistor 2"
#define WOS_THERMAL_METRIC_EC_THERMISTOR_2_TEMP_UNIT "deg C"

#define WOS_THERMAL_METRIC_EC_THERMISTOR_2_COOLING_ID 21
#define WOS_THERMAL_METRIC_EC_THERMISTOR_2_COOLING_NAME "Zone id : 32 EC thermistor 2 Passive Cooling"
#define WOS_THERMAL_METRIC_EC_THERMISTOR_2_COOLING_DESCRIPTION "Passive Cooling of EC thermistor 2"
#define WOS_THERMAL_METRIC_EC_THERMISTOR_2_COOLING_UNIT "%"

// EC thermistor 3
#define WOS_THERMAL_METRIC_EC_THERMISTOR_3_ZONE_NAME "EC thermistor 3"
#define WOS_THERMAL_METRIC_EC_THERMISTOR_3_TEMP_ID 22
#define WOS_THERMAL_METRIC_EC_THERMISTOR_3_TEMP_NAME "Zone id : 33 EC thermistor 3 Temperature"
#define WOS_THERMAL_METRIC_EC_THERMISTOR_3_TEMP_DESCRIPTION "Temperature of EC thermistor 3"
#define WOS_THERMAL_METRIC_EC_THERMISTOR_3_TEMP_UNIT "deg C"

#define WOS_THERMAL_METRIC_EC_THERMISTOR_3_COOLING_ID 23
#define WOS_THERMAL_METRIC_EC_THERMISTOR_3_COOLING_NAME "Zone id : 33 EC thermistor 3 Passive Cooling"
#define WOS_THERMAL_METRIC_EC_THERMISTOR_3_COOLING_DESCRIPTION "Passive Cooling of EC thermistor 3"
#define WOS_THERMAL_METRIC_EC_THERMISTOR_3_COOLING_UNIT "%"

/**
 * @brief Initialize WOS Thermal metrics data
 *
 * Populates the provided metrics_data array with metric information for
 * the WOS Thermal capability, including metric IDs, names, descriptions,
 * and units for all temperature and passive cooling metrics.
 *
 * @param[out] metrics_data Array to be populated with WOS Thermal metric information
 *                          (must have space for WOS_THERMAL_CAPABILITY_METRIC_COUNT entries)
 */
void wos_thermal_capability_init_metrics(struct QcPerfMetricInfo* metrics_data);

/**
 * @brief Initialize WOS Thermal metrics data for available zones
 *
 * This function queries the available thermal zones and initializes metrics
 * only for those zones that are actually present on the system.
 *
 * @param[out] metrics_data Array to be populated with WOS Thermal metric information
 * @param[out] metric_count Pointer to store the number of metrics initialized
 * @return WOS_THERMAL_INFO_SUCCESS on success, error code otherwise
 */
void wos_thermal_capability_init_available_metrics(struct QcPerfMetricInfo* metrics_data, uint8_t* metric_count);

/**
 * @brief Get metric ID for a thermal zone and metric type
 *
 * @param[in] zone_id ID of the thermal zone
 * @param[in] is_cooling Whether to get the cooling metric (true) or temperature metric (false)
 * @param[out] metric_id Pointer to store the metric ID
 * @return WOS_THERMAL_INFO_SUCCESS on success, error code otherwise
 */
void wos_thermal_get_metric_id(uint8_t zone_id, bool is_cooling, uint16_t* metric_id);

#endif /* WOS_THERMAL_INFO_H */
