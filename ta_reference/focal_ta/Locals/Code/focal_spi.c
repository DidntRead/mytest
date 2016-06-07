
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include "focal_log.h"
#include "focal_spi.h"
#include "tlStd.h"
#include "TlApi/TlApi.h"
//#include "drtlspi_api.h"

#include "tlspi_Api.h"
#include "drtlspi_api.h"

//vincent
static struct mt_chip_conf spi_chip_config;
static uint8_t tx_buf[16645];
static uint8_t rx_buf[16645];

//#define _PERFORMANCE_MEASUREMENT_

#if 1
void focal_spi_config(struct mt_chip_conf *chip_config)
{
	chip_config->setuptime = 15;
	chip_config->holdtime = 15;
	chip_config->high_time = 11;//Note this is OEM specific
	chip_config->low_time = 11;//Note this is OEM specific
	chip_config->cs_idletime = 20;
	chip_config->ulthgh_thrsh = 0;

	chip_config->cpol = 0;
	chip_config->cpha = 0;
	
	chip_config->rx_mlsb = 1; 
	chip_config->tx_mlsb = 1;

	chip_config->tx_endian = 0;
	chip_config->rx_endian = 0 ;

	chip_config->com_mod = FIFO_TRANSFER; //DMA_TRANSFER;//FIFO_TRANSFER;
	chip_config->pause = 0;
	chip_config->finish_intr = 1;
	chip_config->deassert = 0;
	chip_config->ulthigh = 0;
	chip_config->tckdly = 0;
}
#else
//old from htc
void focal_spi_config(struct mt_chip_conf *chip_config)
{
	chip_config->setuptime = 3;
	chip_config->holdtime = 3;
	chip_config->high_time = 5;
	chip_config->low_time = 5;
	chip_config->cs_idletime = 2;
	chip_config->ulthgh_thrsh = 0;

	chip_config->cpol = 0;
	chip_config->cpha = 0;
	
	chip_config->rx_mlsb = 1; 
	chip_config->tx_mlsb = 1;

	chip_config->tx_endian = 0;
	chip_config->rx_endian = 0 ;

	chip_config->com_mod = FIFO_TRANSFER; //DMA_TRANSFER;//FIFO_TRANSFER;
	chip_config->pause = 0;
	chip_config->finish_intr = 1;
	chip_config->deassert = 0;
	chip_config->ulthigh = 0;
	chip_config->tckdly = 0;
}
#endif

