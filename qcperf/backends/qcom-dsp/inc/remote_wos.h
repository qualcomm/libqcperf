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

#ifndef REMOTE_H
#define REMOTE_H

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __QAIC_REMOTE
#define __QAIC_REMOTE(ff) ff
#endif //__QAIC_REMOTE

#ifndef __QAIC_REMOTE_EXPORT
#define __QAIC_REMOTE_EXPORT
#endif //__QAIC_REMOTE_EXPORT

#ifndef __QAIC_REMOTE_ATTRIBUTE
#define __QAIC_REMOTE_ATTRIBUTE
#endif

#ifndef __QAIC_RETURN
#define __QAIC_RETURN
#endif

#ifndef __QAIC_IN
#define __QAIC_IN
#endif

#ifndef __QAIC_IN_CHAR
#define __QAIC_IN_CHAR
#endif

#ifndef __QAIC_IN_LEN
#define __QAIC_IN_LEN
#endif

#ifndef __QAIC_OUT
#define __QAIC_OUT
#endif

#ifndef __QAIC_INT64PTR
#define __QAIC_INT64PTR uint64_t
#endif

	typedef uint32_t remote_handle;
	typedef uint64_t remote_handle64; //! used by multi domain modules
	//! 64 bit handles are translated to 32 bit values
	//! by the transport layer

	/** Maximum length of URI for remote_handle_open() calls */
#define MAX_DOMAIN_URI_SIZE 12

	/** Domain type for multi-domain RPC calls */
	typedef struct domain_t {
		int id;                         /** Domain ID */
		char uri[MAX_DOMAIN_URI_SIZE];  /** URI for remote_handle_open */
	} domain;

#ifndef remote_buf_
#define remote_buf_
	typedef struct {
		uint64_t pv; // TODO: this variable type is void * in LA implementation
		size_t   nLen;
	} remote_buf;
#endif

	/** 64-bit Remote buffer parameter for RPC calls */
	typedef struct {
		/** Address of a remote buffer */
		uint64_t pv;
		/** Size of a remote buffer */
		int64_t nLen;
	} remote_buf64;

#ifndef remote_dma_handle_
#define remote_dma_handle_
	typedef struct {
		int32_t  fd;
		uint32_t offset;
		uint32_t len;
	} remote_dma_handle;
#endif

	/** 64-bit Remote DMA handle parameter for RPC calls */
	typedef struct {
		/** File descriptor of a remote buffer */
		int32_t fd;
		/** Offset of the file descriptor */
		uint32_t offset;
		/** Size of buffer */
		uint32_t len;
	} remote_dma_handle64;

#ifndef remote_arg_
#define remote_arg_
	typedef union {
		remote_buf        buf;
		remote_handle     h;
		remote_handle64   h64; //! used by multi domain modules
		remote_dma_handle dma;
	} remote_arg;
#endif
	/** 64-bit Remote Arg structure for RPC calls */
	typedef union {
		/** 64-bit remote buffer */
		remote_buf64 buf;
		/** non-domains remote handle */
		remote_handle h;
		/** multi-domains remote handle */
		remote_handle64 h64;
		/** 64-bit remote dma handle */
		remote_dma_handle64 dma;
	} remote_arg64;

	/** Job id of Async job queued to DSP */
	typedef uint64_t fastrpc_async_jobid;

	/* Casts a pointer to store to remote_buf.pv type */
#define PTR_TO_REMOTE_BUF_PV(ptr)               ((uint64_t) (uintptr_t) ptr)

	/* Casts remote_buf.pv type to store to pointer type */
#define REMOTE_BUF_PV_TO_PTR(pv)                ((void*) (uintptr_t) pv)

	/*Retrives method attribute from the scalars parameter*/
#define REMOTE_SCALARS_METHOD_ATTR(dwScalars)   (((dwScalars) >> 29) & 0x7)

	/*Retrives method index from the scalars parameter*/
#define REMOTE_SCALARS_METHOD(dwScalars)        (((dwScalars) >> 24) & 0x1f)

	/*Retrives number of input buffers from the scalars parameter*/
#define REMOTE_SCALARS_INBUFS(dwScalars)        (((dwScalars) >> 16) & 0x0ff)

	/*Retrives number of output buffers from the scalars parameter*/
#define REMOTE_SCALARS_OUTBUFS(dwScalars)       (((dwScalars) >> 8) & 0x0ff)

	/*Retrives number of input handles from the scalars parameter*/
#define REMOTE_SCALARS_INHANDLES(dwScalars)     (((dwScalars) >> 4) & 0x0f)

	/*Retrives number of output handles from the scalars parameter*/
#define REMOTE_SCALARS_OUTHANDLES(dwScalars)    ((dwScalars) & 0x0f)

#define REMOTE_SCALARS_MAKEX(nAttr,nMethod,nIn,nOut,noIn,noOut) \
	((((uint32_t)   (nAttr) &  0x7) << 29) | \
	 (((uint32_t) (nMethod) & 0x1f) << 24) | \
	 (((uint32_t)     (nIn) & 0xff) << 16) | \
	 (((uint32_t)    (nOut) & 0xff) <<  8) | \
	 (((uint32_t)    (noIn) & 0x0f) <<  4) | \
	 ((uint32_t)   (noOut) & 0x0f))

#define REMOTE_SCALARS_MAKE(nMethod,nIn,nOut)  REMOTE_SCALARS_MAKEX(0,nMethod,nIn,nOut,0,0)

#define REMOTE_SCALARS_LENGTH(sc) (REMOTE_SCALARS_INBUFS(sc) +\
		REMOTE_SCALARS_OUTBUFS(sc) +\
		REMOTE_SCALARS_INHANDLES(sc) +\
		REMOTE_SCALARS_OUTHANDLES(sc))

#define NUM_DOMAINS         550
#define NUM_SESSIONS        13

#define VMID_MASK           0xff000000
#define CLIENT_ID_MASK      0x00ff0000
#define DOMAIN_ID_MASK      0x0000ffff
#define DOMAIN_ID_SHIFT     0
#define CLIENT_ID_SHIFT     16
#define VMID_SHIFT          24
#define VM_HOST             0

