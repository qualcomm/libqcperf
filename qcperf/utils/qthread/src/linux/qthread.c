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
 * @brief Linux implementation of thread management functions
 * @author Skand Gupta (skangupt@qti.qualcomm.com)
 *
 * This file implements the thread management functions defined in qthread.h
 * for Linux platforms. It provides a cross-platform abstraction layer over
 * the POSIX threading APIs, including thread creation, joining, termination,
 * and other thread management operations.
 *
 * The implementation uses POSIX thread functions like pthread_create, pthread_join,
 * and pthread_cancel to provide the functionality required by the qthread interface.
 */

#include "qthread.h"
#include <pthread.h>
#include <stdlib.h>

enum QThreadReturnCode thread_create(struct QThreadAttributes* thread_request, struct QThreadInfo* thread_info) {
    enum QThreadReturnCode result = RET_QTHREAD_CREATE_FAILED;
    int thread_status             = -1;
    pthread_t* thread_handle      = NULL;

    if (NULL == thread_request || NULL == thread_info) {
        result = RET_QTHREAD_NULL_POINTER;
    } else {
        thread_handle = (pthread_t*)calloc(1, sizeof(pthread_t));
        if (NULL == thread_handle) {
            result = RET_QTHREAD_CREATE_FAILED;
        } else {
            thread_status = pthread_create(thread_handle, NULL, (void* (*)(void*))thread_request->thread_fn, (void*)thread_request->thread_params);
            if (0 == thread_status) {
                thread_info->thread_handle = (void*)thread_handle;
                result                     = RET_QTHREAD_CREATE_SUCCESS;
            } else {
                free(thread_handle);
                thread_handle = NULL;
                result        = RET_QTHREAD_CREATE_FAILED;
            }
        }
    }

    return result;
}

enum QThreadReturnCode thread_destroy(struct QThreadInfo* thread_info) {
    enum QThreadReturnCode result = RET_QTHREAD_DESTROY_FAILED;

    if (NULL == thread_info) {
        result = RET_QTHREAD_NULL_POINTER;
    } else if (NULL == thread_info->thread_handle) {
        result = RET_QTHREAD_INVALID_HANDLE;
    } else {
        pthread_t* thread_handle = (pthread_t*)(thread_info->thread_handle);
        free(thread_handle);
        thread_info->thread_handle = NULL;
        result                     = RET_QTHREAD_DESTROY_SUCCESS;
    }

    return result;
}

enum QThreadReturnCode thread_join(struct QThreadInfo* thread_info) {
    enum QThreadReturnCode result = RET_QTHREAD_JOIN_FAILED;
    int thread_status             = -1;

    if (NULL == thread_info) {
        result = RET_QTHREAD_NULL_POINTER;
    } else if (NULL == thread_info->thread_handle) {
        result = RET_QTHREAD_INVALID_HANDLE;
    } else {
        pthread_t* thread_handle = (pthread_t*)(thread_info->thread_handle);
        thread_status            = pthread_join(*thread_handle, NULL);
        if (0 == thread_status) {
            result = RET_QTHREAD_JOIN_SUCCESS;
        } else {
            result = RET_QTHREAD_JOIN_FAILED;
        }
    }

    return result;
}

enum QThreadReturnCode thread_current_thread_id(struct QThreadInfo* thread_info) {
    enum QThreadReturnCode result = RET_QTHREAD_NULL_POINTER;
    pthread_t thread_self;

    if (NULL == thread_info) {
        result = RET_QTHREAD_NULL_POINTER;
    } else {
        thread_self                = pthread_self();
        thread_info->thread_handle = (void*)&thread_self;
        thread_info->thread_id     = (uint64_t)thread_self;
        result                     = RET_QTHREAD_CURRENT_THREAD_SUCCESS;
    }

    return result;
}

enum QThreadReturnCode thread_terminate(struct QThreadInfo* thread_info) {
    enum QThreadReturnCode result = RET_QTHREAD_TERMINATE_FAILED;
    int cancel_status             = -1;

    if (NULL == thread_info) {
        result = RET_QTHREAD_NULL_POINTER;
    } else if (NULL == thread_info->thread_handle) {
        result = RET_QTHREAD_INVALID_HANDLE;
    } else {
        pthread_t* thread_handle = (pthread_t*)(thread_info->thread_handle);
        cancel_status            = pthread_cancel(*thread_handle);
        if (0 == cancel_status) {
            result = RET_QTHREAD_TERMINATE_SUCCESS;
        } else {
            result = RET_QTHREAD_TERMINATE_FAILED;
        }
    }

    return result;
}
