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
 * @file linux_thermal_lib.c
 * @brief Implementation of the Linux Thermal library
 * @author Vijay Kumbhani (vkumbhan@qti.qualcomm.com)
 *
 * This file provides the implementation skeleton for the API declared in
 * linux_thermal_lib.h. The function bodies are currently stubs that only perform
 * basic argument validation and return success; the sysfs enumeration,
 * classification, and sampling logic is to be filled in.
 */

#include <dirent.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linux_thermal_lib.h"
#include "linux_thermal_info.h"
#include "qcperf_common.h"

/* Number of logical domains in each metrics view */
#define LINUX_THERMAL_TEMP_DOMAIN_COUNT 16
#define LINUX_THERMAL_COOL_DOMAIN_COUNT 12

/* ============================================================================
 * Internal library state (file-scope; not accessible outside this translation
 * unit). These point at the library-owned metrics that the discover/info calls
 * hand back to the caller.
 * ============================================================================ */
static struct LinuxThermalTempMetrics*    g_temp_metrics = NULL;
static struct LinuxThermalCoolingMetrics* g_cool_metrics = NULL;
static volatile bool g_is_initialized                    = false;
static bool g_temp_discovered                            = false;  // temperature zones read into g_temp_metrics
static bool g_cool_discovered                            = false;  // cooling devices read into g_cool_metrics

/*
 * Persistent sysfs value-node handles, opened once at discovery and kept open so
 * the per-sample refresh (thermal_zones_info / cooling_devices_info) only needs
 * to rewind + re-read instead of re-opening. Parallel to each domain's entries:
 * g_temp_node[d][i] is the thermal_zoneN/temp handle for temperature domain d
 * entry i; g_cool_node[d][i] is the cooling_deviceN/cur_state handle. max_state
 * is a constant device capability, read once at discovery (no persistent handle).
 */
static FILE** g_temp_node[LINUX_THERMAL_TEMP_DOMAIN_COUNT] = {0};
static FILE** g_cool_node[LINUX_THERMAL_COOL_DOMAIN_COUNT] = {0};

/* ============================================================================
 * Domain classification
 * ----------------------------------------------------------------------------
 * A raw kernel zone/device is mapped to a logical domain by matching its
 * thermal_zoneN/type (or cooling_deviceN/type) string against these prefixes.
 * The index is the position of the matching domain within the metrics struct
 * (see the domain pointer arrays in the scan helpers below).
 * ============================================================================ */
struct LinuxThermalPrefix {
    const char* prefix; /**< Zone/device type prefix to match */
    uint8_t len;        /**< Precomputed strlen(prefix) */
    uint8_t index;      /**< Domain index within the metrics struct */
};

/* Temperature-zone type prefixes -> temperature domain index (len precomputed) */
static const struct LinuxThermalPrefix g_temp_prefix[] = {
    {"cpu", 3, 0},      {"gpu", 3, 1},  {"nsp", 3, 2},      {"ddr", 3, 3},
    {"mdmss", 5, 4},    {"qmx", 3, 5},  {"video", 5, 6},    {"camera", 6, 7},
    {"wireless", 8, 8}, {"usb", 3, 9},  {"sdr", 3, 10},     {"mmw", 3, 11},
    {"pm", 2, 12},      {"sys", 3, 13}, {"battery", 7, 14}, {"socd", 4, 15},
    {"vbat", 4, 14},  /* battery voltage node -> BATTERY domain (value is mV, not milli-C) */
};

/* Cooling-device type prefixes -> cooling domain index (len precomputed) */
static const struct LinuxThermalPrefix g_cool_prefix[] = {
    {"cpu", 3, 0},   {"pause-cpu", 9, 0}, {"ddr", 3, 1},  {"gpu", 3, 2},  {"kgsl", 4, 2},
    {"cdsp", 4, 3},  {"display", 7, 4},   {"panel", 5, 4}, {"ufs", 3, 5}, {"battery", 7, 6},
    {"modem", 5, 7}, {"pa_", 3, 8},       {"mmw", 3, 9},   {"dsds", 4, 10}, {"thermal-pause", 13, 11},
};

/**
 * @brief Classify a zone/device type string into a domain index
 *
 * @param[in] type  Zone/device type string (from the sysfs type node)
 * @param[in] table Prefix table to match against
 * @param[in] count Number of entries in table
 *
 * @return Matching domain index, or -1 if the type belongs to no domain
 */
static int linux_thermal_classify(const char* type, const struct LinuxThermalPrefix* table, size_t count) {
    int index = -1;

    // PMIC battery-current-limit ("-bcl-") and battery-current ("-ibat-") nodes
    // (e.g. pmh0101-bcl-lvl0, pmih010x-ibat-lvl1) report current limits/levels,
    // not temperatures, so exclude them from any domain. The "-...-" form is
    // specific to these level nodes and does not match the modem_bcl cooling
    // device (which has no surrounding dashes).
    if (NULL != strstr(type, "-bcl-") || NULL != strstr(type, "-ibat-")) {
        return -1;
    }

    for (size_t i = 0; i < count; i++) {
        if (0 == strncmp(type, table[i].prefix, table[i].len)) {
            index = (int)table[i].index;
            break;
        }
    }

    return index;
}

