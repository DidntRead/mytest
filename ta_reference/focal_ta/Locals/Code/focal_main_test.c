/**
 * @file   focal_main_test.c
 *
 */
	 
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#include "tci.h"
#include "spi.h"

#include "tlStd.h"
#include "TlApi/TlApi.h"

#include "focal_log.h"


#include "focal_main_test.h"
#include "focal_lib_test.h"

#include "drtlspi_api.h"
#include "drspi_Api.h"


#include "focal_spi.h"

/* add for alipay begin*/
//#include "drfocalApi.h"
/* add for alipay end*/

 
 //vincent
static struct mt_chip_conf spi_chip_config;
static uint8_t tx_buf[16645];
static uint8_t rx_buf[16645];
 
 //#define _PERFORMANCE_MEASUREMENT_
 
#if 1
void focal_test_spi_config(struct mt_chip_conf *chip_config)
{
	 chip_config->setuptime = 3;
	 chip_config->holdtime = 3;
	 chip_config->high_time = 39;//Note this is OEM specific
	 chip_config->low_time = 39;//Note this is OEM specific
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
#endif
 
uint32_t focal_test_spi_register_test(void)
{
	 int i=0;
#if 0	//lib_test
	 int32_t x,y,z;
#endif
//	 int ret =0;
	 uint32_t tlRet = 0;
	 
	 LOGD("[focal_test ta spi]register test\n");
	 
	 focal_test_spi_init(0,0);
	 
	 memset(tx_buf,0,5);
	 memset(rx_buf,0,5);
	 tx_buf[0] = 0x08;
	 tx_buf[1] = 0xF7;
	 tx_buf[2] = 0x14;
	 tx_buf[3] = 0x00;
	 LOGD("[focal_test ta spi]drSpiSend!\n");
	 tlRet = drSpiSend( tx_buf, rx_buf, 4, &spi_chip_config, 1);
	 
	 LOGD("[focal_test ta spi]tx_buf[0]=%x:drSpiSend's ret=%d\n",tx_buf[0],tlRet);
	 
	 for(i=0;i<4;i++) {
		 LOGD("Register---> tx_buf[%d] = %x, rx_buf[%d] == %x\n",i, tx_buf[i], i,rx_buf[i]);
	 }
	 	 
#if 0	//lib_test
	 x = 111; 
	 y = 222;
	 z = focal_lib_test(x,y);
	 if(z == 333)
	 {
	 	LOGD("[focal_lib_test 333]z = %d!\n",z);
	 }
	 else
	 {
	 	LOGD("[focal_lib_test]z = %d!\n",z);
	 }
#endif
	 memset(tx_buf,0,5);
	 memset(rx_buf,0,5);
	 tx_buf[0] = 0x08;
	 tx_buf[1] = 0xF7;
	 tx_buf[2] = 0x10;
	 tx_buf[3] = 0x00;
	 LOGD("[focal_test ta spi]drSpiSend!\n");
	 tlRet = drSpiSend( tx_buf, rx_buf, 4, &spi_chip_config, 1);
	 
	 LOGD("[focal_test ta spi]tx_buf[0]=%x:drSpiSend's ret=%d\n",tx_buf[0],tlRet);
	 
	 for(i=0;i<4;i++) {
		 LOGD("Register---> tx_buf[%d] = %x, rx_buf[%d] == %x\n",i, tx_buf[i], i,rx_buf[i]);
	 }

	 return tlRet;
}

uint32_t focal_test_spi_register_test1(void)
{
	 int i=0;
//	 int ret =0;
	 uint32_t tlRet = 0;
	 
	 LOGD("[focal_test ta spi]register test\n");
	 
	 focal_test_spi_init(0,0);
	 
	 memset(tx_buf,0,5);
	 memset(rx_buf,0,5);
	 tx_buf[0] = 0x08;
	 tx_buf[1] = 0xF7;
	 tx_buf[2] = 0x10;
	 tx_buf[3] = 0x00;
	 tlRet = drSpiSend( tx_buf, rx_buf, 4, &spi_chip_config, 1);
	 
	 LOGD("[focal_test ta spi]tx_buf[0]=%x:drSpiSend's ret=%d\n",tx_buf[0],tlRet);
	 
	 for(i=0;i<4;i++) {
		 LOGD("Register---> tx_buf[%d] = %x, rx_buf[%d] == %x\n",i, tx_buf[i], i,rx_buf[i]);
	 }

	 return tlRet;
}

 
void focal_test_spi_chipcfg(void)
{
	
}
 
int focal_test_spi_test(void)
{
 int ret = 0;
 return ret;
}

void focal_test_read_int_loop(void)
{
}


 // ------------------------------------------------------------------------------------------------
 int focal_test_spi_init(uint32_t freq_low_khz, uint32_t freq_high_khz)
 //int focal_test_spi_init(void)
 {	 
	 LOGD("[focal_test ta]MTK TZ spi: %s\n", __func__);
	 spi_chip_config.high_time = freq_high_khz;
	 spi_chip_config.low_time= freq_low_khz;

	 focal_test_spi_config(&spi_chip_config);
	 
	 return 0;
 }
 
 // ------------------------------------------------------------------------------------------------
 int focal_test_spi_destroy(void)
 {
	 LOGD("%s\n", __func__);
	 return 0;
 }
 
 // ------------------------------------------------------------------------------------------------
 int focal_test_spi_select_low_freq(void)
 {
	 //LOGD("%s\n", __func__);
	 return 0;
 }
 
 // ------------------------------------------------------------------------------------------------
 int focal_test_spi_select_high_freq(void)
 {
	 //LOGD("%s\n", __func__);
	 return 0;
 }

 #if 0
 // ------------------------------------------------------------------------------------------------
 int focal_test_spi_read(uint8_t *buff, size_t num_bytes)
 {
	 LOGD("%s\n", __func__);
	 return 0;
 }
 
 // ------------------------------------------------------------------------------------------------

 int focal_test_spi_write(uint8_t *buff, size_t num_bytes)
 {
#if 0
	 tlApiResult_t tlRet;

#ifdef _PERFORMANCE_MEASUREMENT_
	 uint32_t ts_dummy = 0;
	 uint32_t ts_start = 0;
	 uint32_t ts_stop = 0;
#endif
 
	 //LOGD("%s\n", __func__);
	 if (num_bytes >= 32)
		 spi_chip_config.com_mod = DMA_TRANSFER;
	 else
		 spi_chip_config.com_mod = FIFO_TRANSFER;
 
	 //LOGD("before drSpiSend(), size = %d\n", num_bytes);
#ifdef _PERFORMANCE_MEASUREMENT_
	 focal_test_get_timestamp(&ts_dummy, &ts_start);
#endif
	 tlRet = drSpiSend(buff, buff, num_bytes, &spi_chip_config, 1);
#ifdef _PERFORMANCE_MEASUREMENT_
	 focal_test_get_timestamp(&ts_dummy, &ts_stop);
	 if ((ts_stop - ts_start) > 500) LOGD("KPI focal_test_spi_write(), size = %d, tlRet= %d. time: %d\n", num_bytes, tlRet, (ts_stop - ts_start));
#endif
	 //LOGD("after drSpiSend(), size = %d, tlRet= %d\n", num_bytes, tlRet);
 
	 return tlRet;
#else
	return 0;
 #endif
 }
 
 // ------------------------------------------------------------------------------------------------
 int focal_test_spi_writeread(uint8_t *tx, size_t tx_bytes, uint8_t *rx, size_t rx_bytes)
 {
 #if 0
	 tlApiResult_t tlRet;
	 uint32_t total_bytes = tx_bytes + rx_bytes;
	 int i = 0;
#ifdef _PERFORMANCE_MEASUREMENT_
	 uint32_t ts_dummy = 0;
	 uint32_t ts_start = 0;
	 uint32_t ts_stop = 0;
#endif
	 
	 //LOGD("vincent %s: tx_bytes: %d, rx_bytes: %d\n", __func__,tx_bytes,rx_bytes);
	 if (total_bytes >= 32)
		 spi_chip_config.com_mod = DMA_TRANSFER;
	 else
		 spi_chip_config.com_mod = FIFO_TRANSFER;
 //  LOGD("before drSpiSend(), size = %d\n", total_bytes);
#ifdef _PERFORMANCE_MEASUREMENT_
	 focal_test_get_timestamp(&ts_dummy, &ts_start);
#endif
	 tlRet = drSpiSend(tx, rx, total_bytes, &spi_chip_config, 1);
#ifdef _PERFORMANCE_MEASUREMENT_
	 focal_test_get_timestamp(&ts_dummy, &ts_stop);
	 if ((ts_stop - ts_start) > 500) LOGD("KPI focal_test_spi_writeread(), size = %d, tlRet= %d. time: %d\n", total_bytes, tlRet, (ts_stop - ts_start));
#endif
 //  LOGD("after drSpiSend(), size = %d\n", total_bytes);
	 //LOGD("focal_test: %s: ret of drSpiSend == %d\n",__func__,tlRet);
	 
	 return tlRet;
#else
	return 0;
#endif
 }
#endif

 // ------------------------------------------------------------------------------------------------
 int focal_test_spi_cs_low(void)
 {
	 //LOGD("%s\n", __func__);
	 return 0;
 }
 
 // ------------------------------------------------------------------------------------------------
 int focal_test_spi_cs_high(void)
 {
	 //LOGD("%s\n", __func__);
	 return 0;
 }
 
 // ------------------------------------------------------------------------------------------------
 int focal_test_hw_reset_high(void)
 {
	 LOGD("%s\n", __func__);
	 return 0;
 }
 
 // ------------------------------------------------------------------------------------------------
 int focal_test_hw_reset_low(void)
 {
	 LOGD("%s\n", __func__);
	 return 0;
 }
 
 // ------------------------------------------------------------------------------------------------
 #if 0
 int focal_test_delay_us(int usec)
 {
	 LOGD("%s\n", __func__);
	 usec = 0;
	 return 0;
 }
#endif


