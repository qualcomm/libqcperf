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
 * @file qcom_linux_gpu_utils.h
 * @brief Sysfs readers for the qcom-linux-gpu backend
 *
 * The Adreno kgsl driver exposes GPU state under /sys/class/kgsl/kgsl-3d0.
 * Every reader takes the sysfs directory as a parameter instead of
 * hard-coding it.
 */

#ifndef QCOM_LINUX_GPU_UTILS_H
#define QCOM_LINUX_GPU_UTILS_H

#include "qcperf_common.h"

/** Default kgsl sysfs directory for the GPU */
#define QCOM_LINUX_GPU_SYSFS_ROOT "/sys/class/kgsl/kgsl-3d0"

/** File holding GPU busy time as "<N> %" */
#define QCOM_LINUX_GPU_BUSY_NODE "gpu_busy_percentage"

/** File holding the current GPU clock in Hz */
#define QCOM_LINUX_GPU_CLOCK_NODE "gpuclk"

/**
 * @brief Read GPU utilization
 * @param[in]  sysfs_root kgsl sysfs directory
 * @param[out] percent    utilization in percent (0-100)
 * @return QC_PERF_RETURN_CODE_SUCCESS, or QC_PERF_RETURN_CODE_FAILED if the
 *         node is missing or its content is not a number
 */
enum QcPerfReturnCode qcom_linux_gpu_util_read_utilization(const char* sysfs_root, double* percent);

/**
 * @brief Read the current GPU clock
 * @param[in]  sysfs_root kgsl sysfs directory
 * @param[out] mhz        clock in MHz
 * @return QC_PERF_RETURN_CODE_SUCCESS, or QC_PERF_RETURN_CODE_FAILED if the
 *         node is missing or its content is not a number
 */
enum QcPerfReturnCode qcom_linux_gpu_util_read_clock_mhz(const char* sysfs_root, double* mhz);

#endif /* QCOM_LINUX_GPU_UTILS_H */