/**
 * @brief Read a sysfs type node into buf (trailing whitespace stripped)
 *
 * @param[in]  path    Absolute path of the type node to read
 * @param[out] buf     Buffer to receive the NUL-terminated type string
 * @param[in]  buf_len Size of buf in bytes
 *
 * @return true if the node was opened and a value read, false otherwise
 */
static bool linux_thermal_read_type(const char* path, char* buf, size_t buf_len) {
    bool ok  = false;
    FILE* fp = fopen(path, "r");

    if (NULL != fp) {
        if (NULL != fgets(buf, (int)buf_len, fp)) {
            size_t len = strlen(buf);
            while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r' || buf[len - 1] == ' ' || buf[len - 1] == '\t')) {
                buf[len - 1] = '\0';
                len--;
            }
            ok = true;
        }
        fclose(fp);
    }

    return ok;
}

/**
 * @brief Read an integer value from a sysfs node
 *
 * @param[in]  path      Absolute path of the value node to read
 * @param[out] out_value Populated with the parsed value (as uint32_t)
 *
 * @return true if the node was opened and parsed, false otherwise
 */
static bool linux_thermal_read_value(const char* path, uint32_t* out_value) {
    bool ok  = false;
    FILE* fp = fopen(path, "r");

    if (NULL != fp) {
        if (1 == fscanf(fp, "%" SCNu32, out_value)) {
            ok = true;
        }
        fclose(fp);
    }

    return ok;
}

/**
 * @brief Read an integer value from an already-open sysfs node handle
 *
 * Rewinds the stream to the start (discarding any buffered content so the kernel
 * regenerates a fresh value) and re-parses it. Used on the per-sample refresh
 * path so value nodes do not need to be re-opened every sample.
 *
 * @param[in]  fp        Open sysfs value-node handle (may be NULL)
 * @param[out] out_value Populated with the parsed value (as uint32_t)
 *
 * @return true if the handle was valid and a value was parsed, false otherwise
 */
static bool linux_thermal_read_value_fp(FILE* fp, uint32_t* out_value) {
    bool ok = false;

    if (NULL != fp && 0 == fseek(fp, 0, SEEK_SET)) {
        if (1 == fscanf(fp, "%" SCNu32, out_value)) {
            ok = true;
        }
    }

    return ok;
}

/**
 * @brief Allocate a temperature domain's inner buffers sized to its len
 *
 * Allocates the id[] and temp[] arrays (len entries each) and the name pointer
 * array. The name strings share a single contiguous arena (one allocation)
 * whose base is name[0], so freeing is a single free of that base. No-op when
 * the block is NULL or empty.
 *
 * @param[in,out] data Domain block whose buffers are allocated
 */
static void linux_thermal_alloc_temp_buffers(struct LinuxThermalTempData* data) {
    uint8_t* name_store = NULL;

    if (NULL != data && data->len > 0) {
        name_store = (uint8_t*)calloc((size_t)data->len, LINUX_THERMAL_ZONE_NAME_MAX_LEN);
        data->id   = (uint8_t*)calloc(data->len, sizeof(uint8_t));
        data->temp = (uint32_t*)calloc(data->len, sizeof(uint32_t));
        data->name = (uint8_t**)calloc(data->len, sizeof(uint8_t*));
        if (NULL != data->name && NULL != name_store) {
            for (uint8_t i = 0; i < data->len; i++) {
                data->name[i] = name_store + (size_t)i * LINUX_THERMAL_ZONE_NAME_MAX_LEN;
            }
        } else {
            free(name_store);
        }
    }
}

/**
 * @brief Allocate a cooling domain's inner buffers sized to its len
 *
 * Allocates the id[], cur_state[] and max_state[] arrays (len entries each) and
 * the name pointer array. The name strings share a single contiguous arena (one
 * allocation) whose base is name[0], so freeing is a single free of that base.
 * No-op when the block is NULL or empty.
 *
 * @param[in,out] data Domain block whose buffers are allocated
 */
static void linux_thermal_alloc_cooling_buffers(struct LinuxThermalCoolingData* data) {
    uint8_t* name_store = NULL;

    if (NULL != data && data->len > 0) {
        name_store = (uint8_t*)calloc((size_t)data->len, LINUX_THERMAL_ZONE_NAME_MAX_LEN);
        data->id        = (uint8_t*)calloc(data->len, sizeof(uint8_t));
        data->cur_state = (uint8_t*)calloc(data->len, sizeof(uint8_t));
        data->max_state = (uint8_t*)calloc(data->len, sizeof(uint8_t));
        data->name      = (uint8_t**)calloc(data->len, sizeof(uint8_t*));
        if (NULL != data->name && NULL != name_store) {
            for (uint8_t i = 0; i < data->len; i++) {
                data->name[i] = name_store + (size_t)i * LINUX_THERMAL_ZONE_NAME_MAX_LEN;
            }
        } else {
            free(name_store);
        }
    }
}

