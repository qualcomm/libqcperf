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
 * @brief Linux implementation of mutex management functions
 * @author Skand Gupta (skangupt@qti.qualcomm.com)
 *
 * This file implements the mutex management functions defined in qmutex.h
 * for Linux platforms. It provides a cross-platform abstraction layer over
 * the POSIX threading APIs, including mutex creation, locking, unlocking,
 * and destruction operations.
 *
 * The implementation uses POSIX thread functions like pthread_mutex_init,
 * pthread_mutex_lock, pthread_mutex_unlock, and pthread_mutex_destroy to
 * provide the functionality required by the qmutex interface.
 */

#include "qmutex.h"
#include <pthread.h>

enum QMutexReturnCode mutex_create(const struct MutexAttributes* mutex_request, struct MutexInfo* mutex_info) {
    enum QMutexReturnCode return_code = RETURN_CODE_MUTEX_CREATE_FAILED;
    if (mutex_request == NULL || mutex_info == NULL) {
        return_code = RETURN_CODE_MUTEX_NULL_POINTER;
    } else {
        pthread_mutex_t* lock = (pthread_mutex_t*)calloc(1, sizeof(pthread_mutex_t));
        if (lock == NULL) {
            return_code = RETURN_CODE_MUTEX_NULL_POINTER;
        } else {
            mutex_info->p_mutex_handle = (void*)lock;
            uint16_t return_init       = (uint16_t)pthread_mutex_init(mutex_info->p_mutex_handle, NULL);
            if (return_init == 0) {
                return_code = RETURN_CODE_MUTEX_CREATE_SUCCESS;
            }
        }
    }
    return return_code;
}

enum QMutexReturnCode mutex_open(const struct MutexAttributes* mutex_request, struct MutexInfo* mutex_info) {
    enum QMutexReturnCode return_code = RETURN_CODE_MUTEX_NULL_POINTER;
    if (mutex_request == NULL || mutex_info == NULL) {
        // NOT IMPLEMENTED
    } else {
    }
    return return_code;
}

enum QMutexReturnCode mutex_lock(const struct MutexInfo* mutex_info) {
    enum QMutexReturnCode return_code = RETURN_CODE_MUTEX_LOCK_FAILED;
    if (mutex_info == NULL || mutex_info->p_mutex_handle == NULL) {
        return_code = RETURN_CODE_MUTEX_NULL_POINTER;
    } else {
        pthread_mutex_t* lock = (pthread_mutex_t*)mutex_info->p_mutex_handle;
        uint32_t return_lock  = (uint32_t)pthread_mutex_lock(lock);
        if (return_lock == 0) {
            return_code = RETURN_CODE_MUTEX_LOCK_SUCCESS;
        } else {
            return_code = RETURN_CODE_MUTEX_LOCK_FAILED;
        }
    }
    return return_code;
}

enum QMutexReturnCode mutex_unlock(const struct MutexInfo* mutex_info) {
    enum QMutexReturnCode return_code = RETURN_CODE_MUTEX_UNLOCK_FAILED;
    if (mutex_info == NULL || mutex_info->p_mutex_handle == NULL) {
        return_code = RETURN_CODE_MUTEX_NULL_POINTER;
    } else {
        pthread_mutex_t* lock = (pthread_mutex_t*)mutex_info->p_mutex_handle;
        uint32_t return_lock  = (uint32_t)pthread_mutex_unlock(lock);
        if (return_lock == 0) {
            return_code = RETURN_CODE_MUTEX_UNLOCK_SUCCESS;
        } else {
            return_code = RETURN_CODE_MUTEX_UNLOCK_FAILED;
        }
    }
    return return_code;
}

enum QMutexReturnCode mutex_destroy(struct MutexInfo* mutex_info) {
    enum QMutexReturnCode return_code = RETURN_CODE_MUTEX_DESTROY_FAILED;
    if (mutex_info == NULL || mutex_info->p_mutex_handle == NULL) {
        return_code = RETURN_CODE_MUTEX_NULL_POINTER;
    } else {
        pthread_mutex_t* lock = (pthread_mutex_t*)mutex_info->p_mutex_handle;
        uint32_t return_lock  = (uint32_t)pthread_mutex_destroy(lock);
        if (return_lock == 0) {
            return_code = RETURN_CODE_MUTEX_DESTROY_SUCCESS;
        } else {
            return_code = RETURN_CODE_MUTEX_DESTROY_FAILED;
        }
        free(lock);
    }
    return return_code;
}
