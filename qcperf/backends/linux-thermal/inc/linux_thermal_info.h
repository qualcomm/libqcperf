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
 * @file linux_thermal_info.h
 * @brief Metric definitions and initialization functions for the Linux Thermal backend
 * @author Vijay Kumbhani (vkumbhan@qti.qualcomm.com)
 *
 * This header defines the capability configuration, streaming/sampling rates,
 * and metric layout for the Linux Thermal backend. The backend exports one
 * temperature metric per thermal zone discovered under /sys/class/thermal.
 * Because the set of thermal zones (and their names/types) is platform
 * dependent, metrics are populated dynamically at initialization time from the
 * zones actually present on the system.
 */

#ifndef LINUX_THERMAL_INFO_H
#define LINUX_THERMAL_INFO_H

#include <stdint.h>

#include "qcperf_common.h"

/* ============================================================================
 * Capability configuration
 * ============================================================================ */

#define LINUX_THERMAL_CAPABILITY_ID 0
#define LINUX_THERMAL_CAPABILITIES_LEN 1
#define LINUX_THERMAL_CAPABILITY "thermal"

/**
 * @def LINUX_THERMAL_MAX_ZONES
 * @brief Maximum number of raw kernel thermal zones the backend will scan
 *
 * Upper bound on the number of thermal_zoneN entries scanned under
 * /sys/class/thermal. The kernel may expose many raw zones; each is mapped by
 * its type onto one of the logical temperature domains defined below. This
 * bounds the raw-zone scan arrays only, not the exported metric count.
 */
#define LINUX_THERMAL_MAX_ZONES 64

/**
 * @def LINUX_THERMAL_TEMP_METRIC_COUNT
 * @brief Number of temperature metrics (one per logical thermal domain)
 *
 * CPU, GPU, NPU/NSP, DDR, Modem, QMX Fabric, Video, Camera, Wireless, USB,
 * RF/SDR, Multimedia, PMIC, System/Skin, Battery, SoC.
 * See the *_TEMP_ID macros below for the sysfs sensor types feeding each.
 */
#define LINUX_THERMAL_TEMP_METRIC_COUNT 16

/**
 * @def LINUX_THERMAL_COOLING_METRIC_COUNT
 * @brief Number of cooling metrics (one per logical cooling domain)
 *
 * CPU, DDR, GPU, NPU, Display, Storage, Battery, Modem, RF/SDR, Multimedia,
 * DSDS, Thermal Framework.
 * See the *_COOLING_ID macros below for the sysfs cooling devices feeding each.
 */
#define LINUX_THERMAL_COOLING_METRIC_COUNT 12

/**
 * @def LINUX_THERMAL_CAPABILITY_METRIC_COUNT
 * @brief Total number of metrics exported by the backend (temperature + cooling)
 */
#define LINUX_THERMAL_CAPABILITY_METRIC_COUNT (LINUX_THERMAL_TEMP_METRIC_COUNT + LINUX_THERMAL_COOLING_METRIC_COUNT)

/* ============================================================================
 * Streaming / sampling rates (milliseconds)
 * ============================================================================ */

#define LINUX_THERMAL_STREAMING_RATES_LEN 4
#define LINUX_THERMAL_STREAMING_RATES 200, 500, 1000, 2000

#define LINUX_THERMAL_SAMPLING_RATES_LEN 5
#define LINUX_THERMAL_SAMPLING_RATES 50, 100, 200, 500, 1000

/* ============================================================================
 * Metrics
 * ============================================================================ */

/** Unit reported for every thermal-domain temperature metric */
#define LINUX_THERMAL_TEMP_UNIT "deg C"

/*
 * Sensor mapping
 * --------------
 * Each logical domain below aggregates one or more raw kernel thermal zones
 * (matched by their thermal_zoneN/type string). The "Sensors:" comment on each
 * domain lists the sysfs zone-type patterns that feed it ('*' = wildcard).
 * The backend implementation matches each raw zone's type against these
 * patterns to assign its temperature to the corresponding domain metric.
 */

