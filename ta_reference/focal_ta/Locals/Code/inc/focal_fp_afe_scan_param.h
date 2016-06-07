#ifndef __FOCAL_FP_AFE_SCAN_PARAM_H__
#define __FOCAL_FP_AFE_SCAN_PARAM_H__

union PAGC_GN
{
	struct _GAIN{
		unsigned short reserve;
		unsigned short afe_cds_gn_sel;	//agc3
		unsigned short adc_gn_sel;	//agc4
		unsigned short afe_s2_gn_sel; //agc2
		unsigned short afe_s1_gn_sel; //agc1
	}GAIN;
	unsigned short pagc_gn;
};
struct AGC_ADJUST_PARAM
{
	union PAGC_GN PagcGn;		//0x37
	unsigned short dac1_base;	//0x38
	unsigned short dac2_gray;	//0x39

	unsigned char   m_Image_Score;
	unsigned short m_ring_value;
	unsigned short m_ring_diff;
	unsigned short m_ring_base;
};

union AFE_SCAN_MODE
{
	 	struct _SCAN_MODE_{
		unsigned short reserve;
		unsigned short mix_para_sel;
		unsigned short dacl_para_sel;
		unsigned short afe_ch_sel_sequence;
		unsigned short scan_mode_sel;
		unsigned short agc_num_sel;
		unsigned short row_addr_inc_step;
	}SCAN_MODE;
	unsigned short scan_mode;
};

struct COLUMN_ROW_START_END_MSG
{
	unsigned short startColumn;
	unsigned short startRow;
	unsigned short endColumn;
	unsigned short endRow;
	short  maxFirstValue;
	short  minFirstValue;

	double factor2;
};

struct FpAfeScanParamStru
{
	unsigned short m_macroblock_scan_time;
	unsigned short m_mb_freq_sel;
	unsigned short m_scan_mode;
	struct AGC_ADJUST_PARAM m_agc_param;
	union AFE_SCAN_MODE m_afeScanMode;

	struct COLUMN_ROW_START_END_MSG ColumnRowStarEnd;

};

#endif/*__FOCAL_FP_AFE_SCAN_PARAM_H__*/
