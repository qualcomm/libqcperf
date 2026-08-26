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
 * @file qcperf_backends.h
 * @brief Unified backend interface for QcPerf
 * @author Skand Gupta (skangupt@qti.qualcomm.com)
 *
 * This file defines the backend initialization function pointer type and
 * maintains a registry of all available backend initialization functions.
 * It serves as a unified interface between the core library and all backend
 * implementations, allowing the core to remain decoupled from specific backends.
 *
 * Each entry in backend_init_fns[] corresponds to a QcPerfBackendId enum value.
 * Backends that are not compiled in (i.e. their QCPERF_ENABLED_* definition is
 * absent) have a NULL entry. The core checks for NULL before invoking an entry
 * and returns QC_PERF_RETURN_CODE_INVALID_BACKEND_ID for disabled backends.
 *
 * To add a new backend:
 *   1. Add its QCPERF_ENABLED_<NAME> guard and #include below.
 *   2. Add a corresponding NULL / &init_fn entry in backend_init_fns[].
 *   3. Add its name to QCPERF_PLATFORM_SUPPORTED_BACKENDS in cmake/BuildConfig.cmake.
 */

#ifndef QC_PERF_BACKENDS_H
#define QC_PERF_BACKENDS_H

#if defined(QCPERF_ENABLED_DUMMY)
#include "dummy.h"
#endif

#if defined(QCPERF_ENABLED_QCOM_LINUX_CPU)
#include "qcom_linux_cpu.h"
#endif

#if defined(QCPERF_ENABLED_QCOM_LINUX_NPU) || defined(QCPERF_ENABLED_WOS_NPU)
#include "qcom_dsp_npu.h"
#endif

#if defined(QCPERF_ENABLED_WOS_POWER)
#include "wos_power_backend.h"
#endif

#if defined(QCPERF_ENABLED_WOS_THERMAL)
#include "wos_thermal.h"
#endif

#if defined(QCPERF_ENABLED_WOS_CPU)
#include "wos_cpu.h"
#endif

/**
 * @typedef backend_init_t
 * @brief Function pointer type for backend initialization functions
 *
 * This typedef defines the signature for backend initialization functions.
 * Each backend must provide an initialization function matching this signature.
 *
 * @param[in,out] backend_private Pointer to the backend private data structure
 * @return QcPerfReturnCode indicating success or failure of initialization
 */
typedef enum QcPerfReturnCode (*backend_init_t)(struct QcPerfBackendPrivate*);

/**
 * @brief Array of backend initialization function pointers
 *
 * This static array contains pointers to all backend initialization functions,
 * indexed by QcPerfBackendId. Entries for backends that are not compiled in
 * are set to NULL. The core checks for NULL before invoking an entry.
 *
 */
static backend_init_t backend_init_fns[] = {
#if defined(QCPERF_ENABLED_DUMMY)
    &qcperf_dummy_create,           /* QC_PERF_BACKEND_DUMMY */
#else
    NULL,                           /* QC_PERF_BACKEND_DUMMY (disabled) */
#endif
#if defined(QCPERF_ENABLED_QCOM_LINUX_CPU)
    &qcperf_qcom_linux_cpu_create,  /* QC_PERF_BACKEND_QCOM_LINUX_CPU */
#else
    NULL,                           /* QC_PERF_BACKEND_QCOM_LINUX_CPU (disabled) */
#endif
#if defined(QCPERF_ENABLED_QCOM_LINUX_NPU) || defined(QCPERF_ENABLED_WOS_NPU)
    &qcperf_dsp_npu_create,         /* QC_PERF_BACKEND_DSP_NPU */
#else
    NULL,                           /* QC_PERF_BACKEND_DSP_NPU (disabled) */
#endif
#if defined(QCPERF_ENABLED_WOS_POWER)
    &wos_power_backend_create,      /* QC_PERF_BACKEND_POWER */
#else
    NULL,                           /* QC_PERF_BACKEND_POWER (disabled) */
#endif
#if defined(QCPERF_ENABLED_WOS_THERMAL)
    &qcperf_wos_thermal_backend_create, /* QC_PERF_BACKEND_THERMAL */
#else
    NULL,                           /* QC_PERF_BACKEND_THERMAL (disabled) */
#endif
#if defined(QCPERF_ENABLED_WOS_CPU)
    &wos_cpu_create,                /* QC_PERF_BACKEND_WOS_CPU */
#else
    NULL,                           /* QC_PERF_BACKEND_WOS_CPU (disabled) */
#endif
};

#endif  // QC_PERF_BACKENDS_H
