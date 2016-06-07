/*
 * Copyright (c) 2014 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */


#ifndef __DRAPIMMEXT_H__
#define __DRAPIMMEXT_H__


#include "DrApiCommon.h"

#ifdef __cplusplus
extern "C" {
#endif


#if TBASE_API_LEVEL >= 5

// Extended MAP API for "extended memory layout TDrivers"

//------------------------------------------------------------------------------
/** Maps a client task buffer indicated by client task ID, its virtual address and length and attributes
 *  to a free memory area in TDriver space. The address is chosen automatically.
 *
 * @param client: [in] client task ID
 * @param startVirtClient: [in] virtual address of the buffer in client task space
 * @param length: [in] length of the buffer
 * @param attr: [in] map attributes requested
 * @param startVirtServer: [out] virtual address obtained of the buffer in TDriver space
 * @returns DRAPI_OK in case of success
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
_DRAPI_EXTERN_C drApiResult_t drApiMapTaskBuffer(
    const taskid_t    taskId,
    const void        *startVirtClient,
    const size_t      length,
    const uint32_t    attr,
    void              **startVirtServer
);

//------------------------------------------------------------------------------
/** Maps a physical buffer indicated by its physical address, length and attributes
 *  to a free memory area in TDriver space. The address is chosen automatically.
 *
 * @param startPhys: [in] physical address
 * @param length: [in] length of the buffer
 * @param attr: [in] map attributes requested
 * @param startVirtServer: [out] virtual address obtained of the buffer in TDriver space
 * @returns DRAPI_OK in case of success
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
_DRAPI_EXTERN_C drApiResult_t drApiMapPhysicalBuffer(
    const uint64_t    startPhys,
    const size_t      length,
    const uint32_t    attr,
    void              **startVirtServer
);

//------------------------------------------------------------------------------
/** Unmaps a buffer (physical or task one) indicated by its virtual address
 *
 * @param startVirtServer: [in] virtual address of the buffer in TDriver space
 * @returns DRAPI_OK in case of success
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
_DRAPI_EXTERN_C drApiResult_t drApiUnmapBuffer(const void *startVirtServer);
#define drApiUnmap drApiUnmapBuffer

//------------------------------------------------------------------------------
/** Unmaps all the buffers of a task indicated by its task ID
 *
 * @param clientTaskId: [in] client task ID
 * @returns DRAPI_OK in case of success
 * @returns any combination of DRAPI_ERROR_CREATE(Driver specific error code, MTK error code)
 */
_DRAPI_EXTERN_C drApiResult_t drApiUnmapTaskBuffers(const taskid_t clientTaskId);


#endif /* TBASE_API_LEVEL */


#ifdef __cplusplus
}
#endif

#endif // __DRAPIMMEXT_H__
