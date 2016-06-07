#include <unistd.h>
//#include <fcntl.h>
#include <stdarg.h>

#include "focal_common.h"
#include "focal_sensor.h"
#include "focal_fp_afe_scan_param.h"
#include "focal_fp_spi.h"
#include "focal_registerlist.h"
#include "focal_log.h"

#define TAG "focal_sensor"

#define FOCAL_SENSOR_DEBUG
#ifdef FOCAL_SENSOR_DEBUG
#define FT_DBG(fmt, args...)	LOGD("[focal_sensor]"fmt, ##args);
#else
#define FT_DBG(fmt, args...) 	do{}while(0)
#endif


#define SLEEPTIME		(20 * 1000)
/******************************************************************************/
struct FpAfeScanParamStru g_FpAfeScanParam = {
	.m_macroblock_scan_time = 4,
	.m_agc_param = {
		.PagcGn = {
			.GAIN = {
				.afe_cds_gn_sel = 0,
				.adc_gn_sel = 0,
				.afe_s2_gn_sel = 0,
				.afe_s1_gn_sel = 6,
			},
		},
		.dac1_base = 0x200,
		.dac2_gray = 0x200,
	},
};

enum FP_DRV_VOLTAGE_TYPE {
	DRV_VOLTAGE_LOW = 0,//3v
	DRV_VOLTAGE_MID = 1,//5v
	DRV_VOLTAGE_HIGH = 2,//9v
};

static enum FP_DRV_VOLTAGE_TYPE drv_voltage_type = DRV_VOLTAGE_LOW;

#define FP_IMAGE_START_ADDRESS	0x1C00
#define MAX_VALUE_RATIO 	14
#define MID_RATIO_F			0.05

#define FP_SENSOR_WIDTH_MAX 		160
#define FP_SENSOR_HEIGHT_MAX		160

static short m_DAC_OFFSET;
static short m_DAC_RATIO;
static unsigned short m_HistBuf[0x400];
static unsigned short m_ImageBuf[FP_SENSOR_WIDTH_MAX * FP_SENSOR_HEIGHT_MAX];

extern unsigned short g_sensor_width;
extern unsigned short g_sensor_height;

static void focal_usleep(int ms)
{
	int i = 0, j = 0;

	for (i = 0; i < ms; i++)
	{
		while(j < 35000)
		{
	    		j++;
		}
	}
}
 
static unsigned char ReadSfrRegister(unsigned char RegAddr, unsigned char *pRegValue)
{
	int err = focal_fp_sensor_read_sfr_register(RegAddr, pRegValue);

	if(err < 0)
	{
		return RET_ERROR_RW;
	}
	return RET_OK;
}

static unsigned char WriteSfrRegister(unsigned char RegAddr, unsigned char RegValue)
{
	int err = focal_fp_sensor_write_sfr_register(RegAddr, RegValue);

	if(err < 0)
	{
		return RET_ERROR_RW;
	}
	return RET_OK;
}

static unsigned char ReadSRamRegister(unsigned short RegAddr, unsigned short *pRegValue)
{
	int err = focal_fp_sensor_read_sram_register(RegAddr, pRegValue);

	if(err < 0)
	{
		return RET_ERROR_RW;
	}
	return RET_OK;
}

static unsigned char WriteSRamRegister(unsigned short RegAddr, unsigned short RegValue)
{
	int err = focal_fp_sensor_write_sram_register(RegAddr, RegValue);

	if(err < 0)
	{
		return RET_ERROR_RW;
	}
	return RET_OK;
}

static unsigned char SRamWriteBit(unsigned short startAddr, ...)
{
	va_list argp;
	unsigned short value = 0;
	char flag = 0, bit = 0;
	int para = 0, sum = 0, argno = 1;

	va_start(argp, startAddr); //\BF杀\E4\B2\CE\CA\FD\CE薹\A8\B7\E2装
	while (1) {
		para = va_arg(argp, int);
		if (0 == argno % 2) {
			value = value << bit;
			value |= para;
		} else {
			bit = para;
			sum += bit;
		}
		argno++;
		if (1 == flag)
			break;
		if (argno > 32) {
			return RET_ERROR_PARAM;
		}
		if (16 == sum) {
			flag = 1;
		}
	}
	va_end(argp);

	WriteSRamRegister(startAddr, value);

	return RET_OK;
}

static unsigned char SRamBulkReadData(unsigned short startAddr, unsigned short *pReadBuf, unsigned short ReadLen)
{
	return focal_fp_sensor_readbulk_sram(startAddr, pReadBuf, ReadLen);
}

static void Convert2FpImage(unsigned short *psrcImgData, int srclen, unsigned char *pDstImgData)
{
	unsigned short datavalue = 0x00;
	int i = 0, j = 0;

	for(i = 0; i < srclen; i++)
	{
		datavalue = psrcImgData[i];

		pDstImgData[j] = (unsigned char)((datavalue>>2) / g_FpAfeScanParam.m_macroblock_scan_time);

		j++;
	}
}

static int GetFrameData(unsigned char *pHistData)
{
	int err = 0;
	unsigned short value = 0x00;
	unsigned short Timecnt = 0;

	////////////////////////////////////////////////////////////////////
    ////////////////////////  鍏ㄥ浘閲囨牱  ///////////////////////////////
    ////////////////////////////////////////////////////////////////////

	err = SRamWriteBit(0x1834, 0x03, 0x00,
							 0x02, 0x00,
							 0x01, 0x00,
							 0x01, 0x00,
							 0x01, 0x00,
							 0x01, 0x00,
							 0x01, 0x00,
							 0x01, 0x00,
							 0x02, 0x00,
							 0x01, 0x01,
							 0x01, 0x00,
							 0x01, 0x00);
	if(err != 0)
		return err;

	err = SRamWriteBit(REG_dac1_base, 0x06, 0x00, 0x0A, g_FpAfeScanParam.m_agc_param.dac1_base);//6,0, A,80                        dac1_base
	if(err != 0)
		return err;

	err = SRamWriteBit(REG_AFE_CONTROL_BASE, 0x03, 0x00, 0x05, 0x00, 0x08, 0x00);  //sensor_addr0_start 5:璧峰鍒�, 8:璧峰琛�
	if(err != 0)
		return err;

	err = SRamWriteBit(REG_sensor_addr0_end, 0x03, 0x00, 0x05,
			g_sensor_width / 8 - 1, 0x08, g_sensor_height - 1);  //sensor_addr0_end
	if(err != 0)
		return err;


	err = SRamWriteBit(REG_pagc_gn, 0x07, 0x00,
			0x02, g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.afe_cds_gn_sel,
			0x02, g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.adc_gn_sel,
			0x02, g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.afe_s2_gn_sel,
			0x03, g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.afe_s1_gn_sel); //for virtual touch                   pagc_gn

	if(err != 0)
		return err;


	err = SRamWriteBit(REG_dac2_gray, 0x06, 0x00, 0x0A, g_FpAfeScanParam.m_agc_param.dac2_gray);
	if(err != 0)
		return err;

	err = SRamWriteBit(REG_macroblock_scan_time, 0x08, 0x00, 0x08, 0x04);
	if(err != 0)
		return err;

	err = SRamWriteBit(REG_afe_ctrl_reg, 0x0D, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01);    //afe_ctrl_reg
	if(err != 0)
		return err;

	while(1)
	{
		err = ReadSRamRegister(0x1a04, &value); // 鏌ヨraw_flag鏄惁涓�1锛�1琛ㄧず鎵弿缁撴潫
		if(err != 0)
			return err;

		if (1 == (value&0x1))
		{
			WriteSRamRegister(0x1a04, 1);	//灏唕aw_flag娓�0
			break;
		}
		else
		{
			focal_usleep(4000);
			Timecnt++;
			if(Timecnt > 100)
				return RET_ERROR;
		}
	}
	memset(m_ImageBuf, 0, sizeof(m_ImageBuf));
	err = SRamBulkReadData(FP_IMAGE_START_ADDRESS, m_ImageBuf, g_sensor_width * g_sensor_height);
	if(0 == err)
	{
		Convert2FpImage(m_ImageBuf, g_sensor_width * g_sensor_height, pHistData);
	}
//	int i;
//	for (i = 0; i < 3000; i += 176)
//	{
//		LOGD("_yanj_%d %d %d %d %d %d %d %d %d %d \n", m_ImageBuf[i], m_ImageBuf[i + 1], m_ImageBuf[i + 2], m_ImageBuf[i + 3],
//				m_ImageBuf[i + 4], m_ImageBuf[i + 5], m_ImageBuf[i + 6], m_ImageBuf[i + 7], m_ImageBuf[i + 8], m_ImageBuf[i + 9],
//				m_ImageBuf[i + 10]);
//	}

	return RET_OK;//1013,add

}

