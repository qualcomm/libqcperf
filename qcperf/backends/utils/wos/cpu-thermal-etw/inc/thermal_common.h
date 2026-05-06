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
 * @file thermal_common.h
 * @brief Common definitions and functions for thermal monitoring using ETW
 * @author Qualcomm Technologies, Inc.
 *
 * This file provides common structures, constants, and functions for thermal
 * monitoring on Windows platforms using Event Tracing for Windows (ETW).
 * It includes functionality for managing thermal zone information, ETW sessions,
 * and thermal data collection.
 */

#ifndef THERMAL_COMMON_H
#define THERMAL_COMMON_H

#include <stdint.h>

/**
 * @brief Maximum number of thermal zones supported
 */
#define MAX_NUM_OF_ZONES 64

/**
 * @brief Maximum length for thermal zone descriptive names
 */
#define MAX_THERMAL_ZONE_DESCRIPTIVE_NAME_LENGTH 64

/**
 * @brief Maximum length for thermal zone BIOS names
 */
#define MAX_THERMAL_ZONE_BIOS_NAME_LENGTH 32

/**
 * @brief Maximum thermal zone ID value
 */
#define MAX_THERMAL_ZONE_ID 100

/**
 * @brief Maximum length for thermal zone names
 */
#define MAX_THERMAL_ZONE_NAME_LENGTH 64

/**
 * @brief Macro to enable thermal ETW providers
 *
 * @param trace_callback Callback function for ETW events
 * @param trace ETW trace object
 * @param main_provider Main ETW provider
 * @param polling_provider Polling ETW provider
 */
#define ENABLE_THERMAL_PROVIDERS(trace_callback, trace, main_provider, polling_provider) \
    {                                                                                    \
        main_provider.any(ANY_FLAG_ACPI_PROVIDER);                                       \
        main_provider.add_on_event_callback(trace_callback);                             \
        trace.enable(main_provider);                                                     \
        trace.enable(polling_provider);                                                  \
    }

