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
 * @file qcom_linux_gpu_info.c
 * @brief Metric description tables for the qcom-linux-gpu backend
 */

#include <stdio.h>
#include <string.h>

#include "qcom_linux_gpu_info.h"

struct MetricDefinition {
    const char* name;
    const char* description;
    const char* unit;
};

static const struct MetricDefinition k_metrics[LINUX_GPU_CAPABILITY_0_METRIC_COUNT] = {
    [LINUX_GPU_METRIC_UTILIZATION] = {"GPU Utilization", "Percentage of time the GPU was busy", "%"},
    [LINUX_GPU_METRIC_CLOCK]       = {"GPU Clock", "Current GPU clock frequency", "MHz"},
};

void linux_gpu_capability_0_init_metrics(struct QcPerfMetricInfo* metrics_data) {
    struct QcPerfMetricInfo* m = NULL;

    for (uint32_t i = 0; i < LINUX_GPU_CAPABILITY_0_METRIC_COUNT; i++) {
        m = &metrics_data[i];

        m->metric_id = (uint16_t)i;
        snprintf(m->metric_name, METRIC_NAME_MAX_LEN, "%s", k_metrics[i].name);
        m->metric_name_len = strlen(m->metric_name);
        snprintf(m->metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", k_metrics[i].description);
        m->metric_description_len = strlen(m->metric_description);
        snprintf(m->metric_unit, MAX_METRIC_UNIT_LEN, "%s", k_metrics[i].unit);
        m->metric_unit_len = strlen(m->metric_unit);
    }
}
