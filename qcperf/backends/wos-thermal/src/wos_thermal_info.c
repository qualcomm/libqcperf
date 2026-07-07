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
 * @file wos_thermal_info.c
 * @brief Metric initialization implementation for WOS Thermal backend
 * @author Skand Gupta (skangupt@qti.qualcomm.com)
 *
 * This file implements the initialization function that populates metric
 * information structures with static definitions for the WOS Thermal backend's
 * temperature and passive cooling monitoring capability.
 *
 * The implementation initializes metric information for 22 thermal zones, with
 * each zone having two metrics: temperature (in degrees Celsius) and passive
 * cooling (as a percentage). These metrics are used by the WOS Thermal backend
 * to report thermal conditions to applications.
 */

#include <stdio.h>
#include <string.h>
#include "wos_thermal_info.h"
#include "wos_thermal_logger.h"
#include "qcperf_common.h"
#include "thermal_common.h"

// Mapping from zone ID to metric IDs: index TZ_TEMPERATURE = temperature metric, TZ_PASSIVE_COOLING = cooling metric
#define TZ_TEMPERATURE 0
#define TZ_PASSIVE_COOLING 1
static uint16_t g_zone_id_to_metric_id_map[MAX_THERMAL_ZONE_ID][2] = {0};

/**
 * @brief Initialize WOS Thermal metrics data
 *
 * This function populates the metric information for WOS Thermal capability
 * using the static macro definitions from wos_thermal_info.h. For each thermal
 * zone, it initializes two metrics:
 * 1. Temperature metric (in degrees Celsius)
 * 2. Passive cooling metric (as a percentage)
 *
 * The function sets the metric ID, name, description, and unit for each metric,
 * along with the corresponding string lengths. This information is used by the
 * QcPerf framework to provide metadata about the metrics to applications.
 *
 * @param[out] metrics_data Array to be populated with WOS Thermal metric information
 *                          (must have space for WOS_THERMAL_CAPABILITY_METRIC_COUNT entries)
 */