/* CPU thermal domain */
/* Sensors: cpu-* + cpullc-* */
#define LINUX_THERMAL_METRIC_CPU_TEMP_ID 0
#define LINUX_THERMAL_METRIC_CPU_TEMP_NAME "CPU Temperature"
#define LINUX_THERMAL_METRIC_CPU_TEMP_DESCRIPTION "Temperature of the CPU thermal domain (cpu-*, cpullc-*)"
#define LINUX_THERMAL_METRIC_CPU_TEMP_UNIT LINUX_THERMAL_TEMP_UNIT

/* GPU thermal domain */
/* Sensors: gpu-* + gpuss-* */
#define LINUX_THERMAL_METRIC_GPU_TEMP_ID 1
#define LINUX_THERMAL_METRIC_GPU_TEMP_NAME "GPU Temperature"
#define LINUX_THERMAL_METRIC_GPU_TEMP_DESCRIPTION "Temperature of the GPU thermal domain (gpu-*, gpuss-*)"
#define LINUX_THERMAL_METRIC_GPU_TEMP_UNIT LINUX_THERMAL_TEMP_UNIT

/* NPU / NSP thermal domain */
/* Sensors: nspvxu*, nspmxu*, nspllv*, nspq6 */
#define LINUX_THERMAL_METRIC_NPU_TEMP_ID 2
#define LINUX_THERMAL_METRIC_NPU_TEMP_NAME "NPU Temperature"
#define LINUX_THERMAL_METRIC_NPU_TEMP_DESCRIPTION "Temperature of the NPU/NSP thermal domain (nspvxu*, nspmxu*, nspllv*, nspq6)"
#define LINUX_THERMAL_METRIC_NPU_TEMP_UNIT LINUX_THERMAL_TEMP_UNIT

/* DDR memory thermal domain */
/* Sensors: ddr, sdram* (SDRAM memory sensors are memory temperatures, not RF/SDR) */
#define LINUX_THERMAL_METRIC_DDR_TEMP_ID 3
#define LINUX_THERMAL_METRIC_DDR_TEMP_NAME "DDR Temperature"
#define LINUX_THERMAL_METRIC_DDR_TEMP_DESCRIPTION "Temperature of the DDR memory thermal domain (ddr, sdram*)"
#define LINUX_THERMAL_METRIC_DDR_TEMP_UNIT LINUX_THERMAL_TEMP_UNIT

/* Modem thermal domain */
/* Sensors: mdmss-* */
#define LINUX_THERMAL_METRIC_MODEM_TEMP_ID 4
#define LINUX_THERMAL_METRIC_MODEM_TEMP_NAME "Modem Temperature"
#define LINUX_THERMAL_METRIC_MODEM_TEMP_DESCRIPTION "Temperature of the Modem thermal domain (mdmss-*)"
#define LINUX_THERMAL_METRIC_MODEM_TEMP_UNIT LINUX_THERMAL_TEMP_UNIT

/* QMX Fabric thermal domain */
/* Sensors: qmx-* */
#define LINUX_THERMAL_METRIC_QMX_TEMP_ID 5
#define LINUX_THERMAL_METRIC_QMX_TEMP_NAME "QMX Temperature"
#define LINUX_THERMAL_METRIC_QMX_TEMP_DESCRIPTION "Temperature of the QMX Fabric thermal domain (qmx-*)"
#define LINUX_THERMAL_METRIC_QMX_TEMP_UNIT LINUX_THERMAL_TEMP_UNIT

/* Video (VPU) thermal domain */
/* Sensors: video */
#define LINUX_THERMAL_METRIC_VIDEO_TEMP_ID 6
#define LINUX_THERMAL_METRIC_VIDEO_TEMP_NAME "Video Temperature"
#define LINUX_THERMAL_METRIC_VIDEO_TEMP_DESCRIPTION "Temperature of the Video (VPU) thermal domain (video)"
#define LINUX_THERMAL_METRIC_VIDEO_TEMP_UNIT LINUX_THERMAL_TEMP_UNIT

