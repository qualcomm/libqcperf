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
 * @file qcv.h
 * @brief Condition variable utility library for cross-platform condition variable management
 * @author Skand Gupta (skangupt@qti.qualcomm.com)
 */

#ifndef QCV_H
#define QCV_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * @brief Maximum size for condition variable name
 */
#define CV_NAME_SIZE 128

#define MS_SECOND_MULTIPLIER 1000U

/**
 * @brief Return codes for condition variable operations
 */
enum QCvReturnCode {
    RETURN_CODE_CV_CREATE_SUCCESS = 0,        /**< Condition variable creation successful */
    RETURN_CODE_CV_CREATE_FAILED,             /**< Condition variable creation failed */
    RETURN_CODE_CV_CREATE_EXISTS,             /**< Condition variable already exists */
    RETURN_CODE_CV_OPEN_SUCCESS,              /**< Condition variable open successful */
    RETURN_CODE_CV_OPEN_FAILED,               /**< Condition variable open failed */
    RETURN_CODE_CV_NULL_POINTER,              /**< Null pointer provided */
    RETURN_CODE_CV_WAIT_FAILED,               /**< Condition variable wait failed */
    RETURN_CODE_CV_WAIT_COMPLETED,            /**< Condition variable wait completed */
    RETURN_CODE_CV_WAIT_TIMEOUT,              /**< Condition variable wait timed out */
    RETURN_CODE_CV_NOTIFY_SUCCESS,            /**< Condition variable notify successful */
    RETURN_CODE_CV_NOTIFY_ALL_SUCCESS,        /**< Condition variable notify all successful */
    RETURN_CODE_CV_NOTIFY_FAILED,             /**< Condition variable notify failed */
    RETURN_CODE_CV_NOTIFY_ALL_FAILED,         /**< Condition variable notify all failed */
    RETURN_CODE_CV_DESTROY_FAILED,            /**< Condition variable destruction failed */
    RETURN_CODE_CV_DESTROY_SUCCESS,           /**< Condition variable destruction successful */
    RETURN_CODE_CV_INVALID_HANDLE,            /**< Invalid condition variable handle */
    RETURN_CODE_CV_MUTEX_INVALID_HANDLE,      /**< Invalid mutex handle */
    RETURN_CODE_CV_MUTEX_MEMORY_CALLOC_FAILED, /**< Mutex memory allocation failed */
    RETURN_CODE_CV_COND_DESTROY_FAILED,        /**< Thread conditional variable destroy failed */
    RETURN_CODE_CV_MUTEX_DESTROY_FAILED        /**< Mutex destroy failed */
};

/**
 * @brief Condition variable attributes structure
 */
struct CvAttributes {
    char cv_name[CV_NAME_SIZE]; /**< Condition variable name */
    size_t cv_name_len;         /**< Length of condition variable name */
};

/**
 * @brief Condition variable information structure
 */
struct CvInfo {
    void* p_cv_handle;    /**< Condition variable handle */
    void* p_mutex_handle; /**< Associated mutex handle */
};

/**
 * @brief Create a new condition variable with the specified attributes
 *
 * This function creates a new condition variable with the attributes specified in cv_request.
 *
 * @param[in] cv_request Condition variable attributes
 * @param[out] cv_info Condition variable handle. Same structure needs to be passed to the destroy call
 * @return enum QCvReturnCode RETURN_CODE_CV_CREATE_SUCCESS if condition variable creation successful
 * @return enum QCvReturnCode RETURN_CODE_CV_CREATE_FAILED if condition variable creation failed
 * @return enum QCvReturnCode RETURN_CODE_CV_NULL_POINTER if cv_request or cv_info is NULL
 */
enum QCvReturnCode cv_create(const struct CvAttributes* cv_request, struct CvInfo* cv_info);