/**
 * @brief Initialize the library: scan /sys/class/thermal once, classify every
 *        thermal zone and cooling device into a domain, and allocate the
 *        per-domain buffers sized to the counted lengths.
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_ALREADY_INITIALIZED if the library is already initialized
 * @return QC_PERF_RETURN_CODE_CALLOC_FAILED if a metrics allocation fails
 * @return QC_PERF_RETURN_CODE_FAILED if the thermal sysfs root is unavailable
 */
enum QcPerfReturnCode linux_thermal_lib_init(void) {
    enum QcPerfReturnCode return_code          = QC_PERF_RETURN_CODE_SUCCESS;
    DIR* dir                                   = NULL;
    struct dirent* entry                       = NULL;
    char path[LINUX_THERMAL_NODE_PATH_MAX_LEN] = {0};
    char type[LINUX_THERMAL_ZONE_NAME_MAX_LEN] = {0};
    bool is_zone                               = false;
    bool is_cool                               = false;
    int idx                                    = -1;

    if (true == g_is_initialized) {
        return_code = QC_PERF_RETURN_CODE_ALREADY_INITIALIZED;
    } else {
        // Open the thermal sysfs root once and scan every entry
        dir = opendir(LINUX_THERMAL_SYSFS_ROOT);
        if (NULL == dir) {
            return_code = QC_PERF_RETURN_CODE_FAILED;
        } else {
            g_temp_metrics = (struct LinuxThermalTempMetrics*)calloc(1, sizeof(struct LinuxThermalTempMetrics));
            g_cool_metrics = (struct LinuxThermalCoolingMetrics*)calloc(1, sizeof(struct LinuxThermalCoolingMetrics));

            if (NULL == g_temp_metrics || NULL == g_cool_metrics) {
                return_code = QC_PERF_RETURN_CODE_CALLOC_FAILED;
            } else {
                // Per-domain slots, indexed to match the classification prefix tables
                struct LinuxThermalTempData** temp_domain[LINUX_THERMAL_TEMP_DOMAIN_COUNT] = {
                    &g_temp_metrics->cpu,        &g_temp_metrics->gpu,    &g_temp_metrics->npu,      &g_temp_metrics->ddr,
                    &g_temp_metrics->modem,      &g_temp_metrics->qmx,    &g_temp_metrics->video,    &g_temp_metrics->camera,
                    &g_temp_metrics->wireless,   &g_temp_metrics->usb,    &g_temp_metrics->rf_sdr,   &g_temp_metrics->multimedia,
                    &g_temp_metrics->pmic,       &g_temp_metrics->system, &g_temp_metrics->battery,  &g_temp_metrics->soc,
                };
                struct LinuxThermalCoolingData** cool_domain[LINUX_THERMAL_COOL_DOMAIN_COUNT] = {
                    &g_cool_metrics->cpu,        &g_cool_metrics->ddr,        &g_cool_metrics->gpu,     &g_cool_metrics->npu,
                    &g_cool_metrics->display,    &g_cool_metrics->storage,    &g_cool_metrics->battery, &g_cool_metrics->modem,
                    &g_cool_metrics->rf_sdr,     &g_cool_metrics->multimedia, &g_cool_metrics->dsds,    &g_cool_metrics->thermal_fw,
                };

                // Single pass: identify each entry as a thermal zone or cooling device,
                // read its type, classify it, and bump the matching domain's len.
                while (NULL != (entry = readdir(dir))) {
                    is_zone = (0 == strncmp(entry->d_name, LINUX_THERMAL_ZONE_PREFIX, strlen(LINUX_THERMAL_ZONE_PREFIX)));
                    is_cool = (0 == strncmp(entry->d_name, LINUX_THERMAL_COOLING_DEVICE_PREFIX, strlen(LINUX_THERMAL_COOLING_DEVICE_PREFIX)));
                    if (false == is_zone && false == is_cool) {
                        continue;
                    }

                    snprintf(path, sizeof(path), "%s/%s/type", LINUX_THERMAL_SYSFS_ROOT, entry->d_name);
                    if (false == linux_thermal_read_type(path, type, sizeof(type))) {
                        continue;
                    }

                    if (is_zone) {
                        idx = linux_thermal_classify(type, g_temp_prefix, sizeof(g_temp_prefix) / sizeof(g_temp_prefix[0]));
                        if (idx >= 0) {
                            if (NULL == *temp_domain[idx]) {
                                *temp_domain[idx] = (struct LinuxThermalTempData*)calloc(1, sizeof(struct LinuxThermalTempData));
                            }
                            if (NULL != *temp_domain[idx] && (*temp_domain[idx])->len < LINUX_THERMAL_MAX_NAMES) {
                                (*temp_domain[idx])->len++;
                            }
                        }
                    } else {
                        idx = linux_thermal_classify(type, g_cool_prefix, sizeof(g_cool_prefix) / sizeof(g_cool_prefix[0]));
                        if (idx >= 0) {
                            if (NULL == *cool_domain[idx]) {
                                *cool_domain[idx] = (struct LinuxThermalCoolingData*)calloc(1, sizeof(struct LinuxThermalCoolingData));
                            }
                            if (NULL != *cool_domain[idx] && (*cool_domain[idx])->len < LINUX_THERMAL_MAX_NAMES) {
                                (*cool_domain[idx])->len++;
                            }
                        }
                    }
                }

                // Counting complete: allocate each populated domain's buffers to its len,
                // plus its parallel array of persistent value-node handles.
                for (uint8_t i = 0; i < LINUX_THERMAL_TEMP_DOMAIN_COUNT; i++) {
                    linux_thermal_alloc_temp_buffers(*temp_domain[i]);
                    if (NULL != *temp_domain[i] && (*temp_domain[i])->len > 0) {
                        g_temp_node[i] = (FILE**)calloc((*temp_domain[i])->len, sizeof(FILE*));
                    }
                }
                for (uint8_t i = 0; i < LINUX_THERMAL_COOL_DOMAIN_COUNT; i++) {
                    linux_thermal_alloc_cooling_buffers(*cool_domain[i]);
                    if (NULL != *cool_domain[i] && (*cool_domain[i])->len > 0) {
                        g_cool_node[i] = (FILE**)calloc((*cool_domain[i])->len, sizeof(FILE*));
                    }
                }

                g_is_initialized = true;
            }

            closedir(dir);

            // On allocation failure, release any partial allocation
            if (QC_PERF_RETURN_CODE_CALLOC_FAILED == return_code) {
                free(g_temp_metrics);
                g_temp_metrics = NULL;
                free(g_cool_metrics);
                g_cool_metrics = NULL;
            }
        }
    }