/* Camera thermal domain */
/* Sensors: camera-* */
#define LINUX_THERMAL_METRIC_CAMERA_TEMP_ID 7
#define LINUX_THERMAL_METRIC_CAMERA_TEMP_NAME "Camera Temperature"
#define LINUX_THERMAL_METRIC_CAMERA_TEMP_DESCRIPTION "Temperature of the Camera thermal domain (camera-*)"
#define LINUX_THERMAL_METRIC_CAMERA_TEMP_UNIT LINUX_THERMAL_TEMP_UNIT

/* Wireless (WLAN/BT) thermal domain */
/* Sensors: wireless */
#define LINUX_THERMAL_METRIC_WIRELESS_TEMP_ID 8
#define LINUX_THERMAL_METRIC_WIRELESS_TEMP_NAME "Wireless Temperature"
#define LINUX_THERMAL_METRIC_WIRELESS_TEMP_DESCRIPTION "Temperature of the Wireless (WLAN/BT) thermal domain (wireless)"
#define LINUX_THERMAL_METRIC_WIRELESS_TEMP_UNIT LINUX_THERMAL_TEMP_UNIT

/* USB thermal domain */
/* Sensors: usb */
#define LINUX_THERMAL_METRIC_USB_TEMP_ID 9
#define LINUX_THERMAL_METRIC_USB_TEMP_NAME "USB Temperature"
#define LINUX_THERMAL_METRIC_USB_TEMP_DESCRIPTION "Temperature of the USB thermal domain (usb)"
#define LINUX_THERMAL_METRIC_USB_TEMP_UNIT LINUX_THERMAL_TEMP_UNIT

/* RF / SDR thermal domain */
/* Sensors: sdr0, sdr0_pa (NOT sdram*, which is SDRAM memory -> DDR domain) */
#define LINUX_THERMAL_METRIC_RF_SDR_TEMP_ID 10
#define LINUX_THERMAL_METRIC_RF_SDR_TEMP_NAME "RF/SDR Temperature"
#define LINUX_THERMAL_METRIC_RF_SDR_TEMP_DESCRIPTION "Temperature of the RF/SDR thermal domain (sdr0, sdr0_pa)"
#define LINUX_THERMAL_METRIC_RF_SDR_TEMP_UNIT LINUX_THERMAL_TEMP_UNIT

/* Multimedia (mmWave) thermal domain */
/* Sensors: mmw0-3 */
#define LINUX_THERMAL_METRIC_MULTIMEDIA_TEMP_ID 11
#define LINUX_THERMAL_METRIC_MULTIMEDIA_TEMP_NAME "Multimedia Temperature"
#define LINUX_THERMAL_METRIC_MULTIMEDIA_TEMP_DESCRIPTION "Temperature of the Multimedia thermal domain (mmw0-3)"
#define LINUX_THERMAL_METRIC_MULTIMEDIA_TEMP_UNIT LINUX_THERMAL_TEMP_UNIT

/* PMIC thermal domain */
/* Sensors: pmh*, pmih*, pm8010*, pmd802x*, pmr735d* */
#define LINUX_THERMAL_METRIC_PMIC_TEMP_ID 12
#define LINUX_THERMAL_METRIC_PMIC_TEMP_NAME "PMIC Temperature"
#define LINUX_THERMAL_METRIC_PMIC_TEMP_DESCRIPTION "Temperature of the PMIC thermal domain (pmh*, pmih*, pm8010*, pmd802x*, pmr735d*)"
#define LINUX_THERMAL_METRIC_PMIC_TEMP_UNIT LINUX_THERMAL_TEMP_UNIT