static unsigned char focal_abs(unsigned char val)
{
	if (val < 0)
		return -val;
	
	return val;
}

static short GetDACOFFSET(void)
{
	//unsigned short datavalue = 0x00;
	int i = 0;
	unsigned char FpImagebuf[g_sensor_width * g_sensor_height];
	unsigned short h[4][0x100];
	unsigned int sum_0,sum_1,sum_Value;
	unsigned char ucMaxValue,ucMinValue,ucAverage;

	memset(h, 0, sizeof(h));

	GetFrameData(FpImagebuf);

	sum_0 = 0;
	sum_1 = 0;
	sum_Value = 0;

	ucMaxValue = 0x00;
	ucMinValue = 0xff;

	for ( i = 0; i < 0x400; i++)
	{
		sum_0 += i;
		sum_1 += i*FpImagebuf[i];
		sum_Value += FpImagebuf[i];

		if (ucMaxValue < FpImagebuf[i])
		{
			ucMaxValue = FpImagebuf[i];
		}

		if (ucMinValue > FpImagebuf[i])
		{
			ucMinValue = FpImagebuf[i];
		}
	}

	ucAverage = sum_Value / 0x400;

	if ((focal_abs(ucAverage-ucMaxValue) < 6) || (focal_abs(ucAverage-ucMinValue) < 6))
	{
		return (unsigned short)(sum_1/sum_0);
	}
	else
	{
		return 0xff;
	}
	return 0;
}

static void RestoreDefaultValue(void)
{
	g_FpAfeScanParam.m_macroblock_scan_time = 4;
	//m_scan_mode
	g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.afe_s1_gn_sel = 6;//agc1
	g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.afe_s2_gn_sel = 0;//2;//agc2
	g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.afe_cds_gn_sel = 0;	//agc3
	g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.adc_gn_sel = 0;	//agc4

	g_FpAfeScanParam.m_agc_param.dac1_base = 0x200;
	g_FpAfeScanParam.m_agc_param.dac2_gray = 0x200;
}

unsigned short g_h[4][0x401];
static int GetSensor4Histogram(void)
{

	int err = 0, i = 0;
	unsigned short value = 0x00;
	unsigned short Timecnt = 0;
	
	memset(g_h, 0, sizeof(g_h));

	err = SRamWriteBit(REG_macroblock_scan_time, 0x08, 0x00, 0x08, 0x01);  //浣�8浣嶄负閲囨牱娆℃暟
	if(err != 0)
		return err;

	err = SRamWriteBit(0x1834, 0x03, 0x00,
							 0x02, 0x00,
							 0x01, 0x00,
							 0x01, 0x00,
							 0x01, 0x00,
							 0x01, 0x00,
							 0x01, 0x00,
							 0x01, 0x00,
							 0x02, 0x00,
							 0x01, 0x01,
							 0x01, 0x00,
							 0x01, 0x00);
	if(err != 0)
		return err;

	err = SRamWriteBit(REG_pagc_gn, 0x07, 0x00,
		0x02, 0x00, //g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.adc_gn_sel,
		0x02, 0x00, //g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.afe_cds_gn_sel,
		0x02, 0x00, //g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.afe_s2_gn_sel,
		0x03, 0x06); //g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.afe_s1_gn_sel); //for virtual touch                   pagc_gn
	if(err != 0)
		return err;

	//g_FpAfeScanParam.m_agc_param.dac1_base = 0x2a0;
	err = SRamWriteBit(REG_dac1_base, 0x06, 0x00, 0x0A, g_FpAfeScanParam.m_agc_param.dac1_base);//6,0, A,80                        dac1_base
	if(err != 0)
		return err;

	//g_FpAfeScanParam.m_agc_param.dac2_gray = 0x100;
	err = SRamWriteBit(REG_dac2_gray, 0x06, 0x00, 0x0A, g_FpAfeScanParam.m_agc_param.dac2_gray);//6,0, A,80
	if(err != 0)
		return err;

	SRamWriteBit(0x1A00, 0x08, g_FpAfeScanParam.m_macroblock_scan_time, 0x08, 0x00);
	//SRamWriteBit(0x1A00, 0x08, 1, 0x08, 0x00);
	//SRamWriteBit(0x1821, 0x08, 0x00, 0x08, 0x01);

	///////////////////////绗竴鍧楅噰鏍�///////////////////////////////
	//AFE Image Scanning Start

	SRamWriteBit(0x1800, 0x03, 0x00, 0x05, 0x00, 0x08, 0x00);		// sensor_addr0_start 5锛氳捣濮嬪垪锛�8锛氳捣濮嬭
	SRamWriteBit(0x1810, 0x03, 0x00, 0x05, g_sensor_width / 8 / 2 - 1, 0x08, g_sensor_height / 2 - 1);			// sensor_addr0_end 5锛氱粨鏉熷垪锛�8锛氱粨鏉熻

	err = SRamWriteBit(REG_afe_ctrl_reg, 0x0D, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01);    //afe_ctrl_reg
	if(err != 0)
		return err;

	while(1)
	{
		err = ReadSRamRegister(0x1a04, &value); // 鏌ヨraw_flag鏄惁涓�1锛�1琛ㄧず鎵弿缁撴潫
		if(err != 0)
			return err;

		if (1 == (value&0x1))
		{
			WriteSRamRegister(0x1a04, 1);	//灏唕aw_flag娓�0
			break;
		}
		else
		{
			focal_usleep(4000);
			Timecnt++;
			if(Timecnt > 100)
				return RET_ERROR;
		}
	}

	//read image data
	memset(m_HistBuf, 0, sizeof(m_HistBuf));
	err = SRamBulkReadData(0x0000, m_HistBuf, 0x400);
	if(0 == err)
	{
		for(i = 0; i < 0x400; i++)
		{
			g_h[0][i] = m_HistBuf[i];
		}
	}

	//AFE Image Scanning End and close
	//err = SRamWriteBit(REG_afe_ctrl_reg, 0x0D, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00);    //afe_ctrl_reg
	//if(err != 0)
	//	return err;

	//Sleep(10);

	///////////////////////绗簩鍧楅噰鏍�///////////////////////////////
	//AFE Image Scanning Start
	//鐩存柟鍥惧尯鍩熸竻闆�   % afe_adj_reg
	SRamWriteBit(0x1834, 0x03, 0x00, 0x02, 0x00,
		0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
		0x01, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x01,
		0x01, 0x00, 0x01, 0x00);

	SRamWriteBit(0x1800, 0x03, 0x00, 0x05, 0x00, 0x08, /*0x50*/g_sensor_height / 2);		// sensor_addr0_start 5锛氳捣濮嬪垪锛�8锛氳捣濮嬭
	SRamWriteBit(0x1810, 0x03, 0x00, 0x05, g_sensor_width / 8 / 2 - 1, 0x08, g_sensor_height - 1);			// sensor_addr0_end 5锛氱粨鏉熷垪锛�8锛氱粨鏉熻

	err = SRamWriteBit(REG_afe_ctrl_reg, 0x0D, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01);    //afe_ctrl_reg
	if(err != 0)
		return err;

	while(1)
	{
		err = ReadSRamRegister(0x1a04, &value); // 鏌ヨraw_flag鏄惁涓�1锛�1琛ㄧず鎵弿缁撴潫
		if(err != 0)
			return err;

		if (1 == (value&0x1))
		{
			WriteSRamRegister(0x1a04, 1);	//灏唕aw_flag娓�0
			break;
		}
		else
		{
			focal_usleep(4000);
			Timecnt++;
			if(Timecnt > 100)
				return RET_ERROR;
		}
	}

	//read image data
	memset(m_HistBuf, 0, sizeof(m_HistBuf));
	err = SRamBulkReadData(0x0000, m_HistBuf, 0x400);
	if(0 == err)
	{
		for(i = 0; i < 0x400; i++)
		{
			g_h[1][i] = m_HistBuf[i];
		}
	}

	///////////////////////绗�3鍧楅噰鏍�///////////////////////////////
	//AFE Image Scanning Start
	//鐩存柟鍥惧尯鍩熸竻闆�   % afe_adj_reg
	SRamWriteBit(0x1834, 0x03, 0x00, 0x02, 0x00,
		0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
		0x01, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x01,
		0x01, 0x00, 0x01, 0x00);
	SRamWriteBit(0x1800, 0x03, 0x00, 0x05, /*0x0a*/g_sensor_width / 8 / 2, 0x08, 0x00);		// sensor_addr0_start 5锛氳捣濮嬪垪锛�8锛氳捣濮嬭
	SRamWriteBit(0x1810, 0x03, 0x00, 0x05, /*0x13*/g_sensor_width /8 - 1, 0x08, /*0x4f*/g_sensor_height / 2 - 1);			// sensor_addr0_end 5锛氱粨鏉熷垪锛�8锛氱粨鏉熻

	err = SRamWriteBit(REG_afe_ctrl_reg, 0x0D, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01);    //afe_ctrl_reg
	if(err != 0)
		return err;

	while(1)
	{
		err = ReadSRamRegister(0x1a04, &value); // 鏌ヨraw_flag鏄惁涓�1锛�1琛ㄧず鎵弿缁撴潫
		if(err != 0)
			return err;

		if (1 == (value & 0x1))
		{
			WriteSRamRegister(0x1a04, 1);	//灏唕aw_flag娓�0
			break;
		}
		else
		{
			focal_usleep(4000);
			Timecnt++;
			if(Timecnt > 100)
				return RET_ERROR;
		}
	}

	//read image data
	memset(m_HistBuf, 0, sizeof(m_HistBuf));
	err = SRamBulkReadData(0x0000, m_HistBuf, 0x400);
	if(0 == err)
	{
		for(i = 0; i < 0x400; i++)
		{
			g_h[2][i] = m_HistBuf[i];
		}
	}

	///////////////////////绗�4鍧楅噰鏍�///////////////////////////////
	//AFE Image Scanning Start
	//鐩存柟鍥惧尯鍩熸竻闆�   % afe_adj_reg
	SRamWriteBit(0x1834, 0x03, 0x00, 0x02, 0x00,
		0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
		0x01, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x01,
		0x01, 0x00, 0x01, 0x00);
	SRamWriteBit(0x1800, 0x03, 0x00, 0x05, /*0x0a*/g_sensor_width / 8 / 2, 0x08, /*0x50*/g_sensor_height / 2);		// sensor_addr0_start 5锛氳捣濮嬪垪锛�8锛氳捣濮嬭
	SRamWriteBit(0x1810, 0x03, 0x00, 0x05, /*0x13*/g_sensor_width / 8 - 1, 0x08, /*0x9f*/g_sensor_height - 1);			// sensor_addr0_end 5锛氱粨鏉熷垪锛�8锛氱粨鏉熻

	err = SRamWriteBit(REG_afe_ctrl_reg, 0x0D, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01);    //afe_ctrl_reg
	if(err != 0)
		return err;

	while(1)
	{
		err = ReadSRamRegister(0x1a04, &value); // 鏌ヨraw_flag鏄惁涓�1锛�1琛ㄧず鎵弿缁撴潫
		if(err != 0)
			return err;

		if (1 == (value & 0x1))
		{
			WriteSRamRegister(0x1a04, 1);	//灏唕aw_flag娓�0
			break;
		}
		else
		{
			focal_usleep(4000);
			Timecnt++;
			if(Timecnt > 100)
				return RET_ERROR;
		}
	}

	//read image data
	memset(m_HistBuf, 0, sizeof(m_HistBuf));
	err = SRamBulkReadData(0x0000, m_HistBuf, 0x400);
	if(0 == err)
	{
		for(i = 0; i < 0x400; i++)
		{
			g_h[3][i] = m_HistBuf[i];
		}
	}

	return RET_OK;
}

