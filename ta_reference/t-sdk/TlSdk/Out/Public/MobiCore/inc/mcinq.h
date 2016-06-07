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

#ifndef NQ_H_
#define NQ_H_

/** \name NQ Size Defines
 * Minimum and maximum count of elements in the notification queue.
 * @{ */
#define MIN_NQ_ELEM 1   /**< Minimum notification queue elements. */
#define MAX_NQ_ELEM 64 /**< Maximum notification queue elements. */
/** @} */

/** \name NQ Length Defines
 * Minimum and maximum notification queue length.
 * @{ */
#define MIN_NQ_LEN (MIN_NQ_ELEM * sizeof(notification_t))   /**< Minimum notification length (in bytes). */
#define MAX_NQ_LEN (MAX_NQ_ELEM * sizeof(notification_t))   /**< Maximum notification length (in bytes). */
/** @} */

/** \name Session ID Defines
 * Standard Session IDs.
 * @{ */
#define SID_MCP       0           /**< MCP session ID is used when directly communicating with the MobiCore (e.g. for starting and stopping of trustlets). */
#define SID_INVALID   0xffffffff  /**< Invalid session id is returned in case of an error. */
/** @} */

/** Notification data structure. */
typedef struct{
    uint32_t sessionId; /**< Session ID. */
    int32_t payload;    /**< Additional notification information. */
} notification_t;

/** Notification payload codes.
 * 0 indicated a plain simple notification,
 * a positive value is a termination reason from the task,
 * a negative value is a termination reason from MobiCore.
 * Possible negative values are given below.
 */
typedef enum {
    ERR_INVALID_EXIT_CODE   = -1, /**< task terminated, but exit code is invalid */
    ERR_SESSION_CLOSE       = -2, /**< task terminated due to session end, no exit code available */
    ERR_INVALID_OPERATION   = -3, /**< task terminated due to invalid operation */
    ERR_INVALID_SID         = -4, /**< session ID is unknown */
    ERR_SID_NOT_ACTIVE      = -5, /**< session is not active */
    ERR_SESSION_KILLED      = -6, /**< session was force-killed (due to an administrative command). */
} notificationPayload_t;

/** Declaration of the notification queue header.
 * layout as specified in the data structure specification.
 */
typedef struct {
    uint32_t writeCnt;  /**< Write counter. */
    uint32_t readCnt;   /**< Read counter. */
    uint32_t queueSize; /**< Queue size. */
} notificationQueueHeader_t;

/** Queue struct which defines a queue object.
 * The queue struct is accessed by the queue<operation> type of
 * function. elementCnt must be a power of two and the power needs
 * to be smaller than power of uint32_t (obviously 32).
 */
typedef struct {
    notificationQueueHeader_t hdr;              /**< Queue header. */
    notification_t notification[MIN_NQ_ELEM];   /**< Notification elements. */
} notificationQueue_t;

#endif /** NQ_H_ */

/** @} */
