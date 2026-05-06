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
 * @file qlist.c
 * @brief Implementation of doubly linked list utility library
 * @author Skand Gupta (skangupt@qti.qualcomm.com)
 *
 * This file implements the doubly linked list functions defined in qlist.h,
 * providing a generic, thread-safe linked list data structure. The implementation
 * supports operations such as creating a list, adding elements to the front or back,
 * inserting elements at specific positions, removing elements, and traversing the list.
 *
 * The list implementation is thread-safe, using mutexes to protect critical sections
 * during operations that modify the list structure. It also provides comprehensive
 * error handling and validation to ensure robust operation.
 */

#include "qlist.h"

enum QListReturnCode list_create(struct List** pp_list, const char mutex_name[]) {
    enum QListReturnCode return_code = RETURN_CODE_LIST_SUCCESS;
    struct List* plist               = (struct List*)calloc(1, sizeof(struct List));

    if (plist != NULL) {
        plist->head = NULL;
        plist->tail = NULL;
        plist->size = 0;

        plist->mutex_attributes = (struct MutexAttributes*)calloc(1, sizeof(struct MutexAttributes));
        if (plist->mutex_attributes != NULL) {
            plist->mutex_info = (struct MutexInfo*)calloc(1, sizeof(struct MutexInfo));
            if (plist->mutex_info != NULL) {
                plist->mutex_attributes->mutex_name_len = (uint8_t)snprintf((char*)plist->mutex_attributes->mutex_name, MUTEX_NAME_SIZE, "%s", (char*)mutex_name);
                enum QMutexReturnCode mutex_return_code = mutex_create(plist->mutex_attributes, plist->mutex_info);
                if (mutex_return_code == RETURN_CODE_MUTEX_CREATE_SUCCESS) {
                    *pp_list = plist;
                } else {
                    // LOG_MSG(QERROR, "%s", "ERROR : qlist : Unable to create __qlist-mutex__.\n");
                    if (plist->mutex_info != NULL) {
                        free(plist->mutex_info);
                        plist->mutex_info = NULL;
                    }

                    if (plist->mutex_attributes != NULL) {
                        free(plist->mutex_attributes);
                        plist->mutex_attributes = NULL;
                    }

                    if (plist != NULL) {
                        free(plist);
                        plist = NULL;
                    }
                    return_code = RETURN_CODE_LIST_FAILED;
                }
            } else {
                // LOG_MSG(QERROR, "%s", "ERROR : qlist : Unable to create Mutex Info.\n");
                if (plist->mutex_attributes != NULL) {
                    free(plist->mutex_attributes);
                    plist->mutex_attributes = NULL;
                }

                if (plist != NULL) {
                    free(plist);
                    plist = NULL;
                }
                return_code = RETURN_CODE_LIST_NULL_POINTER;
            }

        } else {
            // LOG_MSG(QERROR, "%s", "ERROR : qlist : Unable to create Mutex attributes.\n");
            if (plist != NULL) {
                free(plist);
                plist = NULL;
            }
            return_code = RETURN_CODE_LIST_NULL_POINTER;
        }
    } else {
        // LOG_MSG(QERROR, "%s", "ERROR : qlist : Unable to create list.\n");
        return_code = RETURN_CODE_LIST_NULL_POINTER;
    }
    // LOG_MSG(QDEBUG, "%s", "exit");
    return return_code;
}

enum QListReturnCode list_check_valid(const struct List* p_list) {
    enum QListReturnCode return_code = RETURN_CODE_LIST_INVALID;
    if (p_list == NULL) {
        return_code = RETURN_CODE_LIST_NULL_POINTER;
    } else {
        if (p_list->head && p_list->tail) {
            if (p_list->size > 0) {
                return_code = RETURN_CODE_LIST_VALID;
            }
        }
        if ((p_list->head == NULL) && (p_list->tail == NULL)) {
            if (p_list->size == 0) {
                return_code = RETURN_CODE_LIST_VALID;
            }
        }
    }
    return return_code;
}

enum QListReturnCode list_size(const struct List* p_list, uint32_t* p_size) {
    enum QListReturnCode return_code       = RETURN_CODE_LIST_FAILED;
    enum QListReturnCode is_valid_ret_code = list_check_valid(p_list);
    if (is_valid_ret_code == RETURN_CODE_LIST_VALID) {
        *p_size     = p_list->size;
        return_code = RETURN_CODE_LIST_SUCCESS;
    } else {
        return_code = is_valid_ret_code;
    }
    return return_code;
}