static unsigned char AutoAdjustAGCStep1FromHist(unsigned short label[])
{
	//unsigned short datavalue = 0x00;
	int i = 0, j = 0;
	unsigned int fpSensorSize = g_sensor_width * g_sensor_height;
	double sum[4] = {0, 0, 0, 0};
	double block_sum[4] = {fpSensorSize/4,fpSensorSize/4,fpSensorSize/4,fpSensorSize/4};
	unsigned short pix_maxvalue[4] = {0,0,0,0};
	unsigned short minvalue[4], maxvalue[4];
	unsigned short medianValue[4];
	short dDeltaBase;
	short dBase;
	short dDelta;
	short DAC1;
	double f1 = 0;
	double f2 = 0;
	double factor2 = 0;
	double midsum = /*3200*/fpSensorSize/8;
	double maxsum = /*6400*/fpSensorSize/4;
	double usTemp2 = 0;
	double ratio = MID_RATIO_F;
	double maxFirstValue = 0;
	double minFirstValue = 0;
	unsigned short startColumn = 0;
	unsigned short startRow = 0;
	unsigned short endColumn = 0;
	unsigned short endRow = 0;
	unsigned short threshold;
	unsigned char agc2 = 0;

	memset(medianValue, 0 , sizeof(medianValue));
	FT_DBG("_________func=%s called start__________\n", __func__);
	/*
	缁熻鏈夋晥鐨勬暟鎹锛屽幓鎺夋瘮杈冧綆娲肩殑鏁版嵁鍜岀壒鍒櫋鐨勬暟鎹�
	*/


	/*
	鑾峰緱宄板��
	*/
	for(j = 0;j < 4; j++)
	{
		for(i = 5;i < 0x400 - 5; i++)
		{
			/*
			鍘绘帀姣旇緝闄＄殑鏁版嵁
			*/
			if (g_h[j][i] > (g_h[j][i-1] + g_h[j][i+1])*4)
			{
				continue;
			}

			if ((g_h[j][i-1] == 0) || (g_h[j][i+1] == 0))
			{
				continue;
			}

			if (g_h[j][i] > pix_maxvalue[j])
			{
				pix_maxvalue[j] = g_h[j][i];
			}

		}
	}

	/*
	鑾峰緱鏈夋晥鐐规暟
	*/
	for(j = 0;j < 4; j++)
	{
		block_sum[j] = 0;
		for(i = 5;i < 0x400 - 5; i++)
		{
			/*
			鍘绘帀姣旇緝闄＄殑鏁版嵁
			*/
			if (g_h[j][i] > (g_h[j][i-1] + g_h[j][i+1])*4)
			{
				continue;
			}

			if ((g_h[j][i-1] == 0) || (g_h[j][i+1] == 0))
			{
				continue;
			}

			if (g_h[j][i] < pix_maxvalue[j]/MAX_VALUE_RATIO)
			{
				continue;
			}

			block_sum[j] += g_h[j][i];
		}
	}

	for (j = 0; j < 4; j++)
	{
		midsum = block_sum[j] / 2;

		for(i = 0; i < 0x400; i++)
		{
			medianValue[j] = i + 1;
			sum[j] += g_h[j][i];

			/*
			鍘绘帀姣旇緝闄＄殑鏁版嵁
			*/
			if (g_h[j][i] > (g_h[j][i-1] + g_h[j][i+1]) * 4)
			{
				continue;
			}

			if ((g_h[j][i-1] == 0) || (g_h[j][i+1] == 0))
			{
				continue;
			}

			if (g_h[j][i] < pix_maxvalue[j]/MAX_VALUE_RATIO)
			{
				continue;
			}

			if (sum[j] > midsum)
			{
				break;
			}
		}
	}

	memset(minvalue, 0, sizeof(minvalue));
	memset(maxvalue, 0, sizeof(maxvalue));

	for (j = 0; j < 4; j++)
	{
		usTemp2 = 0;
		midsum = block_sum[j]/2;

		for (i = 5; i < medianValue[j]; i++)
		{
#if 1
			/*
			鍘绘帀鐗瑰埆闄＄殑鏁版嵁
			*/
			if ((i > 0) && (i < 1023))
			{
				if (g_h[j][i] > (g_h[j][i-1] + g_h[j][i+1])*4)
				{
					continue;
				}

				if ((g_h[j][i-1] == 0) || (g_h[j][i+1] == 0))
				{
					continue;
				}
			}

			if (g_h[j][i] < pix_maxvalue[j]/MAX_VALUE_RATIO)
			{
				continue;
			}

			usTemp2 += g_h[j][i];

			minvalue[j] = i;

			if (usTemp2 >= midsum * ratio)
			{
				break;
			}
#else
			usTemp2 += g_h[j][i];

			if (usTemp2 >= /*3200*/m_FpSensorSize/8 * ratio)
			{
				minvalue[j] = i;
				break;
			}
#endif
		}
	}

	for (j = 0; j < 4; j++)
	{
		midsum = block_sum[j]/2;
		maxsum = block_sum[j];

		usTemp2 = 0;
		for (i = medianValue[j]; i < 1023-5; i++)
		{
#if 1
			/*
			鍘绘帀鐗瑰埆闄＄殑鏁版嵁
			*/
			if ((i > 0) && (i < 1023))
			{
				if (g_h[j][i] > (g_h[j][i-1] + g_h[j][i+1])*4)
				{
					continue;
				}

				if ((g_h[j][i-1] == 0) || (g_h[j][i+1] == 0))
				{
					continue;;
				}
			}

			if (g_h[j][i] < pix_maxvalue[j]/MAX_VALUE_RATIO)
			{
				continue;
			}

			usTemp2 += g_h[j][i];
			maxvalue[j] = i;

			if (usTemp2 >= (maxsum - sum[j] - midsum * ratio))
			{
				break;
			}
#else
			usTemp2 += g_h[j][i];
			if (usTemp2 >= (/*6400*/m_FpSensorSize/4 - sum[j] - /*3200*/m_FpSensorSize/8 * ratio))

			{
				maxvalue[j] = i;
				break;
			}
#endif
		}
	}

	threshold = medianValue[0];
	for(i = 1; i < 4; i++)
	{
		if(medianValue[i] < threshold)
			threshold = medianValue[i];
	}
	for(i = 0; i < 4; i++)
	{
		if(threshold == medianValue[i])
		{
			label[i] = 1;
			break;
		}
	}

	for (i = 0; i < 4; i++)
	{
		if (1 == label[i])
		{
			switch (i)
			{
			case 0:
				startColumn = 0;
				startRow = 0;
				endColumn = g_sensor_width / 8 / 2 - 1;//9;
				endRow = g_sensor_height / 2 - 1;//0x4f;
				maxFirstValue = maxvalue[0];
				minFirstValue = minvalue[0];
				break;

			case 1:
				startColumn = 0;
				startRow = g_sensor_width / 2;//0x50;
				endColumn = g_sensor_height / 8 / 2 - 1;//9;
				endRow = g_sensor_height - 1;//0x9f;
				maxFirstValue = maxvalue[1];
				minFirstValue = minvalue[1];
				break;

			case 2:
				startColumn = g_sensor_width / 8 / 2;//0x0a;
				startRow = 0;
				endColumn = g_sensor_width / 8 - 1;//0x13;
				endRow = g_sensor_height / 2 - 1;//0x4f;
				maxFirstValue = maxvalue[2];
				minFirstValue = minvalue[2];
				break;

			case 3:
				startColumn = g_sensor_width / 8 / 2;//0x0a;
				startRow = g_sensor_height /2;//0x50;
				endColumn = g_sensor_width / 8 - 1;//0x13;
				endRow = g_sensor_height - 1;//0x9f;
				maxFirstValue = maxvalue[3];
				minFirstValue = minvalue[3];
				break;

			default:
				minFirstValue = 1;
				break;
			};

			g_FpAfeScanParam.ColumnRowStarEnd.startColumn = startColumn;
			g_FpAfeScanParam.ColumnRowStarEnd.startRow = startRow;
			g_FpAfeScanParam.ColumnRowStarEnd.endColumn = endColumn;
			g_FpAfeScanParam.ColumnRowStarEnd.endRow = endRow;
			g_FpAfeScanParam.ColumnRowStarEnd.maxFirstValue =(short) maxFirstValue;
			g_FpAfeScanParam.ColumnRowStarEnd.minFirstValue =(short) minFirstValue;
		}
	}

	dDeltaBase = m_DAC_OFFSET;//32;
	dBase = 512 + dDeltaBase;

	dDelta = dBase - threshold;//0.5 * (maxFirstValue + minFirstValue);

	DAC1 = 512 + dDelta;

	if (DAC1 > 1023)
	{
		DAC1 = 1023;
	}
	else if (DAC1 < 0)
	{
		DAC1 = 0;
	}

	if (maxFirstValue == minFirstValue)
	{
		maxFirstValue += 1;
	}

	if (maxFirstValue != threshold)
	{
		f1 = (512 - m_DAC_OFFSET) / ((maxFirstValue - threshold));
	}

	if (threshold != minFirstValue)
	{
		f2 = (512 + m_DAC_OFFSET) / ((threshold - minFirstValue));
	}

	if (f1 < f2)
	{
		factor2 = f1 * 1.1;
	} else
	{
		factor2 = f2 * 1.1;
	}

	//factor2 = 896 / (maxFirstValue - minFirstValue);
	//factor2 = 1023 / (maxFirstValue - minFirstValue);

	if (factor2 >= 10.5)
	{
		factor2 = 10.5;		// 涔熷氨鏄疉GC鏀惧ぇ鍙傛暟涓�3
		agc2 = 3;
	}
	else if (factor2 >= 4.5)
	{
		factor2 = 4.5;		// 涔熷氨鏄疉GC鏀惧ぇ鍙傛暟涓�2
		agc2 = 2;
	}
	else if (factor2 > 2.1)
	{
		factor2 = 2.1;		// 涔熷氨鏄疉GC鏀惧ぇ鍙傛暟涓�1
		agc2 = 1;
	}
	else
	{
		factor2 = 1;		// 涔熷氨鏄疉GC鏀惧ぇ鍙傛暟涓�0
		agc2 = 0;
	}

	g_FpAfeScanParam.ColumnRowStarEnd.factor2 = factor2;

	g_FpAfeScanParam.m_agc_param.dac1_base = DAC1;// + 512;
	g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.afe_s2_gn_sel = agc2;// 2,debug鏃剁洿鎺ヨ缃负2

	return 1;
}

