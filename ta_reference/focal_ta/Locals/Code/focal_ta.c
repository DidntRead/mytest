#include <stdint.h>

#include "focal_def.h"
#include "tlStd.h"
#include "TlApi/TlApi.h"
#include "tci.h"
#include "focal_main.h"
#include "focal_db.h"
#include "MobiCoreDriverApi.h"

#include "focal_platform.h"
#include "focal_main_test.h"
#include "FpSensorLib.h"

DECLARE_TRUSTED_APPLICATION_MAIN_STACK(512*1024);

extern uint32_t focal_mem_hint;
tci_command_id_t _focal_ta_current_command;
int32_t _focal_ta_current_progress;
uint32_t _focal_ta_current_event;

static focal_def_return_t _focal_ta_store_one_template(
        void* data, uint32_t* size, uint32_t index);

static focal_def_return_t _focal_ta_load_one_template(
        void* data, uint32_t size, uint32_t index);

uint16_t g_sensor_width = 88;
uint16_t g_sensor_height = 88;

/******************************************************************************/
static uint32_t _focal_ta_transfer_data(uint32_t cid, tci_message_t* msg) {
    uint32_t ts_dummy = 0;
    uint32_t ts_start = 0;
    uint32_t ts_stop = 0;
    bool mem_valid;
    uint32_t size;
    uint32_t index;
    uint32_t i;
    uint16_t rbuf[88 * 88] = {0};
    uint8_t imagebuf[88 * 88] = {0};
    uint8_t buf = 1;
    uint16_t buf2 = 0;
    void* data;
    focal_lib_return_t ret_lib;
    focal_def_return_t ret_def;
    int32_t ret;
    uint8_t sbuf[24] = {0};
    uint8_t tx_buf[4] = {0};
    uint8_t rx_buf[4] = {0};
     
    ret_def = FOCAL_LIB_ERROR_GENERAL;

    data = msg->data_struct.sVirtualAddr;
    size = msg->data_struct.sVirtualLen;
    index = msg->data;
    char *p = data;
    int *state = data;
    focal_lib_enroll_data_t *enroll_data = data;
    focal_lib_identify_data_t *identity_data = data;
        
    mem_valid = tlApiIsNwdBufferValid(data, size);

    focal_debug("Transfer data at 0x%02X, %d", data, size);

    if (false == mem_valid) {
    	focal_debug("Memory map from TLC invalid for transfer,"
    	        "address 0x%08X, length %d", data, size);

    	return FOCAL_DEF_ERROR_MEMORY;
    }

    	switch (cid) {
        case TCI_FOCAL_CMD_ENROLL:
            focal_debug("%s, CMD_ENROLL", __func__);
            ret_lib = focal_main_enroll(data, size);
            break;

        case TCI_FOCAL_CMD_IDENTIFY:
            focal_debug("%s, CMD_IDENTIFY", __func__);
            ret_lib = focal_main_identify(data, size);
            break;

        case TCI_FOCAL_CMD_BEGIN_IDENTIFY:
            focal_debug("%s, CMD_BEGIN_IDENTIFY", __func__);
            ret_lib = focal_main_begin_identify(data, size);
            break;

        case TCI_FOCAL_CMD_EXTENDED_ENROLL_DATA:
            focal_debug("%s, CMD_EXTENDED_ENROLL_DATA", __func__);
            ret_lib = focal_main_get_extended_data(data, size);
            break;

        case TCI_FOCAL_CMD_DEBUG_RETRIEVE_IMAGE:
            focal_debug("%s, CMD_DEBUG_RETRIEVE_IMAGE", __func__);
            ret_lib = focal_main_debug_retrieve_image(data, size);
            break;
        case TCI_FOCAL_CMD_CHECK_FINGER_PRESENT:
            focal_debug("TCI_FOCAL_CMD_CHECK_FINGER_PRESENT");
            ret = focal_DetectFinger();
            focal_debug("finger state = %d", ret);
            state[0] = ret;
            ret_lib = FOCAL_LIB_OK;
            break;
        case TCI_FOCAL_CMD_GET_IDS:
            focal_debug("%s, CMD_GET_IDS", __func__);
            ret_lib = focal_main_get_ids(data, &size);
            break;

        case TCI_FOCAL_CMD_DEBUG_INJECT_IMAGE:
            focal_debug("%s, CMD_DEBUG_INJECT_IMAGE", __func__);
            ret_lib = focal_main_debug_inject_image(data, size);
            break;

        case TCI_FOCAL_CMD_STORE_ONE_TEMPLATE:
            focal_debug("%s, CMD_STORE_ONE_TEMPLATE", __func__);
            ret_def = _focal_ta_store_one_template(data, &size, index);
            if (FOCAL_DEF_OK == ret_def) {
                ret_lib = FOCAL_LIB_OK;
            } else {
                ret_lib = FOCAL_LIB_ERROR_GENERAL;
            }
            break;

        case TCI_FOCAL_CMD_LOAD_ONE_TEMPLATE:
            focal_debug("%s, CMD_LOAD_ONE_TEMPLATE", __func__);
            ret_def = _focal_ta_load_one_template(data, size, index);
            if (FOCAL_DEF_OK == ret_def) {
                ret_lib = FOCAL_LIB_OK;
            } else {
                ret_lib = FOCAL_LIB_ERROR_GENERAL;
            }
            break;

       case TCI_FOCAL_CMD_DEADPIXEL_TEST:
            focal_debug("%s, TCI_FOCAL_CMD_DEADPIXEL_TEST", __func__);
    //wenfs            ret_def = focal_lib_test_deadpixels((int32_t*)data);
            focal_debug("%s, focal_lib_test_deadpixels, *data=%d", __func__, *((int32_t *)data));
            if (FOCAL_DEF_OK == ret_def) {
                ret_lib = FOCAL_LIB_OK;
            } else {
                ret_lib = FOCAL_LIB_ERROR_GENERAL;
            }
            break;
            
        case TCI_FOCAL_CMD_GET_IMAGE_BUF:
        #if 1    
            //focal_SetFpSensorColsRows(1, 1);
            focal_GetLibVersion(sbuf);
            focal_debug("___focal_version = %s", sbuf);
            focal_SetFpSensorColsRows(88, 88);
            focal_InitFuncSpiWrite(platform_spi_write);
            focal_InitFuncSpiRead(platform_spi_read);
            focal_InitFuncUsleep(platform_usleep);
            focal_GetFpImage(imagebuf);
            memcpy(p, imagebuf, 88 * 88);
            //test();
        #endif    
#if 0
#if 1
            focal_get_timestamp(&ts_dummy, &ts_start);
            ret = GetFpImage(imagebuf);
            if (ret != 0)
            {
                focal_debug("GetFpImage error!");
            }
            focal_debug("GetFpImage success!");
            memcpy(p, imagebuf, 88 * 88);
            focal_get_timestamp(&ts_dummy, &ts_stop);
            focal_debug("==yj1122==[SPISPEED] getimage time1= %d=====", ts_stop - ts_start);
#else	
    	focal_get_timestamp(&ts_dummy, &ts_start);
    	focal_fp_sensor_read_sfr_register(0x10, &buf);
    	focal_get_timestamp(&ts_dummy, &ts_stop);
    	focal_debug("==1122==[SPISPEED] read sfr time1= %d=====", ts_stop - ts_start);
           p[1] = buf;
           focal_fp_sensor_read_sfr_register(0x14, &buf);
    	p[2] = buf;
#if 1	
    	focal_fp_sensor_write_sfr_register(0x10, 0xff);
    	focal_fp_sensor_write_sfr_register(0x14, 0xff);
    	focal_fp_sensor_write_sfr_register(0x09, 0x00);

    	focal_fp_sensor_write_sram_register(0x1c00, 12);
    	focal_fp_sensor_write_sram_register(0x1c01, 21);

    	focal_fp_sensor_read_sram_register(0x1c00, &buf2);
    	p[3] = buf2;
    	focal_fp_sensor_read_sram_register(0x1c01, &buf2);
    	p[4] = buf2;
    	

    	focal_get_timestamp(&ts_dummy, &ts_start);
    	for (i = 0; i < 188; i++)
    	{
    		focal_fp_sensor_write_sram_register(0x1c00 + i, i);
    	}
    	focal_fp_sensor_readbulk_sram(0x1c00, rbuf, 188);
    	for (i = 0; i < 187; i += 5)
    	{
    		focal_fp_sensor_read_sram_register(0x1c00 + i, &buf2);
    		focal_debug("_yanjun_focal_ %d %d %d %d %d \n", rbuf[i], rbuf[i + 1], rbuf[i + 2], rbuf[i + 3], rbuf[i + 4]);
    	}
    	focal_get_timestamp(&ts_dummy, &ts_stop);
    	focal_debug("====yj time2= %d=====", ts_stop - ts_start);
#endif	
#endif
#endif
    	     ret_lib = FOCAL_LIB_OK;
            break;
            
       case TCI_FOCAL_CMD_ENROLL_FINGER:
            focal_debug("_____1____TCI_FOCAL_CMD_ENROLL_FINGER");
            focal_Enroll(enroll_data->finger_id, enroll_data->finger_index, 
                    &(enroll_data->coverage));
            focal_debug("focal  id = %d index = %d coverage = %d", 
                enroll_data->finger_id, enroll_data->finger_index, enroll_data->coverage);
            ret_lib = FOCAL_LIB_OK;
            break;
            
       case TCI_FOCAL_CMD_MATCH_FINGER:
            focal_debug("_____2____TCI_FOCAL_CMD_MATCH_FINGER");
            ret = focal_Identify(&identity_data->index, &identity_data->score);
            if (ret == 0)
                identity_data->result = FOCAL_LIB_IDENTIFY_MATCH;
            else 
                identity_data->result = FOCAL_LIB_IDENTIFY_NO_MATCH;
            ret_lib = FOCAL_LIB_OK;    
            break;
            
       case TCI_FOCAL_CMD_END_ENROLL:
            focal_debug("-----------id = %d----------1\n", enroll_data->finger_id);
            focal_SaveAlgTplData(enroll_data->finger_id, &enroll_data->type,
                &enroll_data->size, enroll_data->tpl);
            focal_debug("---------------------2\n");
            ret_lib = FOCAL_LIB_OK;
            break;
            
       default:
            focal_debug("Unknown command");
            return FOCAL_LIB_ERROR_GENERAL;
    }

    msg->rsp.cmd_ret = ret_lib;
    msg->data_size = size;

    return FOCAL_DEF_OK;
}

