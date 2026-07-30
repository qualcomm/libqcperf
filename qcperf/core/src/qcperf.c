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
 * @file qcperf.c
 * @brief Implementation of the QcPerf library main API
 * @author Skand Gupta (skangupt@qti.qualcomm.com)
 *
 * This file implements the public API functions defined in qcperf.h, providing
 * the core functionality of the QcPerf library. It manages backend connections,
 * handles performance monitoring requests, and coordinates data collection and
 * delivery between backends and applications.
 *
 * The implementation follows a modular design where each backend is represented
 * by a set of function pointers that are initialized during backend connection.
 * This allows for a flexible and extensible architecture where new backends can
 * be added without modifying the core library code.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "qcperf.h"
#include "qcperf_common.h"
#include "qcperf_backends.h"
#include "internal/qcperf_backend_interface.h"
#include "version_info.h"

// Private function declarations for backend-specific implementation of public API
static enum QcPerfReturnCode qcperf_reset_backend_private(enum QcPerfBackendId backend_id);
static enum QcPerfReturnCode qcperf_verify_backend_private(enum QcPerfBackendId backend_id);
static enum QcPerfReturnCode qcperf_set_message_callback(enum QcPerfBackendId backend_id, QcPerfMessageCallback message_callback);

/**
 * @brief Global array of backend interface structures
 *
 * This array stores the function pointers for each backend's implementation
 * of the QcPerf interface. It is allocated during qcperf_init() and freed
 * during qcperf_deinit().
 */
static struct QcPerfBackendPrivate* g_qcperf_backend_info = NULL;

/**
 * @brief Connection status for each backend
 *
 * This array tracks which backends are currently connected.
 * Each element corresponds to a backend ID, where true indicates
 * the backend is connected and false indicates it is not.
 */
bool g_backends_connected[QC_PERF_BACKEND_MAX] = {0};  // [0] = first backend, [1] = second backend, etc.

enum QcPerfReturnCode qcperf_init(void) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_FAILED;
    if (NULL != g_qcperf_backend_info) {
        return_code = QC_PERF_RETURN_CODE_ALREADY_INITIALIZED;
    } else {
        g_qcperf_backend_info = (struct QcPerfBackendPrivate*)calloc(QC_PERF_BACKEND_MAX, sizeof(struct QcPerfBackendPrivate));
        if (NULL != g_qcperf_backend_info) {
            return_code = QC_PERF_RETURN_CODE_SUCCESS;
        } else {
            return_code = QC_PERF_RETURN_CODE_CALLOC_FAILED;
        }
    }
    return return_code;
}

enum QcPerfReturnCode qcperf_version(struct QcPerfVersionInfo* version_info) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_SUCCESS;

    if (NULL == version_info) {
        return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    } else {
        version_info->major = LIBQCPERF_VERSION_MAJOR;
        version_info->minor = LIBQCPERF_VERSION_MINOR;
        version_info->patch = LIBQCPERF_VERSION_PATCH;
        version_info->build = LIBQCPERF_VERSION_BUILD;
    }

    return return_code;
}

enum QcPerfReturnCode qcperf_connect_backend(enum QcPerfBackendId backend_id, QcPerfMessageCallback message_callback) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_FAILED;
    if (NULL == g_qcperf_backend_info) {
        return_code = QC_PERF_RETURN_CODE_NOT_INITIALIZED;
    } else if (0 > backend_id || QC_PERF_BACKEND_MAX <= backend_id) {
        return_code = QC_PERF_RETURN_CODE_INVALID_BACKEND_ID;
    } else if (true == g_backends_connected[backend_id]) {
        return_code = QC_PERF_RETURN_CODE_BACKEND_ALREADY_CONNECTED;
    } else if (NULL == backend_init_fns[backend_id]) {
        return_code = QC_PERF_RETURN_CODE_INVALID_BACKEND_ID;
    } else {
        return_code = backend_init_fns[backend_id](&g_qcperf_backend_info[backend_id]);  // Call the backend creation function for the specified backend
        if (QC_PERF_RETURN_CODE_SUCCESS == return_code) {
            return_code = qcperf_verify_backend_private(backend_id);
            if (QC_PERF_RETURN_CODE_SUCCESS == return_code) {
                g_backends_connected[backend_id] = true;
                if (message_callback != NULL) {
                    return_code = qcperf_set_message_callback(backend_id, message_callback);  // Best case if nessage callback is passed
                }
                return_code = g_qcperf_backend_info[backend_id].qcperf_backend_init();
                if (QC_PERF_RETURN_CODE_SUCCESS == return_code) {
                    return_code = QC_PERF_RETURN_CODE_SUCCESS;
                } else {
                    qcperf_disconnect_backend(backend_id);
                }
            }
        }
    }
    return return_code;
}

