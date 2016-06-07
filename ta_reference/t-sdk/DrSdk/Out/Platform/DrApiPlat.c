/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include "DrApi/DrApi.h"
#include "DrApi/DrApiPlat.h"

typedef volatile unsigned int *P_U32;
#define MT_VER_BASE        (0x08000000)
static unsigned char *mt_ver_vaddr;
#define MT_APHW_VER_VA_BUFFER (mt_ver_vaddr)
#define MT_APHW_CODE        ((P_U32)(MT_APHW_VER_VA_BUFFER+0x00))
#define MT_APHW_SUBCODE     ((P_U32)(MT_APHW_VER_VA_BUFFER+0x04))
#define MT_APHW_VER         ((P_U32)(MT_APHW_VER_VA_BUFFER+0x08))
#define MT_APSW_VER         ((P_U32)(MT_APHW_VER_VA_BUFFER+0x0C))
static unsigned int mt_chip_hw_code = 0x0;
static unsigned int mt_chip_hw_subcode = 0x0;
static unsigned int mt_chip_hw_ver = 0x0;
static unsigned int mt_chip_sw_ver = 0x0;
static unsigned int mt_chip_initialized = 0;

static void mt_platform_init(void)
{
#if TBASE_API_LEVEL >= 5
    if(!mt_chip_initialized)
    {
        drApiResult_t ret;
        
        //map register set
        if((ret = drApiMapPhysicalBuffer((uint64_t)MT_VER_BASE, SIZE_4KB, MAP_HARDWARE, (void **)&MT_APHW_VER_VA_BUFFER)) != DRAPI_OK){
            drApiLogPrintf("[mt_platform_init] map MT_VER_BASE failed! ERROR: %d\n", ret);
        }

        //read register set
        mt_chip_hw_code = *(MT_APHW_CODE); 
        mt_chip_hw_subcode = *(MT_APHW_SUBCODE); 
        mt_chip_hw_ver = *(MT_APHW_VER); 
        mt_chip_sw_ver = *(MT_APSW_VER); 

#if 0
        drApiLogPrintf("[mt_platform_init] MT_APHW_CODE : 0x%x\n", mt_chip_hw_code);
        drApiLogPrintf("[mt_platform_init] MT_APHW_SUBCODE : 0x%x\n", mt_chip_hw_subcode);
        drApiLogPrintf("[mt_platform_init] MT_APHW_VER : 0x%x\n", mt_chip_hw_ver);
        drApiLogPrintf("[mt_platform_init] MT_APSW_VER : 0x%x\n", mt_chip_sw_ver);
#endif
        //unmap register set
        drApiUnmapBuffer(MT_APHW_VER_VA_BUFFER);
        
        mt_chip_initialized = 1;
    }
#endif    
}

unsigned int mt_get_chip_hw_code(void)
{
#if TBASE_API_LEVEL >= 5
    mt_platform_init();

    return mt_chip_hw_code;
#else
    return 0x0;
#endif
}
