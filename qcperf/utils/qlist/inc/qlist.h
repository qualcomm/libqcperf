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
 * @file qlist.h
 * @brief Doubly linked list utility library
 * @author Skand Gupta (skangupt@qti.qualcomm.com)
 */

#ifndef QLIST_H
#define QLIST_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "qmutex.h"

/**
 * @brief Return codes for list operations
 */
enum QListReturnCode {
    RETURN_CODE_LIST_SUCCESS = 0,  /**< Operation successful */
    RETURN_CODE_LIST_FAILED,       /**< Operation failed */
    RETURN_CODE_LIST_NULL_POINTER, /**< Null pointer provided */
    RETURN_CODE_LIST_VALID,        /**< List is valid */
    RETURN_CODE_LIST_INVALID,      /**< List is invalid */
    RETURN_CODE_LIST_EMPTY,        /**< List is empty */
    RETURN_CODE_LIST_NON_EMPTY     /**< List is not empty */
};

/**
 * @brief Node structure for linked list
 */
struct Node {
    void* data;        /**< Node data */
    struct Node* next; /**< Next node */
    struct Node* prev; /**< Previous node */
};

/**
 * @brief List structure
 */
struct List {
    struct Node* head;                        /**< Head node */
    struct Node* tail;                        /**< Tail node */
    uint32_t size;                            /**< List size */
    struct MutexInfo* mutex_info;             /**< Mutex for thread safety */
    struct MutexAttributes* mutex_attributes; /**< Mutex attributes */
};

/**
 * @brief Create a new list
 *
 * This function creates a new doubly linked list with the specified mutex name.
 *
 * @param[out] pp_list Pointer to list handle
 * @param[in] mutex_name Name for the mutex used for thread safety
 * @return enum QListReturnCode RETURN_CODE_LIST_SUCCESS if list creation successful
 * @return enum QListReturnCode RETURN_CODE_LIST_FAILED if list creation failed
 * @return enum QListReturnCode RETURN_CODE_LIST_NULL_POINTER if pp_list is NULL
 */
enum QListReturnCode list_create(struct List** pp_list, const char mutex_name[]);

/**
 * @brief Check if list is valid
 *
 * This function checks if the list is in a valid state.
 *
 * @param[in] p_list List handle
 * @return enum QListReturnCode RETURN_CODE_LIST_VALID if list is valid
 * @return enum QListReturnCode RETURN_CODE_LIST_INVALID if list is invalid
 * @return enum QListReturnCode RETURN_CODE_LIST_NULL_POINTER if p_list is NULL
 */
enum QListReturnCode list_check_valid(const struct List* p_list);

/**
 * @brief Get list size
 *
 * This function retrieves the size of the list.
 *
 * @param[in] p_list List handle
 * @param[out] p_size Pointer to store the list size
 * @return enum QListReturnCode RETURN_CODE_LIST_SUCCESS if operation successful
 * @return enum QListReturnCode RETURN_CODE_LIST_FAILED if operation failed
 * @return enum QListReturnCode RETURN_CODE_LIST_NULL_POINTER if p_list or p_size is NULL
 */
enum QListReturnCode list_size(const struct List* p_list, uint32_t* p_size);

/**
 * @brief Check if list is empty
 *
 * This function checks if the list is empty.
 *
 * @param[in] p_list List handle
 * @return enum QListReturnCode RETURN_CODE_LIST_EMPTY if list is empty
 * @return enum QListReturnCode RETURN_CODE_LIST_NON_EMPTY if list is not empty
 * @return enum QListReturnCode RETURN_CODE_LIST_NULL_POINTER if p_list is NULL
 */
enum QListReturnCode list_check_empty(const struct List* p_list);

/**
 * @brief Get node at specified index
 *
 * This function retrieves the node at the specified index.
 *
 * @param[in] p_list List handle
 * @param[in] index Index of the node (0-based)
 * @param[out] pp_node Pointer to store the node
 * @return enum QListReturnCode RETURN_CODE_LIST_SUCCESS if operation successful
 * @return enum QListReturnCode RETURN_CODE_LIST_FAILED if operation failed
 * @return enum QListReturnCode RETURN_CODE_LIST_NULL_POINTER if p_list or pp_node is NULL
 */
enum QListReturnCode list_get_node(const struct List* p_list, const uint32_t index, struct Node** pp_node);

/**
 * @brief Get data at specified index
 *
 * This function retrieves the data at the specified index.
 *
 * @param[in] p_list List handle
 * @param[in] index Index of the node (0-based)
 * @param[out] pp_data Pointer to store the data
 * @return enum QListReturnCode RETURN_CODE_LIST_SUCCESS if operation successful
 * @return enum QListReturnCode RETURN_CODE_LIST_FAILED if operation failed
 * @return enum QListReturnCode RETURN_CODE_LIST_NULL_POINTER if p_list or pp_data is NULL
 */
enum QListReturnCode list_get_data(const struct List* p_list, const uint32_t index, void** pp_data);

/**
 * @brief Push data to front of list
 *
 * This function adds data to the front of the list.
 *
 * @param[in] p_list List handle
 * @param[in] p_data Data to add
 * @return enum QListReturnCode RETURN_CODE_LIST_SUCCESS if operation successful
 * @return enum QListReturnCode RETURN_CODE_LIST_FAILED if operation failed
 * @return enum QListReturnCode RETURN_CODE_LIST_NULL_POINTER if p_list or p_data is NULL
 */
