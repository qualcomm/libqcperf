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

#include "rpc_symbols.h"
#include "rpcmem_wos.h"
#include <windows.h>
#include <winsvc.h>
#include <string.h>
#include <stdio.h>

#define FASTRPC_DLL_NAME "libcdsprpc.dll"

static const char g_fastrpcServiceName[][16] = {
    "qcadsprpc",   /* ADSP_DOMAIN_ID = 0 */
    "qcadsprpc",   /* SDSP_DOMAIN_ID = 1 */
    "qcadsprpc",   /* MDSP_DOMAIN_ID = 2 */
    "qcnspmcdm",   /* CDSP_DOMAIN_ID = 3 */
    "qcnspmcdm",   /* CDSP1_DOMAIN_ID = 4 */
    "qcadsprpc"    /* GPDSP0_DOMAIN_ID = 5 */
};

static const char g_fastrpcServiceNameNew[][16] = {
    "qcadsprpc_8480",
    "qcadsprpc_8480",
    "qcadsprpc_8480",
    "qcnspmcdm_8480",
    "qcnspmcdm_8480",
    "qcadsprpc_8480"
};

fnremote_handle64_open premote_handle64_open[MAX_NUMBER_OF_DSPs]       = {0};
fnremote_handle64_close premote_handle64_close[MAX_NUMBER_OF_DSPs]     = {0};
fnremote_handle64_invoke premote_handle64_invoke[MAX_NUMBER_OF_DSPs]   = {0};
fnremote_session_control premote_session_control[MAX_NUMBER_OF_DSPs]   = {0};
fnrpcmem_alloc prpcmem_alloc[MAX_NUMBER_OF_DSPs]                      = {0};
fnrpcmem_free prpcmem_free[MAX_NUMBER_OF_DSPs]                        = {0};

static HMODULE g_libraryHandle[MAX_NUMBER_OF_DSPs] = {0};

static int get_fastrpc_dll_path(int q6Processor, char *outPath, int outPathSize) {
    SC_HANDLE scmHandle = NULL;
    SC_HANDLE scmService = NULL;
    LPQUERY_SERVICE_CONFIGA queryServiceConfig = NULL;
    DWORD bufferSize = 0;
    char systemRootPath[MAX_PATH] = {0};
    char *lastSep = NULL;
    char *systemRootToken = NULL;
    int result = -1;

    if (outPath == NULL || outPathSize <= 0) return -1;
    outPath[0] = '\0';

    scmHandle = OpenSCManager(NULL, NULL, STANDARD_RIGHTS_READ);
    if (scmHandle == NULL) return -1;

    if (q6Processor < (int)(sizeof(g_fastrpcServiceName) / sizeof(g_fastrpcServiceName[0]))) {
        scmService = OpenServiceA(scmHandle, g_fastrpcServiceName[q6Processor], SERVICE_QUERY_CONFIG);
        if (scmService == NULL) {
            scmService = OpenServiceA(scmHandle, g_fastrpcServiceNameNew[q6Processor], SERVICE_QUERY_CONFIG);
        }
    }

    if (scmService == NULL) {
        CloseServiceHandle(scmHandle);
        return -1;
    }

    QueryServiceConfigA(scmService, NULL, 0, &bufferSize);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        CloseServiceHandle(scmService);
        CloseServiceHandle(scmHandle);
        return -1;
    }

    queryServiceConfig = (LPQUERY_SERVICE_CONFIGA)LocalAlloc(LMEM_FIXED, bufferSize);
    if (!QueryServiceConfigA(scmService, queryServiceConfig, bufferSize, &bufferSize) || queryServiceConfig == NULL) {
        if (queryServiceConfig) LocalFree(queryServiceConfig);
        CloseServiceHandle(scmService);
        CloseServiceHandle(scmHandle);
        return -1;
    }

    if (GetEnvironmentVariableA("windir", systemRootPath, MAX_PATH) == 0) {
        LocalFree(queryServiceConfig);
        CloseServiceHandle(scmService);
        CloseServiceHandle(scmHandle);
        return -1;
    }

    lastSep = strrchr(queryServiceConfig->lpBinaryPathName, '\\');
    if (lastSep != NULL) {
        *lastSep = '\0';

        systemRootToken = strstr(queryServiceConfig->lpBinaryPathName, "\\SystemRoot");
        if (systemRootToken != NULL) {
            systemRootToken += strlen("\\SystemRoot");
            snprintf(outPath, outPathSize, "%s%s\\%s", systemRootPath, systemRootToken, FASTRPC_DLL_NAME);
            result = 0;
        } else {
            snprintf(outPath, outPathSize, "%s\\%s", queryServiceConfig->lpBinaryPathName, FASTRPC_DLL_NAME);
            result = 0;
        }
    }

    LocalFree(queryServiceConfig);
    CloseServiceHandle(scmService);
    CloseServiceHandle(scmHandle);
    return result;
}

