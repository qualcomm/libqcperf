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
 * @file linux_thermal_info.c
 * @brief Metric initialization implementation for the Linux Thermal backend
 * @author Vijay Kumbhani (vkumbhan@qti.qualcomm.com)
 *
 * This file implements the initialization function that populates metric
 * information structures with the static definitions for the Linux Thermal
 * backend's temperature and cooling monitoring capability.
 *
 * The backend exports one temperature metric per logical thermal domain (16
 * domains) followed by one cooling metric per logical cooling domain (12
 * domains). Metric IDs are contiguous compile-time constants (0..27) defined by
 * the *_TEMP_ID / *_COOLING_ID macros in linux_thermal_info.h, so each metric's
 * id also serves as its slot index in the metrics_data array. This
 * implementation is table driven: g_metric_defs[] lists every metric's id,
 * name, description, and unit, and the metadata is copied into the caller's
 * array in a single pass.
 */

#include <stdio.h>
#include <string.h>

#include "linux_thermal_info.h"
#include "linux_thermal_logger.h"
#include "qcperf_common.h"

/**
 * @struct LinuxThermalMetricDef
 * @brief Static definition of a single metric's metadata
 */
struct LinuxThermalMetricDef {
    uint16_t metric_id;     /**< Metric id (also the metrics_data[] slot index) */
    const char* name;       /**< Human-readable metric name */
    const char* description;/**< Detailed metric description */
    const char* unit;       /**< Unit of measurement */
};

/**
 * @brief Static table of every Linux Thermal metric, ordered by metric id
 *
 * Entries 0..15 are the temperature metrics (one per thermal domain) and
 * entries 16..27 are the cooling metrics (one per cooling domain). The table
 * index matches the metric id defined in linux_thermal_info.h.
 */
