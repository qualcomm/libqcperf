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
 * @file qcperf_backend_enum.h
 * @brief Backend identifier enumeration for QcPerf library
 * @author Skand Gupta (skangupt@qti.qualcomm.com)
 *
 * This header file defines the enumeration of backend identifiers used throughout
 * the QcPerf performance monitoring library. These identifiers are used to specify
 * which backend implementation to connect to, query capabilities from, or perform
 * operations with.
 *
 * The QcPerf library supports multiple backend implementations, each providing
 * different performance monitoring capabilities. This enumeration allows the
 * application to select the appropriate backend for its monitoring needs.
 *
 * @see qcperf_connect_backend
 * @see qcperf_disconnect_backend
 * @see qcperf_get_capabilities_info
 */

#ifndef QCPERF_BACKEND_ENUM_H
#define QCPERF_BACKEND_ENUM_H

/**
 * @enum QcPerfBackendId
 * @brief Identifiers for available performance monitoring backends
 *
 * Enumeration of backend implementations available in the QcPerf library.
 * Each backend provides specific performance monitoring capabilities.
 *
 * @note All enum values are always defined regardless of which backends are
 *       compiled. Whether a backend is actually available at runtime is
 *       controlled by the QCPERF_ENABLED_* compile definitions set via the
 *       BACKENDS CMake option. Attempting to connect to a backend that was
 *       not compiled in will return QC_PERF_RETURN_CODE_INVALID_BACKEND_ID.
 */
enum QcPerfBackendId {
    QC_PERF_BACKEND_DUMMY = 0,      /**< Dummy backend for testing purposes */
    QC_PERF_BACKEND_QCOM_LINUX_CPU, /**< Qualcomm Linux CPU monitoring backend (Linux ARM64) */
    QC_PERF_BACKEND_DSP_NPU,        /**< Qualcomm DSP NPU monitoring backend (Linux ARM64) */
    QC_PERF_BACKEND_POWER,          /**< Power monitoring backend (Windows ARM64) */
    QC_PERF_BACKEND_THERMAL,        /**< Thermal monitoring backend (Windows ARM64) */
    QC_PERF_BACKEND_WOS_CPU,        /**< CPU monitoring backend (Windows ARM64) */
    QC_PERF_BACKEND_MAX,            /**< Sentinel value — total number of backend slots */
};

#endif /* QCPERF_BACKEND_ENUM_H */