    return return_code;
}

/**
 * @brief Free one temperature-domain block and the buffers inside it
 *
 * NULL-checks the block and each inner buffer (id, name[i], temp) before
 * freeing, so partially-populated blocks are handled safely.
 *
 * @param[in] data Domain block to free (may be NULL)
 */
static void linux_thermal_free_temp_data(struct LinuxThermalTempData** data) {
    if (NULL != data && NULL != *data) {
        struct LinuxThermalTempData* block = *data;
        if (NULL != block->id) {
            free(block->id);
            block->id = NULL;
        }
        if (NULL != block->temp) {
            free(block->temp);
            block->temp = NULL;
        }
        if (NULL != block->name) {
            // name strings share one arena whose base is name[0]; free it once.
            if (block->len > 0 && NULL != block->name[0]) {
                free(block->name[0]);
            }
            free(block->name);
            block->name = NULL;
        }
        free(block);
        *data = NULL;
    }
}

/**
 * @brief Free one cooling-domain block and the buffers inside it
 *
 * NULL-checks the block and each inner buffer (id, name[i], cur_state,
 * max_state) before freeing, so partially-populated blocks are handled safely.
 *
 * @param[in] data Domain block to free (may be NULL)
 */
static void linux_thermal_free_cooling_data(struct LinuxThermalCoolingData** data) {
    if (NULL != data && NULL != *data) {
        struct LinuxThermalCoolingData* block = *data;
        if (NULL != block->id) {
            free(block->id);
            block->id = NULL;
        }
        if (NULL != block->cur_state) {
            free(block->cur_state);
            block->cur_state = NULL;
        }
        if (NULL != block->max_state) {
            free(block->max_state);
            block->max_state = NULL;
        }
        if (NULL != block->name) {
            // name strings share one arena whose base is name[0]; free it once.
            if (block->len > 0 && NULL != block->name[0]) {
                free(block->name[0]);
            }
            free(block->name);
            block->name = NULL;
        }
        free(block);
        *data = NULL;
    }
}

/**
 * @brief Close all persistent temperature value-node handles and free the arrays
 *
 * Must run before g_temp_metrics is freed (uses each domain's len). No-op for
 * domains that were never opened.
 */
