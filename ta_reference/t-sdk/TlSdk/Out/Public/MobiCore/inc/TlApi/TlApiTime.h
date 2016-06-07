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
#ifndef __TLAPITIME_H__
#define __TLAPITIME_H__

#include "TlApi/TlApiCommon.h"
#include "TlApi/TlApiError.h"

#if TBASE_API_LEVEL >= 3


//------------------------------------------------------------------------------
/** Real time sources in MobiCore */
typedef enum {
        TS_SOURCE_ILLEGAL       = 0,    /**< Illegal counter source value. */
        TS_SOURCE_SOFTCNT       = 1,    /**< monotonic counter that is reset upon power cycle. */
        TS_SOURCE_SECURE_RTC    = 2,    /**< Secure real time clock that uses underlying hardware clock. */
} tsSource_t;


/** Timestamp Counter.
 * The Timestamp Counter returns global relative Timestamp value of
 * the hardware platform.
 */
typedef uint64_t timestamp_t, *timestamp_ptr;

/** Get timestamp value from the secure world in µs.
 *
 * @param pTimestamp                    pointer to timestamp_t counter value
 * @return TLAPI_OK                     if the timestamp could be retrieved
 * @return E_TLAPI_INVALID_PARAMETER    pTimestamp is NULL or pointer is invalid
 * @return E_TLAPI_NOT_IMPLEMENTED      platform does not support secure timestamps
 */
_TLAPI_EXTERN_C tlApiResult_t tlApiGetSecureTimestamp(timestamp_ptr pTimestamp);

#endif /* TBASE_API_LEVEL */

#endif // __TLAPITIME_H__