#define SET_DOMAIN_MASK(domain, vmid, client) \
	(((domain << DOMAIN_ID_SHIFT) & DOMAIN_ID_MASK) | \
	 ((vmid << VMID_SHIFT) & VMID_MASK) | \
	 ((client << CLIENT_ID_SHIFT) & CLIENT_ID_MASK))

#ifndef DEFAULT_DOMAIN_ID
#define DEFAULT_DOMAIN_ID   0
#endif

#define ADSP_DOMAIN_ID      0
#define SDSP_DOMAIN_ID      1
#define MDSP_DOMAIN_ID      2
#define CDSP_DOMAIN_ID      3
#define CDSP1_DOMAIN_ID     4
#define GPDSP0_DOMAIN_ID    5
#define GPDSP1_DOMAIN_ID    6

	/** Defines the domain names for supported DSPs*/
#define ADSP_DOMAIN_NAME "adsp"
#define MDSP_DOMAIN_NAME "mdsp"
#define SDSP_DOMAIN_NAME "sdsp"
#define CDSP_DOMAIN_NAME "cdsp"
#define CDSP1_DOMAIN_NAME "cdsp1"

#define ADSP_DOMAIN         "&_dom=adsp"
#define CDSP_DOMAIN         "&_dom=cdsp"
#define CDSP1_DOMAIN        "&_dom=cdsp1"
#define GPDSP0_DOMAIN       "&_dom=gpdsp0"
#define GPDSP1_DOMAIN       "&_dom=gpdsp1"
#define MDSP_DOMAIN       "&_dom=mdsp"
#define SDSP_DOMAIN       "&_dom=sdsp"

#define NSP0_DOMAIN	    "&_dom=fastrpc_0100"
#define NSP1_DOMAIN	    "&_dom=fastrpc_0101"
#define NSP2_DOMAIN	    "&_dom=fastrpc_0102"
#define NSP3_DOMAIN	    "&_dom=fastrpc_0103"
#define HPASS0_DOMAIN	    "&_dom=fastrpc_0200"
#define HPASS1_DOMAIN	    "&_dom=fastrpc_0201"
#define HPASS2_DOMAIN	    "&_dom=fastrpc_0202"


#define SESSION_1           "&_session=1"
#define SESSION_2           "&_session=2"
#define SESSION_3           "&_session=3"
#define SESSION_4           "&_session=4"
#define SESSION_5           "&_session=5"
	/* All other values are reserved */

	/** Maximum length of URI for remote_handle_open() calls */
#define MAX_DOMAIN_URI_SIZE 12

	/* opens a remote_handle "name"
	 * returns 0 on success
	 */
	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(remote_handle_open)(const char* name, remote_handle *ph) __QAIC_REMOTE_ATTRIBUTE;
	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(remote_handle64_open)(const char* name, remote_handle64 *ph) __QAIC_REMOTE_ATTRIBUTE;

	/* invokes the remote handle
	 * see retrive macro's on dwScalars format
	 * pra, contains the arguments in the following order, inbufs, outbufs, inhandles, outhandles.
	 * implementors should ignore and pass values asis that the transport doesn't understand.
	 */
	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(remote_handle_invoke)(remote_handle h, uint32_t dwScalars, remote_arg *pra) __QAIC_REMOTE_ATTRIBUTE;
	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(remote_handle64_invoke)(remote_handle64 h, uint32_t dwScalars, remote_arg *pra) __QAIC_REMOTE_ATTRIBUTE;

	/* closes the remote handle
	*/
	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(remote_handle_close)(remote_handle h) __QAIC_REMOTE_ATTRIBUTE;
	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(remote_handle64_close)(remote_handle64 h) __QAIC_REMOTE_ATTRIBUTE;

	/**********************************************************DEVICE_DISCOVERY**********************************************/
#define MAX_DOMAIN_NAMELEN 32
	/** Types of domains (DSPs) */
	typedef enum {
		NSP=1,
		LPASS,
		GPDSP,
		MDSP,
		HPASS,
		MAX_DOM_TYPE,
	} fastrpc_domain_type;

	/** Structure describing each domain */
	typedef struct {
		/*
		 * Logical domain id to be used with fastrpc apis that take
		 * 'domain id' as input.
		 * This can change with every reboot device depending on the
		 * order of domain enumeration.
		 */
		int id;

		/* Name of domain. To be used with URI of remote module names */
		char name[MAX_DOMAIN_NAMELEN];

		/* Will be of "fastrpc_domain_type" */
		fastrpc_domain_type type;

		/* Status of NSP: 0 if domain is down, non-zero if up */
		int status;

		/* Card & SoC on which domain is present (reserved for future use) */
		uint32_t card;
		uint32_t soc_id;

	} fastrpc_domain;


	/* Position of domain type in flags */
#define DOMAINS_LIST_FLAGS_TYPE_POS 5

	/* Helper macro to set domain type in flags */