static void linux_thermal_close_temp_nodes(void) {
    if (NULL != g_temp_metrics) {
        struct LinuxThermalTempData* domain[LINUX_THERMAL_TEMP_DOMAIN_COUNT] = {
            g_temp_metrics->cpu,        g_temp_metrics->gpu,    g_temp_metrics->npu,      g_temp_metrics->ddr,
            g_temp_metrics->modem,      g_temp_metrics->qmx,    g_temp_metrics->video,    g_temp_metrics->camera,
            g_temp_metrics->wireless,   g_temp_metrics->usb,    g_temp_metrics->rf_sdr,   g_temp_metrics->multimedia,
            g_temp_metrics->pmic,       g_temp_metrics->system, g_temp_metrics->battery,  g_temp_metrics->soc,
        };
        for (uint8_t d = 0; d < LINUX_THERMAL_TEMP_DOMAIN_COUNT; d++) {
            if (NULL != g_temp_node[d]) {
                uint8_t len = (NULL != domain[d]) ? domain[d]->len : 0;
                for (uint8_t i = 0; i < len; i++) {
                    if (NULL != g_temp_node[d][i]) {
                        fclose(g_temp_node[d][i]);
                        g_temp_node[d][i] = NULL;
                    }
                }
                free(g_temp_node[d]);
                g_temp_node[d] = NULL;
            }
        }
    }
}

/**
 * @brief Close all persistent cooling value-node handles and free the arrays
 *
 * Must run before g_cool_metrics is freed (uses each domain's len). No-op for
 * domains that were never opened.
 */
static void linux_thermal_close_cool_nodes(void) {
    if (NULL != g_cool_metrics) {
        struct LinuxThermalCoolingData* domain[LINUX_THERMAL_COOL_DOMAIN_COUNT] = {
            g_cool_metrics->cpu,        g_cool_metrics->ddr,        g_cool_metrics->gpu,     g_cool_metrics->npu,
            g_cool_metrics->display,    g_cool_metrics->storage,    g_cool_metrics->battery, g_cool_metrics->modem,
            g_cool_metrics->rf_sdr,     g_cool_metrics->multimedia, g_cool_metrics->dsds,    g_cool_metrics->thermal_fw,
        };
        for (uint8_t d = 0; d < LINUX_THERMAL_COOL_DOMAIN_COUNT; d++) {
            if (NULL != g_cool_node[d]) {
                uint8_t len = (NULL != domain[d]) ? domain[d]->len : 0;
                for (uint8_t i = 0; i < len; i++) {
                    if (NULL != g_cool_node[d][i]) {
                        fclose(g_cool_node[d][i]);
                        g_cool_node[d][i] = NULL;
                    }
                }
                free(g_cool_node[d]);
                g_cool_node[d] = NULL;
            }
        }
    }
}

/**
 * @brief Free the temperature metrics structure and each per-domain block
 *
 * Frees every domain block inside g_temp_metrics (and the buffers inside each),
 * then the structure itself, and clears the global pointer.
 */
static void linux_thermal_free_temp_metrics(void) {
    if (NULL != g_temp_metrics) {
        linux_thermal_free_temp_data(&g_temp_metrics->cpu);
        linux_thermal_free_temp_data(&g_temp_metrics->gpu);
        linux_thermal_free_temp_data(&g_temp_metrics->npu);
        linux_thermal_free_temp_data(&g_temp_metrics->ddr);
        linux_thermal_free_temp_data(&g_temp_metrics->modem);
        linux_thermal_free_temp_data(&g_temp_metrics->qmx);
        linux_thermal_free_temp_data(&g_temp_metrics->video);
        linux_thermal_free_temp_data(&g_temp_metrics->camera);
        linux_thermal_free_temp_data(&g_temp_metrics->wireless);
        linux_thermal_free_temp_data(&g_temp_metrics->usb);
        linux_thermal_free_temp_data(&g_temp_metrics->rf_sdr);
        linux_thermal_free_temp_data(&g_temp_metrics->multimedia);
        linux_thermal_free_temp_data(&g_temp_metrics->pmic);
        linux_thermal_free_temp_data(&g_temp_metrics->system);
        linux_thermal_free_temp_data(&g_temp_metrics->battery);
        linux_thermal_free_temp_data(&g_temp_metrics->soc);
        free(g_temp_metrics);
        g_temp_metrics = NULL;
    }
}

/**
 * @brief Free the cooling metrics structure and each per-domain block
 *
 * Frees every domain block inside g_cool_metrics (and the buffers inside each),
 * then the structure itself, and clears the global pointer.
 */
static void linux_thermal_free_cool_metrics(void) {
    if (NULL != g_cool_metrics) {
        linux_thermal_free_cooling_data(&g_cool_metrics->cpu);
        linux_thermal_free_cooling_data(&g_cool_metrics->ddr);
        linux_thermal_free_cooling_data(&g_cool_metrics->gpu);
        linux_thermal_free_cooling_data(&g_cool_metrics->npu);
        linux_thermal_free_cooling_data(&g_cool_metrics->display);
        linux_thermal_free_cooling_data(&g_cool_metrics->storage);
        linux_thermal_free_cooling_data(&g_cool_metrics->battery);
        linux_thermal_free_cooling_data(&g_cool_metrics->modem);
        linux_thermal_free_cooling_data(&g_cool_metrics->rf_sdr);
        linux_thermal_free_cooling_data(&g_cool_metrics->multimedia);
        linux_thermal_free_cooling_data(&g_cool_metrics->dsds);
        linux_thermal_free_cooling_data(&g_cool_metrics->thermal_fw);
        free(g_cool_metrics);
        g_cool_metrics = NULL;
    }
}

