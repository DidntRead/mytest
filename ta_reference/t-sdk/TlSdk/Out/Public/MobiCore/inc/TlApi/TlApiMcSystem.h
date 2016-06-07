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
 * The MobiCore system API interface provides system information and system functions to Trustlets.
 */
#ifndef __TLAPIMCSYSTEM_H__
#define __TLAPIMCSYSTEM_H__

#include "TlApi/TlApiCommon.h"
#include "TlApi/TlApiError.h"

#include "mcSuid.h"
#include "mcVersionInfo.h"
#include "TlApi/version.h"


//------------------------------------------------------------------------------

/** Get information about the implementation of the MobiCore Trustlet API version.
 *
 * @param tlApiVersion     pointer to tlApi version.
 * @return TLAPI_OK if version has been set
 * @returns E_TLAPI_NULL_POINTER    if one parameter is a null pointer.
 * @returns E_TLAPI_UNMAPPED_BUFFER if one buffer is not entirely mapped in Trustlet address space.
 */
_TLAPI_EXTERN_C tlApiResult_t tlApiGetVersion(
    uint32_t *tlApiVersion);

/** Get information about the underlying MobiCore version.
 * The function tlApiGetMobicoreVersion() provides the MobiCore product id and
 * version information about the various MobiCore interfaces as defined in mcVersionInfo.h
 *
 * @param mcVersionInfo     pointer to version information structure.
 * @returns TLAPI_OK if version has been set
 * @returns E_TLAPI_NULL_POINTER    if one parameter is a null pointer.
 * @returns E_TLAPI_UNMAPPED_BUFFER if one buffer is not entirely mapped in Trustlet address space.
 */
_TLAPI_EXTERN_C tlApiResult_t tlApiGetMobicoreVersion(
    mcVersionInfo_t *mcVersionInfo);

/** Terminate the Trustlet with a exit code.
 * Trustlets can use the tlApiExit() to terminate themselves and return an exit code. This
 * can be used if the initialization fails or an unrecoverable error occurred. The Trustlet
 * will be terminated immediately and this function will not return.
 *
 * @param exitCode exit code
 *
 * @return there is no return code, since the function will not return
 */
_TLAPI_EXTERN_C _TLAPI_NORETURN void tlApiExit(
    uint32_t    exitCode);

/** Get the System on Chip Universal Identifier.
 * @param suid pointer to Suid structure that receives the Suid data
 * @returns TLAPI_OK                if Suid has been successfully read.
 * @returns E_TLAPI_NULL_POINTER    if one parameter is a null pointer.
 * @returns E_TLAPI_UNMAPPED_BUFFER if one buffer is not entirely mapped in Trustlet address space.
 */
_TLAPI_EXTERN_C tlApiResult_t tlApiGetSuid(
    mcSuid_t            *suid);

//------------------------------------------------------------------------------
/* Get the virtual memory type
*
* @param type pointer to address where type is returned
* @param addr start address of checked memory
* @param size size checked memory
*/
#define TLAPI_VIRT_MEM_TYPE_SECURE        (1U<<3)                  /**< The memory area is mapped as secure */
#define TLAPI_VIRT_MEM_TYPE_NON_SECURE    (1U<<4)                  /**< The memory area is mapped as non-secure */

_TLAPI_EXTERN_C tlApiResult_t tlApiGetVirtMemType(
        uint32_t *type,
        addr_t   addr,
        uint32_t size);


//------------------------------------------------------------------------------
/* Helper to simplify NWd buffer testing
 */
static _UNUSED inline bool tlApiIsNwdBufferValid( addr_t addr, uint32_t size )
{
  uint32_t type;
  tlApiResult_t ret = tlApiGetVirtMemType( &type, addr, size );
  return ret==TLAPI_OK && (type&TLAPI_VIRT_MEM_TYPE_NON_SECURE)!=0;
}

// The isNwdBufferValid function name is deprecated, #define ensures backward compatibility
// Please do not use this function name anymore, use tlApiIsNwdBufferValid instead
#define isNwdBufferValid tlApiIsNwdBufferValid

#endif // __TLAPIMCSYSTEM_H__

