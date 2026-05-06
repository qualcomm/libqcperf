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
 * @file qmutex.h
 * @brief Mutex utility library for cross-platform mutex management
 * @author Skand Gupta (skangupt@qti.qualcomm.com)
 */

#ifndef QMUTEX_H
#define QMUTEX_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * @brief Maximum size for mutex name
 */
#define MUTEX_NAME_SIZE 128

/**
 * @brief Return codes for mutex operations
 */
enum QMutexReturnCode {
    RETURN_CODE_MUTEX_CREATE_SUCCESS = 0, /**< Mutex creation successful */
    RETURN_CODE_MUTEX_CREATE_FAILED,      /**< Mutex creation failed */
    RETURN_CODE_MUTEX_CREATE_EXISTS,      /**< Mutex already exists */
    RETURN_CODE_MUTEX_OPEN_SUCCESS,       /**< Mutex open successful */
    RETURN_CODE_MUTEX_OPEN_FAILED,        /**< Mutex open failed */
    RETURN_CODE_MUTEX_NULL_POINTER,       /**< Null pointer provided */
    RETURN_CODE_MUTEX_LOCK_SUCCESS,       /**< Mutex lock successful */
    RETURN_CODE_MUTEX_LOCK_FAILED,        /**< Mutex lock failed */
    RETURN_CODE_MUTEX_UNLOCK_SUCCESS,     /**< Mutex unlock successful */
    RETURN_CODE_MUTEX_UNLOCK_FAILED,      /**< Mutex unlock failed */
    RETURN_CODE_MUTEX_DESTROY_FAILED,     /**< Mutex destruction failed */
    RETURN_CODE_MUTEX_DESTROY_SUCCESS,    /**< Mutex destruction successful */
    RETURN_CODE_MUTEX_INVALID_HANDLE      /**< Invalid mutex handle */
};

/**
 * @brief Mutex attributes structure
 */
struct MutexAttributes {
    uint8_t mutex_name[MUTEX_NAME_SIZE]; /**< Mutex name */
    uint8_t mutex_name_len;              /**< Length of mutex name */
};

/**
 * @brief Mutex information structure
 */
struct MutexInfo {
    void* p_mutex_handle; /**< Mutex handle */
    uint32_t shm_fd;      /**< Shared memory file descriptor */
};

/**
 * @brief Create a new mutex with the specified attributes
 *
 * This function creates a new mutex with the attributes specified in mutex_request.
 *
 * @param[in] mutex_request Mutex attributes
 * @param[out] mutex_info Mutex handle. Same structure needs to be passed to the destroy call
 * @return enum QMutexReturnCode RETURN_CODE_MUTEX_CREATE_SUCCESS if mutex creation successful
 * @return enum QMutexReturnCode RETURN_CODE_MUTEX_CREATE_FAILED if mutex creation failed
 * @return enum QMutexReturnCode RETURN_CODE_MUTEX_NULL_POINTER if mutex_request or mutex_info is NULL
 */
enum QMutexReturnCode mutex_create(const struct MutexAttributes* mutex_request, struct MutexInfo* mutex_info);

/**
 * @brief Open a created mutex with the specified attributes
 *
 * This function opens an existing mutex with the attributes specified in mutex_request.
 *
 * @param[in] mutex_request Mutex attributes
 * @param[out] mutex_info Mutex handle. Same structure needs to be passed to the destroy call
 * @return enum QMutexReturnCode RETURN_CODE_MUTEX_OPEN_SUCCESS if mutex open successful
 * @return enum QMutexReturnCode RETURN_CODE_MUTEX_OPEN_FAILED if mutex open failed
 * @return enum QMutexReturnCode RETURN_CODE_MUTEX_NULL_POINTER if mutex_request or mutex_info is NULL
 */
enum QMutexReturnCode mutex_open(const struct MutexAttributes* mutex_request, struct MutexInfo* mutex_info);

/**
 * @brief Lock a created or open mutex
 *
 * This function locks the mutex specified by mutex_info.
 *
 * @param[in] mutex_info Mutex handle
 * @return enum QMutexReturnCode RETURN_CODE_MUTEX_LOCK_SUCCESS if mutex lock successful
 * @return enum QMutexReturnCode RETURN_CODE_MUTEX_LOCK_FAILED if mutex lock failed
 * @return enum QMutexReturnCode RETURN_CODE_MUTEX_NULL_POINTER if mutex_info is NULL
 * @return enum QMutexReturnCode RETURN_CODE_MUTEX_INVALID_HANDLE if mutex handle is invalid
 */
enum QMutexReturnCode mutex_lock(const struct MutexInfo* mutex_info);

/**
 * @brief Unlock a created or open mutex
 *
 * This function unlocks the mutex specified by mutex_info.
 *
 * @param[in] mutex_info Mutex handle
 * @return enum QMutexReturnCode RETURN_CODE_MUTEX_UNLOCK_SUCCESS if mutex unlock successful
 * @return enum QMutexReturnCode RETURN_CODE_MUTEX_UNLOCK_FAILED if mutex unlock failed
 * @return enum QMutexReturnCode RETURN_CODE_MUTEX_NULL_POINTER if mutex_info is NULL
 * @return enum QMutexReturnCode RETURN_CODE_MUTEX_INVALID_HANDLE if mutex handle is invalid
 */
enum QMutexReturnCode mutex_unlock(const struct MutexInfo* mutex_info);

/**
 * @brief Destroy a created or open mutex
 *
 * This function destroys the mutex specified by mutex_info.
 *
 * @param[in] mutex_info Mutex handle
 * @return enum QMutexReturnCode RETURN_CODE_MUTEX_DESTROY_SUCCESS if mutex destroy successful
 * @return enum QMutexReturnCode RETURN_CODE_MUTEX_DESTROY_FAILED if mutex destroy failed
 * @return enum QMutexReturnCode RETURN_CODE_MUTEX_NULL_POINTER if mutex_info is NULL
 * @return enum QMutexReturnCode RETURN_CODE_MUTEX_INVALID_HANDLE if mutex handle is invalid
 */
enum QMutexReturnCode mutex_destroy(struct MutexInfo* mutex_info);

#endif /* QMUTEX_H */
