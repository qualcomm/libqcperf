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
 * @file dsp_lib.c
 * @brief DSP library implementation
 * @author Himanshu Keshri (hkeshri@qti.qualcomm.com)
 */

#include "qcom_dsp.h"

void get_full_uri_info(const char* uri, char* full_uri, int size, enum DspDomainId domain_id);
enum DspReturnCode set_remote_process_domain(enum fastrpc_process_type, enum DspDomainId);

static remote_handle64 h[DSP_MAX]                                         = {0};
static struct sysmon_query_prof_data* sysmon_query_prof_data_ptr[DSP_MAX] = {0};

void get_full_uri_info(const char* uri, char* full_uri, int size, enum DspDomainId domain_id) {
    memset(full_uri, 0, (size_t)size);
    strlcpy(full_uri, uri, (size_t)size);
    if (domain_id == DSP_ADSP) {
        strlcat(full_uri, ADSP_DOMAIN, (size_t)size);
    } else if (domain_id == DSP_NPU0) {
        strlcat(full_uri, CDSP_DOMAIN, (size_t)size);
    } else {
        memset(full_uri, 0, (size_t)size);
    }
}

enum DspReturnCode set_remote_process_domain(enum fastrpc_process_type process_type, enum DspDomainId domain_id) {
    int fastrpc_err = 0;
    struct remote_process_type data =  {0};
    enum DspReturnCode return_code = RETURN_CODE_DSP_REMOTE_SESSION_CONTROL_SUCCESS;
    if (domain_id == DSP_NPU0) {
        data.process_type = process_type;
        data.domain = CDSP_DOMAIN_ID;
        fastrpc_err = remote_session_control(DSPRPC_CONTROL_UNSIGNED_MODULE, (void*)&data, sizeof(data));
        if(fastrpc_err != 0) {
            return_code = RETURN_CODE_DSP_REMOTE_SESSION_CONTROL_FAILED;
        }
    }
    return return_code;
}

enum DspReturnCode qcom_dsp_init(enum DspDomainId domain_id) {
    enum DspReturnCode return_code = RETURN_CODE_DSP_LIB_FAIL;
    enum DspReturnCode return_remote_code = RETURN_CODE_DSP_REMOTE_SESSION_CONTROL_SUCCESS;
    int sysmon_query_return_code      = -1;
    unsigned int npu_id               = 0;
    char full_uri[256]                = {0};
    
    get_full_uri_info(sysmonquery_URI, full_uri, sizeof(full_uri), domain_id);
    if (domain_id == DSP_NPU0) {
        npu_id = 0;
        return_remote_code = set_remote_process_domain(PROCESS_TYPE_SIGNED, DSP_NPU0);
    }
    
    if(return_remote_code == RETURN_CODE_DSP_REMOTE_SESSION_CONTROL_SUCCESS) {
        sysmon_query_return_code = sysmonquery_open(full_uri, &h[domain_id]);
        if (sysmon_query_return_code != 0) {
            return_code = RETURN_CODE_DSP_SYSMON_QUERY_OPEN_FAILED;
        } else {
            sysmon_query_return_code = sysmonquery_init(h[domain_id], npu_id);
            if (sysmon_query_return_code != 0) {
                return_code = RETURN_CODE_DSP_SYSMON_QUERY_INIT_FAILED;
            } else {
                sysmon_query_prof_data_ptr[domain_id] =
                    (struct sysmon_query_prof_data*)rpcmem_alloc(RPCMEM_DEFAULT_HEAP, (RPCMEM_DEFAULT_FLAGS | RPCMEM_HEAP_NONCOHERENT), sizeof(struct sysmon_query_prof_data));
                if (sysmon_query_prof_data_ptr[domain_id] == NULL) {
                    return_code = RETURN_CODE_DSP_SYSMON_QUERY_RPC_MEM_ALLOC_FAILED;
                } else {
                    memset(sysmon_query_prof_data_ptr[domain_id], 0, sizeof(struct sysmon_query_prof_data));
                    return_code = RETURN_CODE_DSP_LIB_SUCCESS;
                }
            }
        }
    }

    if (return_code != RETURN_CODE_DSP_LIB_SUCCESS) {
        qcom_dsp_deinit(domain_id);
    }
    return return_code;
}

struct sysmon_query_prof_data* qcom_dsp_get_prof_data(enum DspDomainId domain_id, int* no_metrics) {
    unsigned int npu_id                       = 0;
    struct sysmon_query_prof_data* result_ptr = NULL;
    if (no_metrics == NULL) {
        result_ptr = NULL;
    } else {
        if (domain_id == CDSP_DOMAIN_ID) {
            npu_id = 0;
        }
        int result = sysmonquery_get_profdata(h[domain_id], (unsigned char*)sysmon_query_prof_data_ptr[domain_id], sizeof(struct sysmon_query_prof_data), no_metrics, npu_id);
        if (result == 0) {
            result_ptr = sysmon_query_prof_data_ptr[domain_id];
        } else {
            result_ptr = NULL;
        }
    }
    return result_ptr;
}

enum DspReturnCode qcom_dsp_deinit(enum DspDomainId domain_id) {
    enum DspReturnCode return_code = RETURN_CODE_DSP_LIB_FAIL;
    unsigned int npu_id               = 0;
    int sysmon_query_return_code      = -1;
    if (domain_id == CDSP_DOMAIN_ID) {
        npu_id = 0;
    }

    sysmon_query_return_code = sysmonquery_deinit(h[domain_id], npu_id);
    if (sysmon_query_return_code != 0) {
        return_code = RETURN_CODE_DSP_SYSMON_QUERY_DEINIT_FAILED;
    } else {
        return_code = RETURN_CODE_DSP_LIB_SUCCESS;
    }
    if (sysmon_query_prof_data_ptr[domain_id] != NULL) {
        rpcmem_free(sysmon_query_prof_data_ptr[domain_id]);
    }

    if (h[domain_id] != 0) {
        sysmonquery_close(h[domain_id]);
    }
    return return_code;
}