enum QcPerfReturnCode qcperf_get_capabilities_info(enum QcPerfBackendId backend_id, struct QcPerfBackendInfo* backend_info) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_FAILED;
    if (NULL == g_qcperf_backend_info) {
        return_code = QC_PERF_RETURN_CODE_NOT_INITIALIZED;
    } else if (0 > backend_id || QC_PERF_BACKEND_MAX < backend_id || NULL == backend_info) {
        return_code = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
    } else if (false == g_backends_connected[backend_id]) {
        return_code = QC_PERF_RETURN_CODE_BACKEND_NOT_CONNECTED;
    } else {
        return_code = g_qcperf_backend_info[backend_id].qcperf_backend_info(backend_info);  // Call the backend get capabilities function for the specified backend
    }
    return return_code;
}

enum QcPerfReturnCode qcperf_set_data_callback(enum QcPerfBackendId backend_id, QcPerfDataCallback data_callback) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_FAILED;
    if (NULL == g_qcperf_backend_info) {
        return_code = QC_PERF_RETURN_CODE_NOT_INITIALIZED;
    } else if (0 > backend_id || QC_PERF_BACKEND_MAX < backend_id || NULL == data_callback) {
        return_code = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
    } else if (false == g_backends_connected[backend_id]) {
        return_code = QC_PERF_RETURN_CODE_BACKEND_NOT_CONNECTED;
    } else {
        return_code = g_qcperf_backend_info[backend_id].set_data_callback(data_callback);
    }
    return return_code;
}

enum QcPerfReturnCode qcperf_start(enum QcPerfBackendId backend_id, struct QcPerfRequest* request) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_FAILED;
    if (NULL == g_qcperf_backend_info) {
        return_code = QC_PERF_RETURN_CODE_NOT_INITIALIZED;
    } else if (0 > backend_id || QC_PERF_BACKEND_MAX < backend_id || NULL == request) {
        return_code = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
    } else if (false == g_backends_connected[backend_id]) {
        return_code = QC_PERF_RETURN_CODE_BACKEND_NOT_CONNECTED;
    } else {
        return_code = g_qcperf_backend_info[backend_id].qcperf_backend_start(request);
    }
    return return_code;
}

enum QcPerfReturnCode qcperf_stop(enum QcPerfBackendId backend_id, struct QcPerfRequest* request) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_FAILED;
    if (NULL == g_qcperf_backend_info) {
        return_code = QC_PERF_RETURN_CODE_NOT_INITIALIZED;
    } else if (0 > backend_id || QC_PERF_BACKEND_MAX < backend_id || NULL == request) {
        return_code = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
    } else if (false == g_backends_connected[backend_id]) {
        return_code = QC_PERF_RETURN_CODE_BACKEND_NOT_CONNECTED;
    } else {
        return_code = g_qcperf_backend_info[backend_id].qcperf_backend_stop(request);
    }
    return return_code;
}

enum QcPerfReturnCode qcperf_disconnect_backend(enum QcPerfBackendId backend_id) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_FAILED;
    if (NULL == g_qcperf_backend_info) {
        return_code = QC_PERF_RETURN_CODE_NOT_INITIALIZED;
    } else if (0 > backend_id || QC_PERF_BACKEND_MAX < backend_id) {
        return_code = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
    } else if (false == g_backends_connected[backend_id]) {
        return_code = QC_PERF_RETURN_CODE_BACKEND_NOT_CONNECTED;
    } else {
        g_backends_connected[backend_id] = false;
        if (NULL != g_qcperf_backend_info[backend_id].qcperf_backend_deinit) {
            g_qcperf_backend_info[backend_id].qcperf_backend_deinit();
        }
        return_code = qcperf_reset_backend_private(backend_id);
    }
    return return_code;
}

enum QcPerfReturnCode qcperf_deinit(void) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_FAILED;
    if (NULL == g_qcperf_backend_info) {
        return_code = QC_PERF_RETURN_CODE_NOT_INITIALIZED;
    } else {
        // check for any active backends and disconnect them if needed
        for (enum QcPerfBackendId backend_id = 0; backend_id < QC_PERF_BACKEND_MAX; backend_id++) {
            if (true == g_backends_connected[backend_id]) {
                return_code = qcperf_disconnect_backend(backend_id);
            }
        }
        free(g_qcperf_backend_info);
        g_qcperf_backend_info = NULL;
        return_code           = QC_PERF_RETURN_CODE_SUCCESS;
    }
    return return_code;
}

