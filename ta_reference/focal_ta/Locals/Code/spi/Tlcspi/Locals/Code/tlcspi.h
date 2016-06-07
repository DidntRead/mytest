 /*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 * 
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#ifndef TLCDAPC_H_
#define TLCDAPC_H_

#include "MobiCoreDriverApi.h"

mcResult_t tlcOpen(void);

mcResult_t getDevinfo(uint32_t *index, uint32_t *result);


void tlcClose(void);

#endif // TLCDAPC_H_