/******************************************************************************/
static focal_def_return_t _focal_ta_receive_cmd(tci_message_t* msg) {
    focal_lib_return_t ret_lib;
    focal_def_return_t ret_def;
    uint32_t data;
    uint32_t template_size;

    ret_lib = FOCAL_LIB_ERROR_GENERAL;
    data = msg->data;

    switch(msg->cmd.focal_cmd) {

        case TCI_FOCAL_CMD_INIT:
            focal_InitFuncSpiWrite(platform_spi_write);
            focal_InitFuncSpiRead(platform_spi_read);
            focal_InitFuncUsleep(platform_usleep);
            focal_SetFpSensorColsRows(88, 88);
            focal_SetEnrollMaxTpl(8);
            focal_InitFpAlg();
            focal_debug("%s, CMD_INIT", __func__);
            ret_lib = FOCAL_LIB_OK;
//            ret_lib = focal_main_init();
            break;

        case TCI_FOCAL_CMD_DEINIT:
            focal_debug("%s, CMD_DEINIT", __func__);
//            ret_lib = focal_lib_deinit();
            break;

        case TCI_FOCAL_CMD_END_ENROLL:
            focal_debug("%s, CMD_END_ENROLL", __func__);
 //           ret_lib = focal_main_end_enroll(&data);
            break;

        case TCI_FOCAL_CMD_END_IDENTIFY:
            focal_debug("%s, CMD_END_IDENTIFY", __func__);
//            ret_lib = focal_main_end_identify();
            break;

        case TCI_FOCAL_CMD_CHECK_FINGER_PRESENT:
            focal_debug("%s, CMD_CHECK_FINGER_PRESENT", __func__);
//            ret_lib = focal_lib_finger_present(&data);
            break;

        case TCI_FOCAL_CMD_CHECK_FINGER_LOST:
            focal_debug("%s, CMD_CHECK_FINGER_LOST", __func__);
//            ret_lib = focal_lib_check_finger_lost(&data);
            break;

        case TCI_FOCAL_CMD_CAPTURE_IMAGE:
            focal_debug("%s, CMD_CAPTURE_IMAGE begin", __func__);
//            ret_lib = focal_lib_capture_image_raw();//samuel test
            //ret_lib = focal_lib_capture_image();
            focal_debug("%s, CMD_CAPTURE_IMAGE end", __func__);
            break;

        case TCI_FOCAL_CMD_BEGIN_ENROLL:
            focal_debug("%s, CMD_BEGIN_ENROLL", __func__);
//            ret_lib = focal_main_begin_enroll(data);
            break;

        case TCI_FOCAL_CMD_INIT_FINGER_DETECT:
            focal_debug("%s, CMD_INIT_FINGER_DETECT", __func__);
//            ret_lib = focal_lib_finger_present(&data);
            if (ret_lib != FOCAL_LIB_ENABLE_EVENT_FINGER_PRESENT) {
                focal_debug("Finger present in wrong start state %d\n", ret_lib);
//wenfs                focal_lib_check_finger_lost(&data);
            }
            break;

        case TCI_FOCAL_CMD_START_FINGER_DETECT:
            focal_debug("%s, CMD_START_FINGER_DETECT", __func__);
//            ret_lib = focal_lib_finger_present(&data);
            if (ret_lib != FOCAL_LIB_WAIT_EVENT_FINGER_PRESENT) {
                focal_debug("Finger present in the wrong state %d\n", ret_lib);
//                focal_lib_check_finger_lost(&data);
            }
            break;

        case TCI_FOCAL_CMD_DELETE_TEMPLATE:
            focal_debug("%s, CMD_DELETE_TEMPLATE", __func__);
//            ret_def = focal_db_delete_template(data);
            if (ret_def < 0) {
                /* Template not found */
                data = 0;
                ret_lib = FOCAL_LIB_ERROR_PARAMETER;
            } else {
                data = ret_def;
                ret_lib = FOCAL_LIB_OK;
            }
            break;

        case TCI_FOCAL_CMD_WAKEUP_QUALIFICATION:
            focal_debug("%s, CMD_WAKEUP_QUALIFICATION", __func__);
//            ret_lib = focal_lib_wakeup_qualification();
            break;

        case TCI_FOCAL_CMD_WAKEUP_SETUP:
            focal_debug("%s, CMD_WAKEUP_SETUP", __func__);
//            ret_lib = focal_lib_wakeup_setup();
            break;

        case TCI_FOCAL_CMD_GET_TEMPLATE_SIZE:
            focal_debug("%s, CMD_GET_TEMPLATE_SIZE", __func__);
//            template_size = focal_db_get_template_size(data);
            data = MC_SO_SIZE(0, template_size);
            ret_lib = FOCAL_LIB_OK;
            break;

        case TCI_FOCAL_CMD_GET_IDS_COUNT:
            focal_debug("%s, CMD_GET_IDS_COUNT", __func__);
//            data = focal_db_get_id_count();
            ret_lib = FOCAL_LIB_OK;
            break;

        case TCI_FOCAL_CMD_DELETE_ALL_TEMPLATES:
            focal_debug("TCI_FOCAL_CMD_DELETE_ALL_TEMPLATES called \n");
//            focal_db_delete_template_db();
            ret_lib = FOCAL_LIB_OK;
            break;
            
        case TCI_FOCAL_CMD_SPI_REGISTER_TEST:
        	focal_debug("[focal ta]TCI_FOCAL_CMD_SPI_REGISTER_TEST!");
        	data = focal_test_spi_register_test();
        	if(data != 0)
        	{
        		focal_debug("TCI_FOCAL_CMD_SPI_REGISTER_TEST error!\n");
        		ret_lib = FOCAL_LIB_ERROR_SENSOR;
        	}
        	else
        	{
        		focal_debug("TCI_FOCAL_CMD_SPI_REGISTER_TEST ok!\n");
        		ret_lib = FOCAL_LIB_OK;//FOCAL_LIB_OK;
        	}
        	break;
#if 0
        case TCI_FOCAL_CMD_SPI_REGISTER_TEST1:
        	focal_debug("[focal ta]TCI_FOCAL_CMD_SPI_REGISTER_TEST!");
        	data = focal_spi_register_test1();
        	if(data != 0)
        	{
        		focal_debug("TCI_FOCAL_CMD_SPI_REGISTER_TEST error!\n");
        		ret_lib = FOCAL_LIB_ERROR_SENSOR;
        	}
        	else
        	{
        		focal_debug("TCI_FOCAL_CMD_SPI_REGISTER_TEST ok!\n");
        		ret_lib = FOCAL_LIB_OK;//FOCAL_LIB_OK;
        	}
        	break;
#endif
			
        default:
            focal_debug("No recognized command!");
            return FOCAL_LIB_ERROR_GENERAL;
    }

    msg->rsp.cmd_ret = ret_lib;
    msg->data = data;
    focal_debug("%s, returns %d, %d.", __func__, ret_lib, data);

    return FOCAL_DEF_OK;
}