enum QListReturnCode list_check_empty(const struct List* p_list) {
    enum QListReturnCode return_code       = RETURN_CODE_LIST_FAILED;
    enum QListReturnCode is_valid_ret_code = list_check_valid(p_list);
    if (is_valid_ret_code == RETURN_CODE_LIST_VALID) {
        uint32_t lsize;
        enum QListReturnCode returnCodeSize = list_size(p_list, &lsize);
        if (returnCodeSize == RETURN_CODE_LIST_SUCCESS) {
            if (lsize == 0) {
                return_code = RETURN_CODE_LIST_EMPTY;
            } else if (lsize > 0) {
                return_code = RETURN_CODE_LIST_NON_EMPTY;
            } else {
                return_code = RETURN_CODE_LIST_INVALID;
            }
        } else {
            return_code = returnCodeSize;
        }
    } else {
        return_code = is_valid_ret_code;
    }
    return return_code;
}

enum QListReturnCode list_get_node(const struct List* p_list, const uint32_t index, struct Node** pp_node) {
    enum QListReturnCode return_code = RETURN_CODE_LIST_FAILED;
    if (p_list == NULL || pp_node == NULL) {
        return_code = RETURN_CODE_LIST_NULL_POINTER;
    } else {
        mutex_lock(p_list->mutex_info);
        enum QListReturnCode is_valid_ret_code = list_check_valid(p_list);
        if (is_valid_ret_code != RETURN_CODE_LIST_VALID) {
            return_code = is_valid_ret_code;
        } else {
            enum QListReturnCode is_empty_ret_code = list_check_empty(p_list);
            if (is_empty_ret_code == RETURN_CODE_LIST_NON_EMPTY) {
                uint32_t qsize;
                list_size(p_list, &qsize);
                if (index == 0) {
                    *pp_node = p_list->head;
                } else if (index == qsize - 1) {
                    *pp_node = p_list->tail;
                } else {
                    uint32_t back_distance = qsize - index - 1;
                    if (back_distance < index) {
                        struct Node* temp = p_list->tail;
                        for (uint32_t i = 0; i < back_distance; i++) {
                            temp = temp->prev;
                        }
                        *pp_node = temp;
                    } else {
                        struct Node* temp = p_list->head;
                        for (uint32_t i = 0; i < index; i++) {
                            temp = temp->next;
                        }
                        *pp_node = temp;
                    }
                }
                return_code = RETURN_CODE_LIST_SUCCESS;
            } else {
                return_code = is_empty_ret_code;
            }
        }
        mutex_unlock(p_list->mutex_info);
    }
    return return_code;
}

enum QListReturnCode list_get_data(const struct List* p_list, const uint32_t index, void** pp_data) {
    enum QListReturnCode return_code = RETURN_CODE_LIST_FAILED;
    if (p_list == NULL || pp_data == NULL) {
        return_code = RETURN_CODE_LIST_NULL_POINTER;
    } else {
        struct Node* pNode;
        enum QListReturnCode retGetNode = list_get_node(p_list, index, &pNode);
        if (retGetNode == RETURN_CODE_LIST_SUCCESS) {
            *pp_data    = pNode->data;
            return_code = RETURN_CODE_LIST_SUCCESS;
        } else {
            return_code = retGetNode;
        }
    }
    return return_code;
}

enum QListReturnCode list_push_front(struct List* p_list, void* p_data) {
    enum QListReturnCode return_code = RETURN_CODE_LIST_FAILED;
    if (p_list == NULL || p_data == NULL) {
        return_code = RETURN_CODE_LIST_NULL_POINTER;
    } else {
        mutex_lock(p_list->mutex_info);
        enum QListReturnCode is_valid_ret_code = list_check_valid(p_list);
        if (is_valid_ret_code != RETURN_CODE_LIST_VALID) {
            return_code = is_valid_ret_code;
        } else {
            struct Node* pNodeNew = (struct Node*)calloc(1, sizeof(struct Node));
            if (pNodeNew == NULL) {
                return_code = RETURN_CODE_LIST_NULL_POINTER;
            } else {
                pNodeNew->data = p_data;
                if (p_list->head && p_list->tail) {
                    pNodeNew->next     = p_list->head;
                    pNodeNew->prev     = NULL;
                    p_list->head->prev = pNodeNew;
                    p_list->head       = pNodeNew;
                } else {
                    p_list->head = pNodeNew;
                    p_list->tail = pNodeNew;
                }
                p_list->size = p_list->size + 1;
                return_code  = RETURN_CODE_LIST_SUCCESS;
            }
        }
        mutex_unlock(p_list->mutex_info);
    }
    return return_code;
}