/**
 * @brief Release all library-owned metrics (blocks and inner buffers) and reset
 *        to the uninitialized state.
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NOT_INITIALIZED if the library is not initialized
 */
enum QcPerfReturnCode linux_thermal_lib_cleanup(void) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_SUCCESS;

    if (false == g_is_initialized) {
        return_code = QC_PERF_RETURN_CODE_NOT_INITIALIZED;
    } else {
        // Close the persistent value-node handles first (needs the domain lengths),
        // then free each metrics structure and the per-domain blocks inside it.
        linux_thermal_close_temp_nodes();
        linux_thermal_close_cool_nodes();
        linux_thermal_free_temp_metrics();
        linux_thermal_free_cool_metrics();
        g_temp_discovered = false;
        g_cool_discovered = false;
        g_is_initialized  = false;
    }

    return return_code;
}

/**
 * @brief Read every thermal zone's id, type and temperature into the per-domain
 *        temperature blocks and return the library-owned metrics.
 *
 * @param[out] metrics Set to point at the library-owned temperature metrics
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NOT_INITIALIZED if the library is not initialized
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if metrics is NULL
 */
enum QcPerfReturnCode linux_thermal_lib_discover_thermal_zones(struct LinuxThermalTempMetrics** metrics) {
    enum QcPerfReturnCode return_code               = QC_PERF_RETURN_CODE_SUCCESS;
    DIR* dir                                        = NULL;
    struct dirent* entry                            = NULL;
    char path[LINUX_THERMAL_NODE_PATH_MAX_LEN]      = {0};
    char type[LINUX_THERMAL_ZONE_NAME_MAX_LEN]      = {0};
    uint8_t filled[LINUX_THERMAL_TEMP_DOMAIN_COUNT] = {0};
    unsigned int zone_id                            = 0;
    int idx                                         = -1;
    uint8_t k                                       = 0;
    uint32_t milli_deg_c                            = 0;

    if (false == g_is_initialized) {
        return_code = QC_PERF_RETURN_CODE_NOT_INITIALIZED;
    } else if (NULL == metrics) {
        return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else if (true == g_temp_discovered) {
        // Already processed on a prior call: hand back the library-owned metrics directly.
        *metrics = g_temp_metrics;
    } else {
        struct LinuxThermalTempData* domain[LINUX_THERMAL_TEMP_DOMAIN_COUNT] = {
            g_temp_metrics->cpu,        g_temp_metrics->gpu,    g_temp_metrics->npu,      g_temp_metrics->ddr,
            g_temp_metrics->modem,      g_temp_metrics->qmx,    g_temp_metrics->video,    g_temp_metrics->camera,
            g_temp_metrics->wireless,   g_temp_metrics->usb,    g_temp_metrics->rf_sdr,   g_temp_metrics->multimedia,
            g_temp_metrics->pmic,       g_temp_metrics->system, g_temp_metrics->battery,  g_temp_metrics->soc,
        };

        // Read every thermal zone once: parse its id from the entry name, read its
        // type, classify it, and store id/name/temperature into that domain's block.
        dir = opendir(LINUX_THERMAL_SYSFS_ROOT);
        if (NULL != dir) {
            while (NULL != (entry = readdir(dir))) {
                if (1 != sscanf(entry->d_name, LINUX_THERMAL_ZONE_PREFIX "%u", &zone_id)) {
                    continue;
                }

                snprintf(path, sizeof(path), "%s/%s/type", LINUX_THERMAL_SYSFS_ROOT, entry->d_name);
                if (false == linux_thermal_read_type(path, type, sizeof(type))) {
                    continue;
                }

                idx = linux_thermal_classify(type, g_temp_prefix, sizeof(g_temp_prefix) / sizeof(g_temp_prefix[0]));
                if (idx < 0 || NULL == domain[idx] || filled[idx] >= domain[idx]->len) {
                    continue;
                }

                k = filled[idx];
                if (NULL != domain[idx]->id) {
                    domain[idx]->id[k] = (uint8_t)zone_id;
                }
                if (NULL != domain[idx]->name && NULL != domain[idx]->name[k]) {
                    snprintf((char*)domain[idx]->name[k], LINUX_THERMAL_ZONE_NAME_MAX_LEN, "%s", type);
                }
                // Open the temp value node once and keep the handle for cheap re-reads.
                snprintf(path, sizeof(path), LINUX_THERMAL_ZONE_TEMP_NODE_FMT, zone_id);
                if (NULL != g_temp_node[idx]) {
                    g_temp_node[idx][k] = fopen(path, "r");
                }
                if (NULL != domain[idx]->temp && NULL != g_temp_node[idx] && NULL != g_temp_node[idx][k]) {
                    milli_deg_c = 0;
                    if (true == linux_thermal_read_value_fp(g_temp_node[idx][k], &milli_deg_c)) {
                        // Store raw milli-degrees Celsius
                        domain[idx]->temp[k] = milli_deg_c;
                    }
                }
                filled[idx]++;
            }
            closedir(dir);
        }

        g_temp_discovered = true;
        *metrics          = g_temp_metrics;
    }

    return return_code;
}

/**
 * @brief Read every cooling device's id, type, cur_state and max_state into the
 *        per-domain cooling blocks and return the library-owned metrics.
 *
 * @param[out] metrics Set to point at the library-owned cooling metrics
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NOT_INITIALIZED if the library is not initialized
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if metrics is NULL
 */
enum QcPerfReturnCode linux_thermal_lib_discover_cooling_devices(struct LinuxThermalCoolingMetrics** metrics) {
    enum QcPerfReturnCode return_code               = QC_PERF_RETURN_CODE_SUCCESS;
    DIR* dir                                        = NULL;
    struct dirent* entry                            = NULL;
    char path[LINUX_THERMAL_NODE_PATH_MAX_LEN]      = {0};
    char type[LINUX_THERMAL_ZONE_NAME_MAX_LEN]      = {0};
    uint8_t filled[LINUX_THERMAL_COOL_DOMAIN_COUNT] = {0};
    unsigned int device_id                          = 0;
    int idx                                         = -1;
    uint8_t k                                       = 0;
    uint32_t cur                                    = 0;
    uint32_t max                                    = 0;