/******************************************************************************/
static focal_def_return_t _focal_ta_load_one_template(
        void* data, uint32_t size, uint32_t index) {

    tlApiResult_t ret_tlapi;
    focal_def_return_t ret_def;
    size_t plain_size;
    size_t encrypt_size;
    void* template_db;

    encrypt_size = size;
    plain_size = size;

    focal_debug("-->%s", __func__);

    template_db = malloc(plain_size);
    if (NULL == template_db) {
        focal_debug("%s, failed to allocated memory for template loading",
                __func__);

        ret_def = FOCAL_DEF_ERROR_MEMORY;
        goto error;
    }

    ret_tlapi = tlApiUnwrapObjectExt(
                    data,
                    encrypt_size,
                    template_db,
                    &plain_size,
                    TLAPI_UNWRAP_PERMIT_CONTEXT_DEVICE);

    if (E_TLAPI_CR_WRONG_OUTPUT_SIZE == ret_tlapi) {

        focal_debug("unwrapped database too large size %d, real size %d. Trying"
                    "new size", size, plain_size);

        free(template_db);
        template_db = malloc(plain_size);
        if (NULL == template_db) {
            focal_debug("%s, failed to allocated memory for template loading",
                __func__);

            ret_def = FOCAL_LIB_ERROR_MEMORY;
            goto error;
        }

        ret_tlapi = tlApiUnwrapObjectExt(
                        data,
                        encrypt_size,
                        template_db,
                        &plain_size,
                        TLAPI_UNWRAP_PERMIT_CONTEXT_DEVICE);

    }

    if (TLAPI_OK != ret_tlapi) {
        focal_debug("unwrapping data base failed 0x%02X.", ret_tlapi);
        ret_def = FOCAL_DEF_ERROR_TAC;
        goto error;
    }

    focal_debug("Database decrypted successfully! plain_size %d", plain_size);
    ret_def = focal_db_load_one_template(template_db, plain_size, index);
    if (FOCAL_DEF_OK != ret_def){
        focal_debug("%s, failed to load template. %d", __func__, ret_def);
        goto error;
    }

error:
    if (template_db != NULL) {
        free(template_db);
        template_db = NULL;
    }
   if (ret_def == FOCAL_DEF_OK) {
        focal_debug("<--%s, Template decrypted successfully! %d",
                __func__, size);

    } else {
        focal_debug("<--%s, template not decrypted ! %d", __func__, ret_def);
    }

    return ret_def;
}