int focal_spi_capture_test(void)
{
	int i=0;//, irqstatus = 0;
	int ret =0;
	tciReturnCode_t tlRet;
	
	LOGE("vincent==> %s\n",__func__);
	
	focal_spi_init(0,0);
		
	//irqstatus = focal_sensor_ctrl_read_irq(true);
	//LOGD("vincent: %x: ret of drSpiSend == %d. irqstatus: %x\n",tx_buf[0],tlRet,irqstatus);
	memset(tx_buf,0,5);
	memset(rx_buf,0,5);
	tx_buf[0] = 0xfc;
	tlRet = drSpiSend( tx_buf, rx_buf, 3, &spi_chip_config, 1);
	
	LOGD("=========vincent: %x:========== ret of drSpiSend == %d\n",tx_buf[0],tlRet);
	
	for(i=0;i<3;i++) {
		LOGD("vincent---> tx_buf[%d] = %x, rx_buf[%d] == %x\n",i, tx_buf[i], i,rx_buf[i]);
	}

	memset(tx_buf,0,10);
	memset(rx_buf,0,20);
	tx_buf[0] = 0xf8;
	tlRet = drSpiSend( tx_buf, rx_buf, 1, &spi_chip_config, 1);
	for(i=0;i<1;i++) {
		LOGD("vincent---> tx_buf[%d] = %x, rx_buf[%d] == %x\n",i, tx_buf[i], i,rx_buf[i]);
	}
	
	memset(tx_buf,0,10);
	memset(rx_buf,0,20);
	tx_buf[0] = 0x1c;
	tlRet = drSpiSend( tx_buf, rx_buf, 2, &spi_chip_config, 1);
	LOGD("=========vincent: %x:========= ret of drSpiSend == %d\n",tx_buf[0],tlRet);
	for (i=0;i<2;i++) LOGD("command after soft reset tx_buf[%d] = %x: rx_buf[%d] == %x\n",i,tx_buf[i],i,rx_buf[i]);

	focal_spi_chipcfg();

	memset(tx_buf,0,10);
	memset(rx_buf,0,20);
	tx_buf[0] = 0xc0;
	tlRet = drSpiSend( tx_buf, rx_buf, 1, &spi_chip_config, 1);
	LOGD("vincent: %x: ret of drSpiSend == %d\n",tx_buf[0],tlRet);

	memset(tx_buf,0,10);
	memset(rx_buf,0,20);
	tx_buf[0] = 0x1c;
	tlRet = drSpiSend( tx_buf, rx_buf, 2, &spi_chip_config, 1);
	LOGD("vincent: %x: ret of drSpiSend == %d\n",tx_buf[0],tlRet);
	for (i=0;i<2;i++) LOGD("command tx_buf[%d] = %x: rx_buf[%d] == %x\n",i,tx_buf[i],i,rx_buf[i]);
	
	/*
	ret = focal_sensor_start_capture();
	LOGD("vincent: focal_sensor_start_capture ret: %x\n",ret);
	*/
	/*irqstatus = focal_sensor_ctrl_read_irq(true);
	LOGD("vincent: %x: ret of drSpiSend == %d. irqstatus: %x\n",tx_buf[0],tlRet,irqstatus);
*/

	memset(tx_buf,0,16642);
	memset(rx_buf,0,16642);
	tx_buf[0] = 0xc4;
	spi_chip_config.com_mod = DMA_TRANSFER;
	LOGD("=========vincent: %x:========= ret of drSpiSend == %d\n",tx_buf[0],tlRet);
	LOGD("spi_chip_config.com_mod = %d, spi_chip_config.pause = %d\n", spi_chip_config.com_mod, spi_chip_config.pause);
	tlRet = drSpiSend( tx_buf, rx_buf, 16642, &spi_chip_config, 1);
	LOGD("vincent: %x: ret of drSpiSend == %d\n",tx_buf[0],tlRet);

	for (i=0;i<100;i++) LOGD("command for dma transfer %x: rx_buf[%d] == %x\n",tx_buf[0],i,rx_buf[i]);
	
	focal_lib_debug_inject_image(&rx_buf[2], 16640);
	
	/*
	irqstatus = focal_sensor_ctrl_read_irq(true);
	LOGD("vincent: %x: ret of drSpiSend == %d. irqstatus: %x\n",tx_buf[0],tlRet,irqstatus);
	*/
	memset(tx_buf,0,10);
	memset(rx_buf,0,20);
	tx_buf[0] = 0x1c;
	tlRet = drSpiSend( tx_buf, rx_buf, 2, &spi_chip_config, 1);
	LOGD("vincent: %x: ret of drSpiSend == %d\n",tx_buf[0],tlRet);
	
	return ret;
}

