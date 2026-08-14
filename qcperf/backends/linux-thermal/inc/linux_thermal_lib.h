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
 * @file linux_thermal_lib.h
 * @brief Linux Thermal library interface for the Linux Thermal backend
 * @author Vijay Kumbhani (vkumbhan@qti.qualcomm.com)
 *
 * This header declares the library functions used to enumerate the thermal
 * zones and cooling devices exposed by the Linux kernel under /sys/class/thermal,
 * classify them into logical domains, and read their current temperature and
 * cooling-device state, along with the per-domain data structures used to group
 * those readings. These functions are shared between the backend implementation
 * and the metric-info initialization.
 *
 * The kernel reports temperatures in milli-degrees Celsius via thermal_zoneN/temp;
 * the library stores them as-is (milli-degrees Celsius).
 */

#ifndef LINUX_THERMAL_LIB_H
#define LINUX_THERMAL_LIB_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "qcperf_common.h"

/* ============================================================================
 * Sysfs path definitions
 * ============================================================================ */

/** Root directory of the Linux thermal sysfs framework */
#define LINUX_THERMAL_SYSFS_ROOT "/sys/class/thermal"

/** Directory-entry name prefix of a thermal zone (e.g. thermal_zone0) */
#define LINUX_THERMAL_ZONE_PREFIX "thermal_zone"

/** Directory-entry name prefix of a cooling device (e.g. cooling_device0) */
#define LINUX_THERMAL_COOLING_DEVICE_PREFIX "cooling_device"

/** Format string for a thermal zone directory */
#define LINUX_THERMAL_ZONE_DIR_FMT "/sys/class/thermal/thermal_zone%u"

/** Format string for a thermal zone current-temperature node (value in milli-degrees C) */
#define LINUX_THERMAL_ZONE_TEMP_NODE_FMT "/sys/class/thermal/thermal_zone%u/temp"

/** Format string for a thermal zone type node (human-readable zone name) */
#define LINUX_THERMAL_ZONE_TYPE_NODE_FMT "/sys/class/thermal/thermal_zone%u/type"

/** Format string for a cooling device directory */
#define LINUX_THERMAL_COOLING_DEVICE_DIR_FMT "/sys/class/thermal/cooling_device%u"

/** Format string for a cooling device type node (human-readable device name) */
#define LINUX_THERMAL_COOLING_DEVICE_TYPE_NODE_FMT "/sys/class/thermal/cooling_device%u/type"

/** Format string for a cooling device current-state node (current throttling level) */
#define LINUX_THERMAL_COOLING_DEVICE_CUR_STATE_NODE_FMT "/sys/class/thermal/cooling_device%u/cur_state"

/** Format string for a cooling device max-state node (maximum throttling level) */
#define LINUX_THERMAL_COOLING_DEVICE_MAX_STATE_NODE_FMT "/sys/class/thermal/cooling_device%u/max_state"

/** Maximum length for sysfs node path strings */
#define LINUX_THERMAL_NODE_PATH_MAX_LEN 128

/** Maximum length for a thermal zone type/name string */
#define LINUX_THERMAL_ZONE_NAME_MAX_LEN 64

/** Maximum number of sensor name entries tracked per domain */
#define LINUX_THERMAL_MAX_NAMES 32

/** Conversion factor from milli-degrees Celsius to degrees Celsius */
#define LINUX_THERMAL_MILLIDEG_PER_DEG 1000.0

/**
 * @struct LinuxThermalTempData
 * @brief Per-domain temperature readings for one logical thermal domain
 *
 * A domain aggregates a variable number of raw kernel thermal zones. The three
 * arrays are parallel (entry i of each describes the same zone) and share the
 * same length: id[i] is the kernel thermal zone id, name[i] is that zone's
 * type string, and temp[i] is its temperature in milli-degrees Celsius (as read
 * from thermal_zoneN/temp). The pointers reference storage owned by the library;
 * id/temp are NULL and len is 0 when no matching zone is present on the platform.
 */
struct LinuxThermalTempData {
    uint8_t* id;    /**< Kernel thermal zone id of each entry (len entries) */
    uint8_t** name; /**< Zone type/name of each entry (len string pointers) */
    uint32_t* temp; /**< Temperatures of each entry, milli-degrees Celsius (len entries) */
    uint8_t len;    /**< Number of valid entries in id[]/name[]/temp[] */
};

/**
 * @struct LinuxThermalTempMetrics
 * @brief Per-domain view of the raw thermal-zone temperatures
 *
 * Holds one LinuxThermalTempData block per logical temperature domain. The
 * comment on each member lists the raw kernel thermal-zone type patterns that
 * feed it ('*' = wildcard).
 */
