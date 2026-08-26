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

#ifndef __RPC_SYMBOLS_H__
#define __RPC_SYMBOLS_H__

#include "remote_wos.h"
#include <stdint.h>

#define MAX_NUMBER_OF_DSPs 12

typedef int(__cdecl *fnremote_handle64_open)(const char *name, remote_handle64 *ph);
typedef int(__cdecl *fnremote_handle64_close)(remote_handle64 h);
typedef int(__cdecl *fnremote_handle64_invoke)(remote_handle64 h, uint32_t dwScalars, remote_arg *pra);
typedef int(__cdecl *fnremote_session_control)(uint32_t req, void *data, uint32_t datalen);
typedef void *(__cdecl *fnrpcmem_alloc)(int heapid, uint32_t flags, int size);
typedef void(__cdecl *fnrpcmem_free)(void *po);

extern fnremote_handle64_open premote_handle64_open[MAX_NUMBER_OF_DSPs];
extern fnremote_handle64_close premote_handle64_close[MAX_NUMBER_OF_DSPs];
extern fnremote_handle64_invoke premote_handle64_invoke[MAX_NUMBER_OF_DSPs];
extern fnremote_session_control premote_session_control[MAX_NUMBER_OF_DSPs];
extern fnrpcmem_alloc prpcmem_alloc[MAX_NUMBER_OF_DSPs];
extern fnrpcmem_free prpcmem_free[MAX_NUMBER_OF_DSPs];

int fastrpc_load_symbols(int q6Processor, const char *dllPath);
void fastrpc_unload_symbols(int q6Processor);

#endif /* __RPC_SYMBOLS_H__ */
