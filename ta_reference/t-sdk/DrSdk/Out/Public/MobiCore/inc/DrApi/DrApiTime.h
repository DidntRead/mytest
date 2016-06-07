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

#ifndef __DRAPITIME_H__
#define __DRAPITIME_H__

#include "DrApi/DrApiCommon.h"
#include "DrApi/DrApiError.h"

#if TBASE_API_LEVEL >= 3


//------------------------------------------------------------------------------
/** Timestamp Counter.
 * The Timestamp Counter returns global relative Timestamp value of
 * the hardware platform.
 */
typedef uint64_t timestamp_t, *timestamp_ptr;


/** Get timestamp value from the secure world in µs.
 *
 * @param pTimestamp                    pointer to timestamp_t counter value
 * @return DRAPI_OK                     if the timestamp could be retrieved
 * @returns E_DRAPI_INVALID_PARAMETER   pTimestamp is NULL or pointer is invalid
 * @returns E_DRAPI_NOT_IMPLEMENTED     platform does not support secure timestamps
 */
_DRAPI_EXTERN_C drApiResult_t drApiGetSecureTimestamp(timestamp_ptr pTimestamp);

#endif /* TBASE_API_LEVEL */

#endif // __DRAPITIME_H__

/** @} */
