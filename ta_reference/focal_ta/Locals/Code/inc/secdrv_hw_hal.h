/** @addtogroup SECDRV
 * @{
 * Secure Driver
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 */
#ifndef __SECDRV_HW_HAL_H__
#define __SECDRV_HW_HAL_H__

#define MAXIMUM_DATA_LENGTH 1024

#include "secdrv_error.h"

#undef DR_DEBUG
#undef CRYPTO_INTR_MODE

#ifdef CONFIG_SMDK4x12
#define SECMEM_NUM	3
#elif CONFIG_MX
#define SECMEM_NUM	3
#endif

#define SPI_MAX_PORT_NUM	5

#if defined(CONFIG_EXYNOS5433)
#define SPI_PORT	(0x2)
#elif defined(CONFIG_EXYNOS7420)
#define SPI_PORT	(0x4)
#else
#error No platform defined
#endif
#define BUF_SIZE        (32 * 1024)
#define MAX_SPEED	(15000000)
#define CPUMODE		(0x0)
#define DMAMODE		(0x1)

#define IRAM_SECURE_OS_BASE	(0x02073000)
#define IRAM_SECURE_OS_OFFSET	(0x848)

/* DMA buffer size in ION */
#define DMA_MICRO_CODE_SIZE	(0x2000)
#define DMA_TX_BUF_SIZE		(0x8000)
#define DMA_RX_BUF_SIZE		(0x8000)
#define DMA_SECURE_FLAG		(0x13000)
#if defined(CONFIG_EXYNOS7420)
#define DMA_BUF_SIZE		0x40000
#else
#define DMA_BUF_SIZE		0x20000
#endif

/* bits per word */
#define BYTE_SIZE	(0)
#define HWORD_SIZE	(1)
#define WORD_SIZE	(2)

/* Interrupt Thread Source */
#define EINT_THREAD		0

#define TL_ADDR_BASE			(0x80000)
#if defined(CONFIG_EXYNOS5430) || defined(CONFIG_EXYNOS5422) \
	|| defined(CONFIG_EXYNOS5433) ||  defined(CONFIG_EXYNOS7420)
#define TL_ADDR_MAX			(0xFA000)
#elif defined(CONFIG_EXYNOS5420)
#define TL_ADDR_MAX			(0xE0000)
#else
#undef TL_ADDR_MAX /* it will make build error. you should check MAX address size for the SoC */
#endif
#define TL_ADDR_IS_VALID(x)		(((x) >= TL_ADDR_BASE) && ((x) <= (TL_ADDR_MAX - 0x1000)))
#define TL_ADDR(x)			(TL_ADDR_BASE + (x))
#define TL_VA_RESERVED			TL_ADDR(0x00000)
#define TL_VA_DYNAMIC_MAPPING		TL_ADDR(0x10000)
#define TL_VA_CMU0_SFR			TL_ADDR(0x30000)
#define TL_VA_CMU1_SFR			TL_ADDR(0x31000)
#define TL_VA_PMMU_SFR			TL_ADDR(0x32000)
#if defined(CONFIG_EXYNOS5430)
#define TL_VA_TZPC_SFR			TL_ADDR(0x33000)
#elif defined(CONFIG_EXYNOS7420)
#define TL_VA_TZPC12_SFR		TL_ADDR(0x33000)
#define TL_VA_TZPC9_SFR			TL_ADDR(0x34000)
#else
#define TL_VA_TZPC0_SFR			TL_ADDR(0x33000)
#define TL_VA_TZPC1_SFR			TL_ADDR(0x34000)
#define TL_VA_TZPC2_SFR			TL_ADDR(0x35000)
#define TL_VA_TZPC3_SFR			TL_ADDR(0x36000)
#endif
#define TL_VA_TZPC5_SFR			TL_ADDR(0x3B000)
#define TL_VA_SYSREG_SFR		TL_ADDR(0x3C000)
#define TL_VA_SPI_SFR(x)		TL_ADDR(0x42000 + (x * 0x1000))
#define TL_VA_GPIO_SPI_SFR		TL_ADDR(0x47000)
#define TL_VA_GIC			TL_ADDR(0x48000)
#if defined(CONFIG_EXYNOS5430)
#define TL_VA_PDMA			TL_ADDR(0x49000)
#else
#define TL_VA_PDMA1			TL_ADDR(0x49000)
#endif
#define TL_VA_MICROCODE			TL_ADDR(0x4a000)
#define TL_VA_TZASC_0_BASE		TL_ADDR(0x4b000)
#define TL_VA_TZASC_1_BASE		TL_ADDR(0x4c000)
#define TL_VA_SECURE_FLAG		TL_ADDR(0x4d000)
#define TL_VA_TEST_DST			TL_ADDR(0x4f000)
#define TL_VA_TEST_SRC			TL_ADDR(0x50000)
#define TL_VA_RX_BUFF			TL_ADDR(0x52000)
#define TL_VA_TX_BUFF			TL_ADDR(0x53000)
#if defined(CONFIG_EXYNOS7420)
#define TL_VA_TZASC_2_BASE		TL_ADDR(0x54000)
#define TL_VA_TZASC_3_BASE		TL_ADDR(0x55000)
#endif
#define TL_VA_CMU_IMEM_BASE		TL_ADDR(0x56000)

/* Struct for Secure map */
typedef struct {
	uint32_t	cmu_base0;
	uint32_t	cmu_base1;
	uint32_t	pmu_base;
	uint32_t	tzpc_base;
	uint32_t	spi_base[SPI_MAX_PORT_NUM];
	uint32_t	gpio_spi_base;
	uint32_t	gic_base;
} secMap_t;

/* Device Struct */
typedef struct {
	void		*buf_addr;
	unsigned	buf_len;
	unsigned	total_len;
} spi_transfer_t;

typedef struct {
	uint16_t	delay_usecs;
	uint32_t	speed_hz;
	uint8_t		bits_per_word;
	uint8_t		dma_mode;
} spi_config_t;

enum flag_lists {
	OEM_FLAG,
	NUM_FLAGS,
};

struct secFlag_t {
	uint32_t *addr;
	uint32_t value;
};

struct secDrm_ctx_t {
	struct secFlag_t secFlag[NUM_FLAGS];
};

/*
 * SEC XXX function
 */
secDrvResult_t sec_spi_init(uint32_t port, spi_config_t *spi_cfg);
secDrvResult_t sec_spi_write(uint32_t port,
			spi_config_t *spi_cfg, spi_transfer_t *spi_tx);
secDrvResult_t sec_spi_read(uint32_t port,
			spi_config_t *spi_cfg, spi_transfer_t *spi_rx);
secDrvResult_t sec_spi_write_read(uint32_t port, spi_config_t *spi_cfg,
			spi_transfer_t *spi_tx, spi_transfer_t *spi_rx);
secDrvResult_t sec_spi_exit(uint32_t port);
secDrvResult_t sec_dma_init(uint32_t mem_pa);
secDrvResult_t sec_dma_test(void);

secDrvResult_t sec_spi_clock_init(uint32_t port, spi_config_t *spi_cfg);
secDrvResult_t sec_spi_clock_enable(uint32_t port);
secDrvResult_t sec_spi_clock_disable(uint32_t port);

#endif /* __SECDRV_HW_HAL_H__ */

