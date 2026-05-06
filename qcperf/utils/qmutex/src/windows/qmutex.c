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
 * @file qmutex.c
 * @brief Windows implementation of mutex management functions
 * @author Skand Gupta (skangupt@qti.qualcomm.com)
 *
 * This file implements the mutex management functions defined in qmutex.h
 * for Windows platforms. It provides a cross-platform abstraction layer over
 * the Windows synchronization APIs, including mutex creation, locking, unlocking,
 * and destruction operations.
 *
 * The implementation uses Windows API functions like CreateMutex, WaitForSingleObject,
 * ReleaseMutex, and CloseHandle to provide the functionality required by the qmutex interface.
 */

#include "qmutex.h"
#include <Windows.h>

enum QMutexReturnCode mutex_create(const struct MutexAttributes* mutex_request, struct MutexInfo* mutex_info) {
    enum QMutexReturnCode return_code = RETURN_CODE_MUTEX_NULL_POINTER;

    if (NULL == mutex_request || NULL == mutex_info) {
        /* Return code already set to NULL_POINTER */
    } else {
        mutex_info->p_mutex_handle = CreateMutex(NULL,                             /* Default security descriptor */
                                                 FALSE,                            /* Non signaled */
                                                 TEXT(mutex_request->mutex_name)); /* Object name */

        if (NULL == mutex_info->p_mutex_handle) {
            return_code = RETURN_CODE_MUTEX_CREATE_FAILED;
            /* LOG_MSG(QERROR, "Windows CreateEvent API error: %lu", GetLastError()); */
        } else if (GetLastError() == ERROR_ALREADY_EXISTS) {
            return_code = RETURN_CODE_MUTEX_CREATE_EXISTS;
            /* LOG_MSG(QERROR, "%s", "Windows CreateEvent API opened an already existing mutex"); */
        } else {
            return_code = RETURN_CODE_MUTEX_CREATE_SUCCESS;
        }
    }

    return return_code;
}

enum QMutexReturnCode mutex_open(const struct MutexAttributes* mutex_request, struct MutexInfo* mutex_info) {
    enum QMutexReturnCode return_code = RETURN_CODE_MUTEX_NULL_POINTER;

    if (NULL == mutex_request || NULL == mutex_info) {
        /* Return code already set to NULL_POINTER */
    } else {
        mutex_info->p_mutex_handle = OpenMutex(MUTEX_ALL_ACCESS,                 /* Request full access */
                                               FALSE,                            /* Handle not inheritable */
                                               TEXT(mutex_request->mutex_name)); /* Object name */

        if (NULL == mutex_info->p_mutex_handle) {
            return_code = RETURN_CODE_MUTEX_OPEN_FAILED;
            /* LOG_MSG(QERROR, "Windows OpenEvent API error: %lu", GetLastError()); */
        } else {
            /* LOG_MSG(QINFO, "%s", "Windows OpenEvent API a opened mutex."); */
            return_code = RETURN_CODE_MUTEX_OPEN_SUCCESS;
        }
    }

    return return_code;
}

enum QMutexReturnCode mutex_lock(const struct MutexInfo* mutex_info) {
    enum QMutexReturnCode return_code = RETURN_CODE_MUTEX_NULL_POINTER;
    DWORD return_signal_code          = WAIT_OBJECT_0;

    if (NULL == mutex_info) {
        /* Return code already set to NULL_POINTER */
    } else if (NULL == mutex_info->p_mutex_handle) {
        return_code = RETURN_CODE_MUTEX_INVALID_HANDLE;
    } else {
        return_signal_code = WaitForSingleObject(mutex_info->p_mutex_handle, INFINITE);
        if (WAIT_FAILED == return_signal_code) {
            /* LOG_MSG(QERROR, "Windows WaitForSingleObject API WAIT_FAILED error: %lu", GetLastError()); */
            return_code = RETURN_CODE_MUTEX_LOCK_FAILED;
        } else if (WAIT_OBJECT_0 != return_signal_code) {
            /* LOG_MSG(QERROR, "Windows WaitForSingleObject API error: %lu", GetLastError()); */
            return_code = RETURN_CODE_MUTEX_LOCK_SUCCESS;
        } else {
            return_code = RETURN_CODE_MUTEX_LOCK_SUCCESS;
        }
    }

    return return_code;
}

enum QMutexReturnCode mutex_unlock(const struct MutexInfo* mutex_info) {
    enum QMutexReturnCode return_code = RETURN_CODE_MUTEX_NULL_POINTER;
    BOOL isSuccess                    = FALSE;

    if (NULL == mutex_info) {
        /* Return code already set to NULL_POINTER */
    } else if (NULL == mutex_info->p_mutex_handle) {
        return_code = RETURN_CODE_MUTEX_INVALID_HANDLE;
    } else {
        isSuccess = ReleaseMutex(mutex_info->p_mutex_handle);
        if (FALSE == isSuccess) {
            /* LOG_MSG(QERROR, "Windows SetEvent API error: %lu", GetLastError()); */
            return_code = RETURN_CODE_MUTEX_UNLOCK_FAILED;
        } else {
            return_code = RETURN_CODE_MUTEX_UNLOCK_SUCCESS;
        }
    }

    return return_code;
}

enum QMutexReturnCode mutex_destroy(struct MutexInfo* mutex_info) {
    enum QMutexReturnCode return_code = RETURN_CODE_MUTEX_NULL_POINTER;
    BOOL isSuccess                    = FALSE;

    if (NULL == mutex_info) {
        /* Return code already set to NULL_POINTER */
    } else if (NULL == mutex_info->p_mutex_handle) {
        return_code = RETURN_CODE_MUTEX_INVALID_HANDLE;
    } else {
        isSuccess = CloseHandle(mutex_info->p_mutex_handle);
        if (FALSE == isSuccess) {
            /* LOG_MSG(QERROR, "Windows CloseHandle API error: %lu", GetLastError()); */
            return_code = RETURN_CODE_MUTEX_DESTROY_FAILED;
        } else {
            return_code = RETURN_CODE_MUTEX_DESTROY_SUCCESS;
        }
    }

    return return_code;
}
