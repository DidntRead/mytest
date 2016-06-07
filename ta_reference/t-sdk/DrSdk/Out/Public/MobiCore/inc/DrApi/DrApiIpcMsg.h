/*
 * Copyright (c) 2013-2014 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */
#ifndef __DRAPI_IPCMSG_H__
#define __DRAPI_IPCMSG_H__

#include "DrApiCommon.h"

#ifdef __cplusplus
extern "C" {
#endif


//------------------------------------------------------------------------------
/** Possible property ids. */
typedef enum {
    PROPERTY_NONE     = 0,
    PROPERTY_UUID     = 1,
    PROPERTY_SEED     = 2,
    PROPERTY_GP_UUID  = 3,
    PROPERTY_RESERVED = 4,
} property_t;

//------------------------------------------------------------------------------
/** Possible message types/event types of the system. */
typedef enum {
    MSG_NULL = 0,  // Used for initializing state machines
    MSG_RQ                          = 1,
        // Client Request, blocks until MSG_RS is received
        // Client -> Server
    MSG_RS                          = 2,
        // Driver Response, answer to MSG_RQ
        // Server -> Client
    MSG_RD                          = 3,
        // Driver becomes ready
        // Server -> IPCH
    MSG_NOT                         = 4,
        // Notification to NWd for a session, send-only message with no
        // response
        // client/server -> IPCH;
    MSG_CLOSE_TRUSTLET              = 5,
        // Close Trustlet, must be answered by MSG_CLOSE_TRUSTLET_ACK
        // MSH -> IPCH, IPCH -> Server
    MSG_CLOSE_TRUSTLET_ACK          = 6,
        // Close Trustlet Ack, in response to MSG_CLOSE_TRUSTLET
        // Server -> IPCH
    MSG_MAP                         = 7,
        // Map Client into Server, send-only message with no reponse
        //Server -> IPCH;
    MSG_ERR_NOT                     = 8,
        // Error Notification
        // EXCH/SIQH -> IPCH
    MSG_CLOSE_DRIVER                = 9,
        // Close Driver, must be answered with MSG_CLOSE_DRIVER_ACK
        // MSH -> IPCH, IPCH -> Driver/Server
    MSG_CLOSE_DRIVER_ACK            = 10,
        // Close Driver Ack, response to MSG_CLOSE_DRIVER
        // Driver/Server -> IPCH, IPCH -> MSH
    MSG_GET_DRIVER_VERSION          = 11,
        // Get driver version, used for response also
        // Client <-> IPCH
    MSG_GET_DRAPI_VERSION           = 12,
        // Get DrApi version, used for response also
        // Driver <-> IPCH */
    MSG_SET_NOTIFICATION_HANDLER    = 13,
        // Set (change) the SIQ handler thread, used for response also
        // Driver <-> IPCH
    MSG_GET_REGISTRY_ENTRY          = 14,
        // Get registry entry, available only if MC_FEATURE_DEBUG_SUPPORT is
        //   set, used for response also
        // Driver <-> IPCH
    MSG_DRV_NOT                     = 15,
        // Notification to a Trustlet, looks like a notification from NWd for
        //   the Trustlet, send-only message with no response
        // Driver -> Trustlet
    MSG_SET_FASTCALL_HANDLER        = 16,
        // install a FastCall handler, used for response also
        // Driver <-> IPCH
    MSG_GET_CLIENT_ROOT_AND_SP_ID   = 17,
        // get Root DI and SP ID, used for response also
        // Driver <-> IPCH
    MSG_SUSPEND                     = 18,
        // Suspend, requires MSG_SUSPEND_ACK as response
        // MSH -> IPCH, IPCH -> driver
    MSG_SUSPEND_ACK                 = 19,
        // Suspend Ack, response to MSG_SUSPEND
        // driver -> IPCH, IPCH -> MSH
    MSG_RESUME                      = 20,
        // resume, , requires MSG_RESUME_ACK as response
        // MSH -> IPCH, IPCH -> driver
    MSG_RESUME_ACK                  = 21,
        // resume, , response to MSG_RESUME
        // driver ->  IPCH, IPCH -> MSH
    MSG_GET_ENDORSEMENT_SO          = 22,
        // get SO from RTM for the Endorsement functionality
        // Driver <-> IPCH
    MSG_GET_SERVICE_VERSION         = 23,
        // get version of service (TA)
        // Driver <-> IPCH
    MSG_ERROR                       = 24,
        // IPCH returns error to Driver
        // IPCH <-> DRIVER
    MSG_CALL_FASTCALL               = 25,
        // Call fastcall from driver
        // DRIVER -> IPCH -> MTK -> FASTCALL -> return
    MSG_GET_PROPERTY                = 26,
        // get properties of a TA/driver
        // Driver <-> IPCH
    MSG_RQ_EX                       = 27,
        // Client Request, blocks until MSG_RS or MSG_RQ_ERROR is received
        // Client -> Server
    MSG_RQ_ERROR                    = 28,
        // Sent to Client to raise an error in IPC communication level
        // Server -> Client
    MSG_INIT_HEAP                   = 29,
        // init heap of a TA/driver
        // client/server -> IPCH;
    MSG_EXTEND_HEAP                 = 30,
        // extend heap of a TA/driver
        // client/server -> IPCH;
    MSG_MAP_TASK_BUFFER             = 31,
        // Map client task buffer into Server,
        // Server -> IPCH;
    MSG_MAP_PHYSICAL_BUFFER         = 32,
        // Map physical buffer into Server
        // Server -> IPCH;
    MSG_UNMAP_BUFFER                = 33,
        // Unmap buffer from Server
        // Server -> IPCH;
    MSG_GET_AREA_TYPE               = 34,
        // get area attributes
        // Server -> IPCH;
    MSG_UNMAP_TASK_BUFFERS          = 35,
        // Unmap entire client (all its buffers) from Server
        // Server -> IPCH;
    MSG_RESERVED_1                  = 36,
    MSG_RESERVED_2                  = 37,
    MSG_RESERVED_3                  = 38,
    MSG_RESERVED_4                  = 39,
        // Reserved values, do not use!
    MSG_ERR_TEARDOWN                = 40,
        // Error Notification
        // EXCH/SIQH -> IPCH
} message_t;


//------------------------------------------------------------------------------

typedef struct {
    uint32_t        functionId;  /* Function identifier. */
    uint32_t        retVal;
} reqPayloadHeader_t, *reqPayloadHeader_ptr;

//------------------------------------------------------------------------------
/** Waits with infinite timeout for IPC.
 *
 * @param ipcPartner IPC partner to signal.
 * @param pMr0 IPC register 0.
 * @param pMr1 IPC register 1.
 * @param pMr2 IPC register 2.
 *
 * @returns DRAPI_OK in case of success
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
drApiResult_t drApiIpcWaitForMessage(
    threadid_t        *pIpcPartner,
    uint32_t          *pMr0,
    uint32_t          *pMr1,
    uint32_t          *pMr2
);

//------------------------------------------------------------------------------
/**
 * Send ready message or answer to IPCH and waits for a client request
 *
 * @param ipcPeer Destination to send message to.
 * @param ipcMsg IPC message.
 * @param ipcData Additional IPC data.
 *
 * @returns DRAPI_OK in case of success
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
drApiResult_t drApiIpcCallToIPCH(
    threadid_t       *pIpcPeer,
    message_t        *pIpcMsg,
    uint32_t         *pIpcData
);

//------------------------------------------------------------------------------
/** Sets signal.
 * The signal (SIGNAL) is used by a thread to inform another thread about an event.
 * The signal operation is asynchronous, which means that the operation will return
 * immediately without blocking the user. Function uses auto-clear signals, meaning that
 * the signal is cleared automatically when the receiver receives it.
 *
 * It is up to the destination of the signal to pick up and process the information.
 *
 * @param receiver Thread to set the signal for.
 *
 * @returns DRAPI_OK in case of success
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
drApiResult_t drApiIpcSignal(
    const threadid_t  receiver
);

//------------------------------------------------------------------------------
/** Signal wait operation.
 * A thread uses the sigWait operation to check if a signal has occurred. If no signal is
 * pending the thread will block until a signal arrives.
 *
 * @returns DRAPI_OK in case of success
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
drApiResult_t drApiIpcSigWait( void );

//------------------------------------------------------------------------------
/** Notify NWd driver
 *
 * @returns DRAPI_OK in case of success
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
drApiResult_t drApiNotify( void );


//------------------------------------------------------------------------------
/**
 * Makes control syscall with given parameters
 *
 * @param controlid control id
 * @param param1    parameter 1
 * @param param2    parameter 2
 * @param param3    parameter 3
 * @param param4    parameter 4
 * @param *data     set by control syscall
 *
 * @returns DRAPI_OK in case of success
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
drApiResult_t drApiSyscallControl(
    uint32_t controlid,
    uint32_t param1,
    uint32_t param2,
    uint32_t param3,
    uint32_t param4,
    uint32_t *data
);

//------------------------------------------------------------------------------
/**
 * Makes platform syscall with given parameters
 *
 * @param controlid control id
 * @param param1    parameter 1
 * @param param2    parameter 2
 * @param param3    parameter 3
 * @param param4    parameter 4
 * @param *data     set by platform syscall
 *
 * @returns DRAPI_OK in case of success
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
drApiResult_t drApiPlatformControl(
    uint32_t controlid,
    uint32_t param1,
    uint32_t param2,
    uint32_t param3,
    uint32_t param4,
    uint64_t *data
);


//------------------------------------------------------------------------------
/** Reads OEM data starting from given offset
 *
 * @param offset  data offset
 * @param data    set by control syscall
 *
 * @returns DRAPI_OK in case of success
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
drApiResult_t drApiReadOemData(
        const uint32_t  offset,
        uint32_t        *data
);

//------------------------------------------------------------------------------
/** Sends notification to client
 *
 * @param client Client's thread id
 * *
 * @return MTK return code
 */
drApiResult_t drApiNotifyClient(
    const threadid_t  client
);

//------------------------------------------------------------------------------
/** Returns ID of Root and Service Provider ID of the specified client
 *
 * @param rootId ID of Root
 * @param spId Service Provider ID
 * @param client Thread ID
 *
 * @returns DRAPI_OK in case of success
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
drApiResult_t drApiGetClientRootAndSpId(
    uint32_t        *rootId,
    uint32_t        *spId,
    const threadid_t  client
);


//------------------------------------------------------------------------------
/**
 * Handles unknown messages
 * This function has to be called by Driver if it receives a message it doesn’t recognize
 *
 * @param ipcPeer Sender of message.
 * @param ipcMsg IPC message.
 * @param ipcData Additional IPC data.
 *
 * @returns DRAPI_OK in case of success
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
drApiResult_t drApiIpcUnknownMessage(
    threadid_t       *pIpcPeer,
    message_t        *pIpcMsg,
    uint32_t         *pIpcData
);


/**
 * This function extracts the size of the payload sent to the driver
 *
 * @param message combining the length of the payload and the message command .
 *
 * @returns the size of the payload
 */
static inline _UNUSED uint32_t drApiExtractMsgLen(uint32_t msg)
{
    return (msg >> 16) & 0xFFFF;
}


/**
 * This function extracts the command sent to the driver
 *
 * @param message combining the length of the payload and the message command .
 *
 * @returns the command
 */
static inline _UNUSED uint32_t drApiExtractMsgCmd(uint32_t msg)
{
    return msg  & 0xFFFF;
}
//------------------------------------------------------------------------------


#if TBASE_API_LEVEL >= 3
/**
 * Makes request to t-base to update notification thread
 *
 * @param threadNo Number of the new notification thread.
 *
 * @retval DRAPI_OK or relevant error code.
 */
drApiResult_t drApiUpdateNotificationThread(
        threadno_t threadno
);

//------------------------------------------------------------------------------
/** Restarts thread with given ip and sp
 *
 * @param Clients thread id
 * @param property id
 * @param buffer address
 * @param buffer length [in,out]
 *
 * @return MTK return code
 */
drApiResult_t drApiGetClientProperty(
        const threadid_t  client,
        const property_t  property,
        const addr_t      buffer,
        uint32_t          *bufferLen
);

#endif /* TBASE_API_LEVEL */

#ifdef __cplusplus
}
#endif

#endif /** __DRAPI_IPCMSG_H__ */

