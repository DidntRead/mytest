#ifndef __FOCAL_REGISTERLIST_H__
#define  __FOCAL_REGISTERLIST_H__

#define REG_MAIN_CLK_ICG_ID				0x10
#define REG_MAIN_MEN_ICG_SW_ID		  	0x14 
#define REG_ENCRYPTE_EN				   		0x09 
#define REG_PMU_ADD_ADJ_ID			   		0x13 
#define REG_PMU_VDDA_EN_ID			   		0x18 
#define REG_GPIO_OUTEN				   		0x0a 
#define REG_CLK_TEST_EN				   		0x0B 

#define REG_COUNTER_TH1_ID			   		0x19 
#define REG_COUNTER_TH2_ID			   		0x1A 
#define REG_OSC_250K_CTRL_ID			   	0x1B 
#define REG_OSC_250K_EN_ID			   		0x1C 
#define REG_OSC_250K_OE_ID			   		0x1E 
#define REG_OSC_250K_START_ID			   	0x33 
#define REG_FREQ_ADJ_ID				   		0x0E 
#define REG_MAIN_CLK_DIV_ID			   	0x0F 
#define REG_MAIN_LATCH_CTRL			   	0x11 
#define REG_MAIN_CLK_SHUT_DOWN_BOOST_ID	0x12 
#define REG_PMU_SS_EN_N_ID			   		0x16 
#define REG_PMU_VDDA_POR_N_ID			   	0x17 
#define REG_OSC_TEST_SW_ID			   		0x1D 


//=======================================================================
#define REG_AFE_CONTROL_BASE	  	  0x1800  