void wos_thermal_capability_init_metrics(struct QcPerfMetricInfo* metrics_data) {
    // CPU Cluster 0 Thermal Zone - Temperature
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_0_TEMP].metric_id = WOS_THERMAL_METRIC_CPU_CLUSTER_0_TEMP_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_0_TEMP].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_0_TEMP_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_0_TEMP].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_0_TEMP].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_0_TEMP].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_0_TEMP_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_0_TEMP].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_0_TEMP].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_0_TEMP].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_0_TEMP_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_0_TEMP].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_0_TEMP].metric_unit);

    // CPU Cluster 0 Thermal Zone - Passive Cooling
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_0_COOLING].metric_id = WOS_THERMAL_METRIC_CPU_CLUSTER_0_COOLING_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_0_COOLING].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_0_COOLING_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_0_COOLING].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_0_COOLING].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_0_COOLING].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_0_COOLING_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_0_COOLING].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_0_COOLING].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_0_COOLING].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_0_COOLING_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_0_COOLING].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_0_COOLING].metric_unit);

    // CPU Cluster 1 Thermal Zone - Temperature
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_1_TEMP].metric_id = WOS_THERMAL_METRIC_CPU_CLUSTER_1_TEMP_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_1_TEMP].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_1_TEMP_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_1_TEMP].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_1_TEMP].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_1_TEMP].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_1_TEMP_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_1_TEMP].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_1_TEMP].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_1_TEMP].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_1_TEMP_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_1_TEMP].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_1_TEMP].metric_unit);

    // CPU Cluster 1 Thermal Zone - Passive Cooling
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_1_COOLING].metric_id = WOS_THERMAL_METRIC_CPU_CLUSTER_1_COOLING_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_1_COOLING].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_1_COOLING_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_1_COOLING].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_1_COOLING].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_1_COOLING].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_1_COOLING_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_1_COOLING].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_1_COOLING].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_1_COOLING].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_1_COOLING_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_1_COOLING].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_1_COOLING].metric_unit);

    // CPU Cluster 2 Thermal Zone - Temperature
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_2_TEMP].metric_id = WOS_THERMAL_METRIC_CPU_CLUSTER_2_TEMP_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_2_TEMP].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_2_TEMP_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_2_TEMP].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_2_TEMP].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_2_TEMP].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_2_TEMP_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_2_TEMP].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_2_TEMP].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_2_TEMP].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_2_TEMP_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_2_TEMP].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_2_TEMP].metric_unit);

    // CPU Cluster 2 Thermal Zone - Passive Cooling
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_2_COOLING].metric_id = WOS_THERMAL_METRIC_CPU_CLUSTER_2_COOLING_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_2_COOLING].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_2_COOLING_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_2_COOLING].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_2_COOLING].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_2_COOLING].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_2_COOLING_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_2_COOLING].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_2_COOLING].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_2_COOLING].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_2_COOLING_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_2_COOLING].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CPU_CLUSTER_2_COOLING].metric_unit);

    // QMX0 Thermal Zone - Temperature
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX0_TEMP].metric_id = WOS_THERMAL_METRIC_QMX0_TEMP_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX0_TEMP].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QMX0_TEMP_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX0_TEMP].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX0_TEMP].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX0_TEMP].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QMX0_TEMP_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX0_TEMP].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX0_TEMP].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX0_TEMP].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QMX0_TEMP_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX0_TEMP].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX0_TEMP].metric_unit);

    // QMX0 Thermal Zone - Passive Cooling
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX0_COOLING].metric_id = WOS_THERMAL_METRIC_QMX0_COOLING_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX0_COOLING].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QMX0_COOLING_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX0_COOLING].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX0_COOLING].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX0_COOLING].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QMX0_COOLING_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX0_COOLING].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX0_COOLING].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX0_COOLING].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QMX0_COOLING_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX0_COOLING].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX0_COOLING].metric_unit);

    // QMX1 Thermal Zone - Temperature
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX1_TEMP].metric_id = WOS_THERMAL_METRIC_QMX1_TEMP_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX1_TEMP].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QMX1_TEMP_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX1_TEMP].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX1_TEMP].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX1_TEMP].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QMX1_TEMP_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX1_TEMP].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX1_TEMP].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX1_TEMP].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QMX1_TEMP_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX1_TEMP].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX1_TEMP].metric_unit);

    // QMX1 Thermal Zone - Passive Cooling
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX1_COOLING].metric_id = WOS_THERMAL_METRIC_QMX1_COOLING_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX1_COOLING].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QMX1_COOLING_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX1_COOLING].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX1_COOLING].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX1_COOLING].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QMX1_COOLING_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX1_COOLING].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX1_COOLING].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX1_COOLING].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QMX1_COOLING_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX1_COOLING].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX1_COOLING].metric_unit);

    // QMX2 Thermal Zone - Temperature
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX2_TEMP].metric_id = WOS_THERMAL_METRIC_QMX2_TEMP_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX2_TEMP].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QMX2_TEMP_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX2_TEMP].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX2_TEMP].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX2_TEMP].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QMX2_TEMP_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX2_TEMP].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX2_TEMP].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX2_TEMP].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QMX2_TEMP_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX2_TEMP].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX2_TEMP].metric_unit);

    // QMX2 Thermal Zone - Passive Cooling
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX2_COOLING].metric_id = WOS_THERMAL_METRIC_QMX2_COOLING_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX2_COOLING].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QMX2_COOLING_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX2_COOLING].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX2_COOLING].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX2_COOLING].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QMX2_COOLING_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX2_COOLING].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX2_COOLING].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX2_COOLING].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QMX2_COOLING_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QMX2_COOLING].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QMX2_COOLING].metric_unit);

    // GPU Thermal Zone - Temperature
    metrics_data[WOS_THERMAL_METRIC_INDEX_GPU_TEMP].metric_id = WOS_THERMAL_METRIC_GPU_TEMP_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_GPU_TEMP].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_GPU_TEMP_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_GPU_TEMP].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_GPU_TEMP].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_GPU_TEMP].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_GPU_TEMP_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_GPU_TEMP].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_GPU_TEMP].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_GPU_TEMP].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_GPU_TEMP_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_GPU_TEMP].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_GPU_TEMP].metric_unit);

    // GPU Thermal Zone - Passive Cooling
    metrics_data[WOS_THERMAL_METRIC_INDEX_GPU_COOLING].metric_id = WOS_THERMAL_METRIC_GPU_COOLING_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_GPU_COOLING].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_GPU_COOLING_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_GPU_COOLING].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_GPU_COOLING].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_GPU_COOLING].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_GPU_COOLING_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_GPU_COOLING].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_GPU_COOLING].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_GPU_COOLING].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_GPU_COOLING_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_GPU_COOLING].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_GPU_COOLING].metric_unit);

    // NSP Thermal Zone - Temperature
    metrics_data[WOS_THERMAL_METRIC_INDEX_NSP_TEMP].metric_id = WOS_THERMAL_METRIC_NSP_TEMP_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_NSP_TEMP].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_NSP_TEMP_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_NSP_TEMP].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_NSP_TEMP].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_NSP_TEMP].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_NSP_TEMP_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_NSP_TEMP].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_NSP_TEMP].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_NSP_TEMP].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_NSP_TEMP_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_NSP_TEMP].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_NSP_TEMP].metric_unit);

    // NSP Thermal Zone - Passive Cooling
    metrics_data[WOS_THERMAL_METRIC_INDEX_NSP_COOLING].metric_id = WOS_THERMAL_METRIC_NSP_COOLING_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_NSP_COOLING].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_NSP_COOLING_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_NSP_COOLING].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_NSP_COOLING].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_NSP_COOLING].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_NSP_COOLING_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_NSP_COOLING].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_NSP_COOLING].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_NSP_COOLING].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_NSP_COOLING_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_NSP_COOLING].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_NSP_COOLING].metric_unit);

    // QCLimitsPolicy CPU coreparking - Temperature
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_COREPARKING_TEMP].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_COREPARKING_TEMP_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_COREPARKING_TEMP].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_COREPARKING_TEMP_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_COREPARKING_TEMP].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_COREPARKING_TEMP].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_COREPARKING_TEMP].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s",
             WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_COREPARKING_TEMP_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_COREPARKING_TEMP].metric_description_len =
        strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_COREPARKING_TEMP].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_COREPARKING_TEMP].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_COREPARKING_TEMP_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_COREPARKING_TEMP].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_COREPARKING_TEMP].metric_unit);

    // QCLimitsPolicy CPU coreparking - Passive Cooling
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_COREPARKING_COOLING].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_COREPARKING_COOLING_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_COREPARKING_COOLING].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_COREPARKING_COOLING_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_COREPARKING_COOLING].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_COREPARKING_COOLING].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_COREPARKING_COOLING].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s",
             WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_COREPARKING_COOLING_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_COREPARKING_COOLING].metric_description_len =
        strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_COREPARKING_COOLING].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_COREPARKING_COOLING].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_COREPARKING_COOLING_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_COREPARKING_COOLING].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_COREPARKING_COOLING].metric_unit);

    // QCLimitsPolicy CPU DCVS All Clusters - Temperature
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_TEMP].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_TEMP_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_TEMP].metric_name, METRIC_NAME_MAX_LEN, "%s",
             WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_TEMP_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_TEMP].metric_name_len =
        strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_TEMP].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_TEMP].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s",
             WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_TEMP_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_TEMP].metric_description_len =
        strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_TEMP].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_TEMP].metric_unit, MAX_METRIC_UNIT_LEN, "%s",
             WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_TEMP_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_TEMP].metric_unit_len =
        strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_TEMP].metric_unit);

    // QCLimitsPolicy CPU DCVS All Clusters - Passive Cooling
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_COOLING].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_COOLING_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_COOLING].metric_name, METRIC_NAME_MAX_LEN, "%s",
             WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_COOLING_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_COOLING].metric_name_len =
        strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_COOLING].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_COOLING].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s",
             WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_COOLING_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_COOLING].metric_description_len =
        strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_COOLING].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_COOLING].metric_unit, MAX_METRIC_UNIT_LEN, "%s",
             WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_COOLING_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_COOLING].metric_unit_len =
        strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_COOLING].metric_unit);

    // QCLimitsPolicy CPU DCVS Cluster0 - Temperature
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_TEMP].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_TEMP_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_TEMP].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_TEMP_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_TEMP].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_TEMP].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_TEMP].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s",
             WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_TEMP_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_TEMP].metric_description_len =
        strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_TEMP].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_TEMP].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_TEMP_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_TEMP].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_TEMP].metric_unit);

    // QCLimitsPolicy CPU DCVS Cluster0 - Passive Cooling
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_COOLING].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_COOLING_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_COOLING].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_COOLING_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_COOLING].metric_name_len =
        strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_COOLING].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_COOLING].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s",
             WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_COOLING_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_COOLING].metric_description_len =
        strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_COOLING].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_COOLING].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_COOLING_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_COOLING].metric_unit_len =
        strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_COOLING].metric_unit);

    // QCLimitsPolicy CPU DCVS Cluster1 - Temperature
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_TEMP].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_TEMP_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_TEMP].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_TEMP_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_TEMP].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_TEMP].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_TEMP].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s",
             WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_TEMP_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_TEMP].metric_description_len =
        strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_TEMP].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_TEMP].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_TEMP_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_TEMP].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_TEMP].metric_unit);

    // QCLimitsPolicy CPU DCVS Cluster1 - Passive Cooling
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_COOLING].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_COOLING_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_COOLING].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_COOLING_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_COOLING].metric_name_len =
        strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_COOLING].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_COOLING].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s",
             WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_COOLING_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_COOLING].metric_description_len =
        strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_COOLING].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_COOLING].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_COOLING_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_COOLING].metric_unit_len =
        strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_COOLING].metric_unit);

    // QCLimitsPolicy CPU DCVS Cluster2 - Temperature
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_TEMP].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_TEMP_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_TEMP].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_TEMP_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_TEMP].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_TEMP].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_TEMP].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s",
             WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_TEMP_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_TEMP].metric_description_len =
        strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_TEMP].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_TEMP].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_TEMP_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_TEMP].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_TEMP].metric_unit);

    // QCLimitsPolicy CPU DCVS Cluster2 - Passive Cooling
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_COOLING].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_COOLING_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_COOLING].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_COOLING_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_COOLING].metric_name_len =
        strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_COOLING].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_COOLING].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s",
             WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_COOLING_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_COOLING].metric_description_len =
        strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_COOLING].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_COOLING].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_COOLING_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_COOLING].metric_unit_len =
        strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_COOLING].metric_unit);

    // QCLimitsPolicy GPU - Temperature
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_GPU_TEMP].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_GPU_TEMP_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_GPU_TEMP].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_GPU_TEMP_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_GPU_TEMP].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_GPU_TEMP].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_GPU_TEMP].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_GPU_TEMP_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_GPU_TEMP].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_GPU_TEMP].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_GPU_TEMP].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_GPU_TEMP_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_GPU_TEMP].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_GPU_TEMP].metric_unit);

    // QCLimitsPolicy GPU - Passive Cooling
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_GPU_COOLING].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_GPU_COOLING_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_GPU_COOLING].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_GPU_COOLING_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_GPU_COOLING].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_GPU_COOLING].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_GPU_COOLING].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_GPU_COOLING_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_GPU_COOLING].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_GPU_COOLING].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_GPU_COOLING].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_GPU_COOLING_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_GPU_COOLING].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_GPU_COOLING].metric_unit);

    // QCLimitsPolicy NSP - Temperature
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_NSP_TEMP].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_NSP_TEMP_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_NSP_TEMP].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_NSP_TEMP_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_NSP_TEMP].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_NSP_TEMP].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_NSP_TEMP].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_NSP_TEMP_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_NSP_TEMP].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_NSP_TEMP].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_NSP_TEMP].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_NSP_TEMP_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_NSP_TEMP].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_NSP_TEMP].metric_unit);

    // QCLimitsPolicy NSP - Passive Cooling
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_NSP_COOLING].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_NSP_COOLING_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_NSP_COOLING].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_NSP_COOLING_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_NSP_COOLING].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_NSP_COOLING].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_NSP_COOLING].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_NSP_COOLING_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_NSP_COOLING].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_NSP_COOLING].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_NSP_COOLING].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_NSP_COOLING_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_NSP_COOLING].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_NSP_COOLING].metric_unit);

    // QCLimitsPolicy Modem BCL - Temperature
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_BCL_TEMP].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_BCL_TEMP_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_BCL_TEMP].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_BCL_TEMP_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_BCL_TEMP].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_BCL_TEMP].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_BCL_TEMP].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_BCL_TEMP_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_BCL_TEMP].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_BCL_TEMP].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_BCL_TEMP].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_BCL_TEMP_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_BCL_TEMP].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_BCL_TEMP].metric_unit);

    // QCLimitsPolicy Modem BCL - Passive Cooling
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_BCL_COOLING].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_BCL_COOLING_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_BCL_COOLING].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_BCL_COOLING_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_BCL_COOLING].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_BCL_COOLING].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_BCL_COOLING].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s",
             WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_BCL_COOLING_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_BCL_COOLING].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_BCL_COOLING].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_BCL_COOLING].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_BCL_COOLING_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_BCL_COOLING].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_BCL_COOLING].metric_unit);

    // QCLimitsPolicy Modem Skin - Temperature
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_SKIN_TEMP].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_SKIN_TEMP_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_SKIN_TEMP].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_SKIN_TEMP_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_SKIN_TEMP].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_SKIN_TEMP].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_SKIN_TEMP].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_SKIN_TEMP_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_SKIN_TEMP].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_SKIN_TEMP].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_SKIN_TEMP].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_SKIN_TEMP_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_SKIN_TEMP].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_SKIN_TEMP].metric_unit);

    // QCLimitsPolicy Modem Skin - Passive Cooling
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_SKIN_COOLING].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_SKIN_COOLING_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_SKIN_COOLING].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_SKIN_COOLING_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_SKIN_COOLING].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_SKIN_COOLING].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_SKIN_COOLING].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s",
             WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_SKIN_COOLING_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_SKIN_COOLING].metric_description_len =
        strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_SKIN_COOLING].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_SKIN_COOLING].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_SKIN_COOLING_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_SKIN_COOLING].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_MODEM_SKIN_COOLING].metric_unit);

    // QCLimitsPolicy WLAN - Temperature
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_WLAN_TEMP].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_WLAN_TEMP_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_WLAN_TEMP].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_WLAN_TEMP_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_WLAN_TEMP].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_WLAN_TEMP].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_WLAN_TEMP].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_WLAN_TEMP_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_WLAN_TEMP].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_WLAN_TEMP].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_WLAN_TEMP].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_WLAN_TEMP_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_WLAN_TEMP].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_WLAN_TEMP].metric_unit);

    // QCLimitsPolicy WLAN - Passive Cooling
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_WLAN_COOLING].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_WLAN_COOLING_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_WLAN_COOLING].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_WLAN_COOLING_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_WLAN_COOLING].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_WLAN_COOLING].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_WLAN_COOLING].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_WLAN_COOLING_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_WLAN_COOLING].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_WLAN_COOLING].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_WLAN_COOLING].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_WLAN_COOLING_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_WLAN_COOLING].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_QCLIMITSPOLICY_WLAN_COOLING].metric_unit);

    // Critical Thermal Zones for All Internal TSENS - Temperature
    metrics_data[WOS_THERMAL_METRIC_INDEX_CRITICAL_THERMAL_ZONES_TEMP].metric_id = WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_TEMP_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CRITICAL_THERMAL_ZONES_TEMP].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_TEMP_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CRITICAL_THERMAL_ZONES_TEMP].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CRITICAL_THERMAL_ZONES_TEMP].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CRITICAL_THERMAL_ZONES_TEMP].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_TEMP_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CRITICAL_THERMAL_ZONES_TEMP].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CRITICAL_THERMAL_ZONES_TEMP].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CRITICAL_THERMAL_ZONES_TEMP].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_TEMP_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CRITICAL_THERMAL_ZONES_TEMP].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CRITICAL_THERMAL_ZONES_TEMP].metric_unit);

    // Critical Thermal Zones for All Internal TSENS - Passive Cooling
    metrics_data[WOS_THERMAL_METRIC_INDEX_CRITICAL_THERMAL_ZONES_COOLING].metric_id = WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_COOLING_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CRITICAL_THERMAL_ZONES_COOLING].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_COOLING_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CRITICAL_THERMAL_ZONES_COOLING].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CRITICAL_THERMAL_ZONES_COOLING].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CRITICAL_THERMAL_ZONES_COOLING].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_COOLING_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CRITICAL_THERMAL_ZONES_COOLING].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CRITICAL_THERMAL_ZONES_COOLING].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_CRITICAL_THERMAL_ZONES_COOLING].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_COOLING_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_CRITICAL_THERMAL_ZONES_COOLING].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_CRITICAL_THERMAL_ZONES_COOLING].metric_unit);

    // EC thermistor 1 - Temperature
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_1_TEMP].metric_id = WOS_THERMAL_METRIC_EC_THERMISTOR_1_TEMP_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_1_TEMP].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_1_TEMP_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_1_TEMP].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_1_TEMP].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_1_TEMP].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_1_TEMP_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_1_TEMP].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_1_TEMP].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_1_TEMP].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_1_TEMP_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_1_TEMP].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_1_TEMP].metric_unit);

    // EC thermistor 1 - Passive Cooling
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_1_COOLING].metric_id = WOS_THERMAL_METRIC_EC_THERMISTOR_1_COOLING_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_1_COOLING].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_1_COOLING_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_1_COOLING].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_1_COOLING].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_1_COOLING].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_1_COOLING_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_1_COOLING].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_1_COOLING].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_1_COOLING].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_1_COOLING_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_1_COOLING].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_1_COOLING].metric_unit);

    // EC thermistor 2 - Temperature
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_2_TEMP].metric_id = WOS_THERMAL_METRIC_EC_THERMISTOR_2_TEMP_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_2_TEMP].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_2_TEMP_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_2_TEMP].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_2_TEMP].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_2_TEMP].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_2_TEMP_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_2_TEMP].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_2_TEMP].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_2_TEMP].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_2_TEMP_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_2_TEMP].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_2_TEMP].metric_unit);

    // EC thermistor 2 - Passive Cooling
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_2_COOLING].metric_id = WOS_THERMAL_METRIC_EC_THERMISTOR_2_COOLING_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_2_COOLING].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_2_COOLING_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_2_COOLING].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_2_COOLING].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_2_COOLING].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_2_COOLING_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_2_COOLING].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_2_COOLING].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_2_COOLING].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_2_COOLING_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_2_COOLING].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_2_COOLING].metric_unit);

    // EC thermistor 3 - Temperature
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_3_TEMP].metric_id = WOS_THERMAL_METRIC_EC_THERMISTOR_3_TEMP_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_3_TEMP].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_3_TEMP_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_3_TEMP].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_3_TEMP].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_3_TEMP].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_3_TEMP_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_3_TEMP].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_3_TEMP].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_3_TEMP].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_3_TEMP_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_3_TEMP].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_3_TEMP].metric_unit);

    // EC thermistor 3 - Passive Cooling
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_3_COOLING].metric_id = WOS_THERMAL_METRIC_EC_THERMISTOR_3_COOLING_ID;
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_3_COOLING].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_3_COOLING_NAME);
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_3_COOLING].metric_name_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_3_COOLING].metric_name);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_3_COOLING].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_3_COOLING_DESCRIPTION);
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_3_COOLING].metric_description_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_3_COOLING].metric_description);
    snprintf(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_3_COOLING].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_3_COOLING_UNIT);
    metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_3_COOLING].metric_unit_len = strlen(metrics_data[WOS_THERMAL_METRIC_INDEX_EC_THERMISTOR_3_COOLING].metric_unit);
}

