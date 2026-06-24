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
 * @file qcperf_npu_logger.c
 * @brief systemd-compatible daemon that writes NPU metrics to a file once per second.
 *
 * This daemon connects to the QcPerf NPU backend (QC_PERF_BACKEND_DSP_NPU) and
 * samples all four NPU metrics at 100 ms intervals, streaming a snapshot to
 * /var/log/qcperf/npu_metrics.log every 1000 ms (once per second).
 *
 * Each write atomically replaces the output file with a fresh key:value snapshot:
 *
 *   Q6 Utilization:42.50
 *   Q6 Clock:614400.00
 *   HVX Utilization:12.30
 *   HMX Utilization:8.70
 *
 * The daemon handles SIGTERM and SIGINT for clean shutdown.
 *
 * Runtime requirement: /usr/lib/libcdsprpc.so must be present on the target device.
 */

/* Enable POSIX.1-2008 extensions (sigaction, sigemptyset, etc.) */
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "qcperf.h"
#include "qcperf_common.h"

/** @brief Sampling rate in milliseconds — 100 ms is in the NPU backend's supported list. */
#define NPU_LOGGER_SAMPLING_RATE_MS 100

/** @brief Streaming rate in milliseconds — 1000 ms delivers one callback per second. */
#define NPU_LOGGER_STREAMING_RATE_MS 1000

/** @brief Directory that holds the output file. Created at startup if absent. */
#define NPU_LOGGER_OUTPUT_DIR "/var/log/qcperf"

/** @brief Final path of the metrics snapshot file. */
#define NPU_LOGGER_OUTPUT_FILE "/var/log/qcperf/npu_metrics.log"

/** @brief Temporary file used for atomic rename. Must be on the same filesystem. */
#define NPU_LOGGER_TMP_FILE "/var/log/qcperf/.npu_metrics.tmp"

/** @brief Maximum length of a single formatted line in the output file. */
#define NPU_LOGGER_LINE_MAX 128

/**
 * @brief Flag set to 0 by signal handlers to request a clean shutdown.
 *
 * Declared volatile and sig_atomic_t so it is safe to write from a signal
 * handler and read from the main loop without a data race.
 */
static volatile sig_atomic_t g_running = 1;

/**
 * @brief Deep copy of backend capability info used by the data callback.
 *
 * Populated once after qcperf_get_capabilities_info() succeeds and freed
 * during shutdown.  The data callback reads it (read-only) to resolve metric
 * names and units by metric_id.
 */
static struct QcPerfBackendInfo *g_backend_info = NULL;

/* =========================================================================
 * Signal handling
 * ========================================================================= */

/**
 * @brief Signal handler for SIGTERM and SIGINT.
 *
 * Sets g_running to 0 so the main loop exits cleanly on the next iteration.
 *
 * @param[in] signum Signal number (unused beyond satisfying the handler signature).
 */
static void signal_handler(int signum) {
    (void)signum;
    g_running = 0;
}

/* =========================================================================
 * Backend info helpers
 * ========================================================================= */

/**
 * @brief Create a deep copy of backend_info into g_backend_info.
 *
 * Allocates g_backend_info and its nested arrays so the data callback can
 * safely look up metric names after qcperf_get_capabilities_info() returns.
 * On any allocation failure the function frees all memory it has allocated,
 * sets g_backend_info to NULL, and returns a non-success code.
 *
 * @param[in] backend_info Source structure returned by qcperf_get_capabilities_info().
 * @return QC_PERF_RETURN_CODE_SUCCESS on success.
 * @return QC_PERF_RETURN_CODE_INVALID_ARGUMENTS if backend_info is NULL.
 * @return QC_PERF_RETURN_CODE_CALLOC_FAILED if any allocation fails.
 */
