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
 * @file wos_cpu_info.h
 * @brief Metric definitions for the WOS CPU backend
 * @author Vijay Kumbhani (vkumbhan@qti.qualcomm.com)
 *
 * This header defines all metric IDs, names, descriptions, units,
 * capability configuration, and streaming/sampling rates for the
 * WOS CPU backend.
 */

#ifndef WOS_CPU_INFO_H
#define WOS_CPU_INFO_H

#include "qcperf_common.h"

#define WOS_CPU_MAX_CORES 18
#define WOS_CPU_METRICS_PER_CORE 3
#define WOS_CPU_TOTAL_METRICS 2

#define WOS_CPU_CAPABILITY_ID 0
#define WOS_CPU_CAPABILITY_INDEX 0
#define WOS_CPU_CAPABILITIES_LEN 1
#define WOS_CPU_CAPABILITY_NAME "cpu"

#define WOS_CPU_STREAMING_RATES_LEN 3
#define WOS_CPU_STREAMING_RATES 200, 500, 1000

#define WOS_CPU_SAMPLING_RATES_LEN 3
#define WOS_CPU_SAMPLING_RATES 100, 200, 500

/* ============================================================================
 * Total Metrics
 * ============================================================================ */

#define WOS_CPU_TOTAL_UTIL_ID 0 /**< Total CPU utilization */
#define WOS_CPU_TOTAL_FREQ_ID 1 /**< Total (average) CPU frequency */

/* Total metric names */
#define WOS_CPU_METRIC_TOTAL_UTIL_NAME "Total CPU Utilization"
#define WOS_CPU_METRIC_TOTAL_UTIL_DESCRIPTION "Total CPU utilization averaged across all cores"
#define WOS_CPU_METRIC_TOTAL_UTIL_UNIT "%"

#define WOS_CPU_METRIC_TOTAL_FREQ_NAME "Average CPU Frequency"
#define WOS_CPU_METRIC_TOTAL_FREQ_DESCRIPTION "Average CPU frequency across all cores"
#define WOS_CPU_METRIC_TOTAL_FREQ_UNIT "MHz"

/* ============================================================================
 * Per-core metric name/description/unit format strings
 * ============================================================================ */

#define WOS_CPU_CORE_UTIL_NAME_FMT "CPU Core %u Utilization"
#define WOS_CPU_CORE_UTIL_DESC_FMT "CPU core %u utilization percentage"
#define WOS_CPU_CORE_UTIL_UNIT "%"

#define WOS_CPU_CORE_FREQ_NAME_FMT "CPU Core %u Frequency"
#define WOS_CPU_CORE_FREQ_DESC_FMT "CPU core %u current frequency"
#define WOS_CPU_CORE_FREQ_UNIT "MHz"

#define WOS_CPU_CORE_MAX_FREQ_NAME_FMT "CPU Core %u Max Frequency"
#define WOS_CPU_CORE_MAX_FREQ_DESC_FMT "CPU core %u maximum frequency"
#define WOS_CPU_CORE_MAX_FREQ_UNIT "MHz"

/* ============================================================================
 * Per-core Metric IDs
 * ============================================================================ */

/* Core 0: IDs 2-4 */
#define WOS_CPU_CORE_0_UTIL 2
#define WOS_CPU_CORE_0_FREQ 3
#define WOS_CPU_CORE_0_MAX_FREQ 4

/* Core 1: IDs 5-7 */
#define WOS_CPU_CORE_1_UTIL 5
#define WOS_CPU_CORE_1_FREQ 6
#define WOS_CPU_CORE_1_MAX_FREQ 7

/* Core 2: IDs 8-10 */
#define WOS_CPU_CORE_2_UTIL 8
#define WOS_CPU_CORE_2_FREQ 9
#define WOS_CPU_CORE_2_MAX_FREQ 10

/* Core 3: IDs 11-13 */
#define WOS_CPU_CORE_3_UTIL 11
#define WOS_CPU_CORE_3_FREQ 12
#define WOS_CPU_CORE_3_MAX_FREQ 13

