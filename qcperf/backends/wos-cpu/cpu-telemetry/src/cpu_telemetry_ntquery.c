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
 * @file cpu_telemetry_ntquery.c
 * @brief CPU telemetry via NtQuerySystemInformation and CallNtPowerInformation
 * @author Vijay Kumbhani (vkumbhan@qti.qualcomm.com)
 *
 * This implementation uses NtQuerySystemInformation to read per-core
 * idle/kernel/user times for utilization calculation, and
 * CallNtPowerInformation(ProcessorInformation) for per-core
 * CurrentMhz and MaxMhz values.
 */

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif

#ifndef NTDDI_VERSION
#define NTDDI_VERSION 0x06010000
#endif

#include <windows.h>
#include <powrprof.h>
#include <string.h>

#include "cpu_telemetry.h"

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#endif

#define SystemProcessorPerformanceInformation 8

typedef LONG(WINAPI *NtQuerySystemInformation_t)(
    ULONG SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength);

#ifndef PROCESSOR_POWER_INFORMATION_DEFINED
#define PROCESSOR_POWER_INFORMATION_DEFINED
typedef struct _PROCESSOR_POWER_INFORMATION {
    ULONG Number;
    ULONG MaxMhz;
    ULONG CurrentMhz;
    ULONG MhzLimit;
    ULONG MaxIdleState;
    ULONG CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;
#endif

typedef struct _SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION {
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER Reserved1[2];
    ULONG Reserved2;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;

static NtQuerySystemInformation_t g_nt_query                     = NULL;
static uint32_t g_num_cpus                                       = 0;
static bool g_ntquery_initialized                                = false;
static uint64_t g_prev_idle[CPU_TELEMETRY_MAX_CORES]             = {0};
static uint64_t g_prev_kernel[CPU_TELEMETRY_MAX_CORES]           = {0};
static uint64_t g_prev_user[CPU_TELEMETRY_MAX_CORES]             = {0};

enum eCpuTelemetryReturnCode cpuTelemetryNtQuery_init(void) {
    enum eCpuTelemetryReturnCode return_code = RETURN_CODE_CPU_TELEMETRY_SUCCESS;
    HMODULE h_ntdll                                 = NULL;
    SYSTEM_INFO sys_info                            = {0};

    if (true == g_ntquery_initialized) {
        return_code = RETURN_CODE_CPU_TELEMETRY_SUCCESS;
    } else {
        h_ntdll = GetModuleHandleW(L"ntdll.dll");
        if (NULL == h_ntdll) {
            return_code = RETURN_CODE_CPU_TELEMETRY_FAILED;
        } else {
            g_nt_query = (NtQuerySystemInformation_t)GetProcAddress(h_ntdll, "NtQuerySystemInformation");
            if (NULL == g_nt_query) {
                return_code = RETURN_CODE_CPU_TELEMETRY_FAILED;
            } else {
                GetSystemInfo(&sys_info);
                g_num_cpus = sys_info.dwNumberOfProcessors;
                if (g_num_cpus > CPU_TELEMETRY_MAX_CORES) {
                    g_num_cpus = CPU_TELEMETRY_MAX_CORES;
                }

                memset(g_prev_idle, 0, sizeof(g_prev_idle));
                memset(g_prev_kernel, 0, sizeof(g_prev_kernel));
                memset(g_prev_user, 0, sizeof(g_prev_user));

                g_ntquery_initialized = true;
            }
        }
    }

    return return_code;
}

enum eCpuTelemetryReturnCode cpuTelemetryNtQuery_getNumCores(uint32_t *num_cores) {
    enum eCpuTelemetryReturnCode return_code = RETURN_CODE_CPU_TELEMETRY_SUCCESS;
    SYSTEM_INFO sys_info                            = {0};

    if (NULL == num_cores) {
        return_code = RETURN_CODE_CPU_TELEMETRY_INVALID_ARGS;
    } else {
        if (false == g_ntquery_initialized) {
            GetSystemInfo(&sys_info);
            *num_cores = sys_info.dwNumberOfProcessors;
            if (*num_cores > CPU_TELEMETRY_MAX_CORES) {
                *num_cores = CPU_TELEMETRY_MAX_CORES;
            }
        } else {
            *num_cores = g_num_cpus;
        }
    }

    return return_code;
}

enum eCpuTelemetryReturnCode cpuTelemetryNtQuery_collect(struct CpuMetrics *metrics) {
    enum eCpuTelemetryReturnCode return_code                              = RETURN_CODE_CPU_TELEMETRY_SUCCESS;
    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION perf[CPU_TELEMETRY_MAX_CORES] = {0};
    PROCESSOR_POWER_INFORMATION power[CPU_TELEMETRY_MAX_CORES]           = {0};
    ULONG return_length                                                          = 0;
    LONG status                                                                  = 0;
    uint32_t cpu                                                                 = 0;
    uint64_t total_idle_diff                                                     = 0;
    uint64_t total_kernel_diff                                                   = 0;
    uint64_t total_user_diff                                                     = 0;
    uint64_t total_time                                                          = 0;
    uint64_t idle                                                                = 0;
    uint64_t kernel                                                              = 0;
    uint64_t user                                                                = 0;
    double util                                                                  = 0.0;
    uint64_t idle_diff                                                           = 0;
    uint64_t kernel_diff                                                         = 0;
    uint64_t user_diff                                                           = 0;
    uint64_t total_diff                                                          = 0;

    if (NULL == metrics) {
        return_code = RETURN_CODE_CPU_TELEMETRY_INVALID_ARGS;
    } else if (false == g_ntquery_initialized) {
        return_code = RETURN_CODE_CPU_TELEMETRY_NOT_INITIALIZED;
    } else {
        memset(metrics, 0, sizeof(struct CpuMetrics));
        metrics->num_cores = g_num_cpus;

        status = g_nt_query(
            SystemProcessorPerformanceInformation,
            perf,
            (ULONG)(g_num_cpus * sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION)),
            &return_length);

        if (0 != status) {
            return_code = RETURN_CODE_CPU_TELEMETRY_FAILED;
        } else {
            status = (LONG)CallNtPowerInformation(
                ProcessorInformation,
                NULL,
                0,
                power,
                (ULONG)(g_num_cpus * sizeof(PROCESSOR_POWER_INFORMATION)));

            if (STATUS_SUCCESS != status) {
                return_code = RETURN_CODE_CPU_TELEMETRY_FAILED;
            }
        }

        if (RETURN_CODE_CPU_TELEMETRY_SUCCESS == return_code) {
            for (cpu = 0; cpu < g_num_cpus; cpu++) {
                idle   = (uint64_t)perf[cpu].IdleTime.QuadPart;
                kernel = (uint64_t)perf[cpu].KernelTime.QuadPart;
                user   = (uint64_t)perf[cpu].UserTime.QuadPart;
                util   = 0.0;

                if (0 != g_prev_kernel[cpu]) {
                    idle_diff   = idle - g_prev_idle[cpu];
                    kernel_diff = kernel - g_prev_kernel[cpu];
                    user_diff   = user - g_prev_user[cpu];
                    total_diff  = kernel_diff + user_diff;

                    if (total_diff > 0) {
                        util = 100.0 * (double)(total_diff - idle_diff) / (double)total_diff;
                    }

                    total_idle_diff   += idle_diff;
                    total_kernel_diff += kernel_diff;
                    total_user_diff   += user_diff;
                }

                g_prev_idle[cpu]   = idle;
                g_prev_kernel[cpu] = kernel;
                g_prev_user[cpu]   = user;

                metrics->cores[cpu].id     = cpu;
                metrics->cores[cpu].utilization = util;
                metrics->cores[cpu].frequency = power[cpu].CurrentMhz;
                metrics->cores[cpu].maximum_frequency = power[cpu].MaxMhz;

                metrics->total_frequency += (double)power[cpu].CurrentMhz;
            }

            /* Total CPU utilization */
            total_time = total_kernel_diff + total_user_diff;
            if (total_time > 0) {
                metrics->total_utilization = 100.0 * (double)(total_time - total_idle_diff) / (double)total_time;
            }

            /* Average frequency across all cores */
            if (g_num_cpus > 0) {
                metrics->total_frequency = metrics->total_frequency / g_num_cpus;
            }
        }
    }

    return return_code;
}

enum eCpuTelemetryReturnCode cpuTelemetryNtQuery_destroy(void) {
    if (false == g_ntquery_initialized) {
        return RETURN_CODE_CPU_TELEMETRY_SUCCESS;
    }

    g_nt_query    = NULL;
    g_num_cpus    = 0;
    memset(g_prev_idle, 0, sizeof(g_prev_idle));
    memset(g_prev_kernel, 0, sizeof(g_prev_kernel));
    memset(g_prev_user, 0, sizeof(g_prev_user));
    g_ntquery_initialized = false;

    return RETURN_CODE_CPU_TELEMETRY_SUCCESS;
}
