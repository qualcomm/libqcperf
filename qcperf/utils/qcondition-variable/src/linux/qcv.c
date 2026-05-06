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
 * @brief Linux implementation of condition variable management functions
 * @author Skand Gupta (skangupt@qti.qualcomm.com)
 *
 * This file implements the condition variable management functions defined in qcv.h
 * for Linux platforms. It provides a cross-platform abstraction layer over
 * the POSIX threading APIs, including condition variable creation, waiting,
 * signaling, and destruction operations.
 *
 * The implementation uses POSIX thread functions like pthread_cond_init,
 * pthread_cond_wait, pthread_cond_timedwait, pthread_cond_signal,
 * pthread_cond_broadcast, and pthread_cond_destroy to provide the
 * functionality required by the qcv interface.
 */

#include "qcv.h"
#include "qtime.h"
#include <pthread.h>

enum QCvReturnCode cv_create(const struct CvAttributes* cv_request, struct CvInfo* cv_info) {
    enum QCvReturnCode return_code = RETURN_CODE_CV_NULL_POINTER;
    if (cv_request == NULL || cv_info == NULL) {
    } else {
        pthread_cond_t* p_thread_cond   = NULL;  // PTHREAD_COND_INITIALIZER;  // Declaration of thread condition variable
        pthread_mutex_t* p_thread_mutex = NULL;  // PTHREAD_MUTEX_INITIALIZER; // declaring mutex
        int return_signal_code          = 0;
        p_thread_cond                   = calloc(1, sizeof(pthread_cond_t));
        p_thread_mutex                  = calloc(1, sizeof(pthread_mutex_t));
        if (p_thread_cond == NULL) {
            return_code = RETURN_CODE_CV_MUTEX_MEMORY_CALLOC_FAILED;
        } else if (p_thread_mutex == NULL) {
            return_code = RETURN_CODE_CV_MUTEX_MEMORY_CALLOC_FAILED;
        } else {
            return_signal_code = pthread_cond_init(p_thread_cond, NULL);
            if (return_signal_code != 0) {
                return_code = RETURN_CODE_CV_CREATE_FAILED;
            } else {
                return_signal_code = pthread_mutex_init(p_thread_mutex, NULL);
                if (return_signal_code != 0) {
                    return_code = RETURN_CODE_CV_CREATE_FAILED;
                } else {
                    cv_info->p_cv_handle    = (void*)p_thread_cond;
                    cv_info->p_mutex_handle = (void*)p_thread_mutex;
                    return_code             = RETURN_CODE_CV_CREATE_SUCCESS;
                }
            }
        }

        if (return_code != RETURN_CODE_CV_CREATE_SUCCESS) {
            if (p_thread_cond == NULL) {
                free(p_thread_cond);
                p_thread_cond = NULL;
            }
            if (p_thread_mutex == NULL) {
                free(p_thread_mutex);
                p_thread_mutex = NULL;
            }
        }
    }
    return return_code;
}

enum QCvReturnCode cv_wait(const struct CvInfo* cv_info) {
    enum QCvReturnCode return_code = RETURN_CODE_CV_NULL_POINTER;
    if (cv_info == NULL) {
    } else if (cv_info->p_cv_handle == NULL) {
        return_code = RETURN_CODE_CV_INVALID_HANDLE;
    } else if (cv_info->p_mutex_handle == NULL) {
        return_code = RETURN_CODE_CV_MUTEX_INVALID_HANDLE;
    } else {
        pthread_cond_t* p_thread_cond   = NULL;
        pthread_mutex_t* p_thread_mutex = NULL;
        int return_signal_code          = 0;
        p_thread_cond                   = (pthread_cond_t*)cv_info->p_cv_handle;
        p_thread_mutex                  = (pthread_mutex_t*)cv_info->p_mutex_handle;
        pthread_mutex_lock(p_thread_mutex);
        return_signal_code = pthread_cond_wait(p_thread_cond, p_thread_mutex);
        pthread_mutex_unlock(p_thread_mutex);
        if (return_signal_code != 0) {
            return_code = RETURN_CODE_CV_WAIT_FAILED;
        } else {
            return_code = RETURN_CODE_CV_WAIT_COMPLETED;
        }
    }
    return return_code;
}

enum QCvReturnCode cv_wait_for(const struct CvInfo* cv_info, uint32_t timeout) {
    enum QCvReturnCode return_code = RETURN_CODE_CV_NULL_POINTER;
    struct timespec tm_spec        = {0};