#define DOMAINS_LIST_FLAGS_SET_TYPE(flags, type) (flags | (type & ((1 << DOMAINS_LIST_FLAGS_TYPE_POS) - 1)))

	typedef struct {
		fastrpc_domain *domains;
		/*
		 * [in] Domain-info array pointer. Expected to be allocated by client.
		 * Will be populated by fastrpc.
		 */
		/**
		 * @param[in] : Size of the 'domains' array allocated by user.
		 * This has to be greater than or equal to the actual number of available
		 * domains. To query number of domains first, pass 0 in this field.
		 */
		int max_domains;

		/**
		 * @param[out] : This field will be populated with the total number
		 * of available domains. While reading the domains-info in the array,
		 * read only until 'num_domains' elements.
		 */
		int num_domains;

		/**
		 * @param[in] : Bit-mask for the type of request, to be populated by client.
		 * Bits 0-4 : Type of domains to be queried of 'fastrpc_domain_type'.
		 * Only domains of this type will be returned in the 'domains' array.
		 * To get list of all available domains, use 'ALL_DOMAINS' type.
		 * Other bits reserved for future use.
		 */
		uint64_t flags;
	} fastrpc_domains_info;


	typedef enum {
		/* Get domains info */
		FASTRPC_GET_DOMAINS = 0,
	} system_req_id;

	typedef struct system_req_payload {
		system_req_id id;
		union {
			fastrpc_domains_info sys;
		};
	} system_req_payload;

	/* New API can be used to get system info - like number of domains */
	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(remote_system_request)(system_req_payload *req) __QAIC_REMOTE_ATTRIBUTE;

	/********************************************************************************************************************************************************************************************/


	/* remote handle control interface
	*/
	/* request ID for fastrpc latency control */
	enum handle_control_req_id {
		DSPRPC_CONTROL_LATENCY  =   1, 
		DSPRPC_GET_DSP_INFO     =   2, 
		DSPRPC_CONTROL_WAKELOCK =   3, 
		DSPRPC_GET_DOMAIN   =   4,     
		/** Request ID to add a custom path to the hash table */
		DSPRPC_SET_PATH,
		/** Request ID to read a custom path to the hash table */
		DSPRPC_GET_PATH,
	};

	typedef enum remote_rpc_latency_flags {
		RPC_DISABLE_QOS = 0,   
		RPC_PM_QOS = 1,        
		RPC_ADAPTIVE_QOS = 2,  
		/**
		 * After sending invocation to DSP, CPU will enter polling mode instead of
		 * waiting for a glink response. This will boost fastrpc performance by
		 * reducing the CPU wakeup and scheduling times. Enabled only for sync RPC
		 * calls. Using this option also enables PM QoS with a latency of 100 us.
		 */
	} remote_rpc_control_latency_t;

#define STATUS_NOTIF_V2    3

	enum remote_dsp_attributes {
		DOMAIN_SUPPORT              = 0,          
		UNSIGNED_PD_SUPPORT         = 1,          
		HVX_SUPPORT_64B             = 2,          
		HVX_SUPPORT_128B            = 3,          
		VTCM_PAGE                   = 4,          
		VTCM_COUNT                  = 5,          
		ARCH_VER                    = 6,          
		HMX_SUPPORT_DEPTH           = 7,          
		HMX_SUPPORT_SPATIAL         = 8,          
		ASYNC_FASTRPC_SUPPORT       = 9,
		STATUS_NOTIFICATION_SUPPORT = 10,
		DSPSIGNAL_DSP_SUPPORT       = 130,
		FASTRPC_MAX_DSP_ATTRIBUTES  = 256, 
	};
	/**
	 * Structure used for request ID `DSPRPC_CONTROL_WAKELOCK`
	 * in remote handle control interface
	 **/
	struct remote_rpc_control_wakelock {
		/** enable control of wake lock */
		uint32_t enable;
	};

	/**
	 * Structure used for request IDs `DSPRPC_SET_PATH` and
	 * `DSPRPC_GET_PATH` in remote handle control interface.
	 */
	struct remote_control_custom_path {
		/** value size including NULL char */
		int32_t value_size;
		/** key used for storing the path */
		const char* path;
		/** value which will be used for file operations when the corresponding key is specified in the file URI */
		char* value;
	};

	enum fastrpc_map_flags {
		/**
		 * Map memory pages with RW- permission and CACHE WRITEBACK. Same remote 
		 * virtual address will be assigned for subsequent FastRPC calls.
		 */
		FASTRPC_MAP_STATIC = 0,
		FASTRPC_MAP_RESERVED = 1,
		/**
		 * Map memory pages with RW- permission and CACHE WRITEBACK. Mapping tagged with 
		 * a file descriptor. User is responsible for maintenance of CPU and DSP caches 
		 * for the buffer. Get virtual address of buffer on DSP using HAP_mmap_get() and 
		 * HAP_mmap_put() functions.
		 */
		FASTRPC_MAP_FD = 2,
		/**
		 * Mapping delayed until user calls HAP_mmap() and HAP_munmap()
		 * functions on DSP. User is responsible for maintenance of CPU and DSP
		 * caches for the buffer.
		 */
		FASTRPC_MAP_FD_DELAYED = 3,
		FASTRPC_MAP_MAX,
#if 0
		/** Reserved for compatibility **/
		FASTRPC_MAP_RESERVED_4,
		FASTRPC_MAP_RESERVED_5,
		FASTRPC_MAP_RESERVED_6,
		FASTRPC_MAP_RESERVED_7,
		FASTRPC_MAP_RESERVED_8,
		FASTRPC_MAP_RESERVED_9,
		FASTRPC_MAP_RESERVED_10,
		FASTRPC_MAP_RESERVED_11,
		FASTRPC_MAP_RESERVED_12,
		FASTRPC_MAP_RESERVED_13,
		FASTRPC_MAP_RESERVED_14,
		FASTRPC_MAP_RESERVED_15,

		/**
		 * This flag is used to skip CPU mapping,
		 * otherwise behaves similar to FASTRPC_MAP_FD_DELAYED flag.
		 */
		FASTRPC_MAP_FD_NOMAP,
#endif
	};

	typedef struct remote_dsp_capability {
		uint32_t domain;       
		uint32_t attribute_ID; 
		uint32_t capability;   
	}fastrpc_capability;

	typedef struct remote_rpc_get_domain {
		int domain;  
	} remote_rpc_get_domain_t;

	struct remote_rpc_control_latency {
		uint32_t enable;         // enable auto control of rpc latency
		uint32_t latency;        // latency: reserved
	};

	/**
	 * Request IDs for remote session control interface  
	 **/
	enum session_control_req_id {
		FASTRPC_RESERVED_1,                        /** Reserved */
		FASTRPC_THREAD_PARAMS,                     /** Set thread parameters like priority and stack size */
		DSPRPC_CONTROL_UNSIGNED_MODULE,            /** Handle the unsigned module offload request, to be called before remote_handle_open() */
		FASTRPC_RESERVED_2,                        /** Reserved */
		FASTRPC_RELATIVE_THREAD_PRIORITY,          /** To increase/decrease default thread priority */
		FASTRPC_RESERVED_3,                        /** Reserved */
		FASTRPC_REMOTE_PROCESS_KILL,               /** Kill remote process */
		FASTRPC_SESSION_CLOSE,                     /** Close all open handles of requested domain */
		FASTRPC_CONTROL_PD_DUMP,                   /** Enable PD dump feature */
		FASTRPC_REMOTE_PROCESS_EXCEPTION,          /** Trigger Exception in the remote process */
		FASTRPC_REMOTE_PROCESS_TYPE,               /** Query type of process defined by enum fastrpc_process_type */
		FASTRPC_REGISTER_STATUS_NOTIFICATIONS,     /** Enable DSP User process status notifications */
		FASTRPC_PD_INITMEM_SIZE,                    /** Set signed userpd initial memory size  **/
		/** Reserve new FastRPC session */
		FASTRPC_RESERVE_NEW_SESSION,

		/** Get effective domain ID of a FastRPC session */
		FASTRPC_GET_EFFECTIVE_DOMAIN_ID,

		/** Creates the URI needed to load a module in the DSP User PD */
		FASTRPC_GET_URI
	};

	/**
	 * Structure used for request ID `FASTRPC_THREAD_PARAMS`
	 * in remote session control interface
	 **/
	struct remote_rpc_thread_params {
		int domain;                     // Remote subsystem domain ID, pass -1 to set params for all domains
		int prio;                       // user thread priority (1 to 255), pass -1 to use default
		int stack_size;                 // user thread stack size, pass -1 to use default
	};

	/* request ID for fastrpc unsigned module */