    if (false == g_is_initialized) {
        return_code = QC_PERF_RETURN_CODE_NOT_INITIALIZED;
    } else if (NULL == metrics) {
        return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else if (true == g_cool_discovered) {
        // Already processed on a prior call: hand back the library-owned metrics directly.
        *metrics = g_cool_metrics;
    } else {
        struct LinuxThermalCoolingData* domain[LINUX_THERMAL_COOL_DOMAIN_COUNT] = {
            g_cool_metrics->cpu,        g_cool_metrics->ddr,        g_cool_metrics->gpu,     g_cool_metrics->npu,
            g_cool_metrics->display,    g_cool_metrics->storage,    g_cool_metrics->battery, g_cool_metrics->modem,
            g_cool_metrics->rf_sdr,     g_cool_metrics->multimedia, g_cool_metrics->dsds,    g_cool_metrics->thermal_fw,
        };

        // Read every cooling device once: parse its id from the entry name, read its
        // type, classify it, and store id/name/cur_state/max_state into that domain's block.
        dir = opendir(LINUX_THERMAL_SYSFS_ROOT);
        if (NULL != dir) {
            while (NULL != (entry = readdir(dir))) {
                if (1 != sscanf(entry->d_name, LINUX_THERMAL_COOLING_DEVICE_PREFIX "%u", &device_id)) {
                    continue;
                }

                snprintf(path, sizeof(path), "%s/%s/type", LINUX_THERMAL_SYSFS_ROOT, entry->d_name);
                if (false == linux_thermal_read_type(path, type, sizeof(type))) {
                    continue;
                }

                idx = linux_thermal_classify(type, g_cool_prefix, sizeof(g_cool_prefix) / sizeof(g_cool_prefix[0]));
                if (idx < 0 || NULL == domain[idx] || filled[idx] >= domain[idx]->len) {
                    continue;
                }

                k = filled[idx];
                if (NULL != domain[idx]->id) {
                    domain[idx]->id[k] = (uint8_t)device_id;
                }
                if (NULL != domain[idx]->name && NULL != domain[idx]->name[k]) {
                    snprintf((char*)domain[idx]->name[k], LINUX_THERMAL_ZONE_NAME_MAX_LEN, "%s", type);
                }
                // Open the cur_state node once and keep the handle for cheap re-reads.
                snprintf(path, sizeof(path), LINUX_THERMAL_COOLING_DEVICE_CUR_STATE_NODE_FMT, device_id);
                if (NULL != g_cool_node[idx]) {
                    g_cool_node[idx][k] = fopen(path, "r");
                }
                if (NULL != domain[idx]->cur_state && NULL != g_cool_node[idx] && NULL != g_cool_node[idx][k]) {
                    cur = 0;
                    if (true == linux_thermal_read_value_fp(g_cool_node[idx][k], &cur)) {
                        domain[idx]->cur_state[k] = (uint8_t)cur;
                    }
                }
                // max_state is a constant device capability: read it once here.
                if (NULL != domain[idx]->max_state) {
                    max = 0;
                    snprintf(path, sizeof(path), LINUX_THERMAL_COOLING_DEVICE_MAX_STATE_NODE_FMT, device_id);
                    if (true == linux_thermal_read_value(path, &max)) {
                        domain[idx]->max_state[k] = (uint8_t)max;
                    }
                }
                filled[idx]++;
            }
            closedir(dir);
        }

        g_cool_discovered = true;
        *metrics          = g_cool_metrics;
    }

    return return_code;
}

/**
 * @brief Refresh the temperature values and return the library-owned temperature
 *        metrics.
 *
 * @param[out] data Set to point at the library-owned temperature metrics
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NOT_INITIALIZED if the library is not initialized
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if data is NULL
 */
enum QcPerfReturnCode linux_thermal_lib_thermal_zones_info(struct LinuxThermalTempMetrics** data) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_SUCCESS;
    uint32_t milli_deg_c              = 0;

