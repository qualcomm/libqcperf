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
 * @file cpu_telemetry_query.h
 * @brief CPU telemetry interface for WOS CPU backend
 * @author Vijay Kumbhani (vkumbhan@qti.qualcomm.com)
 *
 * This header defines structures and interfaces for collecting CPU
 * performance metrics. Two implementations are provided:
 *   1. NtQuerySystemInformation + CallNtPowerInformation (preferred)
 *   2. Windows PDH Performance Data Helper (fallback)
 */

#ifndef CPU_TELEMETRY_NTQUERY_H
#define CPU_TELEMETRY_NTQUERY_H

#include <stdint.h>
#include <stdbool.h>

#define CPU_TELEMETRY_MAX_CORES 18

/* ============================================================================
 * Enumerations
 * ============================================================================ */

enum eCpuTelemetryReturnCode {
    RETURN_CODE_CPU_TELEMETRY_SUCCESS = 0,
    RETURN_CODE_CPU_TELEMETRY_FAILED,
    RETURN_CODE_CPU_TELEMETRY_INVALID_ARGS,
    RETURN_CODE_CPU_TELEMETRY_NOT_INITIALIZED,
};

enum eCpuTelemetrySource {
    CPU_TELEMETRY_SOURCE_NTQUERY = 0,
    CPU_TELEMETRY_SOURCE_PDH,
};

struct CpuCoreMetrics {
    uint32_t id;
    double utilization;
    uint32_t frequency;
    uint32_t maximum_frequency;
};

struct CpuMetrics {
    double total_utilization;
    double total_frequency;
    uint32_t num_cores;
    struct CpuCoreMetrics cores[CPU_TELEMETRY_MAX_CORES];
};

/* NtQuery implementation (preferred) */
enum eCpuTelemetryReturnCode cpuTelemetryNtQuery_init(void);
enum eCpuTelemetryReturnCode cpuTelemetryNtQuery_getNumCores(uint32_t *num_cores);
enum eCpuTelemetryReturnCode cpuTelemetryNtQuery_collect(struct CpuMetrics *metrics);
enum eCpuTelemetryReturnCode cpuTelemetryNtQuery_destroy(void);

/* PDH implementation (fallback) */
enum eCpuTelemetryReturnCode cpuTelemetryPdh_init(void);
enum eCpuTelemetryReturnCode cpuTelemetryPdh_getNumCores(uint32_t *num_cores);
enum eCpuTelemetryReturnCode cpuTelemetryPdh_collect(struct CpuMetrics *metrics);
enum eCpuTelemetryReturnCode cpuTelemetryPdh_destroy(void);

#endif /* CPU_TELEMETRY_H */