void focal_spi_chipcfg(void)
{
	int i = 0;
	tlApiResult_t tlRet;
	
	memset(tx_buf,0,10);
	memset(rx_buf,0,10);
	
	tx_buf[0] = 0x90;
	tx_buf[1] = 0x09;
	tlRet = drSpiSend( tx_buf, rx_buf, 2, &spi_chip_config, 1);
	//LOGD("=========vincent: %x:========= ret of drSpiSend == %d\n",tx_buf[0],tlRet);
	for(i=0;i<2;i++) {
		LOGD("vincent---> tx_buf[%d] = %x, rx_buf[%d] == %x\n",i, tx_buf[i], i,rx_buf[i]);
	}
	
	memset(tx_buf,0,10);
	memset(rx_buf,0,10);
	tx_buf[0] = 0x68;
	tx_buf[1] = 0x08;
	tx_buf[2] = 0x08;
	tx_buf[3] = 0x08;
	tx_buf[4] = 0x08;
	tx_buf[5] = 0x14;
	tx_buf[6] = 0x14;
	tx_buf[7] = 0x14;
	tx_buf[8] = 0x14;
	tlRet = drSpiSend( tx_buf, rx_buf, 9, &spi_chip_config, 1);

	LOGD("=========vincent: %x:========= ret of drSpiSend == %d\n",tx_buf[0],tlRet);
	for(i=0;i<9;i++) {
		LOGD("vincent---> tx_buf[%d] = %x, rx_buf[%d] == %x\n",i, tx_buf[i], i,rx_buf[i]);
	}
	
	memset(tx_buf,0,10);
	memset(rx_buf,0,10);
	tx_buf[0] = 0x8c;
	tx_buf[1] = 0x02;
	tlRet = drSpiSend( tx_buf, rx_buf, 2, &spi_chip_config, 1);
	LOGD("=========vincent: %x:========= ret of drSpiSend == %d\n",tx_buf[0],tlRet);
	for(i=0;i<2;i++) {
		LOGD("vincent---> tx_buf[%d] = %x, rx_buf[%d] == %x\n",i, tx_buf[i], i,rx_buf[i]);
	}
	
	memset(tx_buf,0,10);
	memset(rx_buf,0,10);
	
	tx_buf[0] = 0xa0;
	//tx_buf[1] = 17;
	//tx_buf[2] = 0;
	tx_buf[1] = 0xa;
	tx_buf[2] = 0x2;
	tlRet = drSpiSend( tx_buf, rx_buf, 3, &spi_chip_config, 1);
	LOGD("=========vincent: %x:========= ret of drSpiSend == %d\n",tx_buf[0],tlRet);
	for(i=0;i<3;i++) {
		LOGD("vincent---> tx_buf[%d] = %x, rx_buf[%d] == %x\n",i, tx_buf[i], i,rx_buf[i]);
	}
	
	memset(tx_buf,0,10);
	memset(rx_buf,0,10);
	tx_buf[0] = 0xa8;
	tx_buf[1] = 0x0f;
	tx_buf[2] = 0x1a;
	//tx_buf[2] = 0x0a;
	tlRet = drSpiSend( tx_buf, rx_buf, 3, &spi_chip_config, 1);
	LOGD("=========vincent: %x:========= ret of drSpiSend == %d\n",tx_buf[0],tlRet);
	for(i=0;i<3;i++) {
		LOGD("vincent---> tx_buf[%d] = %x, rx_buf[%d] == %x\n",i, tx_buf[i], i,rx_buf[i]);
	}
	
	memset(tx_buf,0,10);
	memset(rx_buf,0,10);
	tx_buf[0] = 0x5c;
	tx_buf[1] = 0x0b;
	tlRet = drSpiSend( tx_buf, rx_buf, 2, &spi_chip_config, 1);
	LOGD("=========vincent: %x:========= ret of drSpiSend == %d\n",tx_buf[0],tlRet);
	for(i=0;i<2;i++) {
		LOGD("vincent---> tx_buf[%d] = %x, rx_buf[%d] == %x\n",i, tx_buf[i], i,rx_buf[i]);
	}
	
	memset(tx_buf,0,10);
	memset(rx_buf,0,10);
	tx_buf[0] = 0xd8;
	tx_buf[1] = 0;
	tx_buf[2] = 0x01;
	tx_buf[3] = 32;
	tx_buf[4] = 32;
	for(i=0;i<5;i++) {
		LOGD("vincent---> tx_buf[%d] = %x, rx_buf[%d] == %x\n",i, tx_buf[i], i,rx_buf[i]);
	}
	tlRet = drSpiSend( tx_buf, rx_buf, 5, &spi_chip_config, 1);

	LOGD("=========vincent: %x:========= ret of drSpiSend == %d\n",tx_buf[0],tlRet);
	for(i=0;i<5;i++) {
		LOGD("vincent---> tx_buf[%d] = %x, rx_buf[%d] == %x\n",i, tx_buf[i], i,rx_buf[i]);
	}

}