enum QListReturnCode list_push_back(struct List* p_list, void* p_data) {
    enum QListReturnCode return_code = RETURN_CODE_LIST_FAILED;
    if (p_list == NULL || p_data == NULL) {
        return_code = RETURN_CODE_LIST_NULL_POINTER;
    } else {
        mutex_lock(p_list->mutex_info);
        enum QListReturnCode is_valid_ret_code = list_check_valid(p_list);
        if (is_valid_ret_code != RETURN_CODE_LIST_VALID) {
            return_code = is_valid_ret_code;
        } else {
            struct Node* pNodeNew = (struct Node*)calloc(1, sizeof(struct Node));
            if (pNodeNew == NULL) {
                return_code = RETURN_CODE_LIST_NULL_POINTER;
            } else {
                pNodeNew->data = p_data;
                if (p_list->tail && p_list->head) {
                    p_list->tail->next = pNodeNew;
                    pNodeNew->prev     = p_list->tail;
                    p_list->tail       = pNodeNew;
                } else {
                    p_list->head = pNodeNew;
                    p_list->tail = pNodeNew;
                }
                p_list->size = p_list->size + 1;
                return_code  = RETURN_CODE_LIST_SUCCESS;
            }
        }
        mutex_unlock(p_list->mutex_info);
    }
    return return_code;
}

enum QListReturnCode list_insert_data(struct List* p_list, const uint32_t index, void* p_data) {
    enum QListReturnCode return_code = RETURN_CODE_LIST_FAILED;
    if (p_list == NULL || p_data == NULL) {
        return_code = RETURN_CODE_LIST_NULL_POINTER;
    } else {
        mutex_lock(p_list->mutex_info);
        enum QListReturnCode is_valid_ret_code = list_check_valid(p_list);
        if (is_valid_ret_code != RETURN_CODE_LIST_VALID) {
            return_code = is_valid_ret_code;
        } else {
            uint32_t lsize;
            list_size(p_list, &lsize);
            if (index == 0) {
                mutex_unlock(p_list->mutex_info);
                return_code = list_push_front(p_list, p_data);
            } else if (index == lsize) {
                mutex_unlock(p_list->mutex_info);
                return_code = list_push_back(p_list, p_data);
            } else {
                if ((index > 0) && (index < lsize)) {
                    struct Node* pNodei;
                    enum QListReturnCode retGetNode = list_get_node(p_list, index, &pNodei);
                    if (retGetNode == RETURN_CODE_LIST_SUCCESS) {
                        struct Node* pNodeNew = (struct Node*)calloc(1, sizeof(struct Node));
                        if (pNodeNew != NULL) {
                            pNodeNew->data       = p_data;
                            pNodeNew->prev       = pNodei->prev;
                            pNodeNew->next       = pNodei;
                            pNodeNew->prev->next = pNodeNew;
                            pNodeNew->next->prev = pNodeNew;
                            p_list->size         = p_list->size + 1;
                            return_code          = RETURN_CODE_LIST_SUCCESS;
                        } else {
                            return_code = RETURN_CODE_LIST_NULL_POINTER;
                        }
                    } else {
                        return_code = retGetNode;
                    }
                }
            }
        }
        mutex_unlock(p_list->mutex_info);
    }
    return return_code;
}

