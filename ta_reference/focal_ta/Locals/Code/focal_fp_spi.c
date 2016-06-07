//*********************************************************************************
//Copyright(C),2015-2020,FocalTech
//File name	  :focal_fp_spi.c
//Author		  :yanj
//Date		  :
//Version      	  :v1.0
//Description 	  :
//ModifyRecord :
//**********************************************************************************
#include "focal_common.h"
#include "focal_fp_spi.h"


#include "tlStd.h"
#include "TlApi/TlApi.h"
#include "tci.h"
#include "spi.h"
#include "drtlspi_api.h"
#include "drspi_Api.h"
#include "focal_log.h"

//#define  MTK_SPI_DMA_MODE
#ifdef MTK_SPI_DMA_MODE
#define MAX_SPI_RW_LEN		1024
#else
#define MAX_SPI_RW_LEN		32
#endif

#define FOCAL_FP_SPI_DEBUG
#ifdef FOCAL_FP_SPI_DEBUG
#define FT_DBG(fmt, args...)	LOGD("[focal_fp_spi]"fmt, ##args);
#else
#define FT_DBG(fmt, args...) 	do{}while(0)
#endif

static struct mt_chip_conf spi_chip_config;

void focal_fp_spi_config(struct mt_chip_conf *chip_config)
{
	 chip_config->setuptime = 3;
	 chip_config->holdtime = 3;
	 chip_config->high_time = 15;//Note this is OEM specific
	 chip_config->low_time = 15;//Note this is OEM specific
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

int32_t focal_fp_sensor_write_sfr_register(uint8_t reg_addr, uint8_t reg_value)
{
	int32_t ret;
	uint8_t tx_buf[4] = {0};
	uint8_t rx_buf[4] = {0};

	focal_fp_spi_init(0,0);
	 
	tx_buf[0] = CMD_WRITE_SFR_SINGLE_1;
	tx_buf[1] = CMD_WRITE_SFR_SINGLE_2;
	tx_buf[2] = reg_addr;
	tx_buf[3] = reg_value;

	ret = drSpiSend(tx_buf, rx_buf, 4, &spi_chip_config, 1);
	if (ret != 0)
	{
		LOGE("[focal_ta]:drSpiSend error!");
	}
	
	return ret;
}

int32_t focal_fp_sensor_read_sfr_register(uint8_t reg_addr, uint8_t *reg_value)
{
	uint32_t ts_dummy = 0;
 	uint32_t ts_start = 0;
 	uint32_t ts_stop = 0;
	int32_t ret;
	uint8_t tx_buf[4] = {0};
	uint8_t rx_buf[4] = {0};

	focal_fp_spi_init(0,0);

	tx_buf[0] = CMD_READ_SFR_SINGLE_1;
	tx_buf[1] = CMD_READ_SFR_SINGLE_2;
	tx_buf[2] = reg_addr;

	//focal_get_timestamp(&ts_dummy, &ts_start);
	ret = drSpiSend(tx_buf, rx_buf, 4, &spi_chip_config, 1);
	//focal_get_timestamp(&ts_dummy, &ts_stop);
	//FT_DBG("[SPISPEED] drSpiSend time = %d", ts_stop - ts_start);
	if (ret != 0)
	{
		LOGE("[focal_ta]:drSpiSend error!");
	}

	*reg_value = rx_buf[3];
	FT_DBG("[register:0x%02x].[value:0x%02x]\n", reg_addr, rx_buf[3]);
	
	return ret;
}

int32_t focal_fp_sensor_write_sram_register(uint16_t reg_addr, uint16_t reg_value)
{
	int32_t ret;
	uint8_t tx_buf[8] = {0};
	uint8_t rx_buf[8] = {0};

	focal_fp_spi_init(0,0);

	tx_buf[0] = CMD_WRITE_SRAM_1;
	tx_buf[1] = CMD_WRITE_SRAM_2;
	tx_buf[2] = reg_addr >> 8;
	tx_buf[3] = reg_addr & 0xff;
	tx_buf[4] = 0;
	tx_buf[5] = 2;
	tx_buf[6] = (reg_value >> 8) & 0xff;
	tx_buf[7] = reg_value & 0xff;
	
	ret = drSpiSend(tx_buf, rx_buf, 8, &spi_chip_config, 1);
	if (ret != 0)
	{
		LOGE("[focal_ta]:drSpiSend error!");
	}

	return ret;
}

int32_t focal_fp_sensor_write_sram_register_bit(uint16_t reg_addr, int16_t bit_buf[], int32_t len)
{
	int32_t i = 0, ret;
	int32_t sum_bit = 0, part_bit = 0;
	int32_t part_value = 0, loop = len / 2;
	int16_t reg_value = 0x0, n = 0;
	
	for(i = 0; i < loop; i++)
	{
		part_bit = bit_buf[n];
		part_value = bit_buf[n+1];

		if((sum_bit + part_bit) > 16)
			return -1;
		reg_value = (short) (reg_value << part_bit);
		reg_value |= part_value;
		sum_bit += part_bit;
		n += 2;
	}

	ret = focal_fp_sensor_write_sram_register(reg_addr, reg_value);
	if (ret < 0)
	{
		LOGE("[focal_ta]:focal_fp_sensor_write_sram_register error!");
		return -1;
	}
	
	return 0;
}

int32_t focal_fp_sensor_read_sram_register(uint16_t reg_addr, uint16_t *reg_value)
{
	int32_t ret;
	uint8_t tx_buf[8] = {0};
	uint8_t rx_buf[8] = {0};

	focal_fp_spi_init(0,0);

	tx_buf[0] = CMD_READ_SRAM_1;
	tx_buf[1] = CMD_READ_SRAM_2;
	tx_buf[2] = reg_addr >> 8;
	tx_buf[3] = reg_addr & 0xff;
	tx_buf[4] = 0;
	tx_buf[5] = 2;

	ret = drSpiSend(tx_buf, rx_buf, 8, &spi_chip_config, 1);
	if (ret != 0)
	{
		LOGE("[focal_ta]:drSpiSend error!");
	}
	
	*reg_value = rx_buf[6] << 8 | rx_buf[7];
	//FT_DBG("[register:0x%02x].[value:0x%02x]\n", reg_addr, *reg_value);

	return 0;
} 

uint8_t g_BulkReadBuf[MAX_BULK_READ_LEN * 2];
int32_t focal_fp_sensor_readbulk_sram(uint16_t startAddr, uint16_t *pReadBuf, uint16_t ReadLen)
{
	unsigned short read_len = ReadLen * 2;
	unsigned short recv_len = read_len;
	int i, ret;
	int n = 0;
	int bulkloop = read_len / (MAX_SPI_RW_LEN - 6) + 1;
	uint8_t tx_buf[MAX_SPI_RW_LEN] = {0};
	uint8_t rx_buf[MAX_SPI_RW_LEN] = {0};

	focal_fp_spi_init(0,0);

	memset(g_BulkReadBuf, 0, MAX_BULK_READ_LEN * 2);

	for (n = 0; n < bulkloop; n++) {
		int bulkreadlen = 0;
		if (recv_len >= (MAX_SPI_RW_LEN - 6))
			bulkreadlen = MAX_SPI_RW_LEN - 6;
		else
			bulkreadlen = recv_len % (MAX_SPI_RW_LEN - 6);

		memset(tx_buf, 0, sizeof(tx_buf));
		memset(rx_buf, 0, sizeof(rx_buf));
		tx_buf[0] = CMD_READ_SRAM_1;
		tx_buf[1] = CMD_READ_SRAM_2;
		tx_buf[2] = startAddr >> 8;
		tx_buf[3] = startAddr & 0xff;
		tx_buf[4] = (bulkreadlen - 2) >> 8;
		tx_buf[5] = (bulkreadlen - 2) & 0xff;

		ret = drSpiSend(tx_buf, rx_buf, bulkreadlen + 6, &spi_chip_config, 1);
		if (ret != 0)
		{
			LOGE("[focal_ta]:focal_fp_sensor_readbulk_sram error!");
		}
		startAddr += (MAX_SPI_RW_LEN - 6) / 2;
		recv_len -= (MAX_SPI_RW_LEN - 6);

		memcpy(g_BulkReadBuf + (MAX_SPI_RW_LEN - 6) * n, rx_buf + 6, bulkreadlen);
	}

	n = 0;
	for (i = 0; i < read_len; i += 2) {
		pReadBuf[n] = (unsigned short) g_BulkReadBuf[i] << 8
				| g_BulkReadBuf[i + 1];
		n++;
	}

	return 0;
}

int32_t focal_fp_spi_reset(void)
{
	int32_t ret;
	uint8_t tx_buf[1] = {0};
	uint8_t rx_buf[1] = {0};

	focal_fp_spi_init(0,0);

	tx_buf[0] = 0x70;

	ret = drSpiSend(tx_buf, rx_buf, 1, &spi_chip_config, 1);
	if (ret != 0)
	{
		LOGE("[focal_ta]:focal_fp_spi_reset error!");
	}
	
	return 0;
}

int32_t platform_spi_write(uint8_t *tx_buf, int32_t len)
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

int32_t platform_spi_read(uint8_t *tx_buf, uint8_t *rx_buf, int32_t len)
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