static enum QcPerfReturnCode copy_backend_info(const struct QcPerfBackendInfo *backend_info) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_SUCCESS;
    uint8_t cap_idx = 0;
    uint8_t met_idx = 0;

    if (NULL == backend_info) {
        return_code = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
    } else {
        g_backend_info = (struct QcPerfBackendInfo *)calloc(1, sizeof(struct QcPerfBackendInfo));
        if (NULL == g_backend_info) {
            fprintf(stderr, "[ERROR] calloc failed for g_backend_info\n");
            return_code = QC_PERF_RETURN_CODE_CALLOC_FAILED;
        } else {
            g_backend_info->backend_id = backend_info->backend_id;
            g_backend_info->capabilities_list_length = backend_info->capabilities_list_length;

            g_backend_info->capabilities_list = (struct QcPerfCapabilityInfo *)calloc(backend_info->capabilities_list_length, sizeof(struct QcPerfCapabilityInfo));
            if (NULL == g_backend_info->capabilities_list) {
                fprintf(stderr, "[ERROR] calloc failed for capabilities_list\n");
                free(g_backend_info);
                g_backend_info = NULL;
                return_code = QC_PERF_RETURN_CODE_CALLOC_FAILED;
            } else {
                for (cap_idx = 0; cap_idx < backend_info->capabilities_list_length && QC_PERF_RETURN_CODE_SUCCESS == return_code; cap_idx++) {
                    memcpy(&g_backend_info->capabilities_list[cap_idx], &backend_info->capabilities_list[cap_idx], sizeof(struct QcPerfCapabilityInfo));

                    g_backend_info->capabilities_list[cap_idx].metric_ids_list =
                        (struct QcPerfMetricInfo *)calloc(backend_info->capabilities_list[cap_idx].metric_ids_list_len, sizeof(struct QcPerfMetricInfo));

                    if (NULL == g_backend_info->capabilities_list[cap_idx].metric_ids_list) {
                        fprintf(stderr, "[ERROR] calloc failed for metric_ids_list (cap %u)\n", (unsigned)cap_idx);
                        /* Free all metric_ids_list arrays allocated so far. */
                        for (uint8_t cleanup = 0; cleanup < cap_idx; cleanup++) {
                            free(g_backend_info->capabilities_list[cleanup].metric_ids_list);
                            g_backend_info->capabilities_list[cleanup].metric_ids_list = NULL;
                        }
                        free(g_backend_info->capabilities_list);
                        g_backend_info->capabilities_list = NULL;
                        free(g_backend_info);
                        g_backend_info = NULL;
                        return_code = QC_PERF_RETURN_CODE_CALLOC_FAILED;
                    } else {
                        for (met_idx = 0; met_idx < backend_info->capabilities_list[cap_idx].metric_ids_list_len; met_idx++) {
                            memcpy(&g_backend_info->capabilities_list[cap_idx].metric_ids_list[met_idx], &backend_info->capabilities_list[cap_idx].metric_ids_list[met_idx],
                                   sizeof(struct QcPerfMetricInfo));
                        }
                    }
                }
            }
        }
    }
    return return_code;
}

/**
 * @brief Free all memory owned by g_backend_info and set it to NULL.
 */
static void free_backend_info(void) {
    uint8_t cap_idx = 0;

    if (NULL == g_backend_info) {
        return;
    }

    if (NULL != g_backend_info->capabilities_list) {
        for (cap_idx = 0; cap_idx < g_backend_info->capabilities_list_length; cap_idx++) {
            if (NULL != g_backend_info->capabilities_list[cap_idx].metric_ids_list) {
                free(g_backend_info->capabilities_list[cap_idx].metric_ids_list);
                g_backend_info->capabilities_list[cap_idx].metric_ids_list = NULL;
            }
        }
        free(g_backend_info->capabilities_list);
        g_backend_info->capabilities_list = NULL;
    }

    free(g_backend_info);
    g_backend_info = NULL;
}

/* =========================================================================
 * Metric name / unit lookup
 * ========================================================================= */

/**
 * @brief Look up the name and unit strings for a metric by its ID.
 *
 * Searches the deep-copied g_backend_info for the capability identified by
 * capability_id, then scans its metric_ids_list for metric_id.
 *
 * @param[in]  capability_id  Capability that produced the metric.
 * @param[in]  metric_id      Metric identifier to look up.
 * @param[out] out_name       Set to the metric_name string on success, NULL otherwise.
 */
static void lookup_metric_info(uint8_t capability_id, uint16_t metric_id, const char **out_name) {
    uint8_t met_idx = 0;

    *out_name = NULL;

    if (NULL == g_backend_info || NULL == g_backend_info->capabilities_list || capability_id >= g_backend_info->capabilities_list_length) {
        return;
    }

    if (NULL == g_backend_info->capabilities_list[capability_id].metric_ids_list) {
        return;
    }

    for (met_idx = 0; met_idx < g_backend_info->capabilities_list[capability_id].metric_ids_list_len; met_idx++) {
        if (g_backend_info->capabilities_list[capability_id].metric_ids_list[met_idx].metric_id == metric_id) {
            *out_name = g_backend_info->capabilities_list[capability_id].metric_ids_list[met_idx].metric_name;
            break;
        }
    }
}

/* =========================================================================
 * Data callback
 * ========================================================================= */