int GetFpImageDataFromSRAM(unsigned char * pImage)
{
	int err = 0;
	memset(m_ImageBuf, 0, sizeof(m_ImageBuf));
	err = SRamBulkReadData(FP_IMAGE_START_ADDRESS, m_ImageBuf,
			g_sensor_width * g_sensor_height);
	if (0 == err) {
		Convert2FpImage(m_ImageBuf, g_sensor_width * g_sensor_height,
				pImage);
	}
	return err;
}


static int GetSensorHistogramBlockSecondSample(unsigned char *pHistData)
{

	int err = 0, i = 0;
	//unsigned int StartTime;
	double temp1 = 0, temp2 = 0;
	double ratio = MID_RATIO_F;
	double factor4 = 0;
	double usMedianThirdValue = 0;
	double usMinThirdValue = 0;
	double usMaxThirdValue = 0;
	double block_sum;
	double midsum;
	double maxsum;
	short usDAC2 = 0;
	unsigned short pix_maxvalue;
	unsigned short usMedianSecondValue = 0;
	unsigned short usAGC4 = 0;
	//unsigned short usMinSecondValue = 0;
	//short usMaxSecondValue = 0;
	unsigned short value = 0x00;
	unsigned short Timecnt = 0;
	unsigned int fpSensorSize = g_sensor_width * g_sensor_height;
	unsigned short usDAC1 = g_FpAfeScanParam.m_agc_param.dac1_base;
	unsigned short usAGC2 = g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.afe_s2_gn_sel;
	

	FT_DBG("_________func=%s called start__________\n", __func__);
	////////////////////////////////////////////////////////////////////
    ////////////////////////  鍧椾簩娆￠噰鏍� ///////////////////////////////
    ////////////////////////////////////////////////////////////////////
	err = SRamWriteBit(0x34, 0x03, 0x00,
							 0x02, 0x00,
							 0x01, 0x00,
							 0x01, 0x00,
							 0x01, 0x00,
							 0x01, 0x00,
							 0x01, 0x00,
							 0x01, 0x00,
							 0x02, 0x00,
							 0x01, 0x01,
							 0x01, 0x00,
							 0x01, 0x00);
	if(err != 0)
		return err;

	SRamWriteBit(0x1800, 0x03, 0x00,
		                 0x05, g_FpAfeScanParam.ColumnRowStarEnd.startColumn,
						 0x08, g_FpAfeScanParam.ColumnRowStarEnd.startRow);		// sensor_addr0_start 5锛氳捣濮嬪垪锛�8锛氳捣濮嬭

	SRamWriteBit(0x1810, 0x03, 0x00,
		                 0x05, g_FpAfeScanParam.ColumnRowStarEnd.endColumn,
						 0x08, g_FpAfeScanParam.ColumnRowStarEnd.endRow);			// sensor_addr0_end 5锛氱粨鏉熷垪锛�8锛氱粨鏉熻

	usAGC2 = g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.afe_s2_gn_sel;
	err = SRamWriteBit(REG_pagc_gn, 0x07, 0x00,
		                            0x02, 0x00,
									0x02, 0x00,
									0x02, usAGC2,   // AGC2
									0x03, 0x06);
	if(err != 0)
		return err;

	usDAC1 = g_FpAfeScanParam.m_agc_param.dac1_base;
	err = SRamWriteBit(REG_dac1_base, 0x06, 0x00, 0x0A, usDAC1);
	if(err != 0)
		return err;

	////////////////////////////////////////////////////////////////////
    ////////////////////////  鍧椾笁娆￠噰鏍� ///////////////////////////////
    ////////////////////////////////////////////////////////////////////
	err = SRamWriteBit(0x34, 0x03, 0x00,
							 0x02, 0x00,
							 0x01, 0x00,
							 0x01, 0x00,
							 0x01, 0x00,
							 0x01, 0x00,
							 0x01, 0x00,
							 0x01, 0x00,
							 0x02, 0x00,
							 0x01, 0x01,
							 0x01, 0x00,
							 0x01, 0x00);
	if(err != 0)
		return err;

	err = SRamWriteBit(REG_dac1_base, 0x06, 0x00, 0x0A, usDAC1);
	if(err != 0)
		return err;

	err = SRamWriteBit(REG_afe_ctrl_reg, 0x0D, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01);    //afe_ctrl_reg
	if(err != 0)
		return err;

	value = 0x00;
	Timecnt = 0;

#if 1
	while(1)
	{
		err = ReadSRamRegister(0x1a04, &value); // 鏌ヨraw_flag鏄惁涓�1锛�1琛ㄧず鎵弿缁撴潫
		if(err != 0)
		{
			return err;
		}

		if (1 == (value & 0x1))
		{
			WriteSRamRegister(0x1a04, 1);	//灏唕aw_flag娓�0
			break;
		}
		else
		{
			focal_usleep(4000);
			Timecnt++;
			if(Timecnt > 100)
			{
				return RET_ERROR;
			}
		}
	}
#else
	focal_usleep(SLEEPTIME);
#endif
	//read image data
	memset(m_HistBuf, 0, sizeof(m_HistBuf));
	err = SRamBulkReadData(0x0000, m_HistBuf, 0x400);

	/*
	缁熻鏈夋晥鐨勬暟鎹锛屽幓鎺夋瘮杈冧綆娲肩殑鏁版嵁鍜岀壒鍒櫋鐨勬暟鎹�
	*/
	block_sum = /*6400*/fpSensorSize / 4;
	pix_maxvalue = 0;
	midsum = /*3200*/fpSensorSize / 8;
	maxsum = /*6400*/fpSensorSize / 4;

	/*
	鑾峰緱宄板��
	*/
	for(i = 5;i < 0x400 - 5; i++)
	{
		/*
		鍘绘帀姣旇緝闄＄殑鏁版嵁
		*/
		if (m_HistBuf[i] > (m_HistBuf[i-1] + m_HistBuf[i+1])*4)
		{
			continue;
		}


		if ((m_HistBuf[i-1] == 0) || (m_HistBuf[i+1] == 0))
		{
			continue;;
		}

		if (m_HistBuf[i]  > pix_maxvalue)
		{
			pix_maxvalue = m_HistBuf[i] ;
		}

	}
	/*
	鑾峰緱鏈夋晥鐐规暟
	*/

	block_sum = 0;

	for(i = 5;i < 0x400 - 5; i++)
	{
		/*
		鍘绘帀姣旇緝闄＄殑鏁版嵁
		*/
		if (m_HistBuf[i] > (m_HistBuf[i-1] + m_HistBuf[i+1])*4)
		{
			continue;
		}

		if ((m_HistBuf[i-1] == 0) || (m_HistBuf[i+1] == 0))
		{
			continue;;
		}

		if (m_HistBuf[i] < pix_maxvalue/MAX_VALUE_RATIO)
		{
			continue;
		}

		block_sum += m_HistBuf[i];
	}

	midsum  = block_sum / 2;
	maxsum = block_sum;

	temp1 = 0;
	for (i = 5; i < 1024-5; i++)
	{
		if (m_HistBuf[i] > (m_HistBuf[i-1] + m_HistBuf[i+1])*4)
		{
			continue;
		}

		if ((m_HistBuf[i-1] == 0) || (m_HistBuf[i+1] == 0))
		{
			continue;;
		}

		if (m_HistBuf[i] < pix_maxvalue/MAX_VALUE_RATIO)
		{
			continue;
		}

		temp1 += m_HistBuf[i];

		usMedianSecondValue = i + 1;
		if (temp1 >= midsum)
		{
			break;
		}
	}

	temp2 = 0;
	for (i = 5; i < usMedianThirdValue; i++)
	{
		if ( (i > 0) && (i < 1023))
		{
			if (m_HistBuf[i] > (m_HistBuf[i-1] + m_HistBuf[i+1])*4)
			{
				continue;
			}

			if ((m_HistBuf[i-1] == 0) || (m_HistBuf[i+1] == 0))
			{
				continue;;
			}
		}


		if (m_HistBuf[i] < pix_maxvalue/MAX_VALUE_RATIO)
		{
			continue;
		}


		usMinThirdValue = i;
		temp2 += m_HistBuf[i];
		if (temp2 >= temp1 * ratio)
		{
			break;
		}
	}

	temp2 = 0;
	for (i = 1023-5; i >= (usMedianThirdValue - 1); i--)
	{
		if ( (i > 0) && (i < 1023))
		{
			if (m_HistBuf[i] > (m_HistBuf[i-1] + m_HistBuf[i+1])*4)
			{
				continue;
			}

			if ((m_HistBuf[i-1] == 0) || (m_HistBuf[i+1] == 0))
			{
				continue;;
			}
		}

		if (m_HistBuf[i] < pix_maxvalue/MAX_VALUE_RATIO)
		{
			continue;
		}

		temp2 += m_HistBuf[i];
		usMaxThirdValue = i;

		if (temp2 > (maxsum - temp1) * ratio)
		{
			break;
		}
	}

	if (usMaxThirdValue == usMinThirdValue)
	{
		usMaxThirdValue += 1;
	}

	factor4 = 1023 / (usMaxThirdValue - usMinThirdValue);

	if (factor4 >= 8)
	{
		factor4 = 8;
		usAGC4 = 3;
	}
	else if(factor4 >= 4)
	{
		factor4 = 4;
		usAGC4 = 2;
	}
	else if(factor4 >= 1.9)
	{
		factor4 = 2;
		usAGC4 = 1;
	}
	else
	{
		factor4 = 1;
		usAGC4 = 0;
	}

	//usDAC2 = 1023 - usMaxThirdValue * factor4;
	//usDAC2 += 512;

	usDAC2 = 512 + 512 - usMedianSecondValue;//(usMaxThirdValue + usMinThirdValue)/2;

	if (usDAC2 > 1023)
	{
		usDAC2 = 1023;
	}
	else if (usDAC2 < 0)
	{
		usDAC2 = 0;
	}
	//FT_DBG("__%d__", __LINE__);

	////////////////////////////////////////////////////////////////////
    ////////////////////////  鍏ㄥ浘閲囨牱  ///////////////////////////////
    ////////////////////////////////////////////////////////////////////
	g_FpAfeScanParam.m_agc_param.dac1_base = usDAC1;		//DAC1
	g_FpAfeScanParam.m_agc_param.dac2_gray = usDAC2;        //DAC2
	//g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.afe_s1_gn_sel 			//AGC1
	g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.afe_s2_gn_sel = usAGC2;	//AGC2
	//g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.afe_cds_gn_sel			//AGC3
	g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.adc_gn_sel = usAGC4;		//AGC4
	GetFrameData(pHistData);
	//DMSG("__%d__", __LINE__);

	return RET_OK;
}