#define REG_afe_ctrl_reg				  (REG_AFE_CONTROL_BASE + 0x33)    
#define REG_afe_adj_reg				  (REG_AFE_CONTROL_BASE + 0x34)    
#define REG_afe_state_reg				  (REG_AFE_CONTROL_BASE + 0x35)    
#define REG_macroblock_freq_sel		  (REG_AFE_CONTROL_BASE + 0x20)    
#define REG_macroblock_scan_time		  (REG_AFE_CONTROL_BASE + 0x21)    
#define REG_afe_s2_rst_pull_up		  	  (REG_AFE_CONTROL_BASE + 0x22)    
#define REG_afe_s2_rst_pull_down		  (REG_AFE_CONTROL_BASE + 0x23)    
#define REG_afe_ch_sel_start_position	  (REG_AFE_CONTROL_BASE + 0x24)    
#define REG_afe_ch_sel_switch_position	  (REG_AFE_CONTROL_BASE + 0x25)    
#define REG_afe_cdsx_smp_position		  (REG_AFE_CONTROL_BASE + 0x26)    
#define REG_afe_cdsx_hold_position	  	  (REG_AFE_CONTROL_BASE + 0x27)    
#define REG_dac1_rst_n_pull_up		  (REG_AFE_CONTROL_BASE + 0x28)    
#define REG_dac1_rst_n_pull_down		  (REG_AFE_CONTROL_BASE + 0x29)    
#define REG_col_dummy				  (REG_AFE_CONTROL_BASE + 0x2a)    
#define REG_row_dummy				  (REG_AFE_CONTROL_BASE + 0x2b)    
#define REG_frm_dummy				  (REG_AFE_CONTROL_BASE + 0x2c)    
#define REG_adc_set					  (REG_AFE_CONTROL_BASE + 0x2d)    
#define REG_scan_mode				  (REG_AFE_CONTROL_BASE + 0x2e)    
#define REG_tx_pull_up				  (REG_AFE_CONTROL_BASE + 0x2f)    
#define REG_dac1_update_position		  (REG_AFE_CONTROL_BASE + 0x30)    
#define REG_dac2_update_position		  (REG_AFE_CONTROL_BASE + 0x31)    
#define REG_global_shift_data			  (REG_AFE_CONTROL_BASE + 0x36)    
#define REG_pagc_gn					  (REG_AFE_CONTROL_BASE + 0x37)    
#define REG_dac1_base				  (REG_AFE_CONTROL_BASE + 0x38)    
#define REG_dac2_gray				  (REG_AFE_CONTROL_BASE + 0x39)    
#define REG_sub_idle_cycle			  	  (REG_AFE_CONTROL_BASE + 0x3a)    
#define REG_spoof_scan_cycle			  (REG_AFE_CONTROL_BASE + 0x3b)  
#define REG_spoof_sample_start		  (REG_AFE_CONTROL_BASE + 0x3c)  
#define REG_sensor_addr0_end			  (REG_AFE_CONTROL_BASE + 0x10)  
#define REG_sensor_addr0_start		  (REG_AFE_CONTROL_BASE + 0x00)  
#define REG_sensor_addr1_end			  (REG_AFE_CONTROL_BASE + 0x11)  
#define REG_sensor_addr1_start		  (REG_AFE_CONTROL_BASE + 0x01)  
#define REG_sensor_addr2_end			  (REG_AFE_CONTROL_BASE + 0x12)  
#define REG_sensor_addr2_start		  (REG_AFE_CONTROL_BASE + 0x02)  
#define REG_sensor_addr3_end			  (REG_AFE_CONTROL_BASE + 0x13)  
#define REG_sensor_addr3_start		  (REG_AFE_CONTROL_BASE + 0x03)  
#define REG_sensor_addr4_end			  (REG_AFE_CONTROL_BASE + 0x14)  
#define REG_sensor_addr4_start		  (REG_AFE_CONTROL_BASE + 0x04)  
#define REG_sensor_addr5_end			  (REG_AFE_CONTROL_BASE + 0x15)  
#define REG_sensor_addr5_start		  (REG_AFE_CONTROL_BASE + 0x05)  
#define REG_sensor_addr6_end			  (REG_AFE_CONTROL_BASE + 0x16)  
#define REG_sensor_addr6_start		  (REG_AFE_CONTROL_BASE + 0x06)  
#define REG_sensor_addr7_end			  (REG_AFE_CONTROL_BASE + 0x17)  
#define REG_sensor_addr7_start		  (REG_AFE_CONTROL_BASE + 0x07)  
#define REG_sensor_addr8_end			  (REG_AFE_CONTROL_BASE + 0x18)  
#define REG_sensor_addr8_start		  (REG_AFE_CONTROL_BASE + 0x08)  
#define REG_sensor_addr9_end			  (REG_AFE_CONTROL_BASE + 0x19)  
#define REG_sensor_addr9_start		  (REG_AFE_CONTROL_BASE + 0x09)  
#define REG_sensor_addra_end			  (REG_AFE_CONTROL_BASE + 0x1a)  
#define REG_sensor_addra_start		  (REG_AFE_CONTROL_BASE + 0x0a)  
#define REG_sensor_addrb_end			  (REG_AFE_CONTROL_BASE + 0x1b)  
#define REG_sensor_addrb_start		  (REG_AFE_CONTROL_BASE + 0x0b)  
#define REG_sensor_addrc_end			  (REG_AFE_CONTROL_BASE + 0x1c)  
#define REG_sensor_addrc_start		  (REG_AFE_CONTROL_BASE + 0x0c)  
#define REG_sensor_addrd_end			  (REG_AFE_CONTROL_BASE + 0x1d)  
#define REG_sensor_addrd_start		  (REG_AFE_CONTROL_BASE + 0x0d)  
#define REG_sensor_addre_end			  (REG_AFE_CONTROL_BASE + 0x1e)  
#define REG_sensor_addre_start		  (REG_AFE_CONTROL_BASE + 0x0e)  
#define REG_sensor_addrf_end			  (REG_AFE_CONTROL_BASE + 0x1f)  
#define REG_sensor_addrf_start		  	  (REG_AFE_CONTROL_BASE + 0x0f)  
#define REG_cfg_afe_reg0				  (REG_AFE_CONTROL_BASE + 0x50)  
#define REG_cfg_afe_reg1				  (REG_AFE_CONTROL_BASE + 0x51)  
#define REG_cfg_sensor_reg			  (REG_AFE_CONTROL_BASE + 0x52)  
#define REG_cfg_dac_reg				  (REG_AFE_CONTROL_BASE + 0x53)  
#define REG_cfg_adc_reg				  (REG_AFE_CONTROL_BASE + 0x54)  
#define REG_cfg_ref_reg				  (REG_AFE_CONTROL_BASE + 0x55)  
#define REG_cfg_ndrv0_reg			  (REG_AFE_CONTROL_BASE + 0x56)  
#define REG_cfg_ndrv1_reg			  (REG_AFE_CONTROL_BASE + 0x57)  
#define REG_cfg_ndrv2_reg		  	  (REG_AFE_CONTROL_BASE + 0x58)  
#define REG_cfg_ndrv3_reg			  (REG_AFE_CONTROL_BASE + 0x59)  
#define REG_cfg_test_mode_reg		  (REG_AFE_CONTROL_BASE + 0x5a)  


//=======================================================================
#define REG_FINGER_DETECT_BASE		   		0x1B00  