/**
 * @brief QcPerf data callback — writes a key:value snapshot once per streaming interval.
 *
 * Called by the QcPerf background thread every NPU_LOGGER_STREAMING_RATE_MS milliseconds.
 * The callback writes all metric samples from the current streaming window to a temporary
 * file and then atomically renames it to NPU_LOGGER_OUTPUT_FILE so readers never see a
 * partial write.
 *
 * Only the most recent sample for each metric (the last entry in metric_response for that
 * metric_id) is written; earlier samples within the streaming window are discarded because
 * the file represents a point-in-time snapshot, not a history.
 *
 * Output format (one line per metric):
 * @code
 *   Q6 Utilization:42.50
 *   Q6 Clock:614400.00
 *   HVX Utilization:12.30
 *   HMX Utilization:8.70
 * @endcode
 *
 * @param[in] data  Pointer to the QcPerfData structure delivered by the backend.
 * @return QC_PERF_RETURN_CODE_SUCCESS on success.
 * @return QC_PERF_RETURN_CODE_FAILED if data is NULL or the file cannot be written.
 */
static enum QcPerfReturnCode data_callback(struct QcPerfData *data) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_FAILED;
    FILE *fp = NULL;
    uint32_t idx = 0;
    const char *metric_name = NULL;

    if (NULL == data) {
        fprintf(stderr, "[ERROR] data_callback received NULL data\n");
        return_code = QC_PERF_RETURN_CODE_FAILED;
    } else {
        fp = fopen(NPU_LOGGER_TMP_FILE, "w");
        if (NULL == fp) {
            fprintf(stderr, "[ERROR] fopen(%s) failed: %s\n", NPU_LOGGER_TMP_FILE, strerror(errno));
            return_code = QC_PERF_RETURN_CODE_FAILED;
        } else {
            /*
             * Write the last sample seen for each metric_id.  Iterating in
             * reverse means the first occurrence we find (from the end) is the
             * most recent sample for that metric within the streaming window.
             * We track which metric_ids have already been written using a simple
             * bitmask (metric IDs 0-63 are sufficient for the NPU backend).
             */
            uint64_t written_mask = 0;

            for (idx = data->metric_response_len; idx > 0; idx--) {
                uint16_t mid = data->metric_response[idx - 1].metric_id;
                uint64_t bit = (mid < 64) ? ((uint64_t)1 << mid) : 0;

                if (0 != bit && 0 == (written_mask & bit)) {
                    written_mask |= bit;

                    lookup_metric_info(data->capabilityId, mid, &metric_name);

                    if (NULL == metric_name) {
                        fprintf(fp, "metric_%u:", (unsigned)mid);
                    } else {
                        fprintf(fp, "%s:", metric_name);
                    }

                    switch (data->metric_response[idx - 1].metric_value.data_type) {
                    case QC_PERF_DATA_TYPE_DOUBLE:
                        fprintf(fp, "%.2f", data->metric_response[idx - 1].metric_value.double_value);
                        break;
                    case QC_PERF_DATA_TYPE_UINT64:
                        fprintf(fp, "%llu", (unsigned long long)data->metric_response[idx - 1].metric_value.uint64_value);
                        break;
                    case QC_PERF_DATA_TYPE_INT64:
                        fprintf(fp, "%lld", (long long)data->metric_response[idx - 1].metric_value.int64_value);
                        break;
                    case QC_PERF_DATA_TYPE_BOOL:
                        if (data->metric_response[idx - 1].metric_value.bool_value) {
                            fprintf(fp, "true");
                        } else {
                            fprintf(fp, "false");
                        }
                        break;
                    default:
                        fprintf(fp, "unknown");
                        break;
                    }

                    fprintf(fp, "\n");
                }
            }

            fclose(fp);
            fp = NULL;

            if (0 != rename(NPU_LOGGER_TMP_FILE, NPU_LOGGER_OUTPUT_FILE)) {
                fprintf(stderr, "[ERROR] rename(%s, %s) failed: %s\n", NPU_LOGGER_TMP_FILE, NPU_LOGGER_OUTPUT_FILE, strerror(errno));
                return_code = QC_PERF_RETURN_CODE_FAILED;
            } else {
                return_code = QC_PERF_RETURN_CODE_SUCCESS;
            }
        }
    }
    return return_code;
}

/* =========================================================================
 * Message callback
 * ========================================================================= */

/**
 * @brief QcPerf message callback — forwards backend messages to stderr.
 *
 * DEBUG-level messages are silently suppressed; all other levels are printed
 * with a severity prefix so they appear in the systemd journal.
 *
 * @param[in] message  Pointer to the QcPerfMessage structure from the backend.
 * @return QC_PERF_RETURN_CODE_SUCCESS on success.
 * @return QC_PERF_RETURN_CODE_FAILED if message or message->message is NULL.
 */
