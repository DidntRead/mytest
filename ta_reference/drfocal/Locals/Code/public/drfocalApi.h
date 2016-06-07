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

/**
 * @file   tlDriverApi.h
 * @brief  Contains trustlet API definitions
 *
 */

#ifndef __TLDRIVERAPI_H__
#define __TLDRIVERAPI_H__

#include "tlStd.h"
#include "TlApi/TlApiError.h"
#include "drfocal.h"
//Get the fp list
//return  0:success, other: Error code
_TLAPI_EXTERN_C int alipay_sec_fp_get_ids(int* ids, int* idsCount);

//Get fp name for fp id
//return  0:success, other: Error code
_TLAPI_EXTERN_C int alipay_sec_fp_getname(int id, char *name, int *length);

//Get fp id index
//return  0:success, other: Error code
_TLAPI_EXTERN_C int alipay_sec_fp_get_identify_result(int *id);

//Get fp version
//return  0:success, other: Error code
_TLAPI_EXTERN_C int alipay_sec_fp_getversion(unsigned int* ver);

//Get authenticatorsign need oem do
//return  0:success, other: Error code
//OEM need generate rsa key pair;use privatekey with sha256 sign the input,give pubkey to alipay server.
//_TLAPI_EXTERN_C int alipay_sec_fp_authenticatorsign(unsigned char *input, unsigned long inputLength, unsigned char * signature, unsigned long *signLen);

//Get the fp oem version
//return  0:success, other: Error code
//_TLAPI_EXTERN_C int alipay_sec_authenticator_getversion(unsigned int *authenticatorVersion);

_TLAPI_EXTERN_C tlApiResult_t drfocal_init(void);
_TLAPI_EXTERN_C int drfp_setversion(unsigned int ver);
_TLAPI_EXTERN_C int drfp_setIdName(unsigned int index, /*char *name*/struct tlfocalApiDevice *focaldev);
_TLAPI_EXTERN_C int drfp_setids(struct tlfocalApiDevice *focaldev);
_TLAPI_EXTERN_C int drfp_setPassId(uint32_t passid);
_TLAPI_EXTERN_C int drfp_updateTpl(unsigned int tid, char isadd);
/**
 * Open session to the driver with given data
 *
 * @return  session id
 */
_TLAPI_EXTERN_C uint32_t tlfocalApiOpenSession( void );


/**
 * Close session
 *
 * @param sid  session id
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C tlApiResult_t tlfocalApiCloseSession( uint32_t sid );


/**
 * Inits session data (sample data 01)
 *
 * @param length  data length
 * @param address data address
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C tlApiResult_t tlfocalApiInitSampleData01(
        uint32_t sid,
        uint32_t length,
        uint32_t address);


/**
 * Inits session data (sample data 02)
 *
 * @param data
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C tlApiResult_t tlfocalApiInitSampleData02(
        uint32_t sid,
        uint32_t data);


/**
 * Executes command
 *
 * @param sid        session id
 * @param commandId  command id
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C tlApiResult_t tlfocalApiExecute(
        uint32_t sid,
        struct tlfocalApiDevice *focaldev);


/** tlApi function to call driver via IPC.
 * Sends a MSG_RQ message via IPC to a MobiCore driver.
 *
 * @param driverID The driver to send the IPC to.
 * @param payload MPointer to marshaling parameters to send to the driver.
 * @param payloadSize the size of the payload that will be sent to the driver
 *
 * @return TLAPI_OK
 * @return E_TLAPI_COM_ERROR in case of an IPC error.
 */
_TLAPI_EXTERN_C tlApiResult_t tlfocalApi_callDriverEx(
        uint32_t driver_ID,
        void* payload,
        uint32_t payloadSize);

#endif // __TLDRIVERAPI_H__