/* Core 4: IDs 14-16 */
#define WOS_CPU_CORE_4_UTIL 14
#define WOS_CPU_CORE_4_FREQ 15
#define WOS_CPU_CORE_4_MAX_FREQ 16

/* Core 5: IDs 17-19 */
#define WOS_CPU_CORE_5_UTIL 17
#define WOS_CPU_CORE_5_FREQ 18
#define WOS_CPU_CORE_5_MAX_FREQ 19

/* Core 6: IDs 20-22 */
#define WOS_CPU_CORE_6_UTIL 20
#define WOS_CPU_CORE_6_FREQ 21
#define WOS_CPU_CORE_6_MAX_FREQ 22

/* Core 7: IDs 23-25 */
#define WOS_CPU_CORE_7_UTIL 23
#define WOS_CPU_CORE_7_FREQ 24
#define WOS_CPU_CORE_7_MAX_FREQ 25

/* Core 8: IDs 26-28 */
#define WOS_CPU_CORE_8_UTIL 26
#define WOS_CPU_CORE_8_FREQ 27
#define WOS_CPU_CORE_8_MAX_FREQ 28

/* Core 9: IDs 29-31 */
#define WOS_CPU_CORE_9_UTIL 29
#define WOS_CPU_CORE_9_FREQ 30
#define WOS_CPU_CORE_9_MAX_FREQ 31

/* Core 10: IDs 32-34 */
#define WOS_CPU_CORE_10_UTIL 32
#define WOS_CPU_CORE_10_FREQ 33
#define WOS_CPU_CORE_10_MAX_FREQ 34

/* Core 11: IDs 35-37 */
#define WOS_CPU_CORE_11_UTIL 35
#define WOS_CPU_CORE_11_FREQ 36
#define WOS_CPU_CORE_11_MAX_FREQ 37

/* Core 12: IDs 38-40 */
#define WOS_CPU_CORE_12_UTIL 38
#define WOS_CPU_CORE_12_FREQ 39
#define WOS_CPU_CORE_12_MAX_FREQ 40

/* Core 13: IDs 41-43 */
#define WOS_CPU_CORE_13_UTIL 41
#define WOS_CPU_CORE_13_FREQ 42
#define WOS_CPU_CORE_13_MAX_FREQ 43

/* Core 14: IDs 44-46 */
#define WOS_CPU_CORE_14_UTIL 44
#define WOS_CPU_CORE_14_FREQ 45
#define WOS_CPU_CORE_14_MAX_FREQ 46

/* Core 15: IDs 47-49 */
#define WOS_CPU_CORE_15_UTIL 47
#define WOS_CPU_CORE_15_FREQ 48
#define WOS_CPU_CORE_15_MAX_FREQ 49

/* Core 16: IDs 50-52 */
#define WOS_CPU_CORE_16_UTIL 50
#define WOS_CPU_CORE_16_FREQ 51
#define WOS_CPU_CORE_16_MAX_FREQ 52

/* Core 17: IDs 53-55 */
#define WOS_CPU_CORE_17_UTIL 53
#define WOS_CPU_CORE_17_FREQ 54
#define WOS_CPU_CORE_17_MAX_FREQ 55

/* ============================================================================
 * Per-core metric offsets
 * ============================================================================ */

#define WOS_CPU_UTIL_OFFSET 0     /**< CPU utilization */
#define WOS_CPU_FREQ_OFFSET 1     /**< CPU current frequency */
#define WOS_CPU_MAX_FREQ_OFFSET 2 /**< CPU max frequency */

/* ============================================================================
 * Per-core metric ID lookup macro
 *   WOS_CPU_CORE_BASE_ID(core_id)                              -> UTIL ID
 *   WOS_CPU_CORE_BASE_ID(core_id) + WOS_CPU_FREQ_OFFSET       -> FREQ ID
 *   WOS_CPU_CORE_BASE_ID(core_id) + WOS_CPU_MAX_FREQ_OFFSET   -> MAX_FREQ ID
 * ============================================================================ */

#define WOS_CPU_CORE_BASE_ID(core_id) ((uint16_t)(WOS_CPU_CORE_0_UTIL + (core_id) * WOS_CPU_METRICS_PER_CORE))


#endif /* WOS_CPU_INFO_H */