#define DSPRPC_CONTROL_UNSIGNED_MODULE    (2)
	struct remote_rpc_control_unsigned_module {
		int domain;                      // Remote subsystem domain ID, -1 to set params for all domains
		int enable;                      // enable unsigned module loading
	};

	/**
	 * Structure used for request ID `FASTRPC_PD_INITMEM_SIZE`
	 * in remote session control interface
	 **/
	struct remote_rpc_pd_initmem_size {
		int domain;                      /** Remote subsystem domain ID, -1 to set params for all domains **/
		uint32_t pd_initmem_size;        /** Initial memory allocated for remote userpd; min: 3MB, max:200MB **/
	};
	/**
	 * Structure for remote_session_control,
	 * used with FASTRPC_RESERVE_SESSION request ID
	 * to reserve new fastrpc session of the user PD on the DSP.
	 * Default sesion is always 0 and remains available for any module opened without Session ID.
	 * New session reservation starts with session ID 1.
	 **/
	typedef struct remote_rpc_reserve_new_session {
		/** @param[in] : Domain name of DSP, on which session need to be reserved */
		char *domain_name;
		/** @param[in] : Domain name length, without NULL character */
		uint32_t domain_name_len;
		/** @param[in] : Session name of the reserved sesssion */
		char *session_name;
		/** @param[in] : Session name length, without NULL character */
		uint32_t session_name_len;
		/** @param[out] : Effective Domain ID is the identifier of the session.
		 * Effective Domain ID is the unique identifier representing the session(PD) on DSP.
		 * Effective Domain ID needs to be used in place of Domain ID when application has multiple sessions.
		 */
		uint32_t effective_domain_id;
		/** @param[out] : Session ID of the reserved session.
		 * An application can have multiple sessions(PDs) created on DSP.
		 * session_id 0 is the default session. Clients can reserve session starting from 1.
		 * Currently only 2 sessions are supported session_id 0 and session_id 1.
		 */
		uint32_t session_id;
	} remote_rpc_reserve_new_session_t;

	/**
	 * Structure for remote_session_control,
	 * used with FASTRPC_GET_EFFECTIVE_DOMAIN_ID request ID
	 * to get effective domain id of fastrpc session on the user PD of the DSP
	 **/
	typedef struct remote_rpc_effective_domain_id {
		/** @param[in] : Domain name of DSP */
		char *domain_name;
		/** @param[in] : Domain name length, without NULL character */
		uint32_t domain_name_len;
		/** @param[in] : Session ID of the reserved session. 0 can be used for Default session */
		uint32_t session_id;
		/** @param[out] : Effective Domain ID of session */
		uint32_t effective_domain_id;
	} remote_rpc_effective_domain_id_t;

	/**
	 * Structure for remote_session_control,
	 * used with FASTRPC_GET_URI request ID
	 * to get the URI needed to load the module in the fastrpc user PD on the DSP
	 **/
	typedef struct remote_rpc_get_uri {
		/** @param[in] : Domain name of DSP */
		char *domain_name;
		/** @param[in] : Domain name length, without NULL character */
		uint32_t domain_name_len;
		/** @param[in] : Session ID of the reserved session. 0 can be used for Default session */
		uint32_t session_id;
		/** @param[in] : URI of the module, found in the auto-generated header file*/
		char *module_uri ;
		/** @param[in] : Module URI length, without NULL character */
		uint32_t module_uri_len;
		/** @param[out] : URI containing module, domain and session.
		 * Memory for uri need to be pre-allocated with session_uri_len size.
		 * Typically session_uri_len is 30 characters more than Module URI length.
		 * If size of uri is beyond session_uri_len, remote_session_control fails with AEE_EBADSIZE
		 */
		char *uri ;
		/** @param[in] : URI length */
		uint32_t uri_len;
	} remote_rpc_get_uri_t;

	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(remote_handle_control)(uint32_t req, void* data, uint32_t datalen) __QAIC_REMOTE_ATTRIBUTE;
	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(remote_handle64_control)(remote_handle64 h, uint32_t req, void* data, uint32_t datalen) __QAIC_REMOTE_ATTRIBUTE;


	/* Set remote session parameters
	 *
	 * @param req, request ID
	 * @param data, address of structure with parameters
	 * @param datalen, length of data
	 * @retval, 0 on success
	 */
	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(remote_session_control)(uint32_t req, void *data, uint32_t datalen) __QAIC_REMOTE_ATTRIBUTE;

	/* map memory to the remote domain
	 *
	 * @param fd, fd assosciated with this memory
	 * @param flags, flags to be used for the mapping
	 * @param vaddrin, input address
	 * @param size, size of buffer
	 * @param vaddrout, output address
	 * @retval, 0 on success
	 */
	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(remote_mmap)(int fd, uint32_t flags, uint32_t vaddrin, int size, uint32_t* vaddrout) __QAIC_REMOTE_ATTRIBUTE;
	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(remote_mmap64)(int fd, uint32_t flags, uintptr_t vaddrin, int64_t size, uintptr_t *vaddrout) __QAIC_REMOTE_ATTRIBUTE;

	/* unmap memory from the remote domain
	 *
	 * @param vaddrout, remote address mapped
	 * @param size, size to unmap.  Unmapping a range partially may  not be supported.
	 * @retval, 0 on success, may fail if memory is still mapped
	 */
	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(remote_munmap)(uint32_t vaddrout, int size) __QAIC_REMOTE_ATTRIBUTE;
	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(remote_munmap64)(uintptr_t vaddrout, int64_t size) __QAIC_REMOTE_ATTRIBUTE;

	/* register dmahandle with remote domain
	 *
	 * @param fd, fd associated with pmem allocated memory.
	 * @param size, size associated with fd.
	 * @retval, 0 on success.
	 */
	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE (remote_register_dma_handle)(int fd, uint32_t len) __QAIC_REMOTE_ATTRIBUTE;
	__QAIC_REMOTE_EXPORT __QAIC_RETURN int __QAIC_REMOTE(remote_register_dma_handle_attr)(__QAIC_IN int fd,__QAIC_IN uint32_t len,__QAIC_IN uint32_t attr) __QAIC_REMOTE_ATTRIBUTE;

	/*
	 * Returns extended_domains_id from domain id and session id.
	 * @param domain, domain id
	 * @param session, session id  
	 */
	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(get_extended_domains_id)(int domain, int session) __QAIC_REMOTE_ATTRIBUTE;

	/* Register a file descriptor for a buffer.  This is only valid on
	 * pmem allocated memory.  Users of fastrpc should call this API if
	 * they do not use rpcmem_alloc APIs. This will enable sharing that 
	 * buffer to the dsp via the smmu. This should be defined as pragma weak.
	 *
	 * @param buf, virtual address of the buffer
	 * @param size, size to of the buffer
	 * @fd, the file descriptor, callers can use -1 to deregister.
	 * @attr, map buffer as coherent or non-coherent
	 */
	__QAIC_REMOTE_EXPORT void __QAIC_REMOTE(remote_register_buf)(void* buf, int size, int fd) __QAIC_REMOTE_ATTRIBUTE;
	__QAIC_REMOTE_EXPORT void __QAIC_REMOTE(remote_register_buf_attr)(void* buf, int size, int fd, int attr) __QAIC_REMOTE_ATTRIBUTE;

	/* Register a file descriptor for a buffer.  This is only valid on
	 * pmem allocated memory.  Users of fastrpc should call this API if
	 * they do not use rpcmem_alloc APIs. This will enable sharing that 
	 * buffer to the dsp via the smmu. This should be defined as pragma weak.
	 *
	 * @param ext_domain_id, extended domain id obtained using domain and session id  
	 * @param buf, virtual address of the buffer
	 * @param size, size to of the buffer
	 * @fd, the file descriptor, callers can use -1 to deregister.
	 * @attr, map buffer as coherent or non-coherent
	 */
	__QAIC_REMOTE_EXPORT void __QAIC_REMOTE(remote_register_buf_v2)(int ext_domain_id, void* buf, int size, int fd) __QAIC_REMOTE_ATTRIBUTE;
	__QAIC_REMOTE_EXPORT void __QAIC_REMOTE(remote_register_buf_attr_v2)(int ext_domain_id, void* buf, int size, int fd, int attr) __QAIC_REMOTE_ATTRIBUTE;


	/*
	 * Internal transport prefix
	 */
