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
 * @file qcom_dsp_npu_info.h
 * @brief Static metric definitions and initialization functions for DSP NPU backend
 * @author  Snehal Lalage (slalage@qti.qualcomm.com)
 * This header defines all metric IDs, names, descriptions, and units for the
 * DSP NPU backend's capabilities. It also provides initialization functions to
 * populate metric information structures with these static definitions.
 */

#ifndef QCOM_DSP_NPU_INFO_H
#define QCOM_DSP_NPU_INFO_H

#include "qcperf_common.h"

#define DSP_NPU_CAPABILITY_0_ID 0
#define DSP_NPU_CAPABILITIES_LEN 1

#define DSP_NPU_STREAMING_RATES_LEN 10
#define DSP_NPU_STREAMING_RATES 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000

#define DSP_NPU_SAMPLING_RATES_LEN 6
#define DSP_NPU_SAMPLING_RATES 1, 5, 10, 50, 100, 200

#define DSP_NPU_CAPABILITY_0 "npu"
#define DSP_NPU_CAPABILITY_0_METRIC_COUNT 4

/**
 * @enum DspNpuCapability0MetricIndex
 * @brief Array indices for NPU Capability 0 metrics
 *
 * This enum defines the array indices for accessing metrics in the
 * metrics_data array for NPU Capability 0, providing better code readability.
 */
enum DspNpuCapability0MetricIndex {
    DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_UTILIZATION = 0,
    DSP_NPU_CAPABILITY_0_METRIC_INDEX_Q6_CLOCK,
    DSP_NPU_CAPABILITY_0_METRIC_INDEX_HVX_UTILIZATION,
    DSP_NPU_CAPABILITY_0_METRIC_INDEX_HMX_UTILIZATION,
};

// NPU Metric Definitions
#define DSP_NPU_METRIC_Q6_UTILIZATION_ID 0
#define DSP_NPU_METRIC_Q6_UTILIZATION_NAME "Q6 Utilization"
#define DSP_NPU_METRIC_Q6_UTILIZATION_DESCRIPTION "Average effective Q6 clock utilization with respect to max Q6 clock"
#define DSP_NPU_METRIC_Q6_UTILIZATION_UNIT "%"

#define DSP_NPU_METRIC_Q6_CLOCK_ID 1
#define DSP_NPU_METRIC_Q6_CLOCK_NAME "Q6 Clock"
#define DSP_NPU_METRIC_Q6_CLOCK_DESCRIPTION "Average Q6 clock frequency"
#define DSP_NPU_METRIC_Q6_CLOCK_UNIT "KHz"

#define DSP_NPU_METRIC_HVX_UTILIZATION_ID 2
#define DSP_NPU_METRIC_HVX_UTILIZATION_NAME "HVX Utilization"
#define DSP_NPU_METRIC_HVX_UTILIZATION_DESCRIPTION "Average HVX utilization with respect to max Q6 clock"
#define DSP_NPU_METRIC_HVX_UTILIZATION_UNIT "%"

#define DSP_NPU_METRIC_HMX_UTILIZATION_ID 3
#define DSP_NPU_METRIC_HMX_UTILIZATION_NAME "HMX Utilization"
#define DSP_NPU_METRIC_HMX_UTILIZATION_DESCRIPTION "Average HMX utilization with respect to max Q6 clock"
#define DSP_NPU_METRIC_HMX_UTILIZATION_UNIT "%"

/**
 * @brief Initialize NPU Capability 0 metrics data
 *
 * Populates the provided metrics_data array with metric information for
 * NPU Capability 0, including metric IDs, names, descriptions, and units.
 *
 * @param[out] metrics_data Array to be populated with NPU Capability 0 metric information
 *                          (must have space for DSP_NPU_CAPABILITY_0_METRIC_COUNT entries)
 */
void dsp_npu_capability_0_init_metrics(struct QcPerfMetricInfo *metrics_data);

#endif /* QCOM_DSP_NPU_INFO_H */