#define REG_RING_SLEEP_TIME_ID		   		(REG_FINGER_DETECT_BASE + 0x00)  	
#define REG_RING_TIME_F1_ID			   		(REG_FINGER_DETECT_BASE + 0x01)  	
#define REG_RING_TIME_F2_ID			   		(REG_FINGER_DETECT_BASE + 0x02)  	
#define REG_RING_TIME_ACC_ID	  	   		(REG_FINGER_DETECT_BASE + 0x03)  	
#define REG_RING_TIME_SEC_ID		  		(REG_FINGER_DETECT_BASE + 0x04)  	
#define REG_RING_ACC_H_ID 			  	 	(REG_FINGER_DETECT_BASE + 0x05)  	
#define REG_RING_ACC_L_ID 			   		(REG_FINGER_DETECT_BASE + 0x06)  	
#define REG_SENSOR_ACC_H_ID		   		(REG_FINGER_DETECT_BASE + 0x07)  	
#define REG_SENSOR_ACC_L_ID		   		(REG_FINGER_DETECT_BASE + 0x08)  	
#define REG_FRAUD_ACC_H_ID			   		(REG_FINGER_DETECT_BASE + 0x09)  	
#define REG_FRAUD_ACC_L_ID			   		(REG_FINGER_DETECT_BASE + 0x0A)  	
#define REG_LEAVE_ACC_H_ID			   		(REG_FINGER_DETECT_BASE + 0x0B)  	
#define REG_LEAVE_ACC_L_ID			   		(REG_FINGER_DETECT_BASE + 0x0C)  	
#define REG_ACC_REF_RING_H_ID 		   		(REG_FINGER_DETECT_BASE + 0x0D)  	
#define REG_ACC_REF_RING_L_ID 		   		(REG_FINGER_DETECT_BASE + 0x0E)  	
#define REG_ACC_REF_SENSOR_H_ID	   		(REG_FINGER_DETECT_BASE + 0x0F)  	
#define REG_ACC_REF_SENSOR_L_ID	   		(REG_FINGER_DETECT_BASE + 0x10)  	
#define REG_ACC_REF_FRAUD_H_ID		   		(REG_FINGER_DETECT_BASE + 0x11)  	
#define REG_ACC_REF_FRAUD_L_ID		   		(REG_FINGER_DETECT_BASE + 0x12)  	
#define REG_ACC_REF_LEAVE_H_ID		   		(REG_FINGER_DETECT_BASE + 0x13)  	
#define REG_ACC_REF_LEAVE_L_ID		   		(REG_FINGER_DETECT_BASE + 0x14)  	
#define REG_SENSOR_AREA_ACC_H_ID	   		(REG_FINGER_DETECT_BASE + 0x15)  	
#define REG_SENSOR_AREA_ACC_L_ID	   		(REG_FINGER_DETECT_BASE + 0x16)  	
#define REG_MEET_COND_TIME_ID 		   		(REG_FINGER_DETECT_BASE + 0x17)  

#define REG_HOST_MODE_CMD 			   		(REG_FINGER_DETECT_BASE + 0x20)  	
#define REG_HOST_DEBUG_ID 			   		(REG_FINGER_DETECT_BASE + 0x21)  	
#define REG_SENSOR_BEYOND_ACC_TH_NUM_ID	(REG_FINGER_DETECT_BASE + 0x1F)  	
#define REG_HIGH_VOL_SET_UP_ID		   		(REG_FINGER_DETECT_BASE + 0x18)  	
#define REG_FINGER_DET_STATE_ID		   	(REG_FINGER_DETECT_BASE + 0x19)  	
#define REG_R_RING_CNT_F1_ID			   	(REG_FINGER_DETECT_BASE + 0x1A)  	
#define REG_R_RING_CNT_F2_ID			   	(REG_FINGER_DETECT_BASE + 0x1B)  	
#define REG_R_RING_ACC_H_ID			   	(REG_FINGER_DETECT_BASE + 0x1C)  	
#define REG_R_RING_ACC_L_ID			   	(REG_FINGER_DETECT_BASE + 0x1D)  	
#define REG_R_TOUCH_COORDINATE_ID 	   	(REG_FINGER_DETECT_BASE + 0x1E)  	
#define REG_HIGH_LOW_TH_SW_ID 		   	(REG_FINGER_DETECT_BASE + 0x22)  	
#define REG_R_BUSY_ID 				   		(REG_FINGER_DETECT_BASE + 0x23)  

//=======================================================================
#define REG_FINGER_IMAGE_BASE		   		0x1C00  


#endif /* __FOCAL_REGISTERLIST_H__*/
