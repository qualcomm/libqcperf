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
 * @file qcv.c
 * @brief Windows implementation of condition variable management functions
 * @author Skand Gupta (skangupt@qti.qualcomm.com)
 *
 * This file implements the condition variable management functions defined in qcv.h
 * for Windows platforms. It provides a cross-platform abstraction layer over
 * the Windows synchronization APIs, including condition variable creation, waiting,
 * signaling, and destruction operations.
 *
 * The implementation uses Windows API functions like InitializeConditionVariable,
 * SleepConditionVariableCS, WakeConditionVariable, and WakeAllConditionVariable
 * to provide the functionality required by the qcv interface.
 */

#include "qcv.h"
#include <Windows.h>

enum QCvReturnCode cv_create(const struct CvAttributes* cv_request, struct CvInfo* cv_info) {
    enum QCvReturnCode return_code = RETURN_CODE_CV_NULL_POINTER;

    if (NULL == cv_request || NULL == cv_info) {
        /* Return code already set to NULL_POINTER */
    } else {
        PCONDITION_VARIABLE p_cv_handle  = NULL;
        PCRITICAL_SECTION p_mutex_handle = NULL;

        p_cv_handle    = (PCONDITION_VARIABLE)calloc(1, sizeof(CONDITION_VARIABLE));
        p_mutex_handle = (PCRITICAL_SECTION)calloc(1, sizeof(CRITICAL_SECTION));

        if (NULL == p_cv_handle || NULL == p_mutex_handle) {
            return_code = RETURN_CODE_CV_CREATE_FAILED;
            /* LOG_MSG(QERROR, "Windows InitializeConditionVariable API error: %d", GetLastError()); */

            if (NULL != p_mutex_handle) {
                free(p_mutex_handle);
                p_mutex_handle = NULL;
            }

            if (NULL != p_cv_handle) {
                free(p_cv_handle);
                p_cv_handle = NULL;
            }
        } else {
            InitializeConditionVariable(p_cv_handle);
            InitializeCriticalSection(p_mutex_handle);
            cv_info->p_cv_handle    = (void*)p_cv_handle;
            cv_info->p_mutex_handle = (void*)p_mutex_handle;
            return_code              = RETURN_CODE_CV_CREATE_SUCCESS;
            /* LOG_MSG(QINFO, "%s", "Windows InitializeConditionVariable API a new condition variable"); */
        }
    }

    return return_code;
}

enum QCvReturnCode cv_wait(const struct CvInfo* cv_info) {
    enum QCvReturnCode return_code = RETURN_CODE_CV_NULL_POINTER;
    BOOL return_signal_code       = 0;

    if (NULL == cv_info) {
        /* Return code already set to NULL_POINTER */
    } else if (NULL == cv_info->p_cv_handle) {
        return_code = RETURN_CODE_CV_INVALID_HANDLE;
    } else if (NULL == cv_info->p_mutex_handle) {
        return_code = RETURN_CODE_CV_MUTEX_INVALID_HANDLE;
    } else {
        PCONDITION_VARIABLE p_cv_handle  = NULL;
        PCRITICAL_SECTION p_mutex_handle = NULL;

        p_cv_handle    = (PCONDITION_VARIABLE)cv_info->p_cv_handle;
        p_mutex_handle = (PCRITICAL_SECTION)cv_info->p_mutex_handle;

        EnterCriticalSection(p_mutex_handle);
        return_signal_code = SleepConditionVariableCS(p_cv_handle, p_mutex_handle, INFINITE);
        LeaveCriticalSection(p_mutex_handle);

        if (0 == return_signal_code) {
            return_code = RETURN_CODE_CV_WAIT_FAILED;
        } else {
            return_code = RETURN_CODE_CV_WAIT_COMPLETED;
        }
    }

    return return_code;
}

enum QCvReturnCode cv_wait_for(const struct CvInfo* cv_info, uint32_t timeout_ms) {
    enum QCvReturnCode return_code = RETURN_CODE_CV_NULL_POINTER;
    BOOL return_signal_code       = 0;
    uint32_t lastError            = 0;

