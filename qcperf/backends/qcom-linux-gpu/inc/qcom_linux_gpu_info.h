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
 * @file qcom_linux_gpu_info.h
 * @brief Static metric definitions for the qcom-linux-gpu backend
 */

#ifndef QCOM_LINUX_GPU_INFO_H
#define QCOM_LINUX_GPU_INFO_H

#include "qcperf_common.h"

#define LINUX_GPU_CAPABILITY_0_ID 0
#define LINUX_GPU_CAPABILITIES_LEN 1

#define LINUX_GPU_STREAMING_RATES_LEN 10
#define LINUX_GPU_STREAMING_RATES 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000

#define LINUX_GPU_SAMPLING_RATES_LEN 6
#define LINUX_GPU_SAMPLING_RATES 1, 5, 10, 50, 100, 200

#define LINUX_GPU_CAPABILITY_0 "gpu"
#define LINUX_GPU_CAPABILITY_0_METRIC_COUNT 2

/** Metric IDs; each is also the metric's index in the capability's metric array. */
enum LinuxGpuMetricId {
    LINUX_GPU_METRIC_UTILIZATION = 0, /**< GPU utilization, % */
    LINUX_GPU_METRIC_CLOCK,           /**< Current GPU clock, MHz */
};

/**
 * @brief Populate the metric descriptions for capability 0
 *
 * @param[out] metrics_data Array with space for LINUX_GPU_CAPABILITY_0_METRIC_COUNT entries
 */
void linux_gpu_capability_0_init_metrics(struct QcPerfMetricInfo* metrics_data);

#endif /* QCOM_LINUX_GPU_INFO_H */
