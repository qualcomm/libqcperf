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
 * @file dsp_lib.h
 * @brief DSP library public API — types, return codes, and function declarations
 * @author Himanshu Keshri (hkeshri@qti.qualcomm.com)
 */

#ifndef QCOM_DSP_H_
#define QCOM_DSP_H_

#include "dspquery_stub.h"
#include "remote.h"
#include "rpcmem.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

struct sysmon_query_prof_data {
    float q6_utilization;    // avg effective q6 clock with respect to max q6 clock. (%)
    unsigned int q6_clock;   // avg q6 clock. (KHz)
    float reserved0;         // Reserved field
    float hvx_utilization;   // avg HVX utilization with respect to max q6 clock. (%)
    float hmx_utilization;   // avg HMX utilization with respect to max q6 clock. (%)
    float reserved1;         // Reserved field
    float reserved2;         // Reserved field
    float reserved3;         // Reserved field
    float reserved4;         // Reserved field
    float reserved5;         // Reserved field
    float reserved6;         // Reserved field
    float reserved7;         // Reserved field
    float reserved8;         // Reserved field
    float reserved9;         // Reserved field
};

enum DspDomainId {
    DSP_ADSP = ADSP_DOMAIN_ID,
    DSP_NPU0 = CDSP_DOMAIN_ID,
    DSP_MAX  = CDSP_DOMAIN_ID,
};

enum DspReturnCode {
    RETURN_CODE_DSP_LIB_SUCCESS = 0,
    RETURN_CODE_DSP_LIB_FAIL = 1,
    RETURN_CODE_DSP_SYSMON_QUERY_OPEN_FAILED,
    RETURN_CODE_DSP_SYSMON_QUERY_INIT_FAILED,
    RETURN_CODE_DSP_SYSMON_QUERY_RPC_MEM_ALLOC_FAILED,
    RETURN_CODE_DSP_SYSMON_QUERY_GET_PROF_DATA_FAILED,
    RETURN_CODE_DSP_SYSMON_QUERY_DEINIT_FAILED,
    RETURN_CODE_DSP_REMOTE_SESSION_CONTROL_SUCCESS,
    RETURN_CODE_DSP_REMOTE_SESSION_CONTROL_FAILED,
};

enum DspReturnCode qcom_dsp_init(enum DspDomainId);

struct sysmon_query_prof_data* qcom_dsp_get_prof_data(enum DspDomainId domain_id, int* no_metrics);

enum DspReturnCode qcom_dsp_deinit(enum DspDomainId);

#endif /* QCOM_DSP_H_ */
