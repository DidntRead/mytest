//*********************************************************************************
//Copyright(C),2015-2020,FocalTech
//File name	  :focal_fp_spi.c
//Author		  :yanj
//Date		  :
//Version      	  :v1.1
//Description 	  :
//ModifyRecord :
//**********************************************************************************
#include <stdint.h>

#include "tlStd.h"
#include "TlApi/TlApi.h"
#include "tci.h"
#include "focal_platform.h"
#include "spi.h"
#include "drtlspi_api.h"
#include "drspi_Api.h"
#include "focal_log.h"

#ifdef MTK_SPI_DMA_MODE
#define MAX_SPI_RW_LEN		1024
#else
#define MAX_SPI_RW_LEN		32
#endif

#define FOCAL_PLAFORM_DEBUG
#ifdef FOCAL_PLAFORM_DEBUG
#define FT_DBG(fmt, args...)	LOGD("[focal_fp_spi]"fmt, ##args);
#else
#define FT_DBG(fmt, args...) 	do{}while(0)
#endif

static struct mt_chip_conf spi_chip_config;

void focal_fp_spi_config(struct mt_chip_conf *chip_config)
{
	 chip_config->setuptime = 3;
	 chip_config->holdtime = 3;
	 chip_config->high_time = 25;//Note this is OEM specific
	 chip_config->low_time = 25;//Note this is OEM specific
	 chip_config->cs_idletime = 2;
	 chip_config->ulthgh_thrsh = 0;
 
	 chip_config->cpol = 0;
	 chip_config->cpha = 0;
	 
	 chip_config->rx_mlsb = 1; 
	 chip_config->tx_mlsb = 1;
 
	 chip_config->tx_endian = 0;
	 chip_config->rx_endian = 0 ;
 
	 chip_config->com_mod = FIFO_TRANSFER;//DMA_TRANSFER;//
	 chip_config->pause = 0;//PAUSE_MODE_ENABLE;
	 chip_config->finish_intr = 1;
	 chip_config->deassert = 0;
	 chip_config->ulthigh = 0;
	 chip_config->tckdly = 0;
}

 int focal_fp_spi_init(uint32_t freq_low_khz, uint32_t freq_high_khz)
 {	 
	 spi_chip_config.high_time = freq_high_khz;
	 spi_chip_config.low_time= freq_low_khz;

	 focal_fp_spi_config(&spi_chip_config);
	 
	 return 0;
 }

void platform_usleep(int us)
{
	int i = 0, j = 0;

	for (i = 0; i < us; i++)
	{
		while(j < 35000)
		{
	    		j++;
		}
	}
}

int32_t platform_spi_write(uint8_t *tx_buf, uint32_t len)
{
    int32_t ret;
    uint8_t rx_buf[MAX_SPI_RW_LEN + 12] = {0};

    focal_fp_spi_init(0,0);

    ret = drSpiSend(tx_buf, rx_buf, len, &spi_chip_config, 1);
    if (ret != 0)
    {
        LOGE("[focal_ta]:focal_fp_spi_reset error!");
    }

    return 0;
}

int32_t platform_spi_read(uint8_t *tx_buf, uint8_t *rx_buf, uint32_t len)
{
    int32_t ret;

    focal_fp_spi_init(0,0);

    ret = drSpiSend(tx_buf, rx_buf, len, &spi_chip_config, 1);
    if (ret != 0)
    {
        LOGE("[focal_ta]:focal_fp_spi_reset error!");
    }

    return 0;
}



