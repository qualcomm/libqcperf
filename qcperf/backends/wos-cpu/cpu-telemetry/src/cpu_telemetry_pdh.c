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
 * @file cpu_telemetry_pdh.c
 * @brief CPU telemetry via Windows Performance Data Helper (PDH) APIs
 * @author Vijay Kumbhani (vkumbhan@qti.qualcomm.com)
 *
 * This implementation uses PDH counters (Processor Information / Processor)
 * to collect per-core CPU utilization and frequency metrics.
 */

#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <stdio.h>
#include <string.h>

#include "cpu_telemetry.h"

static PDH_HQUERY g_query                                    = NULL;
static PDH_HCOUNTER g_util_counters[CPU_TELEMETRY_MAX_CORES] = {0};
static PDH_HCOUNTER g_freq_counter[CPU_TELEMETRY_MAX_CORES]  = {0};
static PDH_HCOUNTER g_total_util_counter                     = NULL;
static uint32_t g_num_cores                                  = 0;
static bool g_initialized                                    = false;

enum eCpuTelemetryReturnCode cpuTelemetryPdh_init(void) {
    enum eCpuTelemetryReturnCode return_code = RETURN_CODE_CPU_TELEMETRY_SUCCESS;
    PDH_STATUS status                        = ERROR_SUCCESS;
    SYSTEM_INFO sys_info                     = {0};
    char counter_path[128]                   = {0};
    uint32_t i                               = 0;

    if (true == g_initialized) {
        return_code = RETURN_CODE_CPU_TELEMETRY_SUCCESS;
    } else {
        GetSystemInfo(&sys_info);
        g_num_cores = sys_info.dwNumberOfProcessors;
        if (g_num_cores > CPU_TELEMETRY_MAX_CORES) {
            g_num_cores = CPU_TELEMETRY_MAX_CORES;
        }

        status = PdhOpenQueryA(NULL, 0, &g_query);
        if (ERROR_SUCCESS != status) {
            return_code = RETURN_CODE_CPU_TELEMETRY_FAILED;
        } else {
            /* Add total CPU utilization counter */
            status = PdhAddEnglishCounterA(g_query, "\\Processor Information(_Total)\\% Processor Utility", 0, &g_total_util_counter);
            if (ERROR_SUCCESS != status) {
                /* Fallback to legacy counter */
                status = PdhAddEnglishCounterA(g_query, "\\Processor(_Total)\\% Processor Time", 0, &g_total_util_counter);
                if (ERROR_SUCCESS != status) {
                    PdhCloseQuery(g_query);
                    g_query     = NULL;
                    return_code = RETURN_CODE_CPU_TELEMETRY_FAILED;
                }
            }

            if (RETURN_CODE_CPU_TELEMETRY_SUCCESS == return_code) {
                /* Add per-core utilization counters */
                for (i = 0; i < g_num_cores; i++) {
                    snprintf(counter_path, sizeof(counter_path), "\\Processor Information(0,%u)\\%% Processor Utility", i);
                    status = PdhAddEnglishCounterA(g_query, counter_path, 0, &g_util_counters[i]);
                    if (ERROR_SUCCESS != status) {
                        snprintf(counter_path, sizeof(counter_path), "\\Processor(%u)\\%% Processor Time", i);
                        status = PdhAddEnglishCounterA(g_query, counter_path, 0, &g_util_counters[i]);
                        if (ERROR_SUCCESS != status) {
                            g_util_counters[i] = NULL;
                        }
                    }
                }

                /* Add per-core frequency counters */
                for (i = 0; i < g_num_cores; i++) {
                    snprintf(counter_path, sizeof(counter_path), "\\Processor Information(0,%u)\\Processor Frequency", i);
                    status = PdhAddEnglishCounterA(g_query, counter_path, 0, &g_freq_counter[i]);
                    if (ERROR_SUCCESS != status) {
                        g_freq_counter[i] = NULL;
                    }
                }

                /* Prime the counters (PDH needs two samples for rate counters) */
                PdhCollectQueryData(g_query);
                g_initialized = true;
            }
        }
    }