    if (NULL == cv_info) {
        /* Return code already set to NULL_POINTER */
    } else if (NULL == cv_info->p_cv_handle) {
        return_code = RETURN_CODE_CV_INVALID_HANDLE;
    } else if (NULL == cv_info->p_mutex_handle) {
        return_code = RETURN_CODE_CV_MUTEX_INVALID_HANDLE;
    } else {
        PCONDITION_VARIABLE p_cv_handle  = NULL;
        PCRITICAL_SECTION p_mutex_handle = NULL;

        p_cv_handle    = (PCONDITION_VARIABLE)cv_info->p_cv_handle;
        p_mutex_handle = (PCRITICAL_SECTION)cv_info->p_mutex_handle;

        EnterCriticalSection(p_mutex_handle);
        return_signal_code = SleepConditionVariableCS(p_cv_handle, p_mutex_handle, (DWORD)timeout_ms);
        LeaveCriticalSection(p_mutex_handle);

        if (0 == return_signal_code) {
            lastError = GetLastError();
            if (ERROR_TIMEOUT != lastError) {
                /* LOG_MSG(QERROR, "%s%u", "SleepConditionVariableCS failed, error=", lastError); */
                return_code = RETURN_CODE_CV_WAIT_FAILED;
            } else {
                /* LOG_MSG(QDEBUG, "%s%u%s", "Time-out interval elapsed, timeout =", timeout_ms, " ms"); */
                return_code = RETURN_CODE_CV_WAIT_TIMEOUT;
            }
        } else {
            /* LOG_MSG(QDEBUG, "%s", "Wakeup occurred"); */
            return_code = RETURN_CODE_CV_WAIT_COMPLETED;
        }
    }

    return return_code;
}

enum QCvReturnCode cv_notify(const struct CvInfo* cv_info) {
    enum QCvReturnCode return_code = RETURN_CODE_CV_NULL_POINTER;

    if (NULL == cv_info) {
        /* Return code already set to NULL_POINTER */
    } else if (NULL == cv_info->p_cv_handle) {
        return_code = RETURN_CODE_CV_INVALID_HANDLE;
    } else if (NULL == cv_info->p_mutex_handle) {
        return_code = RETURN_CODE_CV_MUTEX_INVALID_HANDLE;
    } else {
        PCONDITION_VARIABLE p_cv_handle  = NULL;
        PCRITICAL_SECTION p_mutex_handle = NULL;

        p_cv_handle    = (PCONDITION_VARIABLE)cv_info->p_cv_handle;
        p_mutex_handle = (PCRITICAL_SECTION)cv_info->p_mutex_handle;

        WakeConditionVariable(p_cv_handle);
        return_code = RETURN_CODE_CV_NOTIFY_SUCCESS;
    }

    return return_code;
}

enum QCvReturnCode cv_notifyAll(const struct CvInfo* cv_info) {
    enum QCvReturnCode return_code = RETURN_CODE_CV_NULL_POINTER;

    if (NULL == cv_info) {
        /* Return code already set to NULL_POINTER */
    } else if (NULL == cv_info->p_cv_handle) {
        return_code = RETURN_CODE_CV_INVALID_HANDLE;
    } else if (NULL == cv_info->p_mutex_handle) {
        return_code = RETURN_CODE_CV_MUTEX_INVALID_HANDLE;
    } else {
        PCONDITION_VARIABLE p_cv_handle  = NULL;
        PCRITICAL_SECTION p_mutex_handle = NULL;

        p_cv_handle    = (PCONDITION_VARIABLE)cv_info->p_cv_handle;
        p_mutex_handle = (PCRITICAL_SECTION)cv_info->p_mutex_handle;

        WakeAllConditionVariable(p_cv_handle);
        return_code = RETURN_CODE_CV_NOTIFY_ALL_SUCCESS;
    }

    return return_code;
}

enum QCvReturnCode cv_destroy(struct CvInfo* cv_info) {
    enum QCvReturnCode return_code = RETURN_CODE_CV_NULL_POINTER;

    if (NULL == cv_info) {
        /* Return code already set to NULL_POINTER */
    } else if (NULL == cv_info->p_cv_handle) {
        return_code = RETURN_CODE_CV_INVALID_HANDLE;
    } else if (NULL == cv_info->p_mutex_handle) {
        return_code = RETURN_CODE_CV_INVALID_HANDLE;
    } else {
        PCONDITION_VARIABLE p_cv_handle  = NULL;
        PCRITICAL_SECTION p_mutex_handle = NULL;

        p_cv_handle    = (PCONDITION_VARIABLE)cv_info->p_cv_handle;
        p_mutex_handle = (PCRITICAL_SECTION)cv_info->p_mutex_handle;

        DeleteCriticalSection(p_mutex_handle);
        free(cv_info->p_mutex_handle);
        cv_info->p_mutex_handle = NULL;
        free(cv_info->p_cv_handle);
        cv_info->p_cv_handle = NULL;

        return_code = RETURN_CODE_CV_DESTROY_SUCCESS;
    }

    return return_code;
}