int focal_spi_test(void)
{
	int ret = 0;
	int i = 0;
	tlApiResult_t tlRet;

	LOGD("hslu--> %s\n",__func__);
	
	focal_spi_init(0,0);
	
	memset(tx_buf,0,5);
	memset(rx_buf,0,5);
	tx_buf[0] = 0xfc;
	tlRet = drSpiSend( tx_buf, rx_buf, 3, &spi_chip_config, 1);
	
	LOGD("=========vincent: %x:========== ret of drSpiSend == %d\n",tx_buf[0],tlRet);
	
	for(i=0;i<3;i++) {
		LOGD("vincent---> tx_buf[%d] = %x, rx_buf[%d] == %x\n",i, tx_buf[i], i,rx_buf[i]);
	}
	
	memset(tx_buf,0,10);
	memset(rx_buf,0,20);
	tx_buf[0] = 0xf8;
	tlRet = drSpiSend( tx_buf, rx_buf, 1, &spi_chip_config, 1);
	for(i=0;i<1;i++) {
		LOGD("vincent---> tx_buf[%d] = %x, rx_buf[%d] == %x\n",i, tx_buf[i], i,rx_buf[i]);
	}
	
	memset(tx_buf,0,10);
	memset(rx_buf,0,20);
	tx_buf[0] = 0x1c;
	tlRet = drSpiSend( tx_buf, rx_buf, 2, &spi_chip_config, 1);
	LOGD("=========vincent: %x:========= ret of drSpiSend == %d\n",tx_buf[0],tlRet);
	for (i=0;i<2;i++) LOGD("command after soft reset tx_buf[%d] = %x: rx_buf[%d] == %x\n",i,tx_buf[i],i,rx_buf[i]);

	focal_spi_chipcfg();

	memset(tx_buf,0,10);
	memset(rx_buf,0,20);
	tx_buf[0] = 0x20;
	tlRet = drSpiSend( tx_buf, rx_buf, 1, &spi_chip_config, 1);
	for(i=0;i<1;i++) {
		LOGD("vincent---> tx_buf[%d] = %x, rx_buf[%d] == %x\n",i, tx_buf[i], i,rx_buf[i]);
	}
	

	memset(tx_buf,0,10);
	memset(rx_buf,0,20);
	tx_buf[0] = 0x1c;
	tlRet = drSpiSend( tx_buf, rx_buf, 2, &spi_chip_config, 1);
	LOGD("=========vincent: %x:========= ret of drSpiSend == %d\n",tx_buf[0],tlRet);
	for (i=0;i<2;i++) LOGD("command1 tx_buf[%d] = %x: rx_buf[%d] == %x\n",i,tx_buf[i],i,rx_buf[i]);

	memset(tx_buf,0,10);
	memset(rx_buf,0,10);
	tx_buf[0] = 0xd4;
	tlRet = drSpiSend( tx_buf, rx_buf, 3, &spi_chip_config, 1);
	LOGD("=========vincent: %x:========= ret of drSpiSend == %d\n",tx_buf[0],tlRet);
	for (i=0;i<3;i++) LOGD("command tx_buf[%d] = %x: rx_buf[%d] == %x\n",i,tx_buf[i],i,rx_buf[i]);
#if 0
	memset(tx_buf,0,10);
	memset(rx_buf,0,20);
	tx_buf[0] = 0x1c;
	tlRet = drSpiSend( tx_buf, rx_buf, 2, &spi_chip_config, 1);
	LOGD("=========vincent: %x:========= ret of drSpiSend == %d\n",tx_buf[0],tlRet);
	for (i=0;i<2;i++) LOGD("command2 tx_buf[%d] = %x: rx_buf[%d] == %x\n",i,tx_buf[i],i,rx_buf[i]);
#endif
	//focal_spi_capture_test();

	return ret;
}