enum QcPerfReturnCode qcperf_get_error_info(enum QcPerfReturnCode return_code, struct QcPerfReturnCodeInfo* return_info) {
    enum QcPerfReturnCode ret = QC_PERF_RETURN_CODE_FAILED;
    if (NULL == return_info) {
        ret = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
    } else {
        return_info->return_code = return_code;
        switch (return_code) {
        case QC_PERF_RETURN_CODE_SUCCESS:
            return_info->info_str_len = (size_t)snprintf((char*)return_info->info_str, RETURN_CODE_INFO_STRING_MAX_LEN, "%s", "Operation completed successfully");
            break;
        case QC_PERF_RETURN_CODE_FAILED:
            return_info->info_str_len = (size_t)snprintf((char*)return_info->info_str, RETURN_CODE_INFO_STRING_MAX_LEN, "%s", "Requested API call failed due to an internal error");
            break;
        case QC_PERF_RETURN_CODE_NOT_SUPPORTED:
            return_info->info_str_len = (size_t)snprintf((char*)return_info->info_str, RETURN_CODE_INFO_STRING_MAX_LEN, "%s", "The requested operation is not supported by this backend");
            break;
        case QC_PERF_RETURN_CODE_ALREADY_INITIALIZED:
            return_info->info_str_len =
                (size_t)snprintf((char*)return_info->info_str, RETURN_CODE_INFO_STRING_MAX_LEN, "%s", "QCPerf library is already initialized, call qcperf_deinit before reinitializing");
            break;
        case QC_PERF_RETURN_CODE_INVALID_HANDLE:
            return_info->info_str_len = (size_t)snprintf((char*)return_info->info_str, RETURN_CODE_INFO_STRING_MAX_LEN, "%s", "Invalid handle provided to the function, please provide a valid handle");
            break;
        case QC_PERF_RETURN_CODE_INVALID_ARGUMENTS:
            return_info->info_str_len =
                (size_t)snprintf((char*)return_info->info_str, RETURN_CODE_INFO_STRING_MAX_LEN, "%s", "Invalid arguments provided to the function, please check function parameters");
            break;
        case QC_PERF_RETURN_CODE_NOT_INITIALIZED:
            return_info->info_str_len =
                (size_t)snprintf((char*)return_info->info_str, RETURN_CODE_INFO_STRING_MAX_LEN, "%s", "QCPerf library is not initialized, call qcperf_init before using other functions");
            break;
        case QC_PERF_RETURN_CODE_CAPABILITY_NOT_FOUND:
            return_info->info_str_len =
                (size_t)snprintf((char*)return_info->info_str, RETURN_CODE_INFO_STRING_MAX_LEN, "%s", "The requested capability was not found in the backend, check available capabilities");
            break;
        case QC_PERF_RETURN_CODE_INVALID_BUFFER_SIZE:
            return_info->info_str_len = (size_t)snprintf((char*)return_info->info_str, RETURN_CODE_INFO_STRING_MAX_LEN, "%s", "The provided buffer size is invalid or insufficient for the operation");
            break;
        case QC_PERF_RETURN_CODE_NULL_POINTER:
            return_info->info_str_len = (size_t)snprintf((char*)return_info->info_str, RETURN_CODE_INFO_STRING_MAX_LEN, "%s", "A null pointer was provided where a valid pointer is required");
            break;
        case QC_PERF_RETURN_CODE_CALLOC_FAILED:
            return_info->info_str_len = (size_t)snprintf((char*)return_info->info_str, RETURN_CODE_INFO_STRING_MAX_LEN, "%s", "Memory allocation failed during operation, check system resources");
            break;
        case QC_PERF_RETURN_CODE_BACKEND_ALREADY_CONNECTED:
            return_info->info_str_len = (size_t)snprintf((char*)return_info->info_str, RETURN_CODE_INFO_STRING_MAX_LEN, "%s", "Backend is already connected, disconnect first before reconnecting");
            break;
        case QC_PERF_RETURN_CODE_INVALID_BACKEND_ID:
            return_info->info_str_len = (size_t)snprintf((char*)return_info->info_str, RETURN_CODE_INFO_STRING_MAX_LEN, "%s", "Invalid backend identifier provided, check available backends");
            break;
        case QC_PERF_RETURN_CODE_BACKEND_NOT_CONNECTED:
            return_info->info_str_len =
                (size_t)snprintf((char*)return_info->info_str, RETURN_CODE_INFO_STRING_MAX_LEN, "%s", "Backend is not connected, connect the backend before using this function");
            break;
        case QC_PERF_RETURN_CODE_CALLBACK_ALREADY_SET:
            return_info->info_str_len = (size_t)snprintf((char*)return_info->info_str, RETURN_CODE_INFO_STRING_MAX_LEN, "%s", "Callback function is already set for this backend");
            break;
        case QC_PERF_RETURN_CODE_NATIVE_LIBRARY_FAILED:
            return_info->info_str_len = (size_t)snprintf((char*)return_info->info_str, RETURN_CODE_INFO_STRING_MAX_LEN, "%s", "Native library call failed during operation");
            break;
        default:
            return_info->info_str_len = (size_t)snprintf((char*)return_info->info_str, RETURN_CODE_INFO_STRING_MAX_LEN, "%s", "Unknown error code encountered");
            break;
        }
        ret = QC_PERF_RETURN_CODE_SUCCESS;
    }
    return ret;
}