struct LinuxThermalTempMetrics {
    struct LinuxThermalTempData* cpu;        /**< CPU temperatures        : cpu-*, cpullc-* */
    struct LinuxThermalTempData* gpu;        /**< GPU temperatures        : gpu-*, gpuss-* */
    struct LinuxThermalTempData* npu;        /**< NPU/NSP temperatures    : nspvxu*, nspmxu*, nspllv*, nspq6 */
    struct LinuxThermalTempData* ddr;        /**< DDR temperatures        : ddr */
    struct LinuxThermalTempData* modem;      /**< Modem temperatures      : mdmss-* */
    struct LinuxThermalTempData* qmx;        /**< QMX Fabric temperatures : qmx-* */
    struct LinuxThermalTempData* video;      /**< Video temperatures      : video */
    struct LinuxThermalTempData* camera;     /**< Camera temperatures     : camera-* */
    struct LinuxThermalTempData* wireless;   /**< Wireless temperatures   : wireless */
    struct LinuxThermalTempData* usb;        /**< USB temperatures        : usb */
    struct LinuxThermalTempData* rf_sdr;     /**< RF/SDR temperatures     : sdr0, sdr0_pa */
    struct LinuxThermalTempData* multimedia; /**< Multimedia temperatures : mmw0-3 */
    struct LinuxThermalTempData* pmic;       /**< PMIC temperatures       : pmh*, pmih*, pm8010*, pmd802x*, pmr735d* */
    struct LinuxThermalTempData* system;     /**< System/Skin temperatures: sys-therm-* */
    struct LinuxThermalTempData* battery;    /**< Battery temperatures    : battery */
    struct LinuxThermalTempData* soc;        /**< SoC temperatures        : socd */
};

/**
 * @struct LinuxThermalCoolingData
 * @brief Per-domain cooling-device states for one logical cooling domain
 *
 * A domain aggregates a variable number of raw kernel cooling devices. The
 * arrays are parallel (entry i of each describes the same device) and share the
 * same length: id[i] is the kernel cooling device id, name[i] is that device's
 * type string, cur_state[i] is its current throttling level and max_state[i] its
 * maximum level (as read from cooling_deviceN/cur_state and .../max_state). The
 * pointers reference storage owned by the library; they are NULL and len is 0
 * when no matching device is present on the platform. The throttling percentage
 * for a device is cur_state / max_state * 100.
 */
struct LinuxThermalCoolingData {
    uint8_t* id;         /**< Kernel cooling device id of each entry (len entries) */
    uint8_t** name;      /**< Cooling device type/name of each entry (len string pointers) */
    uint8_t* cur_state;  /**< Current throttling level of each entry (len entries) */
    uint8_t* max_state;  /**< Maximum throttling level of each entry (len entries) */
    uint8_t len;         /**< Number of valid entries in id[]/name[]/cur_state[]/max_state[] */
};

/**
 * @struct LinuxThermalCoolingMetrics
 * @brief Per-domain view of the raw cooling-device states
 *
 * Holds one LinuxThermalCoolingData block per logical cooling domain. The
 * comment on each member lists the raw kernel cooling-device type patterns that
 * feed it ('*' = wildcard).
 */
struct LinuxThermalCoolingMetrics {
    struct LinuxThermalCoolingData* cpu;        /**< CPU cooling         : cpu-hotplug*, cpufreq*, pause-cpu*, cpu-cluster* */
    struct LinuxThermalCoolingData* ddr;        /**< DDR cooling         : ddr-cdev */
    struct LinuxThermalCoolingData* gpu;        /**< GPU cooling         : gpu, kgsl */
    struct LinuxThermalCoolingData* npu;        /**< NPU/DSP cooling     : cdsp_hw, cdsp, cdsp_sw_hvx, cdsp_sw_hmx */
    struct LinuxThermalCoolingData* display;    /**< Display cooling     : display-fps, panel0-backlight */
    struct LinuxThermalCoolingData* storage;    /**< Storage cooling     : ufs */
    struct LinuxThermalCoolingData* battery;    /**< Battery cooling     : battery */
    struct LinuxThermalCoolingData* modem;      /**< Modem cooling       : modem_*, modem_current, modem_bcl */
    struct LinuxThermalCoolingData* rf_sdr;     /**< RF/SDR cooling      : pa_* */
    struct LinuxThermalCoolingData* multimedia; /**< Multimedia cooling  : mmw_dsc, mmw_sub1_dsc */
    struct LinuxThermalCoolingData* dsds;       /**< DSDS cooling        : dsds_switch_dsc */
    struct LinuxThermalCoolingData* thermal_fw; /**< Thermal Framework   : thermal-pause-* */
};

/**
 * @brief Initialize the Linux Thermal library
 *
 * Enumerates the thermal zones and cooling devices under /sys/class/thermal,
 * reads each entry's type, and classifies it into one of the logical temperature
 * or cooling domains. The resulting per-domain id and name lists are held in
 * library-owned storage so that subsequent linux_thermal_lib_thermal_zones_info()
 * and linux_thermal_lib_cooling_devices_info() calls can sample the current
 * values without re-reading each entry's type.
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_ALREADY_INITIALIZED if the library is already initialized
 * @return QC_PERF_RETURN_CODE_FAILED if no thermal zones could be enumerated
 */