#define ITRANSPORT_PREFIX               "'\":;./\\"

	/*
	 * Set the mode of operation.
	 *
	 * Some versions of libadsprpc.so lack this function, so users should set
	 * this symbol as weak.
	 *
	 * #pragma weak  remote_set_mode
	 *
	 * @param mode, the mode
	 * @retval, 0 on success
	 */
	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(remote_set_mode)(uint32_t mode) __QAIC_REMOTE_ATTRIBUTE;

	/* Register a file descriptor.  This can be used when users do not have
	 * a mapping to pass to the RPC layer.  The generated address is a mapping
	 * with PROT_NONE, any access to this memory will fail, so it should only
	 * be used as an ID to identify this file descriptor to the RPC layer.
	 *
	 * To deregister use remote_register_buf(addr, size, -1).
	 *
	 *
	 * @param fd, the file descriptor.
	 * @param size, size to of the buffer
	 * @retval, (void*)-1 on failure, address on success.
	 *
	 */
	__QAIC_REMOTE_EXPORT void *__QAIC_REMOTE(remote_register_fd)(int fd, int size) __QAIC_REMOTE_ATTRIBUTE;

	/**
	 * Api to creates a mapping on remote process for an rpcmem buffer with file descriptor.
	 *
	 * @param extended_domain, extended domain id
	 * @param fd, file descriptor
	 * @param addr, apps virtual address of memory
	 * @param offset, buffer offset from start of the buffer
	 * @param length, buffer length
	 * @param flags, mapping control flags
	 * @param return, AEE_SUCCESS on success and error on failure
	 */
	int fastrpc_mmap(int extended_domain, int fd, void *addr, int offset, size_t length, enum fastrpc_map_flags flags);

	/**
	 * Api to remove a mapping associated with file descriptor. *
	 * @param extended_domain, extended domain id
	 * @param fd, file descriptor
	 * @param addr, apps virtual address of memory
	 * @param length, buffer length
	 */
	int fastrpc_munmap (int extended_domain, int fd, void *addr, size_t length);

	typedef uint64_t fastrpc_async_jobid;  /**< Job id filled in the remote_handle_invoke_async() call*/
	/**
	 * @enum fastrpc_async_notify_type
	 * @brief Types of Asynchronous complete notifications
	 */
	enum fastrpc_async_notify_type {
		FASTRPC_ASYNC_NO_SYNC = 0,   /**< synchronous call */
		FASTRPC_ASYNC_CALLBACK,      /**< asynchronous call with response with call back */
		FASTRPC_ASYNC_POLL,         /**< asynchronous call with polling */
		FASTRPC_ASYNC_TYPE_MAX,     /**< reserved */
	};
	/**
	 * @struct fastrpc_async_callback
	 * @brief call back function and context for asynchronous complete notification 
	 */
	typedef struct fastrpc_async_callback {
		void (*fn)(fastrpc_async_jobid jobid, void* context, int result);/**< call back function */
		void *context;                                                  /**< unique context filled by user*/
	}fastrpc_async_callback_t;

	/**
	 * @struct fastrpc_async_descriptor
	 * @brief  descriptor for defining the complete notification mechanism for given remote invocation call.
	 */
	typedef struct fastrpc_async_descriptor {
		enum fastrpc_async_notify_type type; /**< asynchronous notification type */
		fastrpc_async_jobid jobid;           /**< jobid returned in async remote invocation call */
		union {
			fastrpc_async_callback_t cb;      /**< call back function filled by user */
		};
	}fastrpc_async_descriptor_t;

	/**
	 * Makes remote call for given handle asynchronously
	 * This API used for non-domains use case
	 *
	 * @param[in]  h successful handle returned via #remote_handle_open()
	 * @param[in]  desc pointer to structure #fastrpc_async_descriptor_t
	 * @param[in]  pra pointer to union #remote_arg \n
	 * sequence of arguments in following order input buffers, output\n
	 * buffers, in handles and out handles. The number of each type of argument\n
	 * given in dwScalars below.
	 * @param[in]  dwScalars \n
	 * Sequence of the following integers related to invocation: The method ID\
	 * the number of input buffers in pra, the number of out buffers in pra,
	 * the number of in handles in pra and the number out handles in pra.
	 * @return Integer value. Zero for success and non-zero for failure. For failure, returns error code.\n
	 * Expected error codes due to incorrect input arguments are AEE_EINVHANDLE, AEE_EBADPARM. Other than these error codes,
	 * treated as returned from fastRPC framework issues.
	 */
	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(remote_handle_invoke_async)(remote_handle h, fastrpc_async_descriptor_t *desc, uint32_t dwScalars, remote_arg *pra) __QAIC_REMOTE_ATTRIBUTE;

	/**
	 * Makes remote call for given handle asynchronously
	 * This API used for domains use case
	 *
	 * @param[in]  h successful handle returned via #remote_handle_open()
	 * @param[in]  desc pointer to structure #fastrpc_async_descriptor_t
	 * @param[in]  pra pointer to union #remote_arg \n
	 * sequence of arguments in following order input buffers, output\n
	 * buffers, in handles and out handles. The number of each type of argument\n
	 * given in dwScalars below.
	 * @param[in]  dwScalars \n
	 * Sequence of the following integers related to invocation: The method ID\
	 * the number of input buffers in pra, the number of out buffers in pra,
	 * the number of in handles in pra and the number out handles in pra.
	 * @return Integer value. Zero for success and non-zero for failure. For failure, returns error code.\n
	 * Expected error codes due to incorrect input arguments are AEE_EINVHANDLE, AEE_EBADPARM. Other than these error codes,
	 * treated as returned from fastRPC framework issues.
	 */
	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(remote_handle64_invoke_async)(remote_handle64 h, fastrpc_async_descriptor_t *desc, uint32_t dwScalars, remote_arg *pra) __QAIC_REMOTE_ATTRIBUTE;

	/**
	 * Unloads the shared object for given handle
	 * This API used for non-domains use case
	 *
	 * @param[in]  h successful handle returned via #remote_handle_open()
	 * @return Integer value. Zero for success and non-zero for failure. For failure, returns error code.\n
	 * Expected error code due to incorrect input arguments is AEE_EINVHANDLE. Other than this error code,
	 * treated as returned from fastRPC framework issues.
	 */
	/**
	 * @struct remote_rpc_relative_thread_priority
	 * @brief Argument for setting the relative thread priority with request id:FASTRPC_RELATIVE_THREAD_PRIORITY
	 */
	struct remote_rpc_relative_thread_priority {
		int domain;						/**< Remote subsystem domain ID, pass -1 to update priority for all domains */
		int relative_thread_priority;	/**< Relative thread priority increased w.r.t to default priority. Negative value will increase priority. Positive value will decrease priority */
	};

	/**
	 * @struct remote_rpc_process_clean_params
	 * @brief Argument for cleaning remote session on DSP with request id:FASTRPC_REMOTE_PROCESS_KILL
	 */
	struct remote_rpc_process_clean_params {
		int domain;			/**< Remote subsystem domain ID, domain ID of process to recover */
	};

	/**
	 * @struct remote_rpc_session_close
	 * @brief Argument for closing all handles for all domains with request id:FASTRPC_SESSION_CLOSE
	 */
	struct remote_rpc_session_close {
		int domain;			/**< Remote subsystem domain ID, pass -1 to close all handles for all domains */
	};

	/**
	 * @struct remote_rpc_control_pd_dump
	 * @brief Argument for enabling PD dump on User PD of DSP with request id:DSPRPC_CONTROL_PD_DUMP
	 */
	struct remote_rpc_control_pd_dump {
		int domain;			/**< Remote subsystem domain ID, pass -1 to enable PD dump on all domains */
		int enable; 			/**<  1 to enable PD dump, 0 to disable PD dump */
	};

	/**
	 * @struct remote_rpc_process_clean_params
	 * @brief Argument to trigger exception in the User PD of DSP with request id:FASTRPC_REMOTE_PROCESS_EXCEPTION
	 */
	typedef struct remote_rpc_process_clean_params remote_rpc_process_exception;
	/**
	 *  remote_mmap API maps the pages on remote domain
	 *
	 * @param[in] fd  file descriptor for given buffer 
	 * @param[in] flags  one of enum from #remote_mem_map_flags.
	 * @param[in] vaddrin HLOS address for given buffer
	 * @param[in] size Length of the given buffer
	 * @param[out] vaddrout virtual address on remote domain
	 * @return Integer value. Zero for success and non-zero for failure. For failure, returns error code.\n
	 * Expected error code due to incorrect input arguments is AEE_EBADPARM.
	 * Other than these error codes, treated as returned from fastRPC framework issues.
	 * For example:
	 * @code
	 * int nErr = 0
	 * if (remote_mem_map) {
	 *    nErr = remote_mem_map(domain, fd, flags, buf, size, remoteVirtAddr);
	 * } else {
	 *    // symbol not found. Fall back to old API
	 *    nErr = remote_mmap64(fd, flags, buf, size, remoteVirtAddr);
	 * }
	 * return nErr;
	 @endcode
	 */
	/**
	 * Process types
	 * Return values for FASTRPC_REMOTE_PROCESS_TYPE control req ID for remote_handle_control
	 * Return values denote the type of process on remote subsystem
	 **/
	enum fastrpc_process_type {
		/** Signed PD running on the DSP */
		PROCESS_TYPE_SIGNED,

		/** Unsigned PD running on the DSP */
		PROCESS_TYPE_UNSIGNED,

	};

	/**
	 * Structure for remote_session_control,
	 * used with FASTRPC_REMOTE_PROCESS_TYPE request ID
	 * to query the type of PD running defined by enum fastrpc_process_type
	 * @param[in] : Domain of process
	 * @param[out] : Process_type belonging to enum fastrpc_process_type
	 */
	struct remote_process_type {
		/** @param[in] : Domain of process */
		int domain;
		/** @param[out] : Process_type belonging to enum fastrpc_process_type */
		int process_type;
	};

	/**
	 * DSP user PD status notification flags
	 * Status flags for the user PD on the DSP returned by the status notification function
	 *
	 **/
	typedef enum remote_rpc_status_flags {
		/** DSP user process is up */
		FASTRPC_USER_PD_UP,

		/** DSP user process exited */
		FASTRPC_USER_PD_EXIT,

		/** DSP user process forcefully killed. Happens when DSP resources needs to be freed. */
		FASTRPC_USER_PD_FORCE_KILL,

		/** Exception in the user process of DSP. */
		FASTRPC_USER_PD_EXCEPTION,

		/** Subsystem restart of the DSP, where user process is running. */
		FASTRPC_DSP_SSR,

	} remote_rpc_status_flags_t;

	/**
	 * fastrpc_notif_fn_t
	 * Notification call back function
	 *
	 * @param context, context used in the registration
	 * @param domain, domain of the user process
	 * @param session, session id of user process
	 * @param status, status of user process
	 * @retval, 0 on success
	 */
	typedef int (*fastrpc_notif_fn_t)(void *context, int domain, int session, remote_rpc_status_flags_t status);


	/**
	 * Structure for remote_session_control,
	 * used with FASTRPC_REGISTER_STATUS_NOTIFICATIONS request ID
	 * to receive status notifications of the user PD on the DSP
	 **/
	typedef struct remote_rpc_notif_register {
		/** @param[in] : Context of the client */
		void *context;
		/** @param[in] : DSP domain ADSP_DOMAIN_ID, SDSP_DOMAIN_ID, or CDSP_DOMAIN_ID */
		int domain;
		/** @param[in] : Notification function pointer */
		fastrpc_notif_fn_t notifier_fn;
	} remote_rpc_notif_register_t;

	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(remote_mmap)(int fd, uint32_t flags, uint32_t vaddrin, int size, uint32_t* vaddrout) __QAIC_REMOTE_ATTRIBUTE;
	/**
	 * @enum remote_mem_map_flags
	 * @brief Different set of flags for mapping the buffer on remote process domain\n
	 * Create static memory map on remote process with default cache configuration (writeback).
	 * Same remoteVirtAddr will be assigned on remote process when fastrpc call made with local virtual address.
	 * Map scope:
	 * Life time of this mapping is until user unmap using remote_mem_unmap or session close.
	 * No reference counts are used. Behavior of mapping multiple times without unmap is undefined.
	 * Cache maintenance:
	 * Driver clean caches when virtual address passed through RPC calls defined in IDL as a pointer.
	 * User is responsible for cleaning cache when remoteVirtAddr shared to DSP and accessed out of fastrpc method invocations on DSP.
	 * recommended usage:
	 * Map buffers which are reused for long time or until session close. This helps to reduce fastrpc latency.
	 * Memory shared with remote process and accessed only by DSP.
	 */

	enum remote_mem_map_flags {
		REMOTE_MAP_MEM_STATIC      = 0,
		REMOTE_MAP_MAX_FLAG        = REMOTE_MAP_MEM_STATIC + 1,
	};

	/**
	 * Map memory to the remote process on a selected DSP domain
	 * @param[in] domain DSP domain ID. Use -1 for using default domain\n
	 *          Default domain is selected based on library lib(a/m/s/c)dsprpc.so library linked to application.
	 * @param[in] fd file descriptor of memory
	 * @param[in] flags one of enum from #remote_mem_map_flags
	 * @param[in] virtAddr virtual address of buffer
	 * @param[in] size buffer length
	 * @param[out] remoteVirtAddr remote process virtual address
	 * @return Integer value. Zero for success and non-zero for failure. For failure, returns error code.\n
	 * Expected error code due to incorrect input arguments is AEE_EBADPARM
	 * Other than these error codes, treated as returned from fastRPC framework issues.
	 */
	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(remote_mem_map)(int domain, int fd, int flags, uint64_t virtAddr, size_t size, uint64_t* remoteVirtAddr) __QAIC_REMOTE_ATTRIBUTE;

	/**
	 * Unmap memory to the remote process on a selected DSP domain
	 * @param[in] domain DSP domain ID. Use -1 for using default domain. Get domain from multi-domain handle if required.
	 * @param[in] remoteVirtAddr virtual address returned via #remote_mem_map()
	 * @param[in] size buffer length
	 * @return Integer value. Zero for success and non-zero for failure. For failure, returns error code.\n
	 * Expected error code due to incorrect input arguments is AEE_EBADPARM
	 * Other than this error code, treated as returned from fastRPC framework issues.
	 */
	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(remote_mem_unmap)(int domain, uint64_t remoteVirtAddr, size_t size) __QAIC_REMOTE_ATTRIBUTE;

	/**
	 * @enum remote_buf_attributes
	 * @brief Types of buffer attributes
	 */
	enum remote_buf_attributes {
		FASTRPC_ATTR_NON_COHERENT = 2,    /**< Attribute to map a buffer as dma non-coherent Driver perform cache maintenance. */
		FASTRPC_ATTR_COHERENT     = 4,    /**< Attribute to map a buffer as dma coherent Driver skips cache maintenenace. 
						    It will be ignored if a device is marked as dma-coherent in device tree. */
		FASTRPC_ATTR_KEEP_MAP     = 8,    /**< Attribute to keep the buffer persistant until unmap is called explicitly */
		FASTRPC_ATTR_NOMAP        = 16,   /**< Attribute for secure buffers to skip  smmu mapping in fastrpc driver */
		FASTRPC_ATTR_FORCE_NOFLUSH = 32,   /**< Attribute to map buffer such that flush by driver is skipped for that particular buffer.
						     Client has to perform cache maintenance. */
		FASTRPC_ATTR_FORCE_NOINVALIDATE = 64,   /**< Attribute to map buffer such that invalidate by driver is skipped for that particular buffer.
							  Client has to perform cache maintenance. */
		FASTRPC_ATTR_TRY_MAP_STATIC   = 128,    /**< Attribute for persistent mapping a buffer to remote DSP
							  process during buffer registration with the FastRPC driver.
							  This buffer will be automatically mapped during session
							  open and unmapped either at session close or buffer
							  unregistration. The FastRPC library treis to map buffers
							  and ignores error in case of failure. Pre-mapping a buffer    reduces the FastRPC latency. This flag is recommended only
							  for buffers used with latency-critical FastRPC calls. */
	};
	/** Register a file descriptor.  This can be used when users do not have
	 * a mapping to pass to the RPC layer.  The generated address is a mapping
	 * with PROT_NONE, any access to this memory will fail, so it should only
	 * be used as an ID to identify this file descriptor to the RPC layer.
	 *
	 * To deregister use remote_register_buf(addr, size, -1).
	 *
	 * `pragma weak  remote_register_fd`\n
	 * @param[in] fd is the file descriptor.
	 * @param[in] size length the buffer
	 * @return (void*)-1 on failure, address on success.\n
	 * All error codes treated as returned from fastRPC framework issues.
	 */
	__QAIC_REMOTE_EXPORT void *__QAIC_REMOTE(remote_register_fd)(int fd, int size) __QAIC_REMOTE_ATTRIBUTE;

	/** Get status of Async job.  This can be used to query the status of a Async job
	 *
	 * @param[in] jobid  returned during Async job submission via #remote_handle_invoke_async()
	 * @param[in] timeout_us in micro seconds \n
	 *                    timeout = 0, returns immediately with status/result\n
	 *                    timeout > 0, waits for specified time and then returns with status/result \n
	 *                    timeout < 0. waits indefinetely until job completes \n
	 * @param[out] result ouptput pointer \n
	 *                0 on success\n
	 *                error code on failure\n
	 * @return 0 on job completion\n
	 *          if job status is pending, it is not returned from DSP\n
	 * Expected error code due to incorrect input arguments is AEE_EBADPARM. Other than this error code
	 * treated as returned from fastRPC framework issues.
	 */
	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(fastrpc_async_get_status)(fastrpc_async_jobid jobid, int timeout_us, int *result);

	/** Release Async job.  Release async job after receiving status either through callback/poll
	 *
	 * @param[in] jobid returned during Async job submission via #remote_handle_invoke_async()
	 * @return  Zero for success. Non-zero for failure.\n
	 *          AEE_EBUSY, if job status is pending and is not returned from DSP\n
	 *          AEE_EBADPARM, if job id is invalid \n
	 */
	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(fastrpc_release_async_job)(fastrpc_async_jobid jobid);

	/**
	 * By default, Driver runs Parallel mode
	 */
