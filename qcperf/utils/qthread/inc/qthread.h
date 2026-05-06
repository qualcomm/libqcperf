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
 * @file qthread.h
 * @brief Thread utility library for cross-platform thread management
 * @author Skand Gupta (skangupt@qti.qualcomm.com)
 */

#ifndef QTHREAD_H
#define QTHREAD_H

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * @brief Maximum size for thread name
 */
#define THREAD_NAME_SIZE 128

/**
 * @brief Function pointer type for thread functions
 */
typedef void* (*QThreadFunction)(void* param);

/**
 * @brief Thread attributes structure
 */
struct QThreadAttributes {
    uint8_t thread_name[THREAD_NAME_SIZE]; /**< Thread name */
    uint8_t thread_name_len;               /**< Length of thread name */
    uint32_t priority;                     /**< Thread priority */
    uint32_t stack_size;                   /**< Thread stack size */
    uint8_t* stack_address;                /**< Thread stack address */
    QThreadFunction thread_fn;             /**< Thread function */
    void* thread_params;                   /**< Thread function parameters */
};

/**
 * @brief Thread information structure
 */
struct QThreadInfo {
    void* thread_handle; /**< Thread handle */
    uint64_t thread_id;  /**< Thread ID */
};

/**
 * @brief Return codes for thread operations
 */
enum QThreadReturnCode {
    RET_QTHREAD_CREATE_SUCCESS = 0,     /**< Thread creation successful */
    RET_QTHREAD_CREATE_FAILED,          /**< Thread creation failed */
    RET_QTHREAD_NULL_POINTER,           /**< Null pointer provided */
    RET_QTHREAD_DESTROY_FAILED,         /**< Thread destruction failed */
    RET_QTHREAD_DESTROY_SUCCESS,        /**< Thread destruction successful */
    RET_QTHREAD_JOIN_SUCCESS,           /**< Thread join successful */
    RET_QTHREAD_JOIN_FAILED,            /**< Thread join failed */
    RET_QTHREAD_INVALID_HANDLE,         /**< Invalid thread handle */
    RET_QTHREAD_CURRENT_THREAD_SUCCESS, /**< Current thread ID retrieval successful */
    RET_QTHREAD_CURRENT_THREAD_FAILED,  /**< Current thread ID retrieval failed */
    RET_QTHREAD_TERMINATE_FAILED,       /**< Thread termination failed */
    RET_QTHREAD_TERMINATE_SUCCESS       /**< Thread termination successful */
};

/**
 * @brief Create a new thread with the specified attributes, and makes it executable.
 *
 * This function creates a new thread with the attributes specified in thread_request.
 * The thread is immediately executable after creation.
 *
 * @param[in] thread_request Thread attributes
 * @param[out] thread_info Thread handle. Same structure needs to be passed to the destroy call
 * @return enum QThreadReturnCode RET_QTHREAD_CREATE_SUCCESS if thread creation successful
 * @return enum QThreadReturnCode RET_QTHREAD_CREATE_FAILED if thread creation failed
 * @return enum QThreadReturnCode RET_QTHREAD_NULL_POINTER if thread_request or thread_info is NULL
 */
enum QThreadReturnCode thread_create(struct QThreadAttributes* thread_request, struct QThreadInfo* thread_info);

/**
 * @brief Close/free the thread handle
 *
 * This function closes and frees resources associated with the thread handle.
 * It should be called when the thread is no longer needed.
 *
 * @param[in] thread_info Thread handle. Same structure used in thread_create
 * @return enum QThreadReturnCode RET_QTHREAD_DESTROY_SUCCESS if thread destroy successful
 * @return enum QThreadReturnCode RET_QTHREAD_DESTROY_FAILED if thread destroy failed
 * @return enum QThreadReturnCode RET_QTHREAD_NULL_POINTER if thread_info is NULL
 * @return enum QThreadReturnCode RET_QTHREAD_INVALID_HANDLE if thread handle is invalid
 */
enum QThreadReturnCode thread_destroy(struct QThreadInfo* thread_info);

/**
 * @brief Join the thread to main thread
 *
 * This function waits for the specified thread to terminate.
 * It blocks the calling thread until the specified thread terminates.
 *
 * @param[in] thread_info Thread handle. Same structure used in thread_create
 * @return enum QThreadReturnCode RET_QTHREAD_JOIN_SUCCESS if thread join successful
 * @return enum QThreadReturnCode RET_QTHREAD_JOIN_FAILED if thread join failed
 * @return enum QThreadReturnCode RET_QTHREAD_NULL_POINTER if thread_info is NULL
 * @return enum QThreadReturnCode RET_QTHREAD_INVALID_HANDLE if thread handle is invalid
 */
enum QThreadReturnCode thread_join(struct QThreadInfo* thread_info);

/**
 * @brief Get current thread handle and identifier
 *
 * This function retrieves the handle and ID of the current thread.
 * The user should call this function from within the thread function.
 *
 * @param[out] thread_info Thread handle and ID
 * @return enum QThreadReturnCode RET_QTHREAD_CURRENT_THREAD_SUCCESS if successful
 * @return enum QThreadReturnCode RET_QTHREAD_CURRENT_THREAD_FAILED if failed
 * @return enum QThreadReturnCode RET_QTHREAD_NULL_POINTER if thread_info is NULL
 */
enum QThreadReturnCode thread_current_thread_id(struct QThreadInfo* thread_info);

/**
 * @brief Terminate the thread (Windows only)
 *
 * This function forcibly terminates the specified thread.
 * Note: This function is only available on Windows platforms.
 *
 * @param[in] thread_info Thread handle. Same structure used in thread_create
 * @return enum QThreadReturnCode RET_QTHREAD_TERMINATE_SUCCESS if thread termination successful
 * @return enum QThreadReturnCode RET_QTHREAD_TERMINATE_FAILED if thread termination failed
 * @return enum QThreadReturnCode RET_QTHREAD_NULL_POINTER if thread_info is NULL
 * @return enum QThreadReturnCode RET_QTHREAD_INVALID_HANDLE if thread handle is invalid
 */
enum QThreadReturnCode thread_terminate(struct QThreadInfo* thread_info);

#endif /* QTHREAD_H */