enum QListReturnCode list_pop_front(struct List* p_list, void** pp_data) {
    enum QListReturnCode return_code = RETURN_CODE_LIST_FAILED;
    if (p_list == NULL || pp_data == NULL) {
        return_code = RETURN_CODE_LIST_NULL_POINTER;
    } else {
        mutex_lock(p_list->mutex_info);
        enum QListReturnCode is_valid_ret_code = list_check_valid(p_list);
        if (is_valid_ret_code != RETURN_CODE_LIST_VALID) {
            return_code = is_valid_ret_code;
        } else {
            enum QListReturnCode is_empty_ret_code = list_check_empty(p_list);
            if (is_empty_ret_code == RETURN_CODE_LIST_NON_EMPTY) {
                if (p_list->head->next) {
                    struct Node* temp  = p_list->head;
                    p_list->head       = temp->next;
                    p_list->head->prev = NULL;
                    *pp_data           = temp->data;
                    free(temp);
                    temp = NULL;
                } else {
                    *pp_data = p_list->head->data;
                    free(p_list->head);
                    p_list->head = NULL;
                    p_list->tail = NULL;
                }
                p_list->size = p_list->size - 1;
                return_code  = RETURN_CODE_LIST_SUCCESS;
            } else {
                return_code = is_empty_ret_code;
            }
        }
        mutex_unlock(p_list->mutex_info);
    }
    return return_code;
}

enum QListReturnCode list_seek_front(const struct List* p_list, void** pp_data) {
    enum QListReturnCode return_code = RETURN_CODE_LIST_FAILED;
    if (p_list == NULL || pp_data == NULL) {
        return_code = RETURN_CODE_LIST_NULL_POINTER;
    } else {
        mutex_lock(p_list->mutex_info);
        enum QListReturnCode is_valid_ret_code = list_check_valid(p_list);
        if (is_valid_ret_code != RETURN_CODE_LIST_VALID) {
            return_code = is_valid_ret_code;
        } else {
            enum QListReturnCode is_empty_ret_code = list_check_empty(p_list);
            if (is_empty_ret_code == RETURN_CODE_LIST_NON_EMPTY) {
                *pp_data    = p_list->head->data;
                return_code = RETURN_CODE_LIST_SUCCESS;
            } else {
                return_code = is_empty_ret_code;
            }
        }
        mutex_unlock(p_list->mutex_info);
    }
    return return_code;
}

enum QListReturnCode list_popBack(struct List* p_list, void** pp_data) {
    enum QListReturnCode return_code = RETURN_CODE_LIST_FAILED;
    if (p_list == NULL || pp_data == NULL) {
        return_code = RETURN_CODE_LIST_NULL_POINTER;
    } else {
        mutex_lock(p_list->mutex_info);
        enum QListReturnCode is_valid_ret_code = list_check_valid(p_list);
        if (is_valid_ret_code != RETURN_CODE_LIST_VALID) {
            return_code = is_valid_ret_code;
        } else {
            enum QListReturnCode is_empty_ret_code = list_check_empty(p_list);
            if (is_empty_ret_code == RETURN_CODE_LIST_NON_EMPTY) {
                if (p_list->tail->prev) {
                    struct Node* temp  = p_list->tail;
                    p_list->tail       = temp->prev;
                    p_list->tail->next = NULL;
                    *pp_data           = temp->data;
                    free(temp);
                    temp = NULL;
                } else {
                    *pp_data = p_list->tail->data;
                    free(p_list->tail);
                    p_list->tail = NULL;
                    p_list->head = NULL;
                }
                p_list->size = p_list->size - 1;
                return_code  = RETURN_CODE_LIST_SUCCESS;
            } else {
                return_code = is_empty_ret_code;
            }
        }
        mutex_unlock(p_list->mutex_info);
    }
    return return_code;
}

enum QListReturnCode list_seek_back(const struct List* p_list, void** pp_data) {
    enum QListReturnCode return_code = RETURN_CODE_LIST_FAILED;
    if (p_list == NULL || pp_data == NULL) {
        return_code = RETURN_CODE_LIST_NULL_POINTER;
    } else {
        mutex_lock(p_list->mutex_info);
        enum QListReturnCode is_valid_ret_code = list_check_valid(p_list);
        if (is_valid_ret_code != RETURN_CODE_LIST_VALID) {
            return_code = is_valid_ret_code;
        } else {
            enum QListReturnCode is_empty_ret_code = list_check_empty(p_list);
            if (is_empty_ret_code == RETURN_CODE_LIST_NON_EMPTY) {
                *pp_data    = p_list->tail->data;
                return_code = RETURN_CODE_LIST_SUCCESS;
            } else {
                return_code = is_empty_ret_code;
            }
        }
        mutex_unlock(p_list->mutex_info);
    }
    return return_code;
}

