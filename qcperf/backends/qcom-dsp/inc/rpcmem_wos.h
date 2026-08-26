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

#ifndef RPCMEM_H
#define RPCMEM_H

#include <stdint.h>

#define RPCMEM_HEAP_DEFAULT 0x80000000
#define RPCMEM_HEAP_NOREG   0x40000000
#define RPCMEM_HEAP_UNCACHED 0x20000000

#ifdef __cplusplus
extern "C" {
#endif

/**
 * call once to initialize the library
 */
void rpcmem_init(void);
/**
 * call once for cleanup
 */
void rpcmem_deinit(void);

/**
 * allocate via ION a buffer of size
 * @heapid, the heap id to use
 * @flags, ion flags to use to allocate, this is ignored on ics
 * @size, the buffer size to allocate
 * @retval, 0 on failure, pointer to buffer on success
 *
 * Pass RPCMEM_HEAP_DEFAULT for flags if unsure on what heapid
 * and flags to pass. RPCMem internally takes care of picking
 * the right heap id and flags value. For example:
 * buf = rpcmem_alloc(0, RPCMEM_HEAP_DEFAULT, size);
 */

void* rpcmem_alloc(int heapid, uint32_t flags, int size);

/**
 * free buffer, ignores invalid buffers
 */
void rpcmem_free(void* po);

/**
 * returns associated fd
 */
int rpcmem_to_fd(void* po);

#ifdef __cplusplus
}
#endif

#endif //RPCMEM_H