/******************************************************************************/
static focal_def_return_t _focal_ta_store_one_template(
        void* data, uint32_t* size, uint32_t id)
{

    uint32_t plain_size;
    size_t encrypt_size;
    int32_t index;
    uint32_t* template = NULL;

    tlApiResult_t ret_tlapi;
    focal_def_return_t ret_def;
    focal_debug("-->%s storing id %d with size %d", __func__, id, *size);

    index = focal_db_get_db_index_by_id(id);
    if (index < 0) {
        focal_debug("%s index %d for id %d", __func__, index, id);
        ret_def = FOCAL_DEF_ERROR_PARAMETER;
        goto error;
    }

    plain_size = focal_db_get_size_by_index(index);
    if (plain_size == 0) {
        focal_debug("%s No template at index", __func__);
        ret_def = FOCAL_DEF_ERROR_PARAMETER;
        goto error;
    }

    focal_debug("%s, Dont fail me now! %d", __func__, plain_size);
    template = (uint32_t*) malloc(1024 * 1024);
    if (NULL == template) {
        focal_debug("%s, Failed to get memory for template.", __func__);
        ret_def = FOCAL_DEF_ERROR_PARAMETER;
        goto error;
    }

    focal_debug("%s, Dont fail me now!", __func__);
    ret_def = focal_db_store_one_template(template, plain_size, index);
    if (FOCAL_DEF_OK != ret_def) {
        focal_debug("%s, Failed to store template %d", __func__, ret_def);
        goto error;
    }

    encrypt_size = MC_SO_SIZE(0, plain_size);
    if (encrypt_size > *size) {
       focal_debug("%s, Encrypted template size too large! %d %d", __func__,
               encrypt_size, *size);

       ret_def = FOCAL_LIB_ERROR_MEMORY;
       *size = encrypt_size;
       goto error;
    }

    focal_debug("%s, encrypted size %d", __func__, encrypt_size);

    ret_tlapi = tlApiWrapObject(
            template,
            0,
            plain_size,
            data,
            &encrypt_size,
            MC_SO_CONTEXT_DEVICE,
            MC_SO_LIFETIME_PERMANENT,
            NULL,
            TLAPI_WRAP_DEFAULT);

    if (TLAPI_OK != ret_tlapi) {
        focal_debug("%s, Wrapping database failed. %d", __func__, ret_tlapi);
        ret_def = FOCAL_DEF_ERROR_TAC;
        goto error;
    }

    *size = encrypt_size;

error:
    if (template != NULL) {
        free(template);
        template = NULL;
    }
    if (ret_def == FOCAL_DEF_OK) {
        focal_debug("<--%s, template encrypted successfully! %d",
                __func__, *size);

    } else {
        focal_debug("<--%s, template not encrypted ! %d", __func__, ret_def);
    }

    return ret_def;
}