static int InitSensor(void)
{
	int err = 0;
	//Clock Open
	err = WriteSfrRegister(REG_MAIN_CLK_ICG_ID, 0xFF); //Open AFE OTP REGFILE Clock
	if(err != 0)
		return RET_ERROR_RW;
	err = WriteSfrRegister(REG_MAIN_MEN_ICG_SW_ID, 0xFF); //Open AFE OTP REGFILE Clock
	if(err != 0)
		return RET_ERROR_RW;
	

	//Close Encryption
	err = WriteSfrRegister(REG_ENCRYPTE_EN, 0x00);
	if(err != 0)
		return RET_ERROR_RW;

	//LDO OPEN
	err = WriteSfrRegister(REG_PMU_ADD_ADJ_ID, 0x24);
	if(err != 0)
		return RET_ERROR_RW;

	err = WriteSfrRegister(REG_PMU_ADD_ADJ_ID, 0xf6);
	if(err != 0)
		return RET_ERROR_RW;

	err = WriteSfrRegister(REG_PMU_SS_EN_N_ID, 0x01);
	if(err != 0)
		return RET_ERROR_RW;

	err = WriteSfrRegister(REG_PMU_VDDA_EN_ID, 0x03);
	if(err != 0)
		return RET_ERROR_RW;

	//INT SYSCTRL SET
	err = WriteSfrRegister(REG_GPIO_OUTEN, 0xf0);
	if(err != 0)
		return RET_ERROR_RW;

	//AFE Controller Register CFG
	//err = SRamWriteBit(REG_macroblock_freq_sel, 0x06, 0x00, 0x0A, 0x23); //
	g_FpAfeScanParam.m_mb_freq_sel = 0x23;
	err = SRamWriteBit(REG_macroblock_freq_sel, 0x06, 0x00, 0x0A, g_FpAfeScanParam.m_mb_freq_sel); //
	if(err != 0)
		return err;


	//err = SRamWriteBit(REG_macroblock_scan_time, 0x08, 0x00, 0x08, 0x04);                           //8,0, 8,10
	err = SRamWriteBit(REG_macroblock_scan_time, 0x08, 0x00, 0x08, g_FpAfeScanParam.m_macroblock_scan_time);                           //8,0, 8,10  macroblock_scan_time
	if(err != 0)
		return err;

	err = SRamWriteBit(0x182A, 0x01, 0x00, 0x01, 0x01, 0x05, 0x1f,
			0x05, 0x1f, 0x01, 0x01, 0x03, 0x07);
	if(err != 0)
		return err;

	err = SRamWriteBit(0x182B, 0x08, 0x00, 0x01, 0x01, 0x07, 0x08);
	if(err != 0)
		return err;

	err = SRamWriteBit(0x182C, 0x07, 0x00, 0x01, 0x01, 0x08, 0x40);
	if(err != 0)
		return err;

	err = SRamWriteBit(0x182D, 0x06, 0x02, 0x06, 0x00, 0x04, 0x09);
	if(err != 0)
		return err;

	err = SRamWriteBit(REG_scan_mode, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
		0x04, 0x07, 0x02, 0x01, 0x04, 0x00, 0x02, 0x00);  //1,0, 2,0, 1,0, 4,0, 2,0, 4,0, 2,0  scan_mode
	if(err != 0)
		return err;

//	err = SRamWriteBit(REG_pagc_gn, 0x07, 0x00,
//		0x02, g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.adc_gn_sel,
//		0x02, g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.afe_cds_gn_sel,
//		0x02, g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.afe_s2_gn_sel,
//		0x03, g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.afe_s1_gn_sel); //for virtual touch                   pagc_gn
//	if(err != 0)
//		return err;
	err = SRamWriteBit(REG_pagc_gn, 0x07, 0x00,
			0x02, g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.afe_cds_gn_sel,
			0x02, g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.adc_gn_sel,
			0x02, g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.afe_s2_gn_sel,
			0x03, g_FpAfeScanParam.m_agc_param.PagcGn.GAIN.afe_s1_gn_sel); //for virtual touch                   pagc_gn
		if(err != 0)
			return err;

	//m_FpAfeScanParam.m_agc_param.dac1_base = 0x2a0;
	err = SRamWriteBit(REG_dac1_base, 0x06, 0x00, 0x0A, g_FpAfeScanParam.m_agc_param.dac1_base);//6,0, A,80                        dac1_base
	if(err != 0)
		return err;

	//m_FpAfeScanParam.m_agc_param.dac2_gray = 0x100;
	err = SRamWriteBit(REG_dac2_gray, 0x06, 0x00, 0x0A, g_FpAfeScanParam.m_agc_param.dac2_gray);//6,0, A,80
	if(err != 0)
		return err;


	//Full Screen Mode
	//AGC Mode

//	err = SRamWriteBit(REG_sensor_addr0_end, 0x03, 0x00, 0x05, 0x13, 0x08, 0x9F);    //sensor_addr0_end
//	if(err != 0)
//		return err;

	err = SRamWriteBit(REG_sensor_addr0_end, 0x03, 0x00, 0x05,
				g_sensor_width / 8 - 1, 0x08, g_sensor_height - 1);  //sensor_addr0_end
		if(err != 0)
			return err;
	err = SRamWriteBit(REG_sensor_addr0_start, 0x03, 0x00, 0x05, 0x00, 0x08, 0x00);    //sensor_addr0_start
	if(err != 0)
		return err;

	//AFE Adjust Register CFG
	err = SRamWriteBit(REG_afe_adj_reg,
		0x03, 0x00,
		0x02, 0x00,
		0x01, 0x00,
		0x01, 0x00,
		0x01, 0x00,
		0x01, 0x00,
		0x01, 0x00,
		0x01, 0x00,
		0x02, 0x00,
		0x01, 0x01,
		0x01, 0x00,
		0x01, 0x00);    //afe_adj_reg
	if(err != 0)
		return err;

	err = SRamWriteBit(REG_cfg_afe_reg0,
		0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x03, 0x04,
		0x03, 0x04, 0x02, 0x01, 0x02, 0x01, 0x02, 0x01);      //cfg_afe_reg0
	if(err != 0)
		return err;

	err = SRamWriteBit(REG_cfg_afe_reg1,
		0x02, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
		0x01, 0x00, 0x01, 0x01, 0x08, 0xFF);         //cfg_afe_reg1
	if(err != 0)
		return err;

	err = SRamWriteBit(REG_cfg_sensor_reg, 0x0C, 0x00, 0x01, 0x00, 0x03, 0x07);                                //cfg_sensor_reg
	if(err != 0)
		return err;

	err = SRamWriteBit(REG_cfg_dac_reg,
		0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x01,
		0x01, 0x00, 0x04, 0x04, 0x03, 0x00, 0x02, 0x00);    //cfg_dac_reg
	if(err != 0)
		return err;
//	err = SRamWriteBit(REG_cfg_dac_reg,
//			0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
//			0x01, 0x00, 0x04, 0x04, 0x03, 0x00, 0x02, 0x00);    //cfg_dac_reg
//		if(err != 0)
//			return err;

	err = SRamWriteBit(REG_cfg_adc_reg,
		0x04, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x00, 0x04, 0x00,
		0x03, 0x04, 0x02, 0x02);              //cfg_adc_reg
	if(err != 0)
		return err;

	err = SRamWriteBit(REG_cfg_ref_reg,
		0x05, 0x00, 0x03, 0x03, 0x03, 0x03, 0x01, 0x00, 0x01, 0x01,
		0x01, 0x00, 0x02, 0x02);              //cfg_ref_reg
	if(err != 0)
		return err;
#if 1
	//AFE RING Voltage Controll Register CFG
	err = SRamWriteBit(REG_cfg_ndrv0_reg, 0x0B, 0x00, 0x02, 0x00, 0x02, 0x00, 0x01, 0x00);    //cfg_ndrv0_reg
	if(err != 0)
		return err;

	err = SRamWriteBit(REG_cfg_ndrv2_reg, 0x0F, 0x00, 0x01, 0x01);    //cfg_ndrv2_reg
	if(err != 0)
		return err;

	switch (drv_voltage_type)
	{
	case DRV_VOLTAGE_LOW:
		err = SRamWriteBit(REG_cfg_ndrv3_reg,
			0x05, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x03, 0x00,
			0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00);    //cfg_ndrv3_reg 0x608
		if(err != 0)
			return err;
		break;
	case  DRV_VOLTAGE_MID:
		err = SRamWriteBit(REG_cfg_ndrv3_reg,
			0x05, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x03, 0x00,
			0x01, 0x01, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00);    //cfg_ndrv3_reg 0x610
		if(err != 0)
			return err;
		break;
	case DRV_VOLTAGE_HIGH:
		err = SRamWriteBit(REG_cfg_ndrv3_reg,
			0x05, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x03, 0x00,
			0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00);    //cfg_ndrv3_reg 0x618
		if(err != 0)
			return err;
		break;
	default:
		break;
	}
#else
	WriteSRamRegister(0x1856, 0x0001); // NDRV_TX_MV_EN=1,NDRV_RING3P3V_DRV_ADJ=00
	WriteSRamRegister(0x1858, 0x0001); //  pos latch
	WriteSRamRegister(0x1857, 0x0000);
	WriteSRamRegister(0x1859, 0x0400);
#endif
	focal_usleep(10000);   //Wait N ms ... HighVoltage will be stable

	/*
	spi
	*/
//	WriteSfrRegister(0x25,0x38);//ring
//	WriteSfrRegister(0x24,1);

	return RET_OK;//1013,ADD
}