    if (false == g_is_initialized) {
        return_code = QC_PERF_RETURN_CODE_NOT_INITIALIZED;
    } else if (NULL == data) {
        return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        struct LinuxThermalTempData* domain[LINUX_THERMAL_TEMP_DOMAIN_COUNT] = {
            g_temp_metrics->cpu,        g_temp_metrics->gpu,    g_temp_metrics->npu,      g_temp_metrics->ddr,
            g_temp_metrics->modem,      g_temp_metrics->qmx,    g_temp_metrics->video,    g_temp_metrics->camera,
            g_temp_metrics->wireless,   g_temp_metrics->usb,    g_temp_metrics->rf_sdr,   g_temp_metrics->multimedia,
            g_temp_metrics->pmic,       g_temp_metrics->system, g_temp_metrics->battery,  g_temp_metrics->soc,
        };

        // Re-read each zone's temp through its persistent handle (rewind + reparse).
        for (uint8_t d = 0; d < LINUX_THERMAL_TEMP_DOMAIN_COUNT; d++) {
            if (NULL == domain[d] || NULL == domain[d]->temp || NULL == g_temp_node[d]) {
                continue;
            }
            for (uint8_t i = 0; i < domain[d]->len; i++) {
                milli_deg_c = 0;
                if (true == linux_thermal_read_value_fp(g_temp_node[d][i], &milli_deg_c)) {
                    // Store raw milli-degrees Celsius
                    domain[d]->temp[i] = milli_deg_c;
                }
            }
        }

        *data = g_temp_metrics;
    }

    return return_code;
}

/**
 * @brief Refresh the cooling-device states and return the library-owned cooling
 *        metrics.
 *
 * @param[out] data Set to point at the library-owned cooling metrics
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NOT_INITIALIZED if the library is not initialized
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if data is NULL
 */
enum QcPerfReturnCode linux_thermal_lib_cooling_devices_info(struct LinuxThermalCoolingMetrics** data) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_SUCCESS;
    uint32_t cur                      = 0;

    if (false == g_is_initialized) {
        return_code = QC_PERF_RETURN_CODE_NOT_INITIALIZED;
    } else if (NULL == data) {
        return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        struct LinuxThermalCoolingData* domain[LINUX_THERMAL_COOL_DOMAIN_COUNT] = {
            g_cool_metrics->cpu,        g_cool_metrics->ddr,        g_cool_metrics->gpu,     g_cool_metrics->npu,
            g_cool_metrics->display,    g_cool_metrics->storage,    g_cool_metrics->battery, g_cool_metrics->modem,
            g_cool_metrics->rf_sdr,     g_cool_metrics->multimedia, g_cool_metrics->dsds,    g_cool_metrics->thermal_fw,
        };

        // Re-read only cur_state through its persistent handle; max_state is constant
        // and was captured once during discovery.
        for (uint8_t d = 0; d < LINUX_THERMAL_COOL_DOMAIN_COUNT; d++) {
            if (NULL == domain[d] || NULL == domain[d]->cur_state || NULL == g_cool_node[d]) {
                continue;
            }
            for (uint8_t i = 0; i < domain[d]->len; i++) {
                cur = 0;
                if (true == linux_thermal_read_value_fp(g_cool_node[d][i], &cur)) {
                    domain[d]->cur_state[i] = (uint8_t)cur;
                }
            }
        }

        *data = g_cool_metrics;
    }

    return return_code;
}

/**
 * @brief Detach the caller's temperature-metrics view (set *data to NULL); the
 *        storage stays library-owned.
 *
 * @param[in,out] data Caller's metrics pointer; cleared to NULL
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if data is NULL
 */
enum QcPerfReturnCode linux_thermal_lib_free_thermal_zones_info(struct LinuxThermalTempMetrics** data) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == data) {
        return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        // Detach the caller's view; the metrics remain library-owned (freed by cleanup).
        *data = NULL;
    }

    return return_code;
}

/**
 * @brief Detach the caller's cooling-metrics view (set *data to NULL); the
 *        storage stays library-owned.
 *
 * @param[in,out] data Caller's metrics pointer; cleared to NULL
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if data is NULL
 */
enum QcPerfReturnCode linux_thermal_lib_free_cooling_devices_info(struct LinuxThermalCoolingMetrics** data) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == data) {
        return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        // Detach the caller's view; the metrics remain library-owned (freed by cleanup).
        *data = NULL;
    }

    return return_code;
}
