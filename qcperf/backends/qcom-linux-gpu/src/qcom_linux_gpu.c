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
 * @file qcom_linux_gpu.c
 * @brief Qualcomm Linux GPU backend implementation for libqcperf
 *
 * Samples Adreno GPU utilization and clock from the kgsl sysfs nodes on a
 * background thread and streams them through the data callback.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "qcom_linux_gpu.h"
#include "qcom_linux_gpu_utils.h"
#include "qcperf_backend_enum.h"
#include "qcperf_backend_interface.h"
#include "qthread.h"
#include "qtime.h"

#define MESSAGE_MAX_LEN 256

static QcPerfMessageCallback g_message_callback = NULL;
static QcPerfDataCallback g_data_callback       = NULL;

static struct QcPerfBackendInfo g_backend_info = {0};
static struct QcPerfMetricInfo g_metrics_data[LINUX_GPU_CAPABILITY_0_METRIC_COUNT];

static volatile bool g_is_thread_running = false;
static struct QThreadInfo g_thread_info  = {0};

static const uint16_t streaming_rates[LINUX_GPU_STREAMING_RATES_LEN] = {LINUX_GPU_STREAMING_RATES};
static const uint16_t sampling_rates[LINUX_GPU_SAMPLING_RATES_LEN]   = {LINUX_GPU_SAMPLING_RATES};

static inline void send_message(enum QcPerfMessageLevel level, const char* fmt, ...) {
    char msg[MESSAGE_MAX_LEN];
    struct QcPerfMessage msg_struct = {0};
    va_list args;
    int len = 0;

    va_start(args, fmt);
    len = vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    if (len > 0) {
        if (len >= MESSAGE_MAX_LEN) {
            len = MESSAGE_MAX_LEN - 1;
        }
        msg_struct.message        = msg;
        msg_struct.message_length = (size_t)len;
        msg_struct.message_level  = level;
        g_message_callback(&msg_struct);
    }
}

#define SEND_MESSAGE(level, ...)          \
    if (g_message_callback != NULL) {     \
        send_message(level, __VA_ARGS__); \
    }

static bool rate_in_list(const uint16_t* list, uint8_t len, uint16_t rate) {
    for (uint8_t i = 0; i < len; i++) {
        if (list[i] == rate) {
            return true;
        }
    }
    return false;
}

static enum QcPerfReturnCode validate_request(const struct QcPerfRequest* request) {
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_SUCCESS;

    if (LINUX_GPU_CAPABILITY_0_ID != request->capability_id) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Capability ID %u not found", request->capability_id);
        ret = QC_PERF_RETURN_CODE_CAPABILITY_NOT_FOUND;
    } else if (!rate_in_list(sampling_rates, LINUX_GPU_SAMPLING_RATES_LEN, request->sampling_rate)) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Invalid sampling rate %u", request->sampling_rate);
        ret = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
    } else if (!rate_in_list(streaming_rates, LINUX_GPU_STREAMING_RATES_LEN, request->streaming_rate)) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Invalid streaming rate %u", request->streaming_rate);
        ret = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
    }

    return ret;
}

static enum QcPerfReturnCode qcom_linux_gpu_set_message_callback(QcPerfMessageCallback message_callback) {
    g_message_callback = message_callback;
    return QC_PERF_RETURN_CODE_SUCCESS;
}

static enum QcPerfReturnCode qcom_linux_gpu_set_data_callback(QcPerfDataCallback data_callback) {
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == data_callback) {
        ret = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
    } else if (NULL != g_data_callback) {
        ret = QC_PERF_RETURN_CODE_CALLBACK_ALREADY_SET;
    } else {
        g_data_callback = data_callback;
    }

    return ret;
}

static enum QcPerfReturnCode qcom_linux_gpu_init(void) {
    enum QcPerfReturnCode ret        = QC_PERF_RETURN_CODE_SUCCESS;
    double probe                     = 0.0;
    struct QcPerfCapabilityInfo* cap = NULL;

    SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "Initializing qcom-linux-gpu backend");

    /* Fail at connect time, not mid-profile, when the board has no kgsl GPU. */
    if (QC_PERF_RETURN_CODE_SUCCESS != qcom_linux_gpu_util_read_utilization(QCOM_LINUX_GPU_SYSFS_ROOT, &probe)) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Cannot read %s/%s", QCOM_LINUX_GPU_SYSFS_ROOT, QCOM_LINUX_GPU_BUSY_NODE);
        ret = QC_PERF_RETURN_CODE_FAILED;
    } else {
        g_backend_info.capabilities_list = (struct QcPerfCapabilityInfo*)calloc(LINUX_GPU_CAPABILITIES_LEN, sizeof(struct QcPerfCapabilityInfo));
        if (NULL == g_backend_info.capabilities_list) {
            ret = QC_PERF_RETURN_CODE_CALLOC_FAILED;
        } else {
            g_backend_info.backend_id               = QC_PERF_BACKEND_QCOM_LINUX_GPU;
            g_backend_info.capabilities_list_length = LINUX_GPU_CAPABILITIES_LEN;

            linux_gpu_capability_0_init_metrics(g_metrics_data);

            cap                = &g_backend_info.capabilities_list[0];
            cap->capability_id = LINUX_GPU_CAPABILITY_0_ID;
            snprintf(cap->capability_name, CAPABILITY_NAME_MAX_LEN, "%s", LINUX_GPU_CAPABILITY_0);
            cap->capability_name_len = strlen(cap->capability_name);
            cap->metric_ids_list     = g_metrics_data;
            cap->metric_ids_list_len = LINUX_GPU_CAPABILITY_0_METRIC_COUNT;

            for (uint8_t i = 0; i < LINUX_GPU_STREAMING_RATES_LEN && i < MAX_SAMPLING_STREAMING_RATES_LEN; i++) {
                cap->streaming_rate[cap->streaming_rate_len++] = streaming_rates[i];
            }
            for (uint8_t i = 0; i < LINUX_GPU_SAMPLING_RATES_LEN && i < MAX_SAMPLING_STREAMING_RATES_LEN; i++) {
                cap->sampling_rate[cap->sampling_rate_len++] = sampling_rates[i];
            }

            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_INFO, "qcom-linux-gpu backend initialized successfully");
        }
    }

    return ret;
}

static enum QcPerfReturnCode qcom_linux_gpu_backend_info(struct QcPerfBackendInfo* backend_info) {
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == backend_info) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Backend info is NULL");
        ret = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        *backend_info = g_backend_info;
    }

    return ret;
}

/**
 * @brief Sampling thread: reads sysfs every sampling_rate ms and flushes one
 *        batch to the data callback every streaming_rate ms.
 */
static void* collect_gpu_data(void* param) {
    const struct QcPerfRequest* request = (const struct QcPerfRequest*)param;
    const uint64_t streaming_rate_ns    = (uint64_t)request->streaming_rate * 1000000ULL;
    const uint32_t samples_per_stream   = ((uint32_t)request->streaming_rate + request->sampling_rate - 1U) / request->sampling_rate;
    const uint32_t capacity             = samples_per_stream * LINUX_GPU_CAPABILITY_0_METRIC_COUNT;
    struct QcPerfData* data             = (struct QcPerfData*)calloc(1, sizeof(struct QcPerfData));
    uint64_t now                        = 0;
    uint64_t last_stream_time           = 0;
    uint32_t sample_count               = 0;

    if (NULL != data) {
        data->metric_response = (struct QcPerfMetricResponse*)calloc(capacity, sizeof(struct QcPerfMetricResponse));
    }

    if (NULL == data || NULL == data->metric_response) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to allocate memory for performance data");
        g_is_thread_running = false;
    } else {
        data->capabilityId = request->capability_id;
        data->backend_id   = QC_PERF_BACKEND_QCOM_LINUX_GPU;

        get_time_ns(&now);
        last_stream_time = now;

        while (g_is_thread_running) {
            double utilization = 0.0;
            double clock_mhz   = 0.0;

            get_time_ns(&now);

            if (QC_PERF_RETURN_CODE_SUCCESS == qcom_linux_gpu_util_read_utilization(QCOM_LINUX_GPU_SYSFS_ROOT, &utilization) &&
                QC_PERF_RETURN_CODE_SUCCESS == qcom_linux_gpu_util_read_clock_mhz(QCOM_LINUX_GPU_SYSFS_ROOT, &clock_mhz) &&
                data->metric_response_len + LINUX_GPU_CAPABILITY_0_METRIC_COUNT <= capacity) {
                const double values[LINUX_GPU_CAPABILITY_0_METRIC_COUNT] = {
                    [LINUX_GPU_METRIC_UTILIZATION] = utilization,
                    [LINUX_GPU_METRIC_CLOCK]       = clock_mhz,
                };

                for (uint32_t m = 0; m < LINUX_GPU_CAPABILITY_0_METRIC_COUNT; m++) {
                    struct QcPerfMetricResponse* r = &data->metric_response[data->metric_response_len++];

                    r->metric_id                 = g_metrics_data[m].metric_id;
                    r->timestamp                 = now;
                    r->metric_value.data_type    = QC_PERF_DATA_TYPE_DOUBLE;
                    r->metric_value.double_value = values[m];
                }
            } else {
                SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_WARNING, "Failed to read GPU sysfs nodes");
            }

            sample_count++;
            if ((sample_count >= samples_per_stream || now - last_stream_time >= streaming_rate_ns) && data->metric_response_len > 0) {
                g_data_callback(data);
                data->metric_response_len = 0;
                sample_count              = 0;
                last_stream_time          = now;
            }

            if (g_is_thread_running) {
                usleep((useconds_t)(request->sampling_rate * 1000));
            }
        }
    }

    if (NULL != data) {
        free(data->metric_response);
        free(data);
    }

    return NULL;
}

