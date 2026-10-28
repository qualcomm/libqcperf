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
 * @file qcom_linux_gpu_utils.c
 * @brief Sysfs reader implementations for the qcom-linux-gpu backend
 */

#include <stdio.h>

#include "qcom_linux_gpu_utils.h"

static enum QcPerfReturnCode read_sysfs_value(const char* sysfs_root, const char* node, double* value) {
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_FAILED;
    char path[256];
    FILE* fp = NULL;

    if (NULL == sysfs_root || NULL == value || NULL == node) {
        ret = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else if (snprintf(path, sizeof(path), "%s/%s", sysfs_root, node) < (int)sizeof(path)) {
        fp = fopen(path, "r");
        if (NULL != fp) {
            /* Trailing text such as the " %" in gpu_busy_percentage is ignored. */
            if (1 == fscanf(fp, "%lf", value)) {
                ret = QC_PERF_RETURN_CODE_SUCCESS;
            }
            fclose(fp);
        }
    }

    return ret;
}

enum QcPerfReturnCode qcom_linux_gpu_util_read_utilization(const char* sysfs_root, double* percent) { return read_sysfs_value(sysfs_root, QCOM_LINUX_GPU_BUSY_NODE, percent); }

enum QcPerfReturnCode qcom_linux_gpu_util_read_clock_mhz(const char* sysfs_root, double* mhz) {
    double hz                 = 0.0;
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_NULL_POINTER;

    if (NULL != mhz) {
        ret = read_sysfs_value(sysfs_root, QCOM_LINUX_GPU_CLOCK_NODE, &hz);
        if (QC_PERF_RETURN_CODE_SUCCESS == ret) {
            *mhz = hz / 1000000.0;
        }
    }

    return ret;
}
