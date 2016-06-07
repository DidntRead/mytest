/** @addtogroup SECDRV
 * @{
 * Basic SEC encryption driver functions.
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 */
#ifndef __TLAPI_SECDRV_H__
#define __TLAPI_SECDRV_H__

#include "TlApi/TlApiCommon.h"
#include "TlApi/TlApiError.h"
//#include "secdrv_hw_hal.h"

_TLAPI_EXTERN_C tlApiResult_t tlApiSecSPIInit(uint32_t port, spi_config_t *spi_cfg);
_TLAPI_EXTERN_C tlApiResult_t tlApiDmaTest(void);
_TLAPI_EXTERN_C tlApiResult_t tlApiSecSPIExit(uint32_t port);
_TLAPI_EXTERN_C tlApiResult_t tlApiSecSPIWrite(uint32_t port,
		spi_config_t *spi_cfg, spi_transfer_t *spi_tx);
_TLAPI_EXTERN_C tlApiResult_t tlApiSecSPIRead(uint32_t port,
		spi_config_t *spi_cfg, spi_transfer_t *spi_rx);
_TLAPI_EXTERN_C tlApiResult_t tlApiSecSPIWriteRead(uint32_t port,
		spi_config_t *spi_cfg, spi_transfer_t *spi_tx,
					spi_transfer_t *spi_rx);
_TLAPI_EXTERN_C tlApiResult_t tlApiSecSPIDMAInit(unsigned int pa);
_TLAPI_EXTERN_C tlApiResult_t tlApiSecSPIClockInit(uint32_t port, spi_config_t *spi_cfg);
_TLAPI_EXTERN_C tlApiResult_t tlApiSecSPIClockEnable(uint32_t port);
_TLAPI_EXTERN_C tlApiResult_t tlApiSecSPIClockDisable(uint32_t port);

#endif /* __TLAPI_SECDRV_H__ */

