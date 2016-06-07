#ifndef __SPI_DEBUG_H__
#define __SPI_DEBUG_H__

#include <stdint.h>
#include "drCommon.h"

#include "DrApi/DrApiLogging.h"

#define FOCAL_SPI_DEBUG

#define SPI_LOG(string, args...)                                   \
    if(1)                                                           \
    {                                                               \
        drDbgPrintf("[SPI][tz]"string, ##args);                    \
    }

#ifdef FOCAL_SPI_DEBUG
#define SPI_MSG(string, args...)                                   \
    if(1)                                \
    {                                                               \
        drDbgPrintf("[SPI][tz]"string, ##args);                    \
    }
#else
#define SPI_MSG(string, args...)				\
	do{							\
	}while(0)
#endif

#define SPI_ERR(string, args...)                                   \
    if(1)                                                           \
    {                                                               \
        drDbgPrintf("[SPI][tz][ERR]"string, ##args);               \
    }
//--------------------------------------------------------------------------

#endif  // __SPI_DEBUG_H__