/******************************************************************************/
static tci_return_code_t _focal_ta_command(
        tci_message_t* message, tci_command_id_t cid) {

    tci_return_code_t ret = TCI_RET_OK;
    tci_focal_cmd_t fid = message->cmd.focal_cmd;

    /* Process command message */
    _focal_ta_current_command = cid;
    switch (cid) {

        case TCI_CMD_TRANSFER_DATA:
            focal_debug("Recieve transfer data 0x%02X", fid);
            ret = _focal_ta_transfer_data(fid, message);
            if (ret != FOCAL_DEF_OK) {
                ret = TCI_RET_ERR_UNKNOWN_CMD;
            } else {
                ret = TCI_RET_OK;
            }
            break;

        case TCI_CMD_SEND_CMD:
            focal_debug("Recieve command 0x%02X", fid);
            ret = _focal_ta_receive_cmd(message);
            if (ret != FOCAL_DEF_OK) {
                ret = TCI_RET_ERR_UNKNOWN_CMD;
            } else {
                ret = TCI_RET_OK;
            }
            break;
        default:
            focal_debug("Recieving unkown command %d focal command %d", cid, fid);
            ret = TCI_RET_ERR_UNKNOWN_CMD;
            break;

    } /* end switch (cid) */

    return ret;
}

/******************************************************************************/
_TLAPI_ENTRY void tlMain(const addr_t tci_buffer,
        const uint32_t tci_buffer_len) {

    // Initialization sequence
    tci_return_code_t ret;
    tci_command_id_t cid;
    uint32_t tlApiVersion;
    mcVersionInfo_t versionInfo;

    focal_debug("[<t Trusted Application Fingerprint Cards], Build "
            __DATE__ ", " __TIME__ EOL);

    ret = tlApiGetVersion(&tlApiVersion);
    if (TLAPI_OK != ret) {
        tlDbgPrintLnf("tlApiGetVersion failed with ret=0x%08X, exit", ret);
        tlApiExit(ret);
    }
    tlDbgPrintLnf("tlApi version 0x%08X", tlApiVersion);

    ret = tlApiGetMobicoreVersion(&versionInfo);
    if (TLAPI_OK != ret) {
        tlDbgPrintLnf("tlApiGetMobicoreVersion failed with "
                "ret=0x%08X, exit", ret);

        tlApiExit(ret);
    }
    tlDbgPrintLnf("productId        = %s",     versionInfo.productId);
    tlDbgPrintLnf("versionMci       = 0x%08X", versionInfo.versionMci);
    tlDbgPrintLnf("versionSo        = 0x%08X", versionInfo.versionSo);
    tlDbgPrintLnf("versionMclf      = 0x%08X", versionInfo.versionMclf);
    tlDbgPrintLnf("versionContainer = 0x%08X", versionInfo.versionContainer);
    tlDbgPrintLnf("versionMcConfig  = 0x%08X", versionInfo.versionMcConfig);
    tlDbgPrintLnf("versionTlApi     = 0x%08X", versionInfo.versionTlApi);
    tlDbgPrintLnf("versionDrApi     = 0x%08X", versionInfo.versionDrApi);
    tlDbgPrintLnf("versionCmp       = 0x%08X", versionInfo.versionCmp);

    // Check if the size of the given TCI is sufficient
    if ((NULL == tci_buffer) || (sizeof(tci_message_t) > tci_buffer_len)) {
        focal_debug("TCI error");
        focal_debug("TCI buffer: %p", tci_buffer);
        focal_debug("TCI buffer length: %d", tci_buffer_len);
        focal_debug("sizeof(tci_message_t): %d", sizeof(tci_message_t));
        // TCI too small -> end Trustlet
        tlApiExit(TCI_EXIT_ERROR);
    }

    tci_message_t* message = (tci_message_t*) tci_buffer;

    // The Trustlet main loop running infinitely
    for (;;) {

        focal_debug("Wait for notification.");
        // Wait for a notification to arrive
        // (INFINITE timeout is recommended -> not polling!)
        tlApiWaitNotification(TLAPI_INFINITE_TIMEOUT);

        // Dereference commandId once for further usage
        cid = message->cmd.id;

        focal_debug("Got a message, command id = 0x%08X",cid);
        focal_debug("Got a message, data = 0x%08X, address 0x%08X"
                "size %d data_size %d",message->data,
                message->data_struct.sVirtualAddr,
                message->data_struct.sVirtualLen,
                message->data_size);

        // Check if the message received is (still) a response
	if(!TCI_IS_CMD(cid)) {
            // Tell the NWd a response is still pending (optional).
            //tlApiNotify();
	        focal_debug("Receive notification but found a response in the TCI !");
	        
            continue;
        }
        // Tell the NWd a response is still pending (optional)

        ret = _focal_ta_command(message, message->cmd.id);
        focal_debug("Returning from 0x%02X.", cid);
        message->cmd.id = 0;

        // Set up response header -> mask response ID and set return code
        focal_debug("returning 0x%08X for command 0x%02X",ret, cid);
        
        message->rsp.code = ret;
        message->rsp.id = TCI_RSP_ID(cid);

        // Notify back the TLC
        tlApiNotify();
    }
}
