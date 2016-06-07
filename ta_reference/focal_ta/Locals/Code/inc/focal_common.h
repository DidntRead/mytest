#ifndef __FOCAL_COMMON_H__
#define __FOCAL_COMMON_H__


#define IMAGE_MAX_X		88
#define IMAGE_MAX_Y		88


#define CMD_READ_SRAM_1			0x04
#define CMD_READ_SRAM_2			0xfb

#define CMD_WRITE_SRAM_1			0x05
#define CMD_WRITE_SRAM_2			0xfa

#define CMD_READ_SFR_BULK_1		0x06
#define CMD_READ_SFR_BULK_2		0xf9

#define CMD_WRITE_SFR_BULK_1		0x07
#define CMD_WRITE_SFR_BULK_2		0xf8

#define CMD_READ_SFR_SINGLE_1 		0x08
#define CMD_READ_SFR_SINGLE_2 		0xf7

#define CMD_WRITE_SFR_SINGLE_1		0x09
#define CMD_WRITE_SFR_SINGLE_2		0xf6

#define CMD_SOFT_RESET_1			0x0b
#define CMD_SOFT_RESET_2			0xf3

#define CMD_AFE_RESET_1				0x0c
#define CMD_AFE_RESET_2				0xf2


#define FP_DEFAULT_SCAN_TIME 		4

typedef struct {
	short m_macroblock_scan_time;
	short m_mb_freq_sel;
	short m_scan_mode;
	char afe_cds_gn_sel;	//agc3
	char adc_gn_sel;		//agc4
	char afe_s2_gn_sel; 	//agc2
	char afe_s1_gn_sel; 	//agc1
	
	short dac1_base;	//0x38	
	short dac2_gray;	//0x39
} fp_afe_scan_param_t;


#endif/* __FOCAL_COMMON_H__*/
