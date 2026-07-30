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
 * @file qcom_dsp_npu_info.c
 * @brief Metric initialization implementation for DSP NPU backend
 * @author  Snehal Lalage (slalage@qti.qualcomm.com)
 * This file implements the initialization functions that populate metric
 * information structures with static definitions for the DSP NPU backend's
 * capability.
 */

#include <stdio.h>
#include <string.h>
#include "qcom_dsp_npu_info.h"

/**
 * @brief Initialize NPU Capability 0 metrics data
 *
 * This function populates the metric information for NPU Capability 0
 * using the macro definitions from qcom_dsp_npu_info.h
 */
void dsp_npu_capability_0_init_metrics(struct QcPerfMetricInfo *metrics_data) {
    // Q6 Utilization
    metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_UTILIZATION].metric_id = DSP_NPU_METRIC_Q6_UTILIZATION_ID;
    snprintf(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_UTILIZATION].metric_name, METRIC_NAME_MAX_LEN, "%s", DSP_NPU_METRIC_Q6_UTILIZATION_NAME);
    metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_UTILIZATION].metric_name_len = strlen(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_UTILIZATION].metric_name);
    snprintf(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_UTILIZATION].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", DSP_NPU_METRIC_Q6_UTILIZATION_DESCRIPTION);
    metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_UTILIZATION].metric_description_len = strlen(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_UTILIZATION].metric_description);
    snprintf(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_UTILIZATION].metric_unit, MAX_METRIC_UNIT_LEN, "%s", DSP_NPU_METRIC_Q6_UTILIZATION_UNIT);
    metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_UTILIZATION].metric_unit_len = strlen(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_UTILIZATION].metric_unit);

    // Q6 Clock
    metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_CLOCK].metric_id = DSP_NPU_METRIC_Q6_CLOCK_ID;
    snprintf(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_CLOCK].metric_name, METRIC_NAME_MAX_LEN, "%s", DSP_NPU_METRIC_Q6_CLOCK_NAME);
    metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_CLOCK].metric_name_len = strlen(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_CLOCK].metric_name);
    snprintf(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_CLOCK].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", DSP_NPU_METRIC_Q6_CLOCK_DESCRIPTION);
    metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_CLOCK].metric_description_len = strlen(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_CLOCK].metric_description);
    snprintf(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_CLOCK].metric_unit, MAX_METRIC_UNIT_LEN, "%s", DSP_NPU_METRIC_Q6_CLOCK_UNIT);
    metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_CLOCK].metric_unit_len = strlen(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_CLOCK].metric_unit);

    // HVX Utilization
    metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_HVX_UTILIZATION].metric_id = DSP_NPU_METRIC_HVX_UTILIZATION_ID;
    snprintf(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_HVX_UTILIZATION].metric_name, METRIC_NAME_MAX_LEN, "%s", DSP_NPU_METRIC_HVX_UTILIZATION_NAME);
    metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_HVX_UTILIZATION].metric_name_len = strlen(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_HVX_UTILIZATION].metric_name);
    snprintf(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_HVX_UTILIZATION].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", DSP_NPU_METRIC_HVX_UTILIZATION_DESCRIPTION);
    metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_HVX_UTILIZATION].metric_description_len = strlen(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_HVX_UTILIZATION].metric_description);
    snprintf(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_HVX_UTILIZATION].metric_unit, MAX_METRIC_UNIT_LEN, "%s", DSP_NPU_METRIC_HVX_UTILIZATION_UNIT);
    metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_HVX_UTILIZATION].metric_unit_len = strlen(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_HVX_UTILIZATION].metric_unit);

    // HMX Utilization
    metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_HMX_UTILIZATION].metric_id = DSP_NPU_METRIC_HMX_UTILIZATION_ID;
    snprintf(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_HMX_UTILIZATION].metric_name, METRIC_NAME_MAX_LEN, "%s", DSP_NPU_METRIC_HMX_UTILIZATION_NAME);
    metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_HMX_UTILIZATION].metric_name_len = strlen(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_HMX_UTILIZATION].metric_name);
    snprintf(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_HMX_UTILIZATION].metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", DSP_NPU_METRIC_HMX_UTILIZATION_DESCRIPTION);
    metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_HMX_UTILIZATION].metric_description_len = strlen(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_HMX_UTILIZATION].metric_description);
    snprintf(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_HMX_UTILIZATION].metric_unit, MAX_METRIC_UNIT_LEN, "%s", DSP_NPU_METRIC_HMX_UTILIZATION_UNIT);
    metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_HMX_UTILIZATION].metric_unit_len = strlen(metrics_data[DSP_NPU_CAPABILITY_0_METRIC_INDEX_HMX_UTILIZATION].metric_unit);

}