static enum QcPerfReturnCode qcom_linux_gpu_start(struct QcPerfRequest* request) {
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == request) {
        ret = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else if (QC_PERF_RETURN_CODE_SUCCESS != (ret = validate_request(request))) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Request validation failed");
    } else if (NULL == g_data_callback) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Data callback is not set");
        ret = QC_PERF_RETURN_CODE_FAILED;
    } else if (g_is_thread_running) {
        SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_WARNING, "Performance monitoring thread is already running");
        ret = QC_PERF_RETURN_CODE_ALREADY_INITIALIZED;
    } else {
        struct QThreadAttributes thread_attrs = {0};

        thread_attrs.stack_size    = 0;
        thread_attrs.thread_params = request;
        thread_attrs.thread_fn     = collect_gpu_data;
        snprintf((char*)thread_attrs.thread_name, THREAD_NAME_SIZE, "qcperf_gpu_thread");
        thread_attrs.thread_name_len = (uint8_t)strlen((char*)thread_attrs.thread_name);

        /* Set before the thread exists so a stop() right after start() still joins it. */
        g_is_thread_running = true;
        if (RET_QTHREAD_CREATE_SUCCESS != thread_create(&thread_attrs, &g_thread_info)) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to create performance monitoring thread");
            g_is_thread_running = false;
            ret                 = QC_PERF_RETURN_CODE_FAILED;
        }
    }

    return ret;
}

static enum QcPerfReturnCode qcom_linux_gpu_stop(struct QcPerfRequest* request) {
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == request) {
        ret = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else if (g_is_thread_running) {
        g_is_thread_running = false;

        if (RET_QTHREAD_JOIN_SUCCESS != thread_join(&g_thread_info) || RET_QTHREAD_DESTROY_SUCCESS != thread_destroy(&g_thread_info)) {
            SEND_MESSAGE(QC_PERF_MESSAGE_LEVEL_ERROR, "Failed to stop performance monitoring thread");
            ret = QC_PERF_RETURN_CODE_FAILED;
        }
    }

    return ret;
}

static enum QcPerfReturnCode qcom_linux_gpu_deinit(void) {
    free(g_backend_info.capabilities_list);
    g_backend_info.capabilities_list        = NULL;
    g_backend_info.capabilities_list_length = 0;
    g_data_callback                         = NULL;

    return QC_PERF_RETURN_CODE_SUCCESS;
}

enum QcPerfReturnCode qcperf_qcom_linux_gpu_create(struct QcPerfBackendPrivate* backend) {
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == backend) {
        ret = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        backend->qcperf_backend_init   = qcom_linux_gpu_init;
        backend->qcperf_backend_start  = qcom_linux_gpu_start;
        backend->qcperf_backend_stop   = qcom_linux_gpu_stop;
        backend->qcperf_backend_deinit = qcom_linux_gpu_deinit;
        backend->qcperf_backend_info   = qcom_linux_gpu_backend_info;
        backend->set_message_callback  = qcom_linux_gpu_set_message_callback;
        backend->set_data_callback     = qcom_linux_gpu_set_data_callback;
    }

    return ret;
}