/**
 * @brief Initialize WOS Thermal metrics data for available zones
 *
 * This function queries the available thermal zones and initializes metrics
 * only for those zones that are actually present on the system.
 * It maps the zone names to the predefined metrics in the header file.
 *
 * @param[out] metrics_data Array to be populated with WOS Thermal metric information
 * @param[out] metric_count Pointer to store the number of metrics initialized
 */
void wos_thermal_capability_init_available_metrics(struct QcPerfMetricInfo* metrics_data, uint8_t* metric_count) {
    struct ThermalCommonZoneNameMap zone_map = {0};
    enum ThermalCommonReturnCode common_ret  = RETURN_CODE_THERMAL_COMMON_SUCCESS;
    uint8_t metric_index                     = 0;

    // Clear the zone ID to metric ID mapping
    memset(g_zone_id_to_metric_id_map, 0xFF, sizeof(g_zone_id_to_metric_id_map));

    // Initialize metric count to 0
    if (NULL == metric_count) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Metric count pointer is NULL");
        return;
    } else {
        *metric_count = 0;
        // Get available thermal zones
        common_ret = thermal_common_getzone_names(&zone_map);
        if (RETURN_CODE_THERMAL_COMMON_SUCCESS != common_ret) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to get thermal zone names, error: %d", common_ret);
        } else {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Found %d thermal zones", zone_map.zone_names_ids_length);

            // Create a mapping between zone IDs and metric indices
            for (uint8_t i = 0; i < zone_map.zone_names_ids_length; i++) {
                uint8_t zone_id       = zone_map.zone_ids[i];
                const char* zone_name = zone_map.zone_names[i].string;
                bool found_match      = false;

                if (zone_id >= MAX_THERMAL_ZONE_ID) {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_WARNING, "Zone ID %d exceeds maximum allowed (%d), skipping", zone_id, MAX_THERMAL_ZONE_ID - 1);
                    continue;
                } else {
                    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_DEBUG, "Initializing metrics for zone %d: %s", zone_id, zone_name);
                    // Check if this zone matches any of our predefined zones
                    // CPU Cluster 0
                    if (strstr(zone_name, WOS_THERMAL_METRIC_CPU_CLUSTER_0_ZONE_NAME) != NULL) {
                        // Temperature metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_CPU_CLUSTER_0_TEMP_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_0_TEMP_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_0_TEMP_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_0_TEMP_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_TEMPERATURE] = metrics_data[metric_index].metric_id;
                        metric_index++;

                        // Cooling metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_CPU_CLUSTER_0_COOLING_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_0_COOLING_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_0_COOLING_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_0_COOLING_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_PASSIVE_COOLING] = metrics_data[metric_index].metric_id;
                        metric_index++;
                        found_match = true;
                    }
                    // CPU Cluster 1
                    else if (strstr(zone_name, WOS_THERMAL_METRIC_CPU_CLUSTER_1_ZONE_NAME) != NULL) {
                        // Temperature metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_CPU_CLUSTER_1_TEMP_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_1_TEMP_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_1_TEMP_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_1_TEMP_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_TEMPERATURE] = metrics_data[metric_index].metric_id;
                        metric_index++;

                        // Cooling metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_CPU_CLUSTER_1_COOLING_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_1_COOLING_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_1_COOLING_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_1_COOLING_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_PASSIVE_COOLING] = metrics_data[metric_index].metric_id;
                        metric_index++;
                        found_match = true;
                    }
                    // CPU Cluster 2
                    else if (strstr(zone_name, WOS_THERMAL_METRIC_CPU_CLUSTER_2_ZONE_NAME) != NULL) {
                        // Temperature metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_CPU_CLUSTER_2_TEMP_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_2_TEMP_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_2_TEMP_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_2_TEMP_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_TEMPERATURE] = metrics_data[metric_index].metric_id;
                        metric_index++;

                        // Cooling metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_CPU_CLUSTER_2_COOLING_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_2_COOLING_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_2_COOLING_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_CPU_CLUSTER_2_COOLING_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_PASSIVE_COOLING] = metrics_data[metric_index].metric_id;
                        metric_index++;
                        found_match = true;
                    }
                    // QMX0
                    else if (strstr(zone_name, WOS_THERMAL_METRIC_QMX0_ZONE_NAME) != NULL) {
                        // Temperature metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QMX0_TEMP_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QMX0_TEMP_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QMX0_TEMP_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QMX0_TEMP_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_TEMPERATURE] = metrics_data[metric_index].metric_id;
                        metric_index++;

                        // Cooling metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QMX0_COOLING_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QMX0_COOLING_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QMX0_COOLING_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QMX0_COOLING_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_PASSIVE_COOLING] = metrics_data[metric_index].metric_id;
                        metric_index++;
                        found_match = true;
                    }
                    // QMX1
                    else if (strstr(zone_name, WOS_THERMAL_METRIC_QMX1_ZONE_NAME) != NULL) {
                        // Temperature metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QMX1_TEMP_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QMX1_TEMP_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QMX1_TEMP_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QMX1_TEMP_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_TEMPERATURE] = metrics_data[metric_index].metric_id;
                        metric_index++;

                        // Cooling metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QMX1_COOLING_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QMX1_COOLING_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QMX1_COOLING_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QMX1_COOLING_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_PASSIVE_COOLING] = metrics_data[metric_index].metric_id;
                        metric_index++;
                        found_match = true;
                    }
                    // QMX2
                    else if (strstr(zone_name, WOS_THERMAL_METRIC_QMX2_ZONE_NAME) != NULL) {
                        // Temperature metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QMX2_TEMP_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QMX2_TEMP_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QMX2_TEMP_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QMX2_TEMP_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_TEMPERATURE] = metrics_data[metric_index].metric_id;
                        metric_index++;

                        // Cooling metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QMX2_COOLING_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QMX2_COOLING_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QMX2_COOLING_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QMX2_COOLING_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_PASSIVE_COOLING] = metrics_data[metric_index].metric_id;
                        metric_index++;
                        found_match = true;
                    }
                    // GPU
                    else if (strstr(zone_name, WOS_THERMAL_METRIC_GPU_ZONE_NAME) != NULL) {
                        // Temperature metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_GPU_TEMP_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_GPU_TEMP_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_GPU_TEMP_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_GPU_TEMP_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_TEMPERATURE] = metrics_data[metric_index].metric_id;
                        metric_index++;

                        // Cooling metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_GPU_COOLING_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_GPU_COOLING_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_GPU_COOLING_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_GPU_COOLING_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_PASSIVE_COOLING] = metrics_data[metric_index].metric_id;
                        metric_index++;
                        found_match = true;
                    }
                    // NSP
                    else if (strstr(zone_name, WOS_THERMAL_METRIC_NSP_ZONE_NAME) != NULL) {
                        // Temperature metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_NSP_TEMP_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_NSP_TEMP_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_NSP_TEMP_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_NSP_TEMP_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_TEMPERATURE] = metrics_data[metric_index].metric_id;
                        metric_index++;

                        // Cooling metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_NSP_COOLING_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_NSP_COOLING_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_NSP_COOLING_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_NSP_COOLING_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_PASSIVE_COOLING] = metrics_data[metric_index].metric_id;
                        metric_index++;
                        found_match = true;
                    }
                    // QCLimitsPolicy CPU coreparking
                    else if (strstr(zone_name, WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_COREPARKING_ZONE_NAME) != NULL) {
                        // Temperature metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_COREPARKING_TEMP_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_COREPARKING_TEMP_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_COREPARKING_TEMP_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_COREPARKING_TEMP_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_TEMPERATURE] = metrics_data[metric_index].metric_id;
                        metric_index++;

                        // Cooling metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_COREPARKING_COOLING_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_COREPARKING_COOLING_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_COREPARKING_COOLING_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_COREPARKING_COOLING_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_PASSIVE_COOLING] = metrics_data[metric_index].metric_id;
                        metric_index++;
                        found_match = true;
                    }
                    // QCLimitsPolicy CPU DCVS All Clusters
                    else if (strstr(zone_name, WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_ZONE_NAME) != NULL) {
                        // Temperature metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_TEMP_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_TEMP_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_TEMP_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_TEMP_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_TEMPERATURE] = metrics_data[metric_index].metric_id;
                        metric_index++;

                        // Cooling metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_COOLING_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_COOLING_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_COOLING_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_ALL_CLUSTERS_COOLING_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_PASSIVE_COOLING] = metrics_data[metric_index].metric_id;
                        metric_index++;
                        found_match = true;
                    }
                    // QCLimitsPolicy CPU DCVS Cluster0
                    else if (strstr(zone_name, WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_ZONE_NAME) != NULL) {
                        // Temperature metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_TEMP_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_TEMP_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_TEMP_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_TEMP_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_TEMPERATURE] = metrics_data[metric_index].metric_id;
                        metric_index++;

                        // Cooling metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_COOLING_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_COOLING_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_COOLING_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER0_COOLING_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_PASSIVE_COOLING] = metrics_data[metric_index].metric_id;
                        metric_index++;
                        found_match = true;
                    }
                    // QCLimitsPolicy CPU DCVS Cluster1
                    else if (strstr(zone_name, WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_ZONE_NAME) != NULL) {
                        // Temperature metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_TEMP_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_TEMP_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_TEMP_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_TEMP_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_TEMPERATURE] = metrics_data[metric_index].metric_id;
                        metric_index++;

                        // Cooling metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_COOLING_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_COOLING_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_COOLING_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER1_COOLING_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_PASSIVE_COOLING] = metrics_data[metric_index].metric_id;
                        metric_index++;
                        found_match = true;
                    }
                    // QCLimitsPolicy CPU DCVS Cluster2
                    else if (strstr(zone_name, WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_ZONE_NAME) != NULL) {
                        // Temperature metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_TEMP_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_TEMP_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_TEMP_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_TEMP_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_TEMPERATURE] = metrics_data[metric_index].metric_id;
                        metric_index++;

                        // Cooling metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_COOLING_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_COOLING_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_COOLING_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_CPU_DCVS_CLUSTER2_COOLING_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_PASSIVE_COOLING] = metrics_data[metric_index].metric_id;
                        metric_index++;
                        found_match = true;
                    }
                    // QCLimitsPolicy GPU
                    else if (strstr(zone_name, WOS_THERMAL_METRIC_QCLIMITSPOLICY_GPU_ZONE_NAME) != NULL && strstr(zone_name, WOS_THERMAL_METRIC_QCLIMITSPOLICY_GPU_ZONE_NAME) == zone_name) {
                        // Temperature metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_GPU_TEMP_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_GPU_TEMP_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_GPU_TEMP_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_GPU_TEMP_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_TEMPERATURE] = metrics_data[metric_index].metric_id;
                        metric_index++;

                        // Cooling metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_GPU_COOLING_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_GPU_COOLING_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_GPU_COOLING_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_GPU_COOLING_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_PASSIVE_COOLING] = metrics_data[metric_index].metric_id;
                        metric_index++;
                        found_match = true;
                    }
                    // QCLimitsPolicy NSP
                    else if (strstr(zone_name, WOS_THERMAL_METRIC_QCLIMITSPOLICY_NSP_ZONE_NAME) != NULL) {
                        // Temperature metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_NSP_TEMP_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_NSP_TEMP_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_NSP_TEMP_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_NSP_TEMP_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_TEMPERATURE] = metrics_data[metric_index].metric_id;
                        metric_index++;

                        // Cooling metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_NSP_COOLING_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_NSP_COOLING_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_NSP_COOLING_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_NSP_COOLING_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_PASSIVE_COOLING] = metrics_data[metric_index].metric_id;
                        metric_index++;
                        found_match = true;
                    }
                    // QCLimitsPolicy Modem BCL
                    else if (strstr(zone_name, WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_BCL_ZONE_NAME) != NULL) {
                        // Temperature metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_BCL_TEMP_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_BCL_TEMP_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_BCL_TEMP_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_BCL_TEMP_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_TEMPERATURE] = metrics_data[metric_index].metric_id;
                        metric_index++;

                        // Cooling metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_BCL_COOLING_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_BCL_COOLING_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_BCL_COOLING_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_BCL_COOLING_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_PASSIVE_COOLING] = metrics_data[metric_index].metric_id;
                        metric_index++;
                        found_match = true;
                    }
                    // QCLimitsPolicy Modem Skin
                    else if (strstr(zone_name, WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_SKIN_ZONE_NAME) != NULL) {
                        // Temperature metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_SKIN_TEMP_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_SKIN_TEMP_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_SKIN_TEMP_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_SKIN_TEMP_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_TEMPERATURE] = metrics_data[metric_index].metric_id;
                        metric_index++;

                        // Cooling metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_SKIN_COOLING_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_SKIN_COOLING_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_SKIN_COOLING_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_MODEM_SKIN_COOLING_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_PASSIVE_COOLING] = metrics_data[metric_index].metric_id;
                        metric_index++;
                        found_match = true;
                    }
                    // QCLimitsPolicy WLAN
                    else if (strstr(zone_name, WOS_THERMAL_METRIC_QCLIMITSPOLICY_WLAN_ZONE_NAME) != NULL) {
                        // Temperature metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_WLAN_TEMP_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_WLAN_TEMP_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_WLAN_TEMP_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_WLAN_TEMP_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_TEMPERATURE] = metrics_data[metric_index].metric_id;
                        metric_index++;

                        // Cooling metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_QCLIMITSPOLICY_WLAN_COOLING_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_WLAN_COOLING_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_WLAN_COOLING_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_QCLIMITSPOLICY_WLAN_COOLING_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_PASSIVE_COOLING] = metrics_data[metric_index].metric_id;
                        metric_index++;
                        found_match = true;
                    }
                    // Critical Thermal Zones
                    else if (strstr(zone_name, WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_ZONE_NAME) != NULL) {
                        // Temperature metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_TEMP_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_TEMP_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_TEMP_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_TEMP_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_TEMPERATURE] = metrics_data[metric_index].metric_id;
                        metric_index++;

                        // Cooling metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_COOLING_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_COOLING_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_COOLING_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_CRITICAL_THERMAL_ZONES_COOLING_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_PASSIVE_COOLING] = metrics_data[metric_index].metric_id;
                        metric_index++;
                        found_match = true;
                    }
                    // EC thermistor 1
                    else if (strstr(zone_name, WOS_THERMAL_METRIC_EC_THERMISTOR_1_ZONE_NAME) != NULL) {
                        // Temperature metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_EC_THERMISTOR_1_TEMP_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_1_TEMP_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_1_TEMP_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_1_TEMP_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_TEMPERATURE] = metrics_data[metric_index].metric_id;
                        metric_index++;

                        // Cooling metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_EC_THERMISTOR_1_COOLING_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_1_COOLING_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_1_COOLING_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_1_COOLING_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_PASSIVE_COOLING] = metrics_data[metric_index].metric_id;
                        metric_index++;
                        found_match = true;
                    }
                    // EC thermistor 2
                    else if (strstr(zone_name, WOS_THERMAL_METRIC_EC_THERMISTOR_2_ZONE_NAME) != NULL) {
                        // Temperature metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_EC_THERMISTOR_2_TEMP_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_2_TEMP_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_2_TEMP_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_2_TEMP_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_TEMPERATURE] = metrics_data[metric_index].metric_id;
                        metric_index++;

                        // Cooling metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_EC_THERMISTOR_2_COOLING_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_2_COOLING_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_2_COOLING_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_2_COOLING_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_PASSIVE_COOLING] = metrics_data[metric_index].metric_id;
                        metric_index++;
                        found_match = true;
                    }
                    // EC thermistor 3
                    else if (strstr(zone_name, WOS_THERMAL_METRIC_EC_THERMISTOR_3_ZONE_NAME) != NULL) {
                        // Temperature metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_EC_THERMISTOR_3_TEMP_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_3_TEMP_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_3_TEMP_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_3_TEMP_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_TEMPERATURE] = metrics_data[metric_index].metric_id;
                        metric_index++;

                        // Cooling metric
                        metrics_data[metric_index].metric_id = WOS_THERMAL_METRIC_EC_THERMISTOR_3_COOLING_ID;
                        snprintf(metrics_data[metric_index].metric_name, METRIC_NAME_MAX_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_3_COOLING_NAME);
                        metrics_data[metric_index].metric_name_len = strlen(metrics_data[metric_index].metric_name);
                        snprintf(metrics_data[metric_index].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_3_COOLING_DESCRIPTION);
                        metrics_data[metric_index].metric_description_len = strlen(metrics_data[metric_index].metric_description);
                        snprintf(metrics_data[metric_index].metric_unit, MAX_METRIC_UNIT_LEN, "%s", WOS_THERMAL_METRIC_EC_THERMISTOR_3_COOLING_UNIT);
                        metrics_data[metric_index].metric_unit_len = strlen(metrics_data[metric_index].metric_unit);

                        // Store mapping
                        g_zone_id_to_metric_id_map[zone_id][TZ_PASSIVE_COOLING] = metrics_data[metric_index].metric_id;
                        metric_index++;
                        found_match = true;
                    }

                    // If no match was found, use generic naming
                    if (!found_match) {
                        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Metric not defined for zone : %s", zone_name);
                    }
                }
            }

            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_DEBUG, "Initialized %d metrics for %d thermal zones", metric_index, zone_map.zone_names_ids_length);
        }
        // Set the metric count
        *metric_count = metric_index;
    }
}

/**
 * @brief Get metric ID for a thermal zone and metric type
 *
 * @param[in] zone_id ID of the thermal zone
 * @param[in] is_cooling Whether to get the cooling metric (true) or temperature metric (false)
 * @param[out] metric_id Pointer to store the metric ID
 */
void wos_thermal_get_metric_id(uint8_t zone_id, bool is_cooling, uint16_t* metric_id) {
    if (metric_id == NULL) {
        return;
    }

    if (zone_id >= MAX_THERMAL_ZONE_ID) {
        *metric_id = 0xFFFF;  // Invalid zone ID
        return;
    }
    if (true == is_cooling) {
        *metric_id = g_zone_id_to_metric_id_map[zone_id][TZ_PASSIVE_COOLING];
    } else {
        *metric_id = g_zone_id_to_metric_id_map[zone_id][TZ_TEMPERATURE];
    }
}