#ifdef WIN32
#define DllExport __declspec(dllexport)
#else
#define DllExport
#endif
#ifdef __cplusplus
extern "C" {
#endif

#include "qthread.h"
#include "qmutex.h"
#include "QSleep.h"
#include "qcv.h"
#include "qlist.h"

/**
 * @brief GUID for ACPI thermal provider
 */
#define ACPI_PROVIDER_GUID L"{C514638F-7723-485B-BCFC-96565D735D4A}"

/**
 * @brief GUID for polling thermal provider
 */
#define POLLING_PROVIDER_GUID L"{e8a7c168-81ee-465c-8e8e-d39a2ac1ca41}"

/**
 * @brief Return codes for thermal common operations
 */
enum ThermalCommonReturnCode {
    RETURN_CODE_THERMAL_COMMON_SUCCESS,                         /**< Operation successful */
    RETURN_CODE_THERMAL_COMMON_NULL_POINTER,                    /**< Null pointer provided */
    RETURN_CODE_THERMAL_COMMON_MEMORY_ALLOCATION_FAILED,        /**< Memory allocation failed */
    RETURN_CODE_THERMAL_COMMON_NATIVE_FUNCTION_FAILED,          /**< Native function call failed */
    RETURN_CODE_THERMAL_COMMON_INVALID_INPUT,                   /**< Invalid input parameter */
    RETURN_CODE_THERMAL_COMMON_VALUE_WOULD_OVERFLOW,            /**< Value would overflow */
    RETURN_CODE_THERMAL_COMMON_NO_INSTANCES,                    /**< No instances found */
    RETURN_CODE_THERMAL_COMMON_UNCAUGHT_FAILURE,                /**< Uncaught failure */
    RETURN_CODE_THERMAL_COMMON_UNEXPECTED_STATE,                /**< Unexpected state */
    RETURN_CODE_THERMAL_COMMON_CAUGHT_EXCEPTION,                /**< Caught exception */
    RETURN_CODE_THERMAL_COMMON_ZONE_ID_OUT_OF_RANGE,            /**< Zone ID out of range */
    RETURN_CODE_THERMAL_COMMON_ENUMERATE_TRACE_GUIDS_FAILED,    /**< Failed to enumerate trace GUIDs */
    RETURN_CODE_THERMAL_COMMON_NO_TRACES_FOUND,                 /**< No traces found */
    RETURN_CODE_THERMAL_COMMON_QUERY_ALL_TRACES_FAILED,         /**< Failed to query all traces */
    RETURN_CODE_THERMAL_COMMON_NO_QC_TRACES_FOUND,              /**< No Qualcomm traces found */
    RETURN_CODE_THERMAL_COMMON_EVENT_CONTROL_TRACE_STOP_FAILED, /**< Failed to stop event control trace */
    RETURN_CODE_THERMAL_COMMON_START_TRACE_FAILED,              /**< Failed to start trace */
};

/**
 * @brief Structure for thermal zone name strings
 */
struct ThermalCommonString {
    char string[MAX_THERMAL_ZONE_NAME_LENGTH]; /**< String buffer */
    size_t string_length;                      /**< Length of string */
};

/**
 * @brief Structure for mapping thermal zone IDs to names
 */
struct ThermalCommonZoneNameMap {
    struct ThermalCommonString zone_names[MAX_NUM_OF_ZONES]; /**< Array of zone names */
    uint8_t zone_ids[MAX_NUM_OF_ZONES];                      /**< Array of zone IDs */
    uint8_t zone_names_ids_length;                           /**< Number of valid entries in arrays */
    uint8_t zone_id_to_name_index_map[MAX_THERMAL_ZONE_ID];  /**< Map from zone ID to name index */
};

/**
 * @brief Structure for ETW listening thread start request
 */
struct ThermalCommonStartRequest {
    struct QThreadAttributes* thread_attributes; /**< Thread attributes */
    void* callback_condition_variable;           /**< Callback condition variable */
    void* callback_function;                     /**< Callback function (krabs::c_provider_callback *) */
    uint32_t timeout_ms;                         /**< Timeout in milliseconds */
};

/**
 * @brief Structure for managing ETW sessions
 */
struct ThermalCommonSessions {
    void* qlist; /**< List of sessions */
};

/**
 * @brief Start ETW listening thread
 *
 * @param request Start request parameters
 * @param stopped_sessions Container for stopped sessions
 * @return enum ThermalCommonReturnCode Return code indicating success or failure
 */
DllExport enum ThermalCommonReturnCode thermal_common_start_etw_listening_thread(struct ThermalCommonStartRequest* request, struct ThermalCommonSessions* stopped_sessions);

/**
 * @brief Restart stopped ETW sessions
 *
 * @param stopped_sessions Container with stopped sessions to restart
 * @return enum ThermalCommonReturnCode Return code indicating success or failure
 */
DllExport enum ThermalCommonReturnCode thermal_common_restart_stopped_sessions(struct ThermalCommonSessions* stopped_sessions);

/**
 * @brief Parse thermal zone ID from BIOS name
 *
 * @param bios_name BIOS name to parse
 * @param id Pointer to store the parsed ID
 * @return enum ThermalCommonReturnCode Return code indicating success or failure
 */
DllExport enum ThermalCommonReturnCode thermal_common_parse_id_from_bios_name(struct ThermalCommonString* bios_name, uint8_t* id);

/**
 * @brief Initialize thermal common module
 *
 * @return enum ThermalCommonReturnCode Return code indicating success or failure
 */
DllExport enum ThermalCommonReturnCode thermal_common_init();

/**
 * @brief Clean up thermal common module resources
 *
 * @return enum ThermalCommonReturnCode Return code indicating success or failure
 */
DllExport enum ThermalCommonReturnCode thermal_common_cleanup();

/**
 * @brief Get thermal zone names
 *
 * @param map Pointer to store zone name mapping
 * @return enum ThermalCommonReturnCode Return code indicating success or failure
 */
DllExport enum ThermalCommonReturnCode thermal_common_getzone_names(struct ThermalCommonZoneNameMap* map);

#ifdef __cplusplus
}
#endif

#endif  // THERMAL_COMMON_H