/* System / Skin thermal domain */
/* Sensors: sys-therm-* */
#define LINUX_THERMAL_METRIC_SYSTEM_TEMP_ID 13
#define LINUX_THERMAL_METRIC_SYSTEM_TEMP_NAME "System Temperature"
#define LINUX_THERMAL_METRIC_SYSTEM_TEMP_DESCRIPTION "Temperature of the System/Skin thermal domain (sys-therm-*)"
#define LINUX_THERMAL_METRIC_SYSTEM_TEMP_UNIT LINUX_THERMAL_TEMP_UNIT

/* Battery thermal domain */
/* Sensors: battery */
#define LINUX_THERMAL_METRIC_BATTERY_TEMP_ID 14
#define LINUX_THERMAL_METRIC_BATTERY_TEMP_NAME "Battery Temperature"
#define LINUX_THERMAL_METRIC_BATTERY_TEMP_DESCRIPTION "Temperature of the Battery thermal domain (battery)"
#define LINUX_THERMAL_METRIC_BATTERY_TEMP_UNIT LINUX_THERMAL_TEMP_UNIT

/* SoC thermal domain */
/* Sensors: socd */
#define LINUX_THERMAL_METRIC_SOC_TEMP_ID 15
#define LINUX_THERMAL_METRIC_SOC_TEMP_NAME "SoC Temperature"
#define LINUX_THERMAL_METRIC_SOC_TEMP_DESCRIPTION "Temperature of the SoC thermal domain (socd)"
#define LINUX_THERMAL_METRIC_SOC_TEMP_UNIT LINUX_THERMAL_TEMP_UNIT

/* ============================================================================
 * Cooling metrics
 * ============================================================================
 * One cooling metric per logical cooling domain. Each domain aggregates one or
 * more kernel cooling devices (matched by their cooling_deviceN/type string).
 * The "Cooling devices:" comment on each domain lists the sysfs cooling-device
 * type patterns that feed it ('*' = wildcard). The reported value is the
 * cooling level, expressed as a percentage of the device's max_state.
 *
 * Where a cooling domain has a matching temperature domain, the same domain
 * name/token is reused (CPU, DDR, GPU, NPU, Modem, Battery, RF/SDR,
 * Multimedia) so temperature and cooling metrics line up. Domains with no
 * temperature equivalent (Display, Storage, DSDS, Thermal Framework) are new.
 * ============================================================================ */

/** Unit reported for every cooling metric (cooling level as % of max_state) */
#define LINUX_THERMAL_COOLING_UNIT "%"

/* CPU cooling domain */
/* Cooling devices: cpu-hotplug*, cpufreq*, pause-cpu*, cpu-cluster*, idle-cpu* */
#define LINUX_THERMAL_METRIC_CPU_COOLING_ID 16
#define LINUX_THERMAL_METRIC_CPU_COOLING_NAME "CPU Cooling"
#define LINUX_THERMAL_METRIC_CPU_COOLING_DESCRIPTION "Cooling level of the CPU domain (cpu-hotplug*, cpufreq*, pause-cpu*, cpu-cluster*, idle-cpu*): frequency reduction, core offline, core pause, idle injection"
#define LINUX_THERMAL_METRIC_CPU_COOLING_UNIT LINUX_THERMAL_COOLING_UNIT

/* DDR cooling domain */
/* Cooling devices: ddr-cdev */
#define LINUX_THERMAL_METRIC_DDR_COOLING_ID 17
#define LINUX_THERMAL_METRIC_DDR_COOLING_NAME "DDR Cooling"
#define LINUX_THERMAL_METRIC_DDR_COOLING_DESCRIPTION "Cooling level of the DDR domain (ddr-cdev): memory bandwidth throttling"
#define LINUX_THERMAL_METRIC_DDR_COOLING_UNIT LINUX_THERMAL_COOLING_UNIT