    if (cv_info == NULL) {
    } else if (cv_info->p_cv_handle == NULL) {
        return_code = RETURN_CODE_CV_INVALID_HANDLE;
    } else if (cv_info->p_mutex_handle == NULL) {
        return_code = RETURN_CODE_CV_MUTEX_INVALID_HANDLE;
    } else {
        pthread_cond_t* p_thread_cond   = NULL;
        pthread_mutex_t* p_thread_mutex = NULL;
        int return_signal_code          = 0;
        clock_gettime(CLOCK_REALTIME, &tm_spec);
        tm_spec.tv_sec += (timeout / MS_SECOND_MULTIPLIER);  // converting to miliseconds to seconds
        p_thread_cond  = (pthread_cond_t*)cv_info->p_cv_handle;
        p_thread_mutex = (pthread_mutex_t*)cv_info->p_mutex_handle;
        pthread_mutex_lock(p_thread_mutex);
        return_signal_code = pthread_cond_timedwait(p_thread_cond, p_thread_mutex, &tm_spec);
        pthread_mutex_unlock(p_thread_mutex);
        if (return_signal_code != 0) {
            return_code = RETURN_CODE_CV_WAIT_FAILED;
        } else {
            return_code = RETURN_CODE_CV_WAIT_COMPLETED;
        }
    }
    return return_code;
}

enum QCvReturnCode cv_notify(const struct CvInfo* cv_info) {
    enum QCvReturnCode return_code = RETURN_CODE_CV_NULL_POINTER;
    if (cv_info == NULL) {
    } else if (cv_info->p_cv_handle == NULL) {
        return_code = RETURN_CODE_CV_INVALID_HANDLE;
    } else if (cv_info->p_mutex_handle == NULL) {
        return_code = RETURN_CODE_CV_MUTEX_INVALID_HANDLE;
    } else {
        pthread_cond_t* p_thread_cond   = NULL;
        pthread_mutex_t* p_thread_mutex = NULL;
        int return_signal_code          = 0;
        p_thread_cond                   = (pthread_cond_t*)cv_info->p_cv_handle;
        p_thread_mutex                  = (pthread_mutex_t*)cv_info->p_mutex_handle;
        pthread_mutex_lock(p_thread_mutex);
        return_signal_code = pthread_cond_signal(p_thread_cond);
        pthread_mutex_unlock(p_thread_mutex);
        if (return_signal_code != 0) {
            return_code = RETURN_CODE_CV_NOTIFY_FAILED;
        } else {
            return_code = RETURN_CODE_CV_NOTIFY_SUCCESS;
        }
    }
    return return_code;
}

enum QCvReturnCode cv_notifyAll(const struct CvInfo* cv_info) {
    enum QCvReturnCode return_code = RETURN_CODE_CV_NULL_POINTER;
    if (cv_info == NULL) {
    } else if (cv_info->p_cv_handle == NULL) {
        return_code = RETURN_CODE_CV_INVALID_HANDLE;
    } else if (cv_info->p_mutex_handle == NULL) {
        return_code = RETURN_CODE_CV_MUTEX_INVALID_HANDLE;
    } else {
        pthread_cond_t* p_thread_cond   = NULL;
        pthread_mutex_t* p_thread_mutex = NULL;
        int return_signal_code          = 0;
        p_thread_cond                   = (pthread_cond_t*)cv_info->p_cv_handle;
        p_thread_mutex                  = (pthread_mutex_t*)cv_info->p_mutex_handle;
        pthread_mutex_lock(p_thread_mutex);
        return_signal_code = pthread_cond_broadcast(p_thread_cond);
        pthread_mutex_unlock(p_thread_mutex);
        if (return_signal_code != 0) {
            return_code = RETURN_CODE_CV_NOTIFY_ALL_FAILED;
        } else {
            return_code = RETURN_CODE_CV_NOTIFY_ALL_SUCCESS;
        }
    }
    return return_code;
}

enum QCvReturnCode cv_destroy(struct CvInfo* cv_info) {
    enum QCvReturnCode return_code = RETURN_CODE_CV_NULL_POINTER;
    if (cv_info == NULL) {
    } else if (cv_info->p_cv_handle == NULL) {
        return_code = RETURN_CODE_CV_INVALID_HANDLE;
    } else if (cv_info->p_mutex_handle == NULL) {
        return_code = RETURN_CODE_CV_INVALID_HANDLE;
    } else {
        pthread_cond_t* p_thread_cond   = NULL;
        pthread_mutex_t* p_thread_mutex = NULL;
        int return_signal_code          = 0;
        p_thread_cond                   = (pthread_cond_t*)cv_info->p_cv_handle;
        p_thread_mutex                  = (pthread_mutex_t*)cv_info->p_mutex_handle;
        return_signal_code              = pthread_cond_destroy(p_thread_cond);
        if (return_signal_code != 0) {
            return_code          = RETURN_CODE_CV_COND_DESTROY_FAILED;
        }
        return_signal_code = pthread_mutex_destroy(p_thread_mutex);
        if (return_signal_code != 0) {
            return_code          = RETURN_CODE_CV_MUTEX_DESTROY_FAILED;
        }
        free(cv_info->p_mutex_handle);
        cv_info->p_mutex_handle = NULL;
        free(cv_info->p_cv_handle);
        cv_info->p_cv_handle = NULL;
        return_code          = RETURN_CODE_CV_DESTROY_SUCCESS;
    }
    return return_code;
}