/**
 * @brief Wait indefinitely on a condition variable
 *
 * This function waits indefinitely on the condition variable specified by cv_info.
 *
 * @param[in] cv_info Condition variable handle
 * @return enum QCvReturnCode RETURN_CODE_CV_WAIT_COMPLETED if condition variable wait completed
 * @return enum QCvReturnCode RETURN_CODE_CV_WAIT_FAILED if condition variable wait failed
 * @return enum QCvReturnCode RETURN_CODE_CV_NULL_POINTER if cv_info is NULL
 * @return enum QCvReturnCode RETURN_CODE_CV_INVALID_HANDLE if condition variable handle is invalid
 * @return enum QCvReturnCode RETURN_CODE_CV_MUTEX_INVALID_HANDLE if mutex handle is invalid
 */
enum QCvReturnCode cv_wait(const struct CvInfo* cv_info);

/**
 * @brief Wait with timeout on a condition variable
 *
 * This function waits on the condition variable specified by cv_info with a timeout.
 *
 * @param[in] cv_info Condition variable handle
 * @param[in] timeout_ms Timeout in milliseconds
 * @return enum QCvReturnCode RETURN_CODE_CV_WAIT_COMPLETED if condition variable wait completed
 * @return enum QCvReturnCode RETURN_CODE_CV_WAIT_TIMEOUT if condition variable wait timed out
 * @return enum QCvReturnCode RETURN_CODE_CV_WAIT_FAILED if condition variable wait failed
 * @return enum QCvReturnCode RETURN_CODE_CV_NULL_POINTER if cv_info is NULL
 * @return enum QCvReturnCode RETURN_CODE_CV_INVALID_HANDLE if condition variable handle is invalid
 * @return enum QCvReturnCode RETURN_CODE_CV_MUTEX_INVALID_HANDLE if mutex handle is invalid
 */
enum QCvReturnCode cv_wait_for(const struct CvInfo* cv_info, uint32_t timeout_ms);

/**
 * @brief Notify one thread waiting on a condition variable
 *
 * This function notifies one thread waiting on the condition variable specified by cv_info.
 *
 * @param[in] cv_info Condition variable handle
 * @return enum QCvReturnCode RETURN_CODE_CV_NOTIFY_SUCCESS if condition variable notify successful
 * @return enum QCvReturnCode RETURN_CODE_CV_NOTIFY_FAILED if condition variable notify failed
 * @return enum QCvReturnCode RETURN_CODE_CV_NULL_POINTER if cv_info is NULL
 * @return enum QCvReturnCode RETURN_CODE_CV_INVALID_HANDLE if condition variable handle is invalid
 * @return enum QCvReturnCode RETURN_CODE_CV_MUTEX_INVALID_HANDLE if mutex handle is invalid
 */
enum QCvReturnCode cv_notify(const struct CvInfo* cv_info);

/**
 * @brief Notify all threads waiting on a condition variable
 *
 * This function notifies all threads waiting on the condition variable specified by cv_info.
 *
 * @param[in] cv_info Condition variable handle
 * @return enum QCvReturnCode RETURN_CODE_CV_NOTIFY_ALL_SUCCESS if condition variable notify all successful
 * @return enum QCvReturnCode RETURN_CODE_CV_NOTIFY_ALL_FAILED if condition variable notify all failed
 * @return enum QCvReturnCode RETURN_CODE_CV_NULL_POINTER if cv_info is NULL
 * @return enum QCvReturnCode RETURN_CODE_CV_INVALID_HANDLE if condition variable handle is invalid
 * @return enum QCvReturnCode RETURN_CODE_CV_MUTEX_INVALID_HANDLE if mutex handle is invalid
 */
enum QCvReturnCode cv_notifyAll(const struct CvInfo* cv_info);

/**
 * @brief Destroy a condition variable
 *
 * This function destroys the condition variable specified by cv_info.
 *
 * @param[in] cv_info Condition variable handle
 * @return enum QCvReturnCode RETURN_CODE_CV_DESTROY_SUCCESS if condition variable destroy successful
 * @return enum QCvReturnCode RETURN_CODE_CV_DESTROY_FAILED if condition variable destroy failed
 * @return enum QCvReturnCode RETURN_CODE_CV_NULL_POINTER if cv_info is NULL
 * @return enum QCvReturnCode RETURN_CODE_CV_INVALID_HANDLE if condition variable handle is invalid
 */
enum QCvReturnCode cv_destroy(struct CvInfo* cv_info);

#endif /* QCV_H */