/* GPU cooling domain */
/* Cooling devices: gpu, kgsl */
#define LINUX_THERMAL_METRIC_GPU_COOLING_ID 18
#define LINUX_THERMAL_METRIC_GPU_COOLING_NAME "GPU Cooling"
#define LINUX_THERMAL_METRIC_GPU_COOLING_DESCRIPTION "Cooling level of the GPU domain (gpu, kgsl): GPU frequency/power throttling"
#define LINUX_THERMAL_METRIC_GPU_COOLING_UNIT LINUX_THERMAL_COOLING_UNIT

/* NPU / DSP cooling domain */
/* Cooling devices: cdsp_hw, cdsp, cdsp_sw_hvx, cdsp_sw_hmx */
#define LINUX_THERMAL_METRIC_NPU_COOLING_ID 19
#define LINUX_THERMAL_METRIC_NPU_COOLING_NAME "NPU Cooling"
#define LINUX_THERMAL_METRIC_NPU_COOLING_DESCRIPTION "Cooling level of the NPU/DSP domain (cdsp_hw, cdsp, cdsp_sw_hvx, cdsp_sw_hmx): AI/DSP workload throttling"
#define LINUX_THERMAL_METRIC_NPU_COOLING_UNIT LINUX_THERMAL_COOLING_UNIT

/* Display cooling domain (no temperature equivalent) */
/* Cooling devices: display-fps, panel0-backlight */
#define LINUX_THERMAL_METRIC_DISPLAY_COOLING_ID 20
#define LINUX_THERMAL_METRIC_DISPLAY_COOLING_NAME "Display Cooling"
#define LINUX_THERMAL_METRIC_DISPLAY_COOLING_DESCRIPTION "Cooling level of the Display domain (display-fps, panel0-backlight): reduce refresh rate / brightness"
#define LINUX_THERMAL_METRIC_DISPLAY_COOLING_UNIT LINUX_THERMAL_COOLING_UNIT

/* Storage cooling domain (no temperature equivalent) */
/* Cooling devices: ufs */
#define LINUX_THERMAL_METRIC_STORAGE_COOLING_ID 21
#define LINUX_THERMAL_METRIC_STORAGE_COOLING_NAME "Storage Cooling"
#define LINUX_THERMAL_METRIC_STORAGE_COOLING_DESCRIPTION "Cooling level of the Storage domain (ufs): UFS bandwidth throttling"
#define LINUX_THERMAL_METRIC_STORAGE_COOLING_UNIT LINUX_THERMAL_COOLING_UNIT

/* Battery cooling domain */
/* Cooling devices: battery */
#define LINUX_THERMAL_METRIC_BATTERY_COOLING_ID 22
#define LINUX_THERMAL_METRIC_BATTERY_COOLING_NAME "Battery Cooling"
#define LINUX_THERMAL_METRIC_BATTERY_COOLING_DESCRIPTION "Cooling level of the Battery domain (battery): battery charging/discharging thermal protection"
#define LINUX_THERMAL_METRIC_BATTERY_COOLING_UNIT LINUX_THERMAL_COOLING_UNIT

/* Modem / Cellular cooling domain */
/* Cooling devices: modem_*, modem_current, modem_bcl */
#define LINUX_THERMAL_METRIC_MODEM_COOLING_ID 23
#define LINUX_THERMAL_METRIC_MODEM_COOLING_NAME "Modem Cooling"
#define LINUX_THERMAL_METRIC_MODEM_COOLING_DESCRIPTION "Cooling level of the Modem/Cellular domain (modem_*, modem_current, modem_bcl): cellular power reduction"
#define LINUX_THERMAL_METRIC_MODEM_COOLING_UNIT LINUX_THERMAL_COOLING_UNIT

/* RF / PA cooling domain (maps to the RF/SDR temperature domain) */
/* Cooling devices: pa_* */
#define LINUX_THERMAL_METRIC_RF_SDR_COOLING_ID 24
#define LINUX_THERMAL_METRIC_RF_SDR_COOLING_NAME "RF/SDR Cooling"
#define LINUX_THERMAL_METRIC_RF_SDR_COOLING_DESCRIPTION "Cooling level of the RF/SDR domain (pa_*): RF power amplifier reduction"
#define LINUX_THERMAL_METRIC_RF_SDR_COOLING_UNIT LINUX_THERMAL_COOLING_UNIT

