/** @addtogroup TLAPI_SecDrv
 * @{
 * @file
 * Marshaling types and declarations for Secure driver and trustlet.
 *
 * Functions for the marshaling of function ID and parameters.
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 */
#ifndef __TLAPIMARSHAL_SecDrv_H__
#define __TLAPIMARSHAL_SecDrv_H__

#include "TlApi/TlApiError.h"
#include "secdrv_hw_hal.h"

/**
 * Driver ID. Update accordingly
 */
#define SEC_SPI_DRV_ID	830619

void Initialise_Sec_Driver(void);
_NORETURN void SecDriverIPC_Handler(void);

/** Each function must have its own ID.
 * Extend this list if you add a new function.
 */
typedef enum {
	SEC_SPI_INIT = 1,
	SEC_SPI_EXIT,
	SEC_SPI_WRITE,
	SEC_SPI_READ,
	SEC_SPI_WRITE_READ,
	SEC_SPI_DMA_INIT,
	SEC_SPI_CLOCK_INIT,
	SEC_SPI_CLOCK_ENABLE,
	SEC_SPI_CLOCK_DISABLE,
	SEC_DMA_TEST
} Sec_FuncID_t;

/** Invalid session id. Returned in case of an error. */
#define DR_SID_INVALID      0xffffffff

/** Maximum number of parameters. */
#define MAX_MAR_LIST_LENGTH 8                      /**< Maximum list of possible marshaling parameters. */

/*
 * Union of marshaling parameters. */
/* If adding any function, add the marshaling structure here
 */
typedef struct {
    uint32_t countOfNotifications;
    uint32_t intervalOfNotifications;
} drMarshalingData_t, *drMarshalingData_ptr;

/** Marshaled union. */
/* If adding any functions add the marhaling structure here */
typedef struct {
	uint32_t functionId;                       		/**< Function identifier. */
	union {
		struct secFlag_t SecureFlag[NUM_FLAGS];
		drMarshalingData_t params;
		tlApiResult_t retVal;
		uint32_t parameter[MAX_MAR_LIST_LENGTH];	/* untyped parameter list (expands union to 8 entries) */
	} payload;
	uint32_t     sid;           				/** Session identifier */
	uint32_t port;
	uint32_t *mem_pa;
	uint32_t *mem_va;
	spi_config_t *spi_cfg;
	spi_transfer_t *spi_tx;
	spi_transfer_t *spi_rx;

} SecMarshalingParam_t, *SecMarshalingParam_ptr;

#endif /* __TLAPIMARSHAL_SecDrv_H__ */
/** @} */