static enum QcPerfReturnCode message_callback(struct QcPerfMessage *message) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_FAILED;
    const char *level_str = "UNKNOWN";

    if (NULL == message || NULL == message->message) {
        fprintf(stderr, "[ERROR] message_callback received NULL message\n");
        return_code = QC_PERF_RETURN_CODE_FAILED;
    } else {
        switch (message->message_level) {
        case QC_PERF_MESSAGE_LEVEL_DEBUG:
            level_str = "DEBUG";
            break;
        case QC_PERF_MESSAGE_LEVEL_INFO:
            level_str = "INFO";
            break;
        case QC_PERF_MESSAGE_LEVEL_WARNING:
            level_str = "WARNING";
            break;
        case QC_PERF_MESSAGE_LEVEL_ERROR:
            level_str = "ERROR";
            break;
        default:
            level_str = "UNKNOWN";
            break;
        }

        if (message->message_level != QC_PERF_MESSAGE_LEVEL_DEBUG) {
            fprintf(stderr, "[%s] %s\n", level_str, message->message);
        }

        return_code = QC_PERF_RETURN_CODE_SUCCESS;
    }
    return return_code;
}

/* =========================================================================
 * main
 * ========================================================================= */

/**
 * @brief Entry point for the QcPerfNpuLogger daemon.
 *
 * Performs the following steps:
 *  1. Install signal handlers for SIGTERM and SIGINT.
 *  2. Create the output directory /var/log/qcperf if it does not exist.
 *  3. Initialise the QcPerf library.
 *  4. Connect to the NPU backend.
 *  5. Query capabilities and deep-copy the backend info.
 *  6. Register the data callback.
 *  7. Start monitoring (sampling_rate=100 ms, streaming_rate=1000 ms).
 *  8. Sleep in a loop until a signal sets g_running to 0.
 *  9. Stop monitoring, disconnect, deinitialise, and free resources.
 *
 * @return 0 on clean exit, 1 on any fatal error.
 */