enum QListReturnCode list_push_front(struct List* p_list, void* p_data);

/**
 * @brief Push data to back of list
 *
 * This function adds data to the back of the list.
 *
 * @param[in] p_list List handle
 * @param[in] p_data Data to add
 * @return enum QListReturnCode RETURN_CODE_LIST_SUCCESS if operation successful
 * @return enum QListReturnCode RETURN_CODE_LIST_FAILED if operation failed
 * @return enum QListReturnCode RETURN_CODE_LIST_NULL_POINTER if p_list or p_data is NULL
 */
enum QListReturnCode list_push_back(struct List* p_list, void* p_data);

/**
 * @brief Insert data at specified index
 *
 * This function inserts data at the specified index.
 *
 * @param[in] p_list List handle
 * @param[in] index Index at which to insert data (0-based)
 * @param[in] p_data Data to insert
 * @return enum QListReturnCode RETURN_CODE_LIST_SUCCESS if operation successful
 * @return enum QListReturnCode RETURN_CODE_LIST_FAILED if operation failed
 * @return enum QListReturnCode RETURN_CODE_LIST_NULL_POINTER if p_list or p_data is NULL
 */
enum QListReturnCode list_insert_data(struct List* p_list, const uint32_t index, void* p_data);

/**
 * @brief Pop data from front of list
 *
 * This function removes and returns the data from the front of the list.
 *
 * @param[in] p_list List handle
 * @param[out] pp_data Pointer to store the data
 * @return enum QListReturnCode RETURN_CODE_LIST_SUCCESS if operation successful
 * @return enum QListReturnCode RETURN_CODE_LIST_FAILED if operation failed
 * @return enum QListReturnCode RETURN_CODE_LIST_NULL_POINTER if p_list or pp_data is NULL
 */
enum QListReturnCode list_pop_front(struct List* p_list, void** pp_data);

/**
 * @brief Get data from front of list without removing it
 *
 * This function returns the data from the front of the list without removing it.
 *
 * @param[in] p_list List handle
 * @param[out] pp_data Pointer to store the data
 * @return enum QListReturnCode RETURN_CODE_LIST_SUCCESS if operation successful
 * @return enum QListReturnCode RETURN_CODE_LIST_FAILED if operation failed
 * @return enum QListReturnCode RETURN_CODE_LIST_NULL_POINTER if p_list or pp_data is NULL
 */
enum QListReturnCode list_seek_front(const struct List* p_list, void** pp_data);

/**
 * @brief Pop data from back of list
 *
 * This function removes and returns the data from the back of the list.
 *
 * @param[in] p_list List handle
 * @param[out] pp_data Pointer to store the data
 * @return enum QListReturnCode RETURN_CODE_LIST_SUCCESS if operation successful
 * @return enum QListReturnCode RETURN_CODE_LIST_FAILED if operation failed
 * @return enum QListReturnCode RETURN_CODE_LIST_NULL_POINTER if p_list or pp_data is NULL
 */
enum QListReturnCode list_popBack(struct List* p_list, void** pp_data);

/**
 * @brief Get data from back of list without removing it
 *
 * This function returns the data from the back of the list without removing it.
 *
 * @param[in] p_list List handle
 * @param[out] pp_data Pointer to store the data
 * @return enum QListReturnCode RETURN_CODE_LIST_SUCCESS if operation successful
 * @return enum QListReturnCode RETURN_CODE_LIST_FAILED if operation failed
 * @return enum QListReturnCode RETURN_CODE_LIST_NULL_POINTER if p_list or pp_data is NULL
 */
enum QListReturnCode list_seek_back(const struct List* p_list, void** pp_data);

/**
 * @brief Erase node at specified index
 *
 * This function removes the node at the specified index and returns its data.
 *
 * @param[in] p_list List handle
 * @param[in] index Index of the node to erase (0-based)
 * @param[out] pp_data Pointer to store the data
 * @return enum QListReturnCode RETURN_CODE_LIST_SUCCESS if operation successful
 * @return enum QListReturnCode RETURN_CODE_LIST_FAILED if operation failed
 * @return enum QListReturnCode RETURN_CODE_LIST_NULL_POINTER if p_list or pp_data is NULL
 */
enum QListReturnCode list_erase_node(struct List* p_list, const uint32_t index, void** pp_data);

/**
 * @brief Clear all nodes from list
 *
 * This function removes all nodes from the list. Note that it does not free the data.
 *
 * @param[in] p_list List handle
 * @return enum QListReturnCode RETURN_CODE_LIST_SUCCESS if operation successful
 * @return enum QListReturnCode RETURN_CODE_LIST_FAILED if operation failed
 * @return enum QListReturnCode RETURN_CODE_LIST_NULL_POINTER if p_list is NULL
 */
enum QListReturnCode list_clear_all(struct List* p_list);

/**
 * @brief Delete list
 *
 * This function deletes the list and frees all associated resources.
 * Note that it does not free the data stored in the list.
 *
 * @param[in,out] pp_list Pointer to list handle
 * @return enum QListReturnCode RETURN_CODE_LIST_SUCCESS if operation successful
 * @return enum QListReturnCode RETURN_CODE_LIST_FAILED if operation failed
 * @return enum QListReturnCode RETURN_CODE_LIST_NULL_POINTER if pp_list is NULL
 */
enum QListReturnCode list_delete(struct List** pp_list);

#endif /* QLIST_H */
