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
 * @file qthread.c
 * @brief Windows implementation of thread management functions
 * @author Skand Gupta (skangupt@qti.qualcomm.com)
 *
 * This file implements the thread management functions defined in qthread.h
 * for Windows platforms. It provides a cross-platform abstraction layer over
 * the Windows threading APIs, including thread creation, joining, termination,
 * and other thread management operations.
 *
 * The implementation uses Windows API functions like CreateThread, WaitForSingleObject,
 * and TerminateThread to provide the functionality required by the qthread interface.
 */

#include "qthread.h"
#include <windows.h>

enum QThreadReturnCode thread_create(struct QThreadAttributes* thread_request, struct QThreadInfo* thread_info) {
    enum QThreadReturnCode result = RET_QTHREAD_CREATE_FAILED;
    DWORD thread_id               = 0;

    if (NULL == thread_request || NULL == thread_info) {
        result = RET_QTHREAD_NULL_POINTER;
    } else {
        thread_info->thread_handle = CreateThread(NULL,                                              /* Default security attributes */
                                                  thread_request->stack_size,                        /* Stack size */
                                                  (LPTHREAD_START_ROUTINE)thread_request->thread_fn, /* Thread function */
                                                  thread_request->thread_params,                     /* Thread function parameters */
                                                  0,                                                 /* Creation flags */
                                                  &thread_id);                                       /* Thread ID */

        if (NULL == thread_info->thread_handle) {
            result = RET_QTHREAD_CREATE_FAILED;
        } else {
            thread_info->thread_id = (uint64_t)thread_id;
            result                 = RET_QTHREAD_CREATE_SUCCESS;
        }
    }

    return result;
}

enum QThreadReturnCode thread_destroy(struct QThreadInfo* thread_info) {
    enum QThreadReturnCode result = RET_QTHREAD_DESTROY_FAILED;
    BOOL close_result             = FALSE;

    if (NULL == thread_info) {
        result = RET_QTHREAD_NULL_POINTER;
    } else if (NULL == thread_info->thread_handle) {
        result = RET_QTHREAD_INVALID_HANDLE;
    } else {
        close_result               = CloseHandle(thread_info->thread_handle);
        thread_info->thread_handle = NULL;

        if (TRUE == close_result) {
            result = RET_QTHREAD_DESTROY_SUCCESS;
        } else {
            result = RET_QTHREAD_DESTROY_FAILED;
        }
    }

    return result;
}

enum QThreadReturnCode thread_join(struct QThreadInfo* thread_info) {
    enum QThreadReturnCode result = RET_QTHREAD_JOIN_FAILED;
    DWORD wait_result             = 0;

    if (NULL == thread_info) {
        result = RET_QTHREAD_NULL_POINTER;
    } else if (NULL == thread_info->thread_handle) {
        result = RET_QTHREAD_INVALID_HANDLE;
    } else {
        wait_result = WaitForSingleObject(thread_info->thread_handle, INFINITE);

        if (WAIT_OBJECT_0 == wait_result) {
            result = RET_QTHREAD_JOIN_SUCCESS;
        } else {
            result = RET_QTHREAD_JOIN_FAILED;
        }
    }

    return result;
}

enum QThreadReturnCode thread_current_thread_id(struct QThreadInfo* thread_info) {
    enum QThreadReturnCode result = RET_QTHREAD_NULL_POINTER;
    HANDLE thread_self            = NULL;

    if (NULL == thread_info) {
        result = RET_QTHREAD_NULL_POINTER;
    } else {
        thread_self = GetCurrentThread();

        if (NULL == thread_self) {
            result = RET_QTHREAD_CURRENT_THREAD_FAILED;
        } else {
            thread_info->thread_handle = (void*)thread_self;
            thread_info->thread_id     = (uint64_t)GetThreadId(thread_self);
            result                     = RET_QTHREAD_CURRENT_THREAD_SUCCESS;
        }
    }

    return result;
}

enum QThreadReturnCode thread_terminate(struct QThreadInfo* thread_info) {
    enum QThreadReturnCode result = RET_QTHREAD_TERMINATE_FAILED;
    DWORD exit_code               = 0;
    DWORD error_code              = 0;
    BOOL terminate_result         = FALSE;
    BOOL get_exit_code_result     = FALSE;

    if (NULL == thread_info) {
        result = RET_QTHREAD_NULL_POINTER;
    } else if (NULL == thread_info->thread_handle) {
        result = RET_QTHREAD_INVALID_HANDLE;
    } else {
        terminate_result = TerminateThread(thread_info->thread_handle, 0);

        if (FALSE == terminate_result) {
            error_code = GetLastError();
            result     = RET_QTHREAD_TERMINATE_FAILED;
        } else {
            get_exit_code_result = GetExitCodeThread(thread_info->thread_handle, &exit_code);

            if (FALSE == get_exit_code_result) {
                error_code = GetLastError();
                result     = RET_QTHREAD_TERMINATE_FAILED;
            } else {
                result = RET_QTHREAD_TERMINATE_SUCCESS;
            }
        }
    }

    return result;
}