void focal_read_int_loop(void)
{
	int i = 0, j = 0;
	tlApiResult_t tlRet;
	
	for (i=0;i<10;i++) {
		memset(tx_buf,0,20);
		memset(rx_buf,0,20);
			
		tx_buf[0] = 0x1c;
		
		tlRet = drSpiSend( tx_buf, rx_buf, 2, &spi_chip_config, 1);
		LOGD("=========vincent: %x:========= ret of drSpiSend == %d\n",tx_buf[0],tlRet);
		for (j=0;j<3;j++) LOGD("command %x: rx_buf[%d] == %x\n",tx_buf[0],j,rx_buf[j]);
	}
	
}

#if 0
int focal_spi_test(void)
{
	int ret = 0;
	
	int i = 0,z=0, irqstatus = 0;
	struct timespec ts, ts1;
     
    ts.tv_nsec = 500000000;    // 1500ms
    ts.tv_sec = 1;
    
    
        
	//uint8_t data_buf[208*80+1];
		
	tlApiResult_t tlRet;
	//tlDbgPrintLnf("vincent %s\n",__func__);
	LOGD("vincent--> %s\n",__func__);
	
	focal_spi_init(0,0);
	
	//drSpiHwConfig(NULL);
	//drSpiDebug();
	memset(tx_buf,0,5);
	memset(rx_buf,0,5);
	//tx_buf[0] = 0x1c;
	//drSpiSend( tx_buf, rx_buf, 2, NULL, 1);
			
	//memset(rx_buf,0,20);
	/*
	tx_buf[0] = 248;
	tlRet = drSpiSend( tx_buf, rx_buf, 1, &spi_chip_config, 1);
	LOGD("vincent: cmd %x",tx_buf[0]);
	*/
	
	tx_buf[0] = 0xfc;
	        
	tlRet = drSpiSend( tx_buf, rx_buf, 3, &spi_chip_config, 1);
	
	LOGD("=========vincent: %x:========== ret of drSpiSend == %d\n",tx_buf[0],tlRet);
	
	for(i=0;i<5;i++) {
		LOGD("vincent---> rx_buf[%d] == %x\n",i,rx_buf[i]);
	}
	
	memset(rx_buf,0,20);
	tx_buf[0] = 104;
	tlRet = drSpiSend( tx_buf, rx_buf, 1, &spi_chip_config, 1);
	LOGD("=========vincent: %x:========= ret of drSpiSend == %d\n",tx_buf[0],tlRet);
	
	memset(rx_buf,0,20);
	tx_buf[0] = 104;
	tx_buf[1] = 0x10;
	tx_buf[2] = 0x20;
	tx_buf[3] = 0x30;
	tx_buf[4] = 0x40;
	tx_buf[5] = 0x50;
	tx_buf[6] = 0x60;
	tx_buf[7] = 0x70;
	tx_buf[8] = 0x80;
	tlRet = drSpiSend( tx_buf, rx_buf, 9, &spi_chip_config, 1);
	LOGD("=========vincent: %x:========= ret of drSpiSend == %d\n",tx_buf[0],tlRet);
	/*for(i=0;i<10;i++) {
		LOGD("vincent---> rx_buf[%d] == %x\n",i,rx_buf[i]);
	}*/
	
/*		
	memset(tx_buf,0,20);
	
	tx_buf[0] = 104;
	tlRet = drSpiSend( tx_buf, rx_buf, 9, &spi_chip_config, 1);
	LOGD("=========vincent: %x:========= ret of drSpiSend == %d\n",tx_buf[0],tlRet);
	for(i=0;i<10;i++) {
		LOGD("vincent---> rx_buf[%d] == %x\n",i,rx_buf[i]);
	}
	*/
	focal_spi_chipcfg();
	//focal_read_int_loop();

	memset(rx_buf,0,10);
	tx_buf[0] = 0x1c;
		tlRet = drSpiSend( tx_buf, rx_buf, 2, &spi_chip_config, 1);
		LOGD("=========vincent: %x:========= ret of drSpiSend == %d\n",tx_buf[0],tlRet);
		for (i=0;i<5;i++) LOGD("command %x: rx_buf[%d] == %x\n",tx_buf[0],i,rx_buf[i]);
	
	
	memset(rx_buf,0,10);
	tx_buf[0] = 0x20;
	tlRet = drSpiSend( tx_buf, rx_buf, 1, &spi_chip_config, 1);
	LOGD("=========vincent: %x=========: ret of drSpiSend == %d\n",tx_buf[0],tlRet);
		
	memset(rx_buf,0,10);
	tx_buf[0] = 0x1c;
		tlRet = drSpiSend( tx_buf, rx_buf, 2, &spi_chip_config, 1);
		LOGD("=========vincent: %x:========= ret of drSpiSend == %d\n",tx_buf[0],tlRet);
		for (i=0;i<5;i++) LOGD("command %x: rx_buf[%d] == %x\n",tx_buf[0],i,rx_buf[i]);
	
	memset(rx_buf,0,10);
	tx_buf[0] = 0xd4;
	tlRet = drSpiSend( tx_buf, rx_buf, 3, &spi_chip_config, 1);
	LOGD("=========vincent: %x:========= ret of drSpiSend == %d\n",tx_buf[0],tlRet);
	for (i=0;i<5;i++) LOGD("command %x: rx_buf[%d] == %x\n",tx_buf[0],i,rx_buf[i]);
	
	memset(rx_buf,0,10);
	tx_buf[0] = 0x1c;
		tlRet = drSpiSend( tx_buf, rx_buf, 2, &spi_chip_config, 1);
		LOGD("=========vincent: %x:========= ret of drSpiSend == %d\n",tx_buf[0],tlRet);
		for (i=0;i<5;i++) LOGD("command %x: rx_buf[%d] == %x\n",tx_buf[0],i,rx_buf[i]);
		
		
	/*
	for (z=0;z<5;z++){
		memset(rx_buf,0,5);
		
		tx_buf[0] = 0x20;
		tlRet = drSpiSend( tx_buf, rx_buf, 1, &spi_chip_config, 1);
		LOGD("vincent: %x: ret of drSpiSend == %d\n",tx_buf[0],tlRet);
		
		tx_buf[0] = 0xd4;
		tlRet = drSpiSend( tx_buf, rx_buf, 3, &spi_chip_config, 1);
		LOGD("vincent: %x: ret of drSpiSend == %d\n",tx_buf[0],tlRet);
		for (i=0;i<5;i++) LOGD("command %x: rx_buf[%d] == %x\n",tx_buf[0],i,rx_buf[i]);
		
		memset(rx_buf,0,5);
		
		tx_buf[0] = 0x1c;
		tlRet = drSpiSend( tx_buf, rx_buf, 2, &spi_chip_config, 1);
		LOGD("vincent: %x: ret of drSpiSend == %d\n",tx_buf[0],tlRet);
		for (i=0;i<5;i++) LOGD("command %x: rx_buf[%d] == %x\n",tx_buf[0],i,rx_buf[i]);
		
	}*/
	

	return ret;
}
#endif