unsigned char GetFpImage(unsigned char *pFpImgData)
{
	unsigned short label[4];

	//short ScanCnt;
	//unsigned char ret = 0;
	//static unsigned char  InitValue = 0;
	//static short m_GAIN2_REAL_Last = 0;
	//static unsigned char s_FirstGain_SetFlg = 0,m_FirstGain_set = 0,m_FirstGain2_set,m_FirstGain3_set;
	//unsigned short usRegValue;
	//unsigned char regvalue;
#if 0//ring?
	ret = DetectFinger();
	if(ret != RET_OK)
		return RET_ERROR;

	WriteSRamRegister(0x1a04,0x0003);
#endif

	//WriteSfrRegister(0x24,1);

	InitSensor();

	RestoreDefaultValue();

	GetSensor4Histogram();

	memset(label, 0, sizeof(label));

	if(AutoAdjustAGCStep1FromHist(label))
	{
		if(RET_OK == GetSensorHistogramBlockSecondSample(pFpImgData))
		{
			//g_FpAfeScanParam.m_agc_param.m_Image_Score  = (unsigned char)GetImageScore(pFpImgData,
					//FP_SENSOR_WIDTH, FP_SENSOR_HEIGHT);
			//FT_DBG("______________________________RET_OK");
		}
		else
		{
			FT_DBG("______________________________RET_ERROR");
			return RET_ERROR;
		}
	}
	else
	{
		return RET_ERROR;
	}

	return RET_OK;
}