/**
 * @brief Set or update the message callback for a connected backend
 *
 * Registers or updates the callback function that will be invoked when the backend
 * sends messages (e.g., status updates, warnings, or informational messages).
 *
 * @param[in] backend_id The identifier of the backend
 * @param[in] message_callback The callback function to register. Can be NULL to unregister
 *                             the current callback.
 *
 * @return QC_PERF_RETURN_CODE_SUCCESS on success
 * @return QC_PERF_RETURN_CODE_NOT_INITIALIZED if library not initialized
 * @return QC_PERF_RETURN_CODE_INVALID_ARGUMENTS if backend_id is invalid
 * @return QC_PERF_RETURN_CODE_FAILED on failure
 *
 * @note The backend must be connected before calling this function
 * @see qcperf_connect_backend(), QcPerfMessageCallback
 */
static enum QcPerfReturnCode qcperf_set_message_callback(enum QcPerfBackendId backend_id, QcPerfMessageCallback message_callback) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_FAILED;
    if (NULL == g_qcperf_backend_info) {
        return_code = QC_PERF_RETURN_CODE_NOT_INITIALIZED;
    } else if (0 > backend_id || QC_PERF_BACKEND_MAX < backend_id || NULL == message_callback) {
        return_code = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
    } else if (false == g_backends_connected[backend_id]) {
        return_code = QC_PERF_RETURN_CODE_BACKEND_NOT_CONNECTED;
    } else {
        return_code = g_qcperf_backend_info[backend_id].set_message_callback(message_callback);
    }
    return return_code;
}

/**
 * @brief Reset a backend's function pointers to NULL
 *
 * This function resets all function pointers for a specific backend to NULL,
 * effectively clearing its interface. Used during backend disconnection and
 * library deinitialization to ensure clean state.
 *
 * @param[in] backend_id The identifier of the backend to reset
 * @return QC_PERF_RETURN_CODE_SUCCESS on successful reset
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if backend info is NULL
 */
static enum QcPerfReturnCode qcperf_reset_backend_private(enum QcPerfBackendId backend_id) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    if (0 > backend_id || QC_PERF_BACKEND_MAX < backend_id) {
        return_code = QC_PERF_RETURN_CODE_INVALID_ARGUMENTS;
    }
    if (NULL != g_qcperf_backend_info) {
        g_qcperf_backend_info[backend_id].qcperf_backend_init   = NULL;
        g_qcperf_backend_info[backend_id].qcperf_backend_start  = NULL;
        g_qcperf_backend_info[backend_id].qcperf_backend_stop   = NULL;
        g_qcperf_backend_info[backend_id].qcperf_backend_deinit = NULL;
        g_qcperf_backend_info[backend_id].qcperf_backend_info   = NULL;
        g_qcperf_backend_info[backend_id].set_message_callback  = NULL;
        g_qcperf_backend_info[backend_id].set_data_callback     = NULL;
        return_code                                             = QC_PERF_RETURN_CODE_SUCCESS;
    }
    return return_code;
}

/**
 * @brief Verify that all required function pointers are set for a backend
 *
 * This function checks that all function pointers in a backend's interface
 * structure are non-NULL, ensuring the backend is fully initialized and ready to use.
 *
 * @param[in] backend_id The identifier of the backend to verify
 * @return QC_PERF_RETURN_CODE_SUCCESS if all function pointers are set
 * @return QC_PERF_RETURN_CODE_NULL_POINTER if any function pointer is NULL
 */
static enum QcPerfReturnCode qcperf_verify_backend_private(enum QcPerfBackendId backend_id) {
    enum QcPerfReturnCode return_code = QC_PERF_RETURN_CODE_NULL_POINTER;
    if (NULL != g_qcperf_backend_info) {
        if ((NULL != g_qcperf_backend_info[backend_id].qcperf_backend_init) && (NULL != g_qcperf_backend_info[backend_id].qcperf_backend_start) &&
            (NULL != g_qcperf_backend_info[backend_id].qcperf_backend_stop) && (NULL != g_qcperf_backend_info[backend_id].qcperf_backend_deinit) &&
            (NULL != g_qcperf_backend_info[backend_id].qcperf_backend_info) && (NULL != g_qcperf_backend_info[backend_id].set_message_callback) &&
            (NULL != g_qcperf_backend_info[backend_id].set_data_callback)) {
            return_code = QC_PERF_RETURN_CODE_SUCCESS;
        }
    }
    return return_code;
}