// ------------------------------------------------------------------------------------------------
int focal_spi_init(uint32_t freq_low_khz, uint32_t freq_high_khz)
{	
	LOGD("MTK TZ spi: %s\n", __func__);
	focal_spi_config(&spi_chip_config);
	
	return 0;
}

// ------------------------------------------------------------------------------------------------
int focal_spi_destroy(void)
{
	LOGD("%s\n", __func__);
	return 0;
}

// ------------------------------------------------------------------------------------------------
int focal_spi_select_low_freq(void)
{
	//LOGD("%s\n", __func__);
	return 0;
}

// ------------------------------------------------------------------------------------------------
int focal_spi_select_high_freq(void)
{
	//LOGD("%s\n", __func__);
	return 0;
}

// ------------------------------------------------------------------------------------------------
int focal_spi_read(uint8_t *buff, size_t num_bytes)
{
	LOGD("%s\n", __func__);
	return 0;
}

// ------------------------------------------------------------------------------------------------
int focal_spi_write(uint8_t *buff, size_t num_bytes)
{
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
	focal_get_timestamp(&ts_dummy, &ts_start);
#endif
	tlRet = drSpiSend(buff, buff, num_bytes, &spi_chip_config, 1);
#ifdef _PERFORMANCE_MEASUREMENT_
	focal_get_timestamp(&ts_dummy, &ts_stop);
	if ((ts_stop - ts_start) > 500) LOGD("KPI focal_spi_write(), size = %d, tlRet= %d. time: %d\n", num_bytes, tlRet, (ts_stop - ts_start));
#endif
	//LOGD("after drSpiSend(), size = %d, tlRet= %d\n", num_bytes, tlRet);

	return tlRet;
}