static const struct LinuxThermalMetricDef g_metric_defs[LINUX_THERMAL_CAPABILITY_METRIC_COUNT] = {
    /* Temperature metrics */
    {LINUX_THERMAL_METRIC_CPU_TEMP_ID, LINUX_THERMAL_METRIC_CPU_TEMP_NAME, LINUX_THERMAL_METRIC_CPU_TEMP_DESCRIPTION, LINUX_THERMAL_METRIC_CPU_TEMP_UNIT},
    {LINUX_THERMAL_METRIC_GPU_TEMP_ID, LINUX_THERMAL_METRIC_GPU_TEMP_NAME, LINUX_THERMAL_METRIC_GPU_TEMP_DESCRIPTION, LINUX_THERMAL_METRIC_GPU_TEMP_UNIT},
    {LINUX_THERMAL_METRIC_NPU_TEMP_ID, LINUX_THERMAL_METRIC_NPU_TEMP_NAME, LINUX_THERMAL_METRIC_NPU_TEMP_DESCRIPTION, LINUX_THERMAL_METRIC_NPU_TEMP_UNIT},
    {LINUX_THERMAL_METRIC_DDR_TEMP_ID, LINUX_THERMAL_METRIC_DDR_TEMP_NAME, LINUX_THERMAL_METRIC_DDR_TEMP_DESCRIPTION, LINUX_THERMAL_METRIC_DDR_TEMP_UNIT},
    {LINUX_THERMAL_METRIC_MODEM_TEMP_ID, LINUX_THERMAL_METRIC_MODEM_TEMP_NAME, LINUX_THERMAL_METRIC_MODEM_TEMP_DESCRIPTION, LINUX_THERMAL_METRIC_MODEM_TEMP_UNIT},
    {LINUX_THERMAL_METRIC_QMX_TEMP_ID, LINUX_THERMAL_METRIC_QMX_TEMP_NAME, LINUX_THERMAL_METRIC_QMX_TEMP_DESCRIPTION, LINUX_THERMAL_METRIC_QMX_TEMP_UNIT},
    {LINUX_THERMAL_METRIC_VIDEO_TEMP_ID, LINUX_THERMAL_METRIC_VIDEO_TEMP_NAME, LINUX_THERMAL_METRIC_VIDEO_TEMP_DESCRIPTION, LINUX_THERMAL_METRIC_VIDEO_TEMP_UNIT},
    {LINUX_THERMAL_METRIC_CAMERA_TEMP_ID, LINUX_THERMAL_METRIC_CAMERA_TEMP_NAME, LINUX_THERMAL_METRIC_CAMERA_TEMP_DESCRIPTION, LINUX_THERMAL_METRIC_CAMERA_TEMP_UNIT},
    {LINUX_THERMAL_METRIC_WIRELESS_TEMP_ID, LINUX_THERMAL_METRIC_WIRELESS_TEMP_NAME, LINUX_THERMAL_METRIC_WIRELESS_TEMP_DESCRIPTION, LINUX_THERMAL_METRIC_WIRELESS_TEMP_UNIT},
    {LINUX_THERMAL_METRIC_USB_TEMP_ID, LINUX_THERMAL_METRIC_USB_TEMP_NAME, LINUX_THERMAL_METRIC_USB_TEMP_DESCRIPTION, LINUX_THERMAL_METRIC_USB_TEMP_UNIT},
    {LINUX_THERMAL_METRIC_RF_SDR_TEMP_ID, LINUX_THERMAL_METRIC_RF_SDR_TEMP_NAME, LINUX_THERMAL_METRIC_RF_SDR_TEMP_DESCRIPTION, LINUX_THERMAL_METRIC_RF_SDR_TEMP_UNIT},
    {LINUX_THERMAL_METRIC_MULTIMEDIA_TEMP_ID, LINUX_THERMAL_METRIC_MULTIMEDIA_TEMP_NAME, LINUX_THERMAL_METRIC_MULTIMEDIA_TEMP_DESCRIPTION, LINUX_THERMAL_METRIC_MULTIMEDIA_TEMP_UNIT},
    {LINUX_THERMAL_METRIC_PMIC_TEMP_ID, LINUX_THERMAL_METRIC_PMIC_TEMP_NAME, LINUX_THERMAL_METRIC_PMIC_TEMP_DESCRIPTION, LINUX_THERMAL_METRIC_PMIC_TEMP_UNIT},
    {LINUX_THERMAL_METRIC_SYSTEM_TEMP_ID, LINUX_THERMAL_METRIC_SYSTEM_TEMP_NAME, LINUX_THERMAL_METRIC_SYSTEM_TEMP_DESCRIPTION, LINUX_THERMAL_METRIC_SYSTEM_TEMP_UNIT},
    {LINUX_THERMAL_METRIC_BATTERY_TEMP_ID, LINUX_THERMAL_METRIC_BATTERY_TEMP_NAME, LINUX_THERMAL_METRIC_BATTERY_TEMP_DESCRIPTION, LINUX_THERMAL_METRIC_BATTERY_TEMP_UNIT},
    {LINUX_THERMAL_METRIC_SOC_TEMP_ID, LINUX_THERMAL_METRIC_SOC_TEMP_NAME, LINUX_THERMAL_METRIC_SOC_TEMP_DESCRIPTION, LINUX_THERMAL_METRIC_SOC_TEMP_UNIT},

    /* Cooling metrics */
    {LINUX_THERMAL_METRIC_CPU_COOLING_ID, LINUX_THERMAL_METRIC_CPU_COOLING_NAME, LINUX_THERMAL_METRIC_CPU_COOLING_DESCRIPTION, LINUX_THERMAL_METRIC_CPU_COOLING_UNIT},
    {LINUX_THERMAL_METRIC_DDR_COOLING_ID, LINUX_THERMAL_METRIC_DDR_COOLING_NAME, LINUX_THERMAL_METRIC_DDR_COOLING_DESCRIPTION, LINUX_THERMAL_METRIC_DDR_COOLING_UNIT},
    {LINUX_THERMAL_METRIC_GPU_COOLING_ID, LINUX_THERMAL_METRIC_GPU_COOLING_NAME, LINUX_THERMAL_METRIC_GPU_COOLING_DESCRIPTION, LINUX_THERMAL_METRIC_GPU_COOLING_UNIT},
    {LINUX_THERMAL_METRIC_NPU_COOLING_ID, LINUX_THERMAL_METRIC_NPU_COOLING_NAME, LINUX_THERMAL_METRIC_NPU_COOLING_DESCRIPTION, LINUX_THERMAL_METRIC_NPU_COOLING_UNIT},
    {LINUX_THERMAL_METRIC_DISPLAY_COOLING_ID, LINUX_THERMAL_METRIC_DISPLAY_COOLING_NAME, LINUX_THERMAL_METRIC_DISPLAY_COOLING_DESCRIPTION, LINUX_THERMAL_METRIC_DISPLAY_COOLING_UNIT},
    {LINUX_THERMAL_METRIC_STORAGE_COOLING_ID, LINUX_THERMAL_METRIC_STORAGE_COOLING_NAME, LINUX_THERMAL_METRIC_STORAGE_COOLING_DESCRIPTION, LINUX_THERMAL_METRIC_STORAGE_COOLING_UNIT},
    {LINUX_THERMAL_METRIC_BATTERY_COOLING_ID, LINUX_THERMAL_METRIC_BATTERY_COOLING_NAME, LINUX_THERMAL_METRIC_BATTERY_COOLING_DESCRIPTION, LINUX_THERMAL_METRIC_BATTERY_COOLING_UNIT},
    {LINUX_THERMAL_METRIC_MODEM_COOLING_ID, LINUX_THERMAL_METRIC_MODEM_COOLING_NAME, LINUX_THERMAL_METRIC_MODEM_COOLING_DESCRIPTION, LINUX_THERMAL_METRIC_MODEM_COOLING_UNIT},
    {LINUX_THERMAL_METRIC_RF_SDR_COOLING_ID, LINUX_THERMAL_METRIC_RF_SDR_COOLING_NAME, LINUX_THERMAL_METRIC_RF_SDR_COOLING_DESCRIPTION, LINUX_THERMAL_METRIC_RF_SDR_COOLING_UNIT},
    {LINUX_THERMAL_METRIC_MULTIMEDIA_COOLING_ID, LINUX_THERMAL_METRIC_MULTIMEDIA_COOLING_NAME, LINUX_THERMAL_METRIC_MULTIMEDIA_COOLING_DESCRIPTION, LINUX_THERMAL_METRIC_MULTIMEDIA_COOLING_UNIT},
    {LINUX_THERMAL_METRIC_DSDS_COOLING_ID, LINUX_THERMAL_METRIC_DSDS_COOLING_NAME, LINUX_THERMAL_METRIC_DSDS_COOLING_DESCRIPTION, LINUX_THERMAL_METRIC_DSDS_COOLING_UNIT},
    {LINUX_THERMAL_METRIC_THERMAL_FW_COOLING_ID, LINUX_THERMAL_METRIC_THERMAL_FW_COOLING_NAME, LINUX_THERMAL_METRIC_THERMAL_FW_COOLING_DESCRIPTION, LINUX_THERMAL_METRIC_THERMAL_FW_COOLING_UNIT},
};