int main(void) {
    int exit_code = 0;
    enum QcPerfReturnCode rc = QC_PERF_RETURN_CODE_FAILED;
    struct QcPerfBackendInfo *backend_info = NULL;
    struct QcPerfRequest *request = NULL;
    struct sigaction sa = {0};

    /* ------------------------------------------------------------------
     * Step 1: Install signal handlers.
     * ------------------------------------------------------------------ */
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (0 != sigaction(SIGTERM, &sa, NULL)) {
        fprintf(stderr, "[ERROR] sigaction(SIGTERM) failed: %s\n", strerror(errno));
        exit_code = 1;
    } else if (0 != sigaction(SIGINT, &sa, NULL)) {
        fprintf(stderr, "[ERROR] sigaction(SIGINT) failed: %s\n", strerror(errno));
        exit_code = 1;
    } else {
        /* ------------------------------------------------------------------
         * Step 2: Ensure the output directory exists.
         * ------------------------------------------------------------------ */
        if (0 != mkdir(NPU_LOGGER_OUTPUT_DIR, 0755) && EEXIST != errno) {
            fprintf(stderr, "[ERROR] mkdir(%s) failed: %s\n", NPU_LOGGER_OUTPUT_DIR, strerror(errno));
            exit_code = 1;
        } else {
            /* ------------------------------------------------------------------
             * Step 3: Initialise QcPerf.
             * ------------------------------------------------------------------ */
            rc = qcperf_init();
            if (QC_PERF_RETURN_CODE_SUCCESS != rc) {
                fprintf(stderr, "[ERROR] qcperf_init() failed (rc=%d)\n", (int)rc);
                exit_code = 1;
            } else {
                /* ------------------------------------------------------------------
                 * Step 4: Connect to the NPU backend.
                 * ------------------------------------------------------------------ */
                rc = qcperf_connect_backend(QC_PERF_BACKEND_DSP_NPU, &message_callback);
                if (QC_PERF_RETURN_CODE_SUCCESS != rc) {
                    fprintf(stderr, "[ERROR] qcperf_connect_backend(NPU) failed (rc=%d)\n", (int)rc);
                    exit_code = 1;
                } else {
                    /* ------------------------------------------------------------------
                     * Step 5: Query capabilities and deep-copy backend info.
                     * ------------------------------------------------------------------ */
                    backend_info = (struct QcPerfBackendInfo *)calloc(1, sizeof(struct QcPerfBackendInfo));
                    if (NULL == backend_info) {
                        fprintf(stderr, "[ERROR] calloc failed for backend_info\n");
                        exit_code = 1;
                    } else {
                        rc = qcperf_get_capabilities_info(QC_PERF_BACKEND_DSP_NPU, backend_info);
                        if (QC_PERF_RETURN_CODE_SUCCESS != rc || 0 == backend_info->capabilities_list_length || NULL == backend_info->capabilities_list) {
                            fprintf(stderr, "[ERROR] qcperf_get_capabilities_info() failed (rc=%d)\n", (int)rc);
                            exit_code = 1;
                        } else {
                            rc = copy_backend_info(backend_info);
                            if (QC_PERF_RETURN_CODE_SUCCESS != rc) {
                                fprintf(stderr, "[WARNING] copy_backend_info() failed — metric names will be unavailable\n");
                                /* Non-fatal: the daemon can still log metric_id numbers. */
                            }

                            /* ------------------------------------------------------------------
                             * Step 6: Register the data callback.
                             * ------------------------------------------------------------------ */
                            rc = qcperf_set_data_callback(QC_PERF_BACKEND_DSP_NPU, &data_callback);
                            if (QC_PERF_RETURN_CODE_SUCCESS != rc) {
                                fprintf(stderr, "[ERROR] qcperf_set_data_callback() failed (rc=%d)\n", (int)rc);
                                exit_code = 1;
                            } else {
                                /* ------------------------------------------------------------------
                                 * Step 7: Start monitoring.
                                 * ------------------------------------------------------------------ */
                                request = (struct QcPerfRequest *)calloc(1, sizeof(struct QcPerfRequest));
                                if (NULL == request) {
                                    fprintf(stderr, "[ERROR] calloc failed for request\n");
                                    exit_code = 1;
                                } else {
                                    request->capability_id = backend_info->capabilities_list[0].capability_id;
                                    request->sampling_rate = NPU_LOGGER_SAMPLING_RATE_MS;
                                    request->streaming_rate = NPU_LOGGER_STREAMING_RATE_MS;

                                    rc = qcperf_start(QC_PERF_BACKEND_DSP_NPU, request);
                                    if (QC_PERF_RETURN_CODE_SUCCESS != rc) {
                                        fprintf(stderr, "[ERROR] qcperf_start() failed (rc=%d)\n", (int)rc);
                                        exit_code = 1;
                                    } else {
                                        fprintf(stderr, "[INFO] NPU logger started — writing to %s\n", NPU_LOGGER_OUTPUT_FILE);

                                        /* ------------------------------------------------------------------
                                         * Step 8: Run until signalled.
                                         * ------------------------------------------------------------------ */
                                        while (1 == g_running) {
                                            sleep(1);
                                        }

                                        fprintf(stderr, "[INFO] Shutdown signal received — stopping\n");

                                        /* ------------------------------------------------------------------
                                         * Step 9a: Stop monitoring.
                                         * ------------------------------------------------------------------ */
                                        rc = qcperf_stop(QC_PERF_BACKEND_DSP_NPU, request);
                                        if (QC_PERF_RETURN_CODE_SUCCESS != rc) {
                                            fprintf(stderr, "[WARNING] qcperf_stop() failed (rc=%d)\n", (int)rc);
                                        }
                                    }

                                    free(request);
                                    request = NULL;
                                }
                            }
                        }
                    }

                    /* ------------------------------------------------------------------
                     * Step 9b: Disconnect backend.
                     * ------------------------------------------------------------------ */
                    rc = qcperf_disconnect_backend(QC_PERF_BACKEND_DSP_NPU);
                    if (QC_PERF_RETURN_CODE_SUCCESS != rc) {
                        fprintf(stderr, "[WARNING] qcperf_disconnect_backend() failed (rc=%d)\n", (int)rc);
                    }
                }

                /* ------------------------------------------------------------------
                 * Step 9c: Deinitialise QcPerf.
                 * ------------------------------------------------------------------ */
                rc = qcperf_deinit();
                if (QC_PERF_RETURN_CODE_SUCCESS != rc) {
                    fprintf(stderr, "[WARNING] qcperf_deinit() failed (rc=%d)\n", (int)rc);
                }
            }
        }
    }

    /* ------------------------------------------------------------------
     * Step 9d: Free caller-owned memory.
     * ------------------------------------------------------------------ */
    if (NULL != backend_info) {
        free(backend_info);
        backend_info = NULL;
    }

    free_backend_info();

    return exit_code;
}