// ------------------------------------------------------------------------------------------------
int focal_spi_writeread(uint8_t *tx, size_t tx_bytes, uint8_t *rx, size_t rx_bytes)
{
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
//	LOGD("before drSpiSend(), size = %d\n", total_bytes);
#ifdef _PERFORMANCE_MEASUREMENT_
	focal_get_timestamp(&ts_dummy, &ts_start);
#endif
	tlRet = drSpiSend(tx, rx, total_bytes, &spi_chip_config, 1);
#ifdef _PERFORMANCE_MEASUREMENT_
	focal_get_timestamp(&ts_dummy, &ts_stop);
	if ((ts_stop - ts_start) > 500) LOGD("KPI focal_spi_writeread(), size = %d, tlRet= %d. time: %d\n", total_bytes, tlRet, (ts_stop - ts_start));
#endif
//	LOGD("after drSpiSend(), size = %d\n", total_bytes);
#if 0
	//rx byte shift
	for(i=0; i<rx_bytes; i++) {
		//LOGD("rx[%d] == %x, rx[%d] == %x\n",i,rx[i],i+1,rx[i+1]);
		//rx[i] = rx[i+tx_bytes]; //origin
		//rx[tx_bytes+i] = rx[i+tx_bytes];
		LOGD("vincent---> rx[%d] == %x\n",i,rx[i]);
	}
	#endif
	//LOGD("FOCAL: %s: ret of drSpiSend == %d\n",__func__,tlRet);
	
	return tlRet;
}

// ------------------------------------------------------------------------------------------------
int focal_spi_cs_low(void)
{
	//LOGD("%s\n", __func__);
	return 0;
}

// ------------------------------------------------------------------------------------------------
int focal_spi_cs_high(void)
{
	//LOGD("%s\n", __func__);
	return 0;
}

// ------------------------------------------------------------------------------------------------
int focal_hw_reset_high(void)
{
	LOGD("%s\n", __func__);
	return 0;
}

// ------------------------------------------------------------------------------------------------
int focal_hw_reset_low(void)
{
	LOGD("%s\n", __func__);
	return 0;
}

// ------------------------------------------------------------------------------------------------
int focal_delay_us(int usec)
{
	LOGD("%s\n", __func__);
	return 0;
}

// ------------------------------------------------------------------------------------------------