/**
 * @brief Copy a single metric definition into a QcPerfMetricInfo entry
 *
 * Populates the metric id, name, description, and unit (with their respective
 * lengths) from the static definition, truncating to the framework's maximum
 * field sizes if necessary.
 *
 * @param[out] dst Destination metric info entry to populate
 * @param[in]  def Static metric definition to copy from
 */
static void linux_thermal_populate_metric(struct QcPerfMetricInfo* dst, const struct LinuxThermalMetricDef* def) {
    dst->metric_id = def->metric_id;

    snprintf(dst->metric_name, METRIC_NAME_MAX_LEN, "%s", def->name);
    dst->metric_name_len = strlen(dst->metric_name);

    snprintf(dst->metric_description, MAX_METRIC_DESCRIPTION_LEN, "%s", def->description);
    dst->metric_description_len = strlen(dst->metric_description);

    snprintf(dst->metric_unit, MAX_METRIC_UNIT_LEN, "%s", def->unit);
    dst->metric_unit_len = strlen(dst->metric_unit);
}

/**
 * @brief Initialize the Linux Thermal metrics
 *
 * Populates the provided metrics_data array with the metadata for all
 * temperature and cooling metrics defined in linux_thermal_info.h, and reports
 * the number of metrics initialized.
 *
 * @param[out] metrics_data    Array to be populated with metric information
 *                             (must have space for LINUX_THERMAL_CAPABILITY_METRIC_COUNT entries)
 * @param[out] metric_data_len Populated with the number of metrics initialized
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if either argument is NULL
 */
enum QcPerfReturnCode linux_thermal_init_metrics(struct QcPerfMetricInfo* metrics_data, uint8_t* metric_data_len) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == metrics_data || NULL == metric_data_len) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Linux Thermal metric init received NULL pointer");
        return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        for (uint8_t i = 0; i < LINUX_THERMAL_CAPABILITY_METRIC_COUNT; i++) {
            linux_thermal_populate_metric(&metrics_data[i], &g_metric_defs[i]);
        }
        *metric_data_len = LINUX_THERMAL_CAPABILITY_METRIC_COUNT;
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_DEBUG, "Initialized %d Linux Thermal metrics (%d temperature, %d cooling)",
                     LINUX_THERMAL_CAPABILITY_METRIC_COUNT, LINUX_THERMAL_TEMP_METRIC_COUNT, LINUX_THERMAL_COOLING_METRIC_COUNT);
    }

    return return_code;
}

/**
 * @brief Initialize only the metrics for subsystems present on the platform
 *
 * Copies metadata for the metrics whose id is flagged in present[], skipping
 * domains that have no sensors. The output is packed (no gaps), so
 * *metric_data_len reflects the number of enabled metrics.
 *
 * @param[out] metrics_data    Array to populate (space for LINUX_THERMAL_CAPABILITY_METRIC_COUNT)
 * @param[out] metric_data_len Populated with the number of enabled metrics
 * @param[in]  present         Flags indexed by metric id (0..COUNT-1); true = enable
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if any argument is NULL
 */
enum QcPerfReturnCode linux_thermal_init_available_metrics(struct QcPerfMetricInfo* metrics_data, uint8_t* metric_data_len, const bool* present) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_SUCCESS;
    uint8_t count                     = 0;

    if (NULL == metrics_data || NULL == metric_data_len || NULL == present) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Linux Thermal available-metric init received NULL pointer");
        return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        // g_metric_defs is ordered by metric id, so index i == metric id i.
        for (uint8_t i = 0; i < LINUX_THERMAL_CAPABILITY_METRIC_COUNT; i++) {
            if (present[i]) {
                linux_thermal_populate_metric(&metrics_data[count], &g_metric_defs[i]);
                count++;
            }
        }
        *metric_data_len = count;
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_DEBUG, "Enabled %u of %d Linux Thermal metrics for available subsystems",
                     count, LINUX_THERMAL_CAPABILITY_METRIC_COUNT);
    }

    return return_code;
}