enum QListReturnCode list_erase_node(struct List* p_list, const uint32_t index, void** pp_data) {
    enum QListReturnCode return_code = RETURN_CODE_LIST_FAILED;
    if (p_list == NULL || pp_data == NULL) {
        return_code = RETURN_CODE_LIST_NULL_POINTER;
    } else {
        mutex_lock(p_list->mutex_info);
        enum QListReturnCode is_valid_ret_code = list_check_valid(p_list);
        if (is_valid_ret_code != RETURN_CODE_LIST_VALID) {
            return_code = is_valid_ret_code;
        } else {
            enum QListReturnCode is_empty_ret_code = list_check_empty(p_list);
            if (is_empty_ret_code == RETURN_CODE_LIST_NON_EMPTY) {
                uint32_t lsize;
                list_size(p_list, &lsize);
                if (index == 0) {
                    mutex_unlock(p_list->mutex_info);
                    return_code = list_pop_front(p_list, pp_data);
                    mutex_lock(p_list->mutex_info);
                } else if (index == lsize - 1) {
                    mutex_unlock(p_list->mutex_info);
                    return_code = list_popBack(p_list, pp_data);
                    mutex_lock(p_list->mutex_info);
                } else {
                    if ((index > 0) && (index < lsize)) {
                        struct Node* pNodei;
                        mutex_unlock(p_list->mutex_info);
                        enum QListReturnCode retGetNode = list_get_node(p_list, index, &pNodei);
                        mutex_lock(p_list->mutex_info);
                        if (retGetNode == RETURN_CODE_LIST_SUCCESS) {
                            *pp_data           = pNodei->data;
                            pNodei->prev->next = pNodei->next;
                            pNodei->next->prev = pNodei->prev;
                            p_list->size       = p_list->size - 1;
                            free(pNodei);
                            pNodei      = NULL;
                            return_code = RETURN_CODE_LIST_SUCCESS;
                        } else {
                            return_code = retGetNode;
                        }
                    }
                }
            } else {
                return_code = is_empty_ret_code;
            }
        }
        mutex_unlock(p_list->mutex_info);
    }
    return return_code;
}

enum QListReturnCode list_clear_all(struct List* p_list) {
    enum QListReturnCode return_code = RETURN_CODE_LIST_FAILED;
    if (p_list == NULL) {
        return_code = RETURN_CODE_LIST_NULL_POINTER;
    } else {
        mutex_lock(p_list->mutex_info);
        enum QListReturnCode is_valid_ret_code = list_check_valid(p_list);
        if (is_valid_ret_code != RETURN_CODE_LIST_VALID) {
            return_code = is_valid_ret_code;
        } else {
            enum QListReturnCode is_empty_ret_code = list_check_empty(p_list);
            if (is_empty_ret_code == RETURN_CODE_LIST_NON_EMPTY) {
                struct Node* p_node_next = p_list->head;
                struct Node* p_node_cur  = p_list->head;
                while (p_node_cur) {
                    p_node_next = p_node_cur->next;
                    free(p_node_cur);
                    p_list->size = p_list->size - 1;
                    p_node_cur   = p_node_next;
                }
                p_list->head = NULL;
                p_list->tail = NULL;
                return_code  = RETURN_CODE_LIST_SUCCESS;
            } else {
                return_code = is_empty_ret_code;
            }
        }
        mutex_unlock(p_list->mutex_info);
    }
    return return_code;
}

enum QListReturnCode list_delete(struct List** pp_list) {
    enum QListReturnCode return_code = RETURN_CODE_LIST_FAILED;
    if (pp_list == NULL || ((*pp_list) == NULL)) {
        return_code = RETURN_CODE_LIST_NULL_POINTER;
    } else {
        return_code = list_clear_all((*pp_list));
        if (return_code == RETURN_CODE_LIST_SUCCESS || return_code == RETURN_CODE_LIST_EMPTY) {
            mutex_destroy((*pp_list)->mutex_info);
            if ((*pp_list)->mutex_info != NULL) {
                free((*pp_list)->mutex_info);
                (*pp_list)->mutex_info = NULL;
            }

            if ((*pp_list)->mutex_attributes != NULL) {
                free((*pp_list)->mutex_attributes);
                (*pp_list)->mutex_attributes = NULL;
            }

            free((*pp_list));
            *pp_list = NULL;
        }
    }
    return return_code;
}