/* mmWave cooling domain (maps to the Multimedia temperature domain) */
/* Cooling devices: mmw_dsc, mmw_sub1_dsc */
#define LINUX_THERMAL_METRIC_MULTIMEDIA_COOLING_ID 25
#define LINUX_THERMAL_METRIC_MULTIMEDIA_COOLING_NAME "Multimedia Cooling"
#define LINUX_THERMAL_METRIC_MULTIMEDIA_COOLING_DESCRIPTION "Cooling level of the Multimedia (mmWave) domain (mmw_dsc, mmw_sub1_dsc): mmWave power reduction"
#define LINUX_THERMAL_METRIC_MULTIMEDIA_COOLING_UNIT LINUX_THERMAL_COOLING_UNIT

/* DSDS cooling domain (no temperature equivalent) */
/* Cooling devices: dsds_switch_dsc */
#define LINUX_THERMAL_METRIC_DSDS_COOLING_ID 26
#define LINUX_THERMAL_METRIC_DSDS_COOLING_NAME "DSDS Cooling"
#define LINUX_THERMAL_METRIC_DSDS_COOLING_DESCRIPTION "Cooling level of the DSDS domain (dsds_switch_dsc): dual-SIM thermal mitigation"
#define LINUX_THERMAL_METRIC_DSDS_COOLING_UNIT LINUX_THERMAL_COOLING_UNIT

/* Thermal Framework cooling domain (no temperature equivalent) */
/* Cooling devices: thermal-pause-* */
#define LINUX_THERMAL_METRIC_THERMAL_FW_COOLING_ID 27
#define LINUX_THERMAL_METRIC_THERMAL_FW_COOLING_NAME "Thermal Framework Cooling"
#define LINUX_THERMAL_METRIC_THERMAL_FW_COOLING_DESCRIPTION "Cooling level of the Thermal Framework domain (thermal-pause-*): generic kernel thermal pause controls"
#define LINUX_THERMAL_METRIC_THERMAL_FW_COOLING_UNIT LINUX_THERMAL_COOLING_UNIT

/**
 * @brief Initialize the Linux Thermal metrics
 *
 * Populates the provided metrics_data array with the metric information
 * (metric id, name, description, and unit) for all logical thermal domains:
 * the temperature metrics defined by the *_TEMP_ID macros followed by the
 * cooling metrics defined by the *_COOLING_ID macros above.
 *
 * @param[out] metrics_data    Array to be populated with metric information
 *                             (must have space for LINUX_THERMAL_CAPABILITY_METRIC_COUNT entries)
 * @param[out] metric_data_len Populated with the number of metrics initialized
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if either argument is NULL
 */
enum QcPerfReturnCode linux_thermal_init_metrics(struct QcPerfMetricInfo* metrics_data, uint8_t* metric_data_len);

/**
 * @brief Initialize only the metrics for subsystems present on the platform
 *
 * Like linux_thermal_init_metrics(), but copies only the metrics whose id is
 * flagged in present[] (packed with no gaps). Used to advertise metrics only for
 * domains that actually have sensors/devices.
 *
 * @param[out] metrics_data    Array to be populated with metric information
 *                             (must have space for LINUX_THERMAL_CAPABILITY_METRIC_COUNT entries)
 * @param[out] metric_data_len Populated with the number of enabled metrics
 * @param[in]  present         Flags indexed by metric id (0..COUNT-1); true enables the metric
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if any argument is NULL
 */
enum QcPerfReturnCode linux_thermal_init_available_metrics(struct QcPerfMetricInfo* metrics_data, uint8_t* metric_data_len, const bool* present);

#endif /* LINUX_THERMAL_INFO_H */