enum QcPerfReturnCode linux_thermal_lib_init(void);

/**
 * @brief Clean up the Linux Thermal library
 *
 * Resets the library-owned per-domain data populated by linux_thermal_lib_init()
 * and returns the library to the uninitialized state. Any metrics pointer handed
 * out by the discover/info calls must no longer be used afterwards.
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NOT_INITIALIZED if the library is not initialized
 */
enum QcPerfReturnCode linux_thermal_lib_cleanup(void);

/**
 * @brief Expose the discovered thermal zones grouped by domain
 *
 * The thermal zones are enumerated and classified once by
 * linux_thermal_lib_init(). This function sets *metrics to the library-owned
 * LinuxThermalTempMetrics, whose per-domain blocks already carry their id[] and
 * name[] arrays and len. Temperature values are refreshed separately by
 * linux_thermal_lib_thermal_zones_info().
 *
 * @param[out] metrics Set to point at the library-owned metrics structure
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if metrics is NULL
 * @return QC_PERF_RETURN_CODE_NOT_INITIALIZED if the library is not initialized
 */
enum QcPerfReturnCode linux_thermal_lib_discover_thermal_zones(struct LinuxThermalTempMetrics**metrics);

/**
 * @brief Expose the discovered cooling devices grouped by domain
 *
 * The cooling devices are enumerated and classified once by
 * linux_thermal_lib_init(). This function sets *metrics to the library-owned
 * LinuxThermalCoolingMetrics, whose per-domain blocks already carry their id[]
 * and name[] arrays and len. The cur_state/max_state values are refreshed
 * separately by linux_thermal_lib_cooling_devices_info().
 *
 * @param[out] metrics Set to point at the library-owned metrics structure
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if metrics is NULL
 * @return QC_PERF_RETURN_CODE_NOT_INITIALIZED if the library is not initialized
 */
enum QcPerfReturnCode linux_thermal_lib_discover_cooling_devices(struct LinuxThermalCoolingMetrics**metrics);

/**
 * @brief Sample the current temperatures of every thermal zone, grouped by domain
 *
 * Reads thermal_zoneN/temp for each zone discovered by
 * linux_thermal_lib_discover_thermal_zones() (values in milli-degrees Celsius)
 * and stores them in the library-owned per-domain buffers. On success *data is
 * set to the library-owned LinuxThermalTempMetrics,
 * whose per-domain temp[] arrays now hold the refreshed values; a domain with no
 * matching zone has a NULL pointer and a length of 0.
 *
 * @param[out] data Set to point at the library-owned temperature metrics structure
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NOT_INITIALIZED if the library is not initialized
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if data is NULL
 */
enum QcPerfReturnCode linux_thermal_lib_thermal_zones_info(struct LinuxThermalTempMetrics**data);

/**
 * @brief Sample the current and maximum states of every cooling device, grouped by domain
 *
 * Reads cooling_deviceN/cur_state and cooling_deviceN/max_state for each device
 * discovered by linux_thermal_lib_discover_cooling_devices() and stores them in
 * the library-owned per-domain buffers. On success *data is set to the
 * library-owned LinuxThermalCoolingMetrics, whose per-domain cur_state/max_state
 * arrays now hold the refreshed values; a domain with no matching device has NULL
 * pointers and a length of 0.
 *
 * @param[out] data Set to point at the library-owned cooling metrics structure
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NOT_INITIALIZED if the library is not initialized
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if data is NULL
 */
enum QcPerfReturnCode linux_thermal_lib_cooling_devices_info(struct LinuxThermalCoolingMetrics**data);

/**
 * @brief Release a thermal-zone metrics view
 *
 * Detaches the caller's temperature-metrics view by setting *data to NULL. The
 * underlying data is owned by the library and is released by
 * linux_thermal_lib_cleanup(); this call only clears the caller's pointer so it
 * no longer references that storage.
 *
 * @param[in,out] data Caller's metrics pointer; cleared to NULL
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if data is NULL
 */
enum QcPerfReturnCode linux_thermal_lib_free_thermal_zones_info(struct LinuxThermalTempMetrics**data);

/**
 * @brief Release a cooling-device metrics view
 *
 * Detaches the caller's cooling-metrics view by setting *data to NULL. The
 * underlying data is owned by the library and is released by
 * linux_thermal_lib_cleanup(); this call only clears the caller's pointer so it
 * no longer references that storage.
 *
 * @param[in,out] data Caller's metrics pointer; cleared to NULL
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if data is NULL
 */
enum QcPerfReturnCode linux_thermal_lib_free_cooling_devices_info(struct LinuxThermalCoolingMetrics**data);

#endif /* LINUX_THERMAL_LIB_H */