static int AppGetRingAdcValue(void)
{
	unsigned short usRegValue;
	int value;

	ReadSRamRegister(0x1b1c,&usRegValue);
	value = (int)usRegValue << 12;
	ReadSRamRegister(0x1b1d,&usRegValue);
	value += usRegValue;

	return value;
}
/*
Base鍒锋柊
*/
static void AppRingBaseRefresh(void)
{
	g_FpAfeScanParam.m_agc_param.m_ring_base = AppGetRingAdcValue();
	g_FpAfeScanParam.m_agc_param.m_ring_diff = 0;
}

/*
Base鏇存柊
*/
static void AppRingBaseCompensate(short sRingDiff)
{
	if (sRingDiff > 2)
	{
		g_FpAfeScanParam.m_agc_param.m_ring_base -= 2;
	}

	if (sRingDiff < -2)
	{
		g_FpAfeScanParam.m_agc_param.m_ring_base += 2;
	}
}

static unsigned char EnterRingMode(void)
{
	int err = 0;
	//unsigned short value = 0x00;
	//Clock Open
	err = WriteSfrRegister(REG_MAIN_CLK_ICG_ID, 0xFF); //Open AFE OTP REGFILE Clock
	if(err != 0)
		return RET_ERROR_RW;
	err = WriteSfrRegister(REG_MAIN_MEN_ICG_SW_ID, 0xFF); //Open AFE OTP REGFILE Clock
	if(err != 0)
		return RET_ERROR_RW;

	//OPEN 250K OSC
	err = WriteSfrRegister(REG_OSC_250K_EN_ID, 0x1a);  // Open en
	if(err != 0)
		return RET_ERROR_RW;
	err = WriteSfrRegister(REG_OSC_250K_START_ID, 0x0a);  // Open start
	if(err != 0)
		return RET_ERROR_RW;
	err = WriteSfrRegister(REG_OSC_250K_OE_ID, 0x0a);  // Open analog oe
	if(err != 0)
		return RET_ERROR_RW;
	err = WriteSfrRegister(REG_MAIN_LATCH_CTRL,0xc8);  // Open dig oe
	if(err != 0)
		return RET_ERROR_RW;


	// 220k clk to ring

	//NDRV
	err = WriteSRamRegister(1857,0);  //pos latch
	if(err != 0)
		return RET_ERROR_RW;
	err = WriteSRamRegister(1858,0);  // pos latch
	if(err != 0)
		return RET_ERROR_RW;
	err = WriteSRamRegister(1858,1);  //pos latch
	if(err != 0)
		return RET_ERROR_RW;
	err = SRamWriteBit(REG_cfg_ndrv3_reg, 0x05, 0x00, 0x01, 0x01,
						0x01, 0x01, 0x01, 0x00, 0x03, 0x00,
						0x01, 0x00, 0x01, 0x00, 0x03, 0x07);  //EN1 EN2 SW12_EN
	if(err != 0)
		return RET_ERROR_RW;

	// LDO OPEN
	err = WriteSfrRegister(REG_PMU_ADD_ADJ_ID, 0xE4); //Open LATCH
	if(err != 0)
		return RET_ERROR_RW;
	err = WriteSfrRegister(REG_PMU_VDDA_EN_ID, 0x03);  //Open EN
	if(err != 0)
		return RET_ERROR_RW;
	//INT SYSCTRL SET
	err = WriteSfrRegister(REG_GPIO_OUTEN, 0xf0);
	if(err != 0)
		return RET_ERROR_RW;
	err = WriteSfrRegister(REG_CLK_TEST_EN, 0x00); //set clk test en
	if(err != 0)
		return RET_ERROR_RW;
	err = WriteSfrRegister(REG_OSC_TEST_SW_ID, 0x0a);   //clk to test pin GPIO6 CTRL_LS0 ( 9:250k; a:220k; c:48M );
	if(err != 0)
		return RET_ERROR_RW;
	//sfrread(1d);
	//sfrread(0b);

	//RING_DET_MODE

	//sfrwrite(1c,7a);
	err = WriteSRamRegister(REG_HOST_MODE_CMD, 0x0000);
	if(err != 0)
		return RET_ERROR_RW;
	//Sleep(100);

	err = WriteSRamRegister(REG_HOST_DEBUG_ID, 0x0000);    //  0:normal mode ; 1:debug mode ;
	if(err != 0)
		return RET_ERROR_RW;
	err = WriteSRamRegister(REG_RING_SLEEP_TIME_ID, 0x08);  //  RING_SLEEP_TIME_ID ,defaut :2ms ;
	if(err != 0)
		return RET_ERROR_RW;
	err = WriteSRamRegister(REG_RING_TIME_F1_ID, 0x03e8);  //  RING_TIME_F1_ID    ,defaut :4ms ;
	if(err != 0)
		return RET_ERROR_RW;
	err = WriteSRamRegister(REG_RING_TIME_F2_ID, 0x03e8);  //  RING_TIME_F2_ID    ,defaut :4ms ;
	if(err != 0)
		return RET_ERROR_RW;
	err = WriteSRamRegister(REG_RING_TIME_ACC_ID, 0x0004);    //  RING_TIME_ACC_ID   ,defaut :4   ;
	if(err != 0)
		return RET_ERROR_RW;

	err = WriteSRamRegister(REG_ACC_REF_RING_H_ID, g_FpAfeScanParam.m_agc_param.m_ring_base >> 12);    //  ACC_REF_RING_H_ID  ,defaut :0   ;
	if(err != 0)
		return RET_ERROR_RW;
	err = WriteSRamRegister(REG_ACC_REF_RING_L_ID, g_FpAfeScanParam.m_agc_param.m_ring_base & 0xfff);  //  ACC_REF_RING_L_ID  ,defaut :b92  ;
	if(err != 0)
		return RET_ERROR_RW;

	err = WriteSRamRegister(REG_RING_ACC_H_ID, FP_RING_BASE_REFRESH_THRES >> 12);   //  RING_ACC_H_ID      ,defaut :0   ;
	if(err != 0)
		return RET_ERROR_RW;
	err = WriteSRamRegister(REG_RING_ACC_L_ID, FP_RING_BASE_REFRESH_THRES & 0xfff);  //  RING_ACC_L_ID      ,defaut :80  ;
	if(err != 0)
		return RET_ERROR_RW;


	WriteSRamRegister(0x1833,0);
	WriteSRamRegister(0x1855,0x365);
	WriteSfrRegister(0x18,0x00);

	err = WriteSRamRegister(REG_HOST_MODE_CMD, 0x0001);    // HOST_MODE_CMD  1:ring mode ; 3:sensor mode ;
	if(err != 0)
		return RET_ERROR_RW;

	return RET_OK;
}