    return return_code;
}

enum eCpuTelemetryReturnCode cpuTelemetryPdh_getNumCores(uint32_t *num_cores) {
    enum eCpuTelemetryReturnCode return_code = RETURN_CODE_CPU_TELEMETRY_SUCCESS;
    SYSTEM_INFO sys_info                     = {0};

    if (NULL == num_cores) {
        return_code = RETURN_CODE_CPU_TELEMETRY_INVALID_ARGS;
    } else {
        if (false == g_initialized) {
            GetSystemInfo(&sys_info);
            *num_cores = sys_info.dwNumberOfProcessors;
            if (*num_cores > CPU_TELEMETRY_MAX_CORES) {
                *num_cores = CPU_TELEMETRY_MAX_CORES;
            }
        } else {
            *num_cores = g_num_cores;
        }
    }

    return return_code;
}

enum eCpuTelemetryReturnCode cpuTelemetryPdh_collect(struct CpuMetrics *info) {
    enum eCpuTelemetryReturnCode return_code = RETURN_CODE_CPU_TELEMETRY_SUCCESS;
    PDH_STATUS status                        = ERROR_SUCCESS;
    PDH_FMT_COUNTERVALUE counter_value       = {0};
    uint32_t i                               = 0;
    double freq_sum                           = 0.0;

    if (NULL == info) {
        return_code = RETURN_CODE_CPU_TELEMETRY_INVALID_ARGS;
    } else if (false == g_initialized) {
        return_code = RETURN_CODE_CPU_TELEMETRY_NOT_INITIALIZED;
    } else {
        memset(info, 0, sizeof(struct CpuMetrics));
        info->num_cores = g_num_cores;

        status = PdhCollectQueryData(g_query);
        if (ERROR_SUCCESS != status) {
            return_code = RETURN_CODE_CPU_TELEMETRY_FAILED;
        } else {
            /* Get total utilization */
            if (NULL != g_total_util_counter) {
                status = PdhGetFormattedCounterValue(g_total_util_counter, PDH_FMT_DOUBLE, NULL, &counter_value);
                if (ERROR_SUCCESS == status) {
                    info->total_utilization = counter_value.doubleValue;
                }
            }

            /* Get per-core utilization and frequency */
            for (i = 0; i < g_num_cores; i++) {
                info->cores[i].id = i;

                if (NULL != g_util_counters[i]) {
                    status = PdhGetFormattedCounterValue(g_util_counters[i], PDH_FMT_DOUBLE, NULL, &counter_value);
                    if (ERROR_SUCCESS == status) {
                        info->cores[i].utilization = counter_value.doubleValue;
                    }
                }

                if (NULL != g_freq_counter[i]) {
                    status = PdhGetFormattedCounterValue(g_freq_counter[i], PDH_FMT_DOUBLE, NULL, &counter_value);
                    if (ERROR_SUCCESS == status) {
                        info->cores[i].frequency = (uint32_t)counter_value.doubleValue;
                        freq_sum += counter_value.doubleValue;
                    }
                }
            }

            /* Average frequency */
            if (g_num_cores > 0) {
                info->total_frequency = freq_sum / g_num_cores;
            }
        }
    }

    return return_code;
}

enum eCpuTelemetryReturnCode cpuTelemetryPdh_destroy(void) {
    if (false == g_initialized) {
        return RETURN_CODE_CPU_TELEMETRY_SUCCESS;
    }

    if (NULL != g_query) {
        PdhCloseQuery(g_query);
        g_query = NULL;
    }

    memset(g_util_counters, 0, sizeof(g_util_counters));
    memset(g_freq_counter, 0, sizeof(g_freq_counter));
    g_total_util_counter = NULL;
    g_num_cores          = 0;
    g_initialized        = false;

    return RETURN_CODE_CPU_TELEMETRY_SUCCESS;
}