int fastrpc_load_symbols(int q6Processor, const char *dllPath) {
    char resolvedPath[MAX_PATH] = {0};

    if (q6Processor < 0 || q6Processor >= MAX_NUMBER_OF_DSPs) return -1;
    if (g_libraryHandle[q6Processor] != NULL) return 0;

    if (dllPath == NULL || dllPath[0] == '\0') {
        if (get_fastrpc_dll_path(q6Processor, resolvedPath, MAX_PATH) != 0) {
            return -1;
        }
    } else {
        if (get_fastrpc_dll_path(q6Processor, resolvedPath, MAX_PATH) != 0) {
            strncpy_s(resolvedPath, MAX_PATH, dllPath, _TRUNCATE);
        }
    }

    g_libraryHandle[q6Processor] = LoadLibraryA(resolvedPath);
    if (g_libraryHandle[q6Processor] == NULL) return -1;

    premote_handle64_open[q6Processor]   = (fnremote_handle64_open)GetProcAddress(g_libraryHandle[q6Processor], "remote_handle64_open");
    premote_handle64_invoke[q6Processor] = (fnremote_handle64_invoke)GetProcAddress(g_libraryHandle[q6Processor], "remote_handle64_invoke");
    premote_handle64_close[q6Processor]  = (fnremote_handle64_close)GetProcAddress(g_libraryHandle[q6Processor], "remote_handle64_close");
    premote_session_control[q6Processor] = (fnremote_session_control)GetProcAddress(g_libraryHandle[q6Processor], "remote_session_control");
    prpcmem_alloc[q6Processor]           = (fnrpcmem_alloc)GetProcAddress(g_libraryHandle[q6Processor], "rpcmem_alloc");
    prpcmem_free[q6Processor]            = (fnrpcmem_free)GetProcAddress(g_libraryHandle[q6Processor], "rpcmem_free");

    if (!premote_handle64_open[q6Processor] || !premote_handle64_invoke[q6Processor] || !premote_handle64_close[q6Processor]) {
        FreeLibrary(g_libraryHandle[q6Processor]);
        g_libraryHandle[q6Processor] = NULL;
        return -1;
    }
    return 0;
}

void fastrpc_unload_symbols(int q6Processor) {
    if (q6Processor < 0 || q6Processor >= MAX_NUMBER_OF_DSPs) return;
    if (g_libraryHandle[q6Processor] != NULL) {
        FreeLibrary(g_libraryHandle[q6Processor]);
        g_libraryHandle[q6Processor] = NULL;
    }
    premote_handle64_open[q6Processor]   = NULL;
    premote_handle64_invoke[q6Processor] = NULL;
    premote_handle64_close[q6Processor]  = NULL;
    premote_session_control[q6Processor] = NULL;
    prpcmem_alloc[q6Processor]           = NULL;
    prpcmem_free[q6Processor]            = NULL;
}

void* rpcmem_alloc(int heapid, uint32_t flags, int size) {
    int i;
    for (i = 0; i < MAX_NUMBER_OF_DSPs; i++) {
        if (prpcmem_alloc[i] != NULL) {
            return prpcmem_alloc[i](heapid, (uint32_t)flags, size);
        }
    }
    return NULL;
}

void rpcmem_free(void* po) {
    int i;
    for (i = 0; i < MAX_NUMBER_OF_DSPs; i++) {
        if (prpcmem_free[i] != NULL) {
            prpcmem_free[i](po);
            return;
        }
    }
}

size_t strlcpy(char* dst, const char* src, size_t size) {
    size_t src_len = strlen(src);
    if (size > 0) {
        size_t copy_len = (src_len >= size) ? size - 1 : src_len;
        memcpy(dst, src, copy_len);
        dst[copy_len] = '\0';
    }
    return src_len;
}

size_t strlcat(char* dst, const char* src, size_t size) {
    size_t dst_len = strnlen(dst, size);
    size_t src_len = strlen(src);
    if (dst_len < size) {
        size_t remaining = size - dst_len - 1;
        size_t copy_len = (src_len < remaining) ? src_len : remaining;
        memcpy(dst + dst_len, src, copy_len);
        dst[dst_len + copy_len] = '\0';
    }
    return dst_len + src_len;
}