#define REMOTE_MODE_PARALLEL  0

	/**
	 * When operating in SERIAL mode the driver will invalidate output buffers
	 * before calling into the dsp.  This mode should be used when output
	 * buffers have been written to somewhere besides the aDSP.
	 */
#define REMOTE_MODE_SERIAL    1

	/**
	 * Internal transport prefix
	 */
#define ITRANSPORT_PREFIX "'\":;./\\"

	/** remote_set_mode API will be  deprecated in near future. It is discouraged to use this API.\n
	 * Set the mode of operation.\n
	 * This is the default mode for the driver.  While the driver is in parallel
	 * mode it will try to invalidate output buffers after it transfers control
	 * to the dsp.  This allows the invalidate operations to overlap with the
	 * dsp processing the call.  This mode should be used when output buffers
	 * are only read on the application processor and only written on the aDSP.
	 * Some versions of libadsprpc.so lack this function, so users should set
	 * this symbol as weak.\n
	 * `pragma weak  remote_set_mode`
	 * @param[in] mode serial/parallel operation
	 * @return Integer value. Zero for success and non-zero for failure. For failure, returns error code.\n
	 * All error codes, treated as returned from fastRPC framework issues.
	 */
	__QAIC_REMOTE_EXPORT int __QAIC_REMOTE(remote_set_mode)(uint32_t mode) __QAIC_REMOTE_ATTRIBUTE;
	/**
	 * remote_register_fd2
	 * Register a file descriptor.
	 * This can be used when users do not have a mapping to pass to
	 * the RPC layer. The generated address is a mapping with PROT_NONE,
	 * any access to this memory will fail, so it should only be used
	 * as an ID to identify this file descriptor to the RPC layer.
	 *
	 * To deregister use remote_register_buf(addr, size, -1).
	 *
	 * #pragma weak  remote_register_fd2
	 *
	 * @param fd, the file descriptor.
	 * @param size, size to of the buffer
	 * @retval, (void*)-1 on failure, address on success.
	 */
	__QAIC_REMOTE_EXPORT __QAIC_RETURN void *__QAIC_REMOTE(remote_register_fd2)(__QAIC_IN int fd,__QAIC_IN size_t size) __QAIC_REMOTE_ATTRIBUTE;



#ifdef __cplusplus
}
#endif

#endif /* REMOTE_H */