static int GetRingStatus(void)
{
	int err = 0;
	unsigned short value = 0x00;

	err = ReadSRamRegister(REG_FINGER_DET_STATE_ID, &value);    // HOST_MODE_CMD  1:ring mode ; 3:sensor mode ;
	if(err != 0)
		return RET_ERROR_RW;
	else
	{
		if(1 == (value & 0x0001))
			return FP_RING_FINGER_ON;
	}

	return FP_RING_FINGER_OFF;
}

unsigned char DetectFinger(void)
{
	static unsigned char InitValue = 0;
	static unsigned char sRingMode = FP_RING_FINGER_OFF;
	static unsigned char sRingBaseFlg = 0;
	static unsigned char sDelayFrame = 0;
	//unsigned short usRegValue;
	short Dac_1;

	{
		//WriteSfrRegister(0x25,0x38);//ring
		//WriteSfrRegister(0x24,1);
	
		RestoreDefaultValue();

		g_FpAfeScanParam.m_agc_param.m_Image_Score = 0;

		if ((InitValue < 2) && (sRingMode == FP_RING_FINGER_OFF)) {
			if (sRingBaseFlg == 0) {
				g_FpAfeScanParam.m_agc_param.m_ring_base = 0x7fff;

				EnterRingMode();
				focal_usleep(30000);

				sRingBaseFlg = 1;
				AppRingBaseRefresh();

				InitSensor();

				EnterRingMode();

				InitSensor();
			}


			InitValue++;
			/*
			 娴嬭瘯DAC1閲忕▼鐨勮宸�
			 */
			Dac_1 = GetDACOFFSET();

			//if (m_DAC_RATIO <= 0)
			{
				m_DAC_RATIO = 100;
			}

			if (Dac_1 != 0xff) {
				m_DAC_OFFSET = (Dac_1 - 0x80) * 4;
			} else {
				////InitValue --;
			}

			return RET_ERROR;
		}
	}

	//unsigned char ucCnt;

	if (sRingMode == FP_RING_FINGER_OFF) {
		EnterRingMode();

		sRingMode = FP_RING_FINGER_DETECTING;

		WriteSfrRegister(0x12, 0x00);
	}

	if (sRingMode == FP_RING_FINGER_DETECTING) {
		if (GetRingStatus() == FP_RING_FINGER_ON) {
			sRingMode = FP_RING_FINGER_ON_FIRSTTIME;
		} else {
			if (sDelayFrame < 60) {
				sDelayFrame++;

				WriteSfrRegister(0x12, 0x00);

				focal_usleep(10000);

				return RET_ERROR;
			} else {
				sDelayFrame = 0;
				focal_fp_spi_reset();
			}
		}
	}

	if ((sRingMode == FP_RING_FINGER_OFF)
			|| (sRingMode == FP_RING_FINGER_DETECTING)) {
		EnterRingMode();

		focal_usleep(30000);

		g_FpAfeScanParam.m_agc_param.m_ring_value = AppGetRingAdcValue();

		g_FpAfeScanParam.m_agc_param.m_ring_diff =
				g_FpAfeScanParam.m_agc_param.m_ring_base
						- g_FpAfeScanParam.m_agc_param.m_ring_value;

		if (g_FpAfeScanParam.m_agc_param.m_ring_diff
				< (-1) * FP_RING_BASE_REFRESH_THRES) {
			AppRingBaseRefresh();
		} else if (g_FpAfeScanParam.m_agc_param.m_ring_diff
				> FP_RING_BASE_REFRESH_THRES) {
		} else {
			AppRingBaseCompensate(g_FpAfeScanParam.m_agc_param.m_ring_diff);
		}

		EnterRingMode();
		return RET_ERROR;
	}

	//sRingMode = FP_RING_FINGER_ON;

	if (sRingMode == FP_RING_FINGER_ON) {
		EnterRingMode();
		focal_usleep(20000);

		g_FpAfeScanParam.m_agc_param.m_ring_value = AppGetRingAdcValue();

		g_FpAfeScanParam.m_agc_param.m_ring_diff =
				g_FpAfeScanParam.m_agc_param.m_ring_base
						- g_FpAfeScanParam.m_agc_param.m_ring_value;

		//WriteSRamRegister(0x1a04,0x0003);
		if (g_FpAfeScanParam.m_agc_param.m_ring_diff < FP_RING_EXIT_THRES) {
			InitSensor();

			sRingMode = FP_RING_FINGER_OFF;

			InitValue = 0;

			return RET_ERROR;
		}
	}

	WriteSRamRegister(0x1b20, 0x03);

	if (sRingMode == FP_RING_FINGER_ON_FIRSTTIME) {
		sRingMode = FP_RING_FINGER_ON;
	}

	return RET_OK;
}


/*********************************************************************************/
