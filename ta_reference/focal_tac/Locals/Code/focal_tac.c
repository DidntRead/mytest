#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "focal_tac.h"

#include "focal_def.h"
#include "focal_hal.h"
#include "MobiCoreDriverApi.h"
#include "tci.h"
#define LOG_TAG "focal_tac"
#include "Logwrapper/log.h"

#define _FOCAL_TEMPLATE_PATH "/data/templates"
#define _FOCAL_TEMPLATE_NAME "/template.db"
#define _FOCAL_TEMPLATE_FILE _FOCAL_TEMPLATE_PATH _FOCAL_TEMPLATE_NAME
#define _FOCAL_TEMPLATE_NEW_FILE _FOCAL_TEMPLATE_FILE"new"
#define _FOCAL_TEMPLATE_CREATE_MODE 0700

#define FOCAL_DEF_TEMPLATE_DB_MAX_SIZE    (1024 * 500)
#define _TAC_FOCAL_MAX_TEMPLATE_FILES 5
#define DR_FOCAL_UUID \
        { { 0x7, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,\
              0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 } }

#define TL_FOCAL_UUID \
        { { 0x4, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,\
              0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 } }

#define _TLC_FOCAL_TEMPLATE_DB "/data/template.db"

static const uint32_t DEVICE_ID = MC_DEVICE_ID_DEFAULT;
const mcUuid_t tlUuid = TL_FOCAL_UUID;
const mcUuid_t drUuid = DR_FOCAL_UUID;
bool _focal_tac_open = false;

tci_message_t *drTci;
tci_message_t *tlTci;

mcSessionHandle_t tlSessionHandle;
mcSessionHandle_t drSessionHandle;

#define LOG_LINE   printf("____%s____%d ____\n", __func__, __LINE__);
bool focal_tac_check_connection();
uint32_t _focal_tac_tac_error_count = 0;

static focal_def_return_t _focal_tac_load_templates(void);
static focal_def_return_t _focal_tac_store_templates(void);
static focal_def_return_t _focal_tac_share_mem(void);
static focal_def_return_t _focal_tac_send_cmd(
        tci_command_id_t cid, tci_focal_cmd_t fid);

static focal_def_return_t _focal_tac_transfer_data(tci_focal_cmd_t id,
            void* data_struct, uint32_t* data_size);

/******************************************************************************/
focal_def_return_t focal_tac_init() {
    focal_def_return_t ret;

    if (!focal_tac_check_connection()) {
        LOG_E("%s No connection to TA, end command", __func__);
    }
	LOG_I("TCI_FOCAL_CMD_SPI_REGISTER_TEST!");
    ret = _focal_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FOCAL_CMD_SPI_REGISTER_TEST);
    if (ret != FOCAL_DEF_OK) {
        LOG_E("%s Failed to send init command, %d", __func__, ret);
    }
    else
    {
        LOG_I("%s sucess to send init command, %d", __func__, ret);
    }
#if 0
    LOG_I("TCI_FOCAL_CMD_SPI_REGISTER_TEST!");
    ret = _focal_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FOCAL_CMD_SPI_REGISTER_TEST1);
    if (ret != FOCAL_DEF_OK) {
        LOG_E("%s Failed to send init command, %d", __func__, ret);
    }
    else
    {
        LOG_I("%s sucess to send init command, %d", __func__, ret);
    }
#endif	
    LOG_I("focal_tac_init return ok!");
    ret = 0;

#if 0	//focal
		ret = _focal_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FOCAL_CMD_INIT);
		if (ret != FOCAL_DEF_OK) {
			LOG_E("%s Failed to send init command, %d", __func__, ret);
		}
#endif

#if 0	//focal
    ret = _focal_tac_load_templates();
    
    if (ret != FOCAL_DEF_OK) {
       LOG_E("Loading template data base failed.");
    }
#endif
    return ret;
}

/******************************************************************************/
focal_def_return_t focal_tac_deinit() {
    focal_def_return_t ret;

    if (!focal_tac_check_connection()) {
        LOG_E("%s No connection to TA, end command", __func__);
    }
#if 0	//focal
    ret = _focal_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FOCAL_CMD_DEINIT);
    if (ret != FOCAL_DEF_OK) {
        LOG_E("%s Failed to send deinit command, %d", __func__, ret);
    }
#endif
    return ret;
}

/******************************************************************************/
void focal_tac_close(void) {
    mcResult_t ret;
    int spi_ret;

    LOG_I("-->%s Closing the session", __func__);
    ret = mcCloseSession(&tlSessionHandle);
    if (MC_DRV_OK != ret) {
        LOG_E("Closing session to the trustlet failed: %d", ret);
        /* continue even in case of error */
    }

    LOG_I("Closing <t-base device");
    ret = mcCloseDevice(DEVICE_ID);
    if (MC_DRV_OK != ret) {
        LOG_E("Closing <t-base device failed: %d", ret);
        /* continue even in case of error */;
    }

    free(tlTci);
    tlTci = NULL;
    LOG_I("<--%s", __func__);
}

/******************************************************************************/
static size_t _focal_tac_get_TA_file(const char* pPath, uint8_t** ppContent) {
    FILE*   pStream;
    long    filesize;
    uint8_t* content = NULL;

    /* Open the file */
    pStream = fopen(pPath, "rb");
    if (pStream == NULL) {
        fprintf(stderr, "Error: Cannot open file: %s.\n", pPath);
        return 0;
    }

    if (fseek(pStream, 0L, SEEK_END) != 0) {
        fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
        goto error;
    }

    filesize = ftell(pStream);
    if (filesize < 0) {
        fprintf(stderr, "Error: Cannot get the file size: %s.\n", pPath);
        goto error;
    }

    if (filesize == 0) {
        fprintf(stderr, "Error: Empty file: %s.\n", pPath);
        goto error;
    }

    /* Set the file pointer at the beginning of the file */
    if (fseek(pStream, 0L, SEEK_SET) != 0) {
        fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
        goto error;
    }

    /* Allocate a buffer for the content */
    content = (uint8_t*)malloc(filesize);
    if (content == NULL) {
          fprintf(stderr, "Error: Cannot read file: Out of memory.\n");
          goto error;
    }

    /* Read data from the file into the buffer */
    if (fread(content, (size_t)filesize, 1, pStream) != 1) {
        fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
        goto error;
    }

    /* Close the file */
    fclose(pStream);
    *ppContent = content;

    /* Return number of bytes read */
    return (size_t)filesize;

error:
    if (content != NULL) {
        free(content);
    }
    fclose(pStream);
    return 0;

}

/******************************************************************************/
focal_def_return_t focal_tac_open(void) {
    mcResult_t mcRet;
    int spi_ret;
    uint32_t ret;
   // focal_def_return_t def_ret;
    mcVersionInfo_t versionInfo;
    uint8_t* pTrustletData = NULL;
    uint32_t nTrustletSize;

    //LOG_I("[focal tac]tac_open!");
    LOG_I("[focal tac]Opening <t-base device " __DATE__ "," __TIME__);
    mcRet = mcOpenDevice(DEVICE_ID);
    if (MC_DRV_OK != mcRet) {
        LOG_E("[focal tac]Error opening device: %d", mcRet);
        return FOCAL_DEF_ERROR_TAC;
    }

    mcRet = mcGetMobiCoreVersion(MC_DEVICE_ID_DEFAULT, &versionInfo);
    if (MC_DRV_OK != mcRet) {
        LOG_E("[focal tac]mcGetMobiCoreVersion failed %d", mcRet);
        mcCloseDevice(DEVICE_ID);
        return FOCAL_DEF_ERROR_TAC;
    }

    LOG_I("productId        = %s",     versionInfo.productId);
    LOG_I("versionMci       = 0x%08X", versionInfo.versionMci);
    LOG_I("versionSo        = 0x%08X", versionInfo.versionSo);
    LOG_I("versionMclf      = 0x%08X", versionInfo.versionMclf);
    LOG_I("versionContainer = 0x%08X", versionInfo.versionContainer);
    LOG_I("versionMcConfig  = 0x%08X", versionInfo.versionMcConfig);
    LOG_I("versionTlApi     = 0x%08X", versionInfo.versionTlApi);
    LOG_I("versionDrApi     = 0x%08X", versionInfo.versionDrApi);
    LOG_I("versionCmp       = 0x%08X", versionInfo.versionCmp);
    LOG_I("FOCAL TAC build  %s %s", __DATE__, __TIME__);

    tlTci = (tci_message_t*)malloc(sizeof(tci_message_t));
    if (tlTci == NULL) {
        LOG_E("Allocation of trustlet TCI failed");
        mcCloseDevice(DEVICE_ID);
        return FOCAL_DEF_ERROR_MEM;
    }
    memset(tlTci, 0, sizeof(tci_message_t));

    nTrustletSize = _focal_tac_get_TA_file(
                        "/system/app/mcRegistry/04010000000000000000000000000000.tlbin",
                        &pTrustletData);
    if (nTrustletSize == 0) {
        LOG_E("Trustlet not found /system/app/mcRegistry/04010000000000000000000000000000.tlbin");
        free(tlTci);
        tlTci = NULL;
        mcCloseDevice(DEVICE_ID);
        return FOCAL_DEF_ERROR_GENERAL;
    }

    LOG_I("Opening the session %d", nTrustletSize);
    memset(&tlSessionHandle, 0, sizeof(tlSessionHandle));
    // The device ID (default device is used)
    tlSessionHandle.deviceId = DEVICE_ID;
    mcRet = mcOpenTrustlet(
            &tlSessionHandle,
            0,/// MC_SPID_RESERVED_TEST, /* mcSpid_t */
            pTrustletData,
            nTrustletSize,
            (uint8_t *) tlTci,
            sizeof(tci_message_t));

    // Whatever the result is, free the buffer
    free(pTrustletData);
    pTrustletData = NULL;

    if (MC_DRV_OK != mcRet) {
        LOG_E("Open session to the trustlet failed: %d", mcRet);
        free(tlTci);
        tlTci = NULL;
        mcCloseDevice(DEVICE_ID);
        return FOCAL_DEF_ERROR_TAC;

    } else {
        LOG_I("open() succeeded ");
    }
    

    LOG_I("<--%s returns %d", __func__, mcRet);
    if (MC_DRV_OK != mcRet) {
        return FOCAL_DEF_ERROR_TAC;
    } else {
        return FOCAL_DEF_OK;
    }
}

/******************************************************************************/
bool focal_tac_check_connection() {
    focal_def_return_t ret;
    if (tlTci == NULL) {
        LOG_I("%s TAC not started restart.", __func__);
        if (_focal_tac_tac_error_count > 10) {
            return false;
        }
        _focal_tac_tac_error_count++;
        ret = focal_tac_open();
        if (ret != FOCAL_DEF_OK) {
            LOG_E("%s TAC failed to start.", __func__);
            sleep(5);
            focal_tac_close();
            return false;
        }
    }
    _focal_tac_tac_error_count = 0;
    return true;
}

/******************************************************************************/
focal_def_return_t focal_tac_begin_enroll() {
    focal_def_return_t ret;

    LOG_I("-->%s", __func__);
    if (!focal_tac_check_connection()) {
        LOG_E("%s No connection to TA, end command", __func__);
        return FOCAL_DEF_ERROR_TAC;
    }

    ret = _focal_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FOCAL_CMD_BEGIN_ENROLL);
    if (ret != FOCAL_DEF_OK) {
        LOG_E("%s, send command failed with 0x%02X, closing TAC"
                , __func__, ret);
        //focal_tac_close();
        return FOCAL_DEF_ERROR_GENERAL;
    }

    LOG_I("<--%s returns %d", __func__, tlTci->rsp.cmd_ret);
    return tlTci->rsp.cmd_ret;
}

/******************************************************************************/
focal_def_return_t focal_tac_wakeup_setup() {
    focal_def_return_t ret;

    LOG_I("-->%s", __func__);
    if (!focal_tac_check_connection()) {
        LOG_E("%s No connection to TA, end command", __func__);
        return FOCAL_DEF_ERROR_TAC;
    }
    
    ret = _focal_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FOCAL_CMD_WAKEUP_SETUP);
    if (ret != FOCAL_DEF_OK) {
        LOG_E("%s, send command failed with 0x%02X", __func__, ret);
    } 

    LOG_I("<--%s returns %d", __func__, tlTci->rsp.cmd_ret);
    return tlTci->rsp.cmd_ret;
}

/******************************************************************************/
focal_def_return_t focal_tac_wakeup_qualification() {
    focal_def_return_t ret;

    LOG_I("-->%s", __func__);
    if (!focal_tac_check_connection()) {
        LOG_E("%s No connection to TA, end command", __func__);
        return FOCAL_DEF_ERROR_TAC;
    }
    
    ret = _focal_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FOCAL_CMD_WAKEUP_QUALIFICATION);
    if (ret == FOCAL_DEF_ERROR_TAC) {
        LOG_E("%s, send command failed with 0x%02X", __func__, ret);
    } 

    LOG_I("<--%s returns %d", __func__, tlTci->rsp.cmd_ret);
    return tlTci->rsp.cmd_ret;
}

/******************************************************************************/
focal_def_return_t focal_tac_enroll(focal_lib_enroll_data_t* data) {
    focal_def_return_t ret;
    uint32_t data_size = sizeof(focal_lib_enroll_data_t);
    LOG_I("-->%s", __func__);

    if (tlTci == NULL) {
        /* Enroll cannot be run before begin enroll so either the connection
         * was dropped after begin enroll or begin enroll was never called.*/
        LOG_E("<--%s No connection to TA", __func__);
        return FOCAL_DEF_ERROR_GENERAL;
    }

    ret = _focal_tac_transfer_data(TCI_FOCAL_CMD_ENROLL, data, &data_size); 
    if (ret != FOCAL_DEF_OK) {
        LOG_E("<--%s send command failed with 0x%02X, closing TAC.",
                __func__, ret);
        //focal_tac_close();
        return FOCAL_DEF_ERROR_GENERAL;
    }

    LOG_I("<--%s returns %d", __func__, ret);
    return ret;
}

/******************************************************************************/
focal_def_return_t focal_tac_end_enroll(uint32_t* id) {
    focal_def_return_t ret;
    LOG_I("-->%s", __func__);

    LOG_I("End enroll called");
    if (tlTci == NULL) {
        /* End enroll cannot be run before begin enroll so either the connection
         * was dropped after begin enroll or begin enroll was never called.*/
        LOG_E("%s No connection to TA", __func__);
        return FOCAL_DEF_ERROR_GENERAL;
    }

    ret = _focal_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FOCAL_CMD_END_ENROLL);
    if (ret != FOCAL_DEF_OK) {
        LOG_E("%s: failed", __func__);
        return ret;
    }
    *id = tlTci->data;

    ret = _focal_tac_store_templates();
    if (ret != FOCAL_DEF_OK) {
        LOG_E("%s: store templates failed", __func__);
        return ret;
    }

    LOG_I("<--%s returns %d", __func__, ret);
    return ret;
}

/******************************************************************************/
focal_def_return_t focal_tac_get_extended_enroll(focal_lib_extended_enroll_t* data) {
    focal_def_return_t ret;
    LOG_I("-->%s", __func__);

    if (tlTci == NULL) {
        LOG_I("<--%s No communication setup", __func__);
        return FOCAL_DEF_ERROR_GENERAL;
    }
    uint32_t data_size = data->size;
    LOG_I("Extend Enroll data called");

    ret = _focal_tac_transfer_data(TCI_FOCAL_CMD_EXTENDED_ENROLL_DATA, data->data, 
            &data_size);

    LOG_I("<--%s returns %d", __func__, ret);
    return ret;
}

/******************************************************************************/
focal_def_return_t focal_tac_init_finger_detect(uint32_t* data) {
    focal_def_return_t ret;
    LOG_I("-->%s", __func__);

    if (tlTci == NULL) {
        LOG_I("<--%s No communication setup", __func__);
        return FOCAL_DEF_ERROR_GENERAL;
    }

    ret = _focal_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FOCAL_CMD_INIT_FINGER_DETECT);
    *data = tlTci->data;

    LOG_I("<--%s returns %d, tlTci->rsp.cmd_ret=%d", __func__, ret, tlTci->rsp.cmd_ret);
    return tlTci->rsp.cmd_ret;
}

/******************************************************************************/
focal_def_return_t focal_tac_start_finger_detect(uint32_t* data) {
    focal_def_return_t ret;
    LOG_I("-->%s", __func__);

    if (tlTci == NULL) {
        LOG_I("<--%s No communication setup", __func__);
        return FOCAL_DEF_ERROR_GENERAL;
    }
    LOG_I("Start finger detect called");

    ret = _focal_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FOCAL_CMD_START_FINGER_DETECT);
    *data = tlTci->data;

    LOG_I("<--%s returns %d, tlTci->rsp.cmd_ret=%d", __func__, ret, tlTci->rsp.cmd_ret);
    return tlTci->rsp.cmd_ret;
}

/******************************************************************************/
focal_def_return_t focal_tac_check_finger_present(uint32_t* data) {
    focal_def_return_t ret;
    LOG_I("-->%s", __func__);

    if (!focal_tac_check_connection()) {
        LOG_I("<--%s No communication setup", __func__);
        return FOCAL_DEF_ERROR_GENERAL;
    }
    ret = _focal_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FOCAL_CMD_CHECK_FINGER_PRESENT);
    if (ret == FOCAL_DEF_ERROR_GENERAL) {
        LOG_E("<--%s communication error %d", __func__, ret);
        return ret; 
    }

    *data = tlTci->rsp.cmd_ret;

    LOG_I("<--%s returns %d", __func__, ret);
    return ret;
}

/******************************************************************************/
focal_def_return_t focal_tac_check_finger_lost(uint32_t* data) {
    focal_def_return_t ret;
    LOG_I("-->%s", __func__);

    if (!focal_tac_check_connection()) {
        LOG_E("<--%s No connection to TA %d", __func__, ret);
        return FOCAL_DEF_ERROR_GENERAL;
    }

    ret = _focal_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FOCAL_CMD_CHECK_FINGER_LOST);
    if (ret == FOCAL_DEF_ERROR_GENERAL) {
        LOG_E("<--%s communication error %d", __func__, ret);
        return ret;
    }
        
    *data = tlTci->data;

    LOG_I("<--%s returns %d data %d", __func__, ret, *data);
    return ret;
}

/******************************************************************************/
focal_def_return_t focal_tac_capture_image() {
    focal_def_return_t ret;
    LOG_I("-->%s", __func__);

    if (tlTci == NULL) {
        LOG_I("<--%s No communication setup", __func__);
        return FOCAL_DEF_ERROR_GENERAL;
    }
    ret = _focal_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FOCAL_CMD_CAPTURE_IMAGE);

    LOG_I("<--%s returns %d", __func__, ret);
    return ret;
}

/******************************************************************************/
focal_def_return_t focal_tac_remove_template(uint32_t id) {
    focal_def_return_t ret;
    char template_name[100];
    int error;
    uint32_t index;
    LOG_I("-->%s", __func__);

    LOG_I("Remove template %d", id);

    if (tlTci == NULL) {
        return FOCAL_DEF_ERROR_GENERAL;
    }
    tlTci->data = id;
    ret = _focal_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FOCAL_CMD_DELETE_TEMPLATE);
    if (ret != FOCAL_DEF_OK) {
        LOG_E("Error when removing template, id = %d", id);
    }

    /* Save the new database */
    _focal_tac_store_templates();

    LOG_I("<--%s returns %d", __func__, ret);
    return ret;
}

focal_def_return_t focal_tac_remove_all_templates() {
	focal_def_return_t ret;
    int error;
    uint32_t index;
    LOG_I("-->%s", __func__);

    LOG_I("Remove all template");

    if (tlTci == NULL) {
        return FOCAL_DEF_ERROR_GENERAL;
    }
    ret = _focal_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FOCAL_CMD_DELETE_ALL_TEMPLATES);
    if (ret != FOCAL_DEF_OK) {
        LOG_E("Error when removing all template. (TCI_FOCAL_CMD_DELETE_ALL_TEMPLATES)");
    }

    /* Save the new database */
    _focal_tac_store_templates();

    LOG_I("<--%s returns %d", __func__, ret);
    return ret;
}

/******************************************************************************/
focal_def_return_t focal_tac_begin_identify(uint32_t* ids, uint32_t length) {
    focal_def_return_t ret;
    uint32_t size;
    LOG_I("-->%s", __func__);

    if (!focal_tac_check_connection()) {
        LOG_E("<--%s No connection to TA %d", __func__, ret);
        return FOCAL_DEF_ERROR_GENERAL;
    }

    size = length * sizeof(uint32_t);
    
    ret = _focal_tac_transfer_data(TCI_FOCAL_CMD_BEGIN_IDENTIFY, ids, &size);

    LOG_I("<--%s returns %d", __func__, ret);
    return ret;
}

/******************************************************************************/
focal_def_return_t focal_tac_identify(focal_lib_identify_data_t* data) {
    focal_def_return_t ret;
    uint32_t size;
    LOG_I("-->%s", __func__);

    size = sizeof(focal_lib_identify_data_t);

    if (tlTci == NULL) {
        return FOCAL_DEF_ERROR_GENERAL;
    }

    ret = _focal_tac_transfer_data(TCI_FOCAL_CMD_IDENTIFY, data, &size);
    if (ret != FOCAL_DEF_OK) {
        LOG_I("<--%s Returns %d %d %d %d", __func__, ret,
                data->result, data->index, data->score);
    } else {
        LOG_I("<--%s Returns %d", __func__, ret);
    }
    return ret;
}

/******************************************************************************/
focal_def_return_t focal_tac_end_identify() {
    focal_def_return_t ret;
    LOG_I("-->%s", __func__);

    if (tlTci == NULL) {
        return FOCAL_DEF_ERROR_GENERAL;
    }

    ret = _focal_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FOCAL_CMD_END_IDENTIFY);
    if (ret != FOCAL_DEF_OK) {
        LOG_I("<--%s returns %d", __func__, ret);
        return ret;
    } 
    
    ret = _focal_tac_store_templates();

    LOG_I("<--%s returns %d", __func__, ret);
    return ret;
}

/******************************************************************************/
focal_def_return_t focal_tac_debug_retrieve_image(uint8_t* image, uint32_t* size) {
    focal_def_return_t ret;
    LOG_I("-->%s", __func__);

    LOG_I("Debug retrieve image");

    if (tlTci == NULL) {
        return FOCAL_DEF_ERROR_GENERAL;
    }
    ret =
        _focal_tac_transfer_data(TCI_FOCAL_CMD_DEBUG_RETRIEVE_IMAGE, image, size);

    LOG_I("<--%s", __func__);
    return ret;
}

/******************************************************************************/
focal_def_return_t focal_tac_debug_inject_image(uint8_t* image, uint32_t size) {
    focal_def_return_t ret;
    uint32_t len = size;

    LOG_I("-->%s size %d", __func__, len);
    if (tlTci == NULL) {
        return FOCAL_DEF_ERROR_GENERAL;
    }

    ret = _focal_tac_transfer_data(TCI_FOCAL_CMD_DEBUG_INJECT_IMAGE, image, &len);

    LOG_I("<--%s", __func__);
    return ret;
}

/******************************************************************************/
focal_def_return_t focal_tac_get_ids(uint32_t* ids, uint32_t* len) {
    focal_def_return_t ret;
    uint32_t original_size = *len;
    LOG_I("-->%s len %d", __func__, *len);
    if (!focal_tac_check_connection()) {
        LOG_E("<--%s connection to TA failed.", __func__);
        return FOCAL_DEF_ERROR_GENERAL;
    }

    LOG_I("Get Ids called.");
    ret = _focal_tac_transfer_data(TCI_FOCAL_CMD_GET_IDS, ids, len); 

    if (*len > original_size) {
        LOG_E("Number of ids in the database larger than expected," 
                " ask for length %d, got length %d", *len, original_size);
        return FOCAL_DEF_ERROR_MEM;
    }
    LOG_I("<--%s", __func__);
    return ret;
}

/******************************************************************************/
focal_def_return_t focal_tac_get_ids_count(uint32_t* count) {
    focal_def_return_t ret;
    LOG_I("-->%s count %d", __func__, *count);
    if (!focal_tac_check_connection()) {
        LOG_E("<--%s connection to TA failed.", __func__);
        return FOCAL_DEF_ERROR_GENERAL;
    }

    ret = _focal_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FOCAL_CMD_GET_IDS_COUNT);
    if (ret != FOCAL_DEF_ERROR_GENERAL) {
        *count = tlTci->data;
    }

    LOG_I("<--%s returnts %d count %d", __func__, ret, *count);
    return ret;
}

/******************************************************************************/
focal_def_return_t focal_tac_deadpixel_test(int32_t* nDeadPixels) {
    focal_def_return_t ret;
    int32_t len = sizeof(int32_t);

    LOG_I("[focal tac spi]-->%s", __func__);
    if (!focal_tac_check_connection()) {
        LOG_E("%s No connection to TA, end command", __func__);
        return FOCAL_DEF_ERROR_TAC;
    }

    ret = _focal_tac_transfer_data(TCI_FOCAL_CMD_DEADPIXEL_TEST, nDeadPixels, &len); 
    if (ret != MC_DRV_OK) {
        LOG_E("%s, send command failed with 0x%02X, closing TAC"
                , __func__, ret);
        return FOCAL_DEF_ERROR_GENERAL;
    }

    LOG_I("<--%s returns %d,nDeadPixels=%d", __func__, tlTci->rsp.cmd_ret,*nDeadPixels);
    return tlTci->rsp.cmd_ret;
}

/******************************************************************************/
static focal_def_return_t _focal_tac_load_templates(void) {
    focal_def_return_t ret = FOCAL_DEF_OK;
    FILE* fp = NULL;
    uint32_t read_result = 0;
    uint8_t* data_buffer = NULL;
    int32_t i = 0, loadidx = 0;
    struct stat stat_info;
    uint32_t template_cnt = 0, template_size = 0;
    uint32_t max_template_size = 0;

    LOG_I("Enter %s", __func__);

    if (NULL == tlTci) {
        LOG_E("TCI has not been set up properly - exiting");
        return FOCAL_DEF_ERROR_TAC;
    }

    //try to load all db file into memory
    fp = fopen(_FOCAL_TEMPLATE_FILE, "r");
    if (NULL == fp) {
        LOG_w("Couldn't open file: %s. No database loaded.\n",
                _FOCAL_TEMPLATE_FILE);

        ret = FOCAL_DEF_OK;
        goto out;
    }

    if (fstat(fileno(fp), &stat_info) != 0) {
        LOG_w("Couldn't get stat for file: %s.\n", _FOCAL_TEMPLATE_FILE);
        ret = FOCAL_DEF_ERROR_GENERAL;
        goto out;
    }

    fread(&template_cnt, sizeof(template_cnt), 1, fp);
    LOG_I("#templates %d", template_cnt);

    fread(&max_template_size, sizeof(max_template_size), 1, fp);
    data_buffer = (uint8_t*) malloc(max_template_size * sizeof(uint8_t));
    if (data_buffer == NULL) {
        LOG_E("Failed to allocate memory (%d)", max_template_size);
        goto out;
    }

    for (i = 0; i < template_cnt; i++) {
        fread(&template_size, sizeof(template_size), 1, fp);
        read_result = fread(data_buffer, 1, template_size, fp);
        if (read_result != template_size) {
            LOG_E("Entire file not read");
            goto out;
        }

        tlTci->data = i;

        ret = _focal_tac_transfer_data(TCI_FOCAL_CMD_LOAD_ONE_TEMPLATE,
                data_buffer, &template_size);

        /* Returning template_size  */
        if (ret != TCI_RET_OK || data_buffer == 0) {
            LOG_w("Loading database failed, ret = %d, size = %d",
                    ret, template_size);
            continue;
        }

    } /* File loop */

out:
    if (data_buffer != NULL) {
        free(data_buffer);
        data_buffer = NULL;
    }

    if (fp != NULL) {
        fclose(fp);
        fp = NULL;
    }

    return ret;

}

/******************************************************************************/
static focal_def_return_t _focal_tac_store_templates(void) {
	focal_def_return_t ret = FOCAL_DEF_OK;
	FILE* fp = NULL;
	uint32_t file_size = FOCAL_DEF_TEMPLATE_DB_MAX_SIZE;
	uint32_t write_result = 0;
    bool file_written = false;
	uint8_t* data_buffer = NULL;
	uint32_t template_cnt = 0;
    uint32_t id_size = 0;
	uint32_t* template_sizes = NULL;
	uint32_t max_template_size = 0;
    uint32_t* ids = NULL;
    uint32_t i;
    struct stat stat_info;
    mcResult_t cmd_ret;

    LOG_I("Enter %s", __func__);

	//count db template
	cmd_ret = _focal_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FOCAL_CMD_GET_IDS_COUNT);
    if (cmd_ret != FOCAL_DEF_OK) {
        LOG_I("Failed to get IDS");
        return FOCAL_DEF_ERROR_GENERAL;
    }
	template_cnt = tlTci->data;

	LOG_I("Template count: %d", template_cnt);

	if (0 == template_cnt) {
		LOG_I("No templates to store");
        // Delete old file and return
        unlink(_FOCAL_TEMPLATE_FILE);
		ret = FOCAL_DEF_OK;
        goto out;
	}

    id_size = template_cnt * sizeof(uint32_t);
    ids = (uint32_t*)malloc(template_cnt * sizeof(uint32_t));
    template_sizes = (uint32_t*)malloc(template_cnt * sizeof(uint32_t));
    if (ids == NULL || template_sizes == NULL) {
        LOG_E("Error: memory allocation failed\n");
        ret = FOCAL_DEF_ERROR_MEMORY;
        goto out;
    }
    focal_tac_get_ids(ids, &id_size);

	for (i = 0; i < template_cnt; i++) {
	    tlTci->data = ids[i];
		cmd_ret = _focal_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FOCAL_CMD_GET_TEMPLATE_SIZE);
        if (cmd_ret != FOCAL_DEF_OK) {
            LOG_I("Failed to get template size, %d", cmd_ret);
            return FOCAL_DEF_ERROR_GENERAL;
        }
		template_sizes[i] = tlTci->data;
		LOG_I("Template database %d (id=%d), size is %d",
                i, ids[i], template_sizes[i]);

		if (template_sizes[i] > max_template_size) {
			max_template_size = template_sizes[i];
		}
    }

    data_buffer = (uint8_t*) malloc(max_template_size * sizeof(uint8_t));
    if (data_buffer == NULL) {
        LOG_E("Failed to allocate memory (%d)", max_template_size);
        ret = FOCAL_DEF_ERROR_MEMORY;
        goto out;
    }

    if ((mkdir(_FOCAL_TEMPLATE_PATH, _FOCAL_TEMPLATE_CREATE_MODE) != 0) 
            && (errno != EEXIST)) {

        LOG_E("Couldn't create directory! (%s)", strerror(errno));
        return FOCAL_DEF_ERROR_GENERAL;
    }

    fp = fopen(_FOCAL_TEMPLATE_NEW_FILE, "w");
    if (NULL == fp) {
        LOG_E("Cannot open file: %s.\n", _FOCAL_TEMPLATE_NEW_FILE);
        ret = FOCAL_DEF_ERROR_GENERAL;
        goto out;
    }
    fwrite(&template_cnt, sizeof(template_cnt), 1, fp);
    fwrite(&max_template_size, sizeof(max_template_size), 1, fp);

	for (i = 0; i < template_cnt; i++) {
        tlTci->data = ids[i];
		cmd_ret = _focal_tac_transfer_data(
                TCI_FOCAL_CMD_STORE_ONE_TEMPLATE,
                data_buffer,
				&template_sizes[i]);
		if (cmd_ret != FOCAL_DEF_OK) {
			LOG_I("Failed to get template data, %d", cmd_ret);
		} else {
			if (template_sizes[i] == 0) {
				continue;
			}

			fwrite(&template_sizes[i], 1, sizeof(template_sizes[i]), fp);
			write_result = fwrite(data_buffer, 1, template_sizes[i], fp);
			if (write_result != template_sizes[i]) {
				LOG_E("Entire file not written %d bytes instead of %d "
                		"bytes err: %d", 
						write_result, template_sizes[i], ferror(fp));
			}
            if (write_result > 0) {
                file_written = true;
            }
		}
	}

    if (!file_written) {
        /* If nothing have been saved. Delete file including header */
		fclose(fp);
		fp = NULL;
        unlink(_FOCAL_TEMPLATE_NEW_FILE);
    }

out:
	if (ids != NULL){
		free(ids);
		ids = NULL;
	}

	if (template_sizes != NULL){
		free(template_sizes);
		template_sizes = NULL;
	}

	if (data_buffer != NULL) {
		free(data_buffer);
		data_buffer = NULL;
	}

	if (fp != NULL) {
		fclose(fp);
		fp = NULL;
	}

    // Rename old file
    if (stat(_FOCAL_TEMPLATE_NEW_FILE, &stat_info) == 0) {
        // new file exists => rename
        rename(_FOCAL_TEMPLATE_NEW_FILE, _FOCAL_TEMPLATE_FILE);
    }

	return ret;
}

/******************************************************************************/
static focal_def_return_t _focal_tac_send_cmd(
        tci_command_id_t cid, tci_focal_cmd_t fid) {

    mcResult_t ret = TCI_RET_OK;
    int32_t errorcode;
    uint32_t keep_waiting = 1;
    uint32_t safety_count = 20;
    //uint32_t timeout = 10000;
    uint32_t timeout = 500;//samuel test

    LOG_I("-->%s command 0x%02X,0x%02X", __func__, cid,fid);
   
    if (tlTci == NULL) {
        LOG_I("<--%s no communication setup.", __func__);
        return FOCAL_DEF_ERROR_GENERAL;
    }
    focal_hal_stop_standby();
    /* Signal the Trusted application */
    tlTci->cmd.id = cid;
    tlTci->cmd.focal_cmd = fid;

    ret = mcNotify(&tlSessionHandle);
    if (MC_DRV_OK != ret) {
        LOG_E("%s Notify failed: %d", __func__, ret);
        ret = TCI_RET_ERR_NOTIFICATION;
        goto error;
    }

    /*  
     *  Wait for the Trusted application response until response 
     *  or until "safety_count" number of tries. Safety count has
     *  never been hit.
     */
    while (keep_waiting && safety_count != 0) {
        ret = mcWaitNotification(&tlSessionHandle, timeout);
        --safety_count;
        if (MC_DRV_ERR_TIMEOUT == ret) {
            if (tlTci->rsp.id == cid) {
                /* Timeout before TA had a chance to answer, keep checking. */
                LOG_E("Early timeout, keep waiting!");
                keep_waiting = 1;
            } else if (tlTci->rsp.id == TCI_RSP_ID(cid)) {
                /* Timeout but detecting that the TA answer was missed.
                 * if not a response the error will be reported in the next step*/
                LOG_E("(%s) Command %d : False timeout, notification was probably lost (#WORKAROUND)", __func__, fid);
                ret = MC_DRV_OK;
                keep_waiting = 0;
            } else {
                LOG_E("(%s) Command %d : False timeout, inconsistent state found in rsp.id field (0x%X)", __func__, fid,tlTci->rsp.id);
                keep_waiting = 1;
            }
        } 
       else if ((TCI_RSP_ID(cid) != tlTci->rsp.id)) {
         LOG_E("%s mcWaitNotification returned: ret=%d, but the rsp.id =0x%02X and TCI_RSP_ID(cid)=0x%02X(#WORKAROUND)",
                __func__, ret, tlTci->rsp.id, TCI_RSP_ID(cid));
                keep_waiting = 1;
       }
       else {
            /* Other error or OK */
            keep_waiting = 0;
        }
    }
    /*  Wait for the Trusted application response */
  //  ret = mcWaitNotification(&tlSessionHandle, MC_INFINITE_TIMEOUT);
    if (MC_DRV_OK != ret) {

        LOG_E("%s wait for response notification failed: 0x%02X 0x%02X",
                __func__, ret, tlTci->rsp.code);

        ret = mcGetSessionErrorCode(&tlSessionHandle,&errorcode);
        if (ret == MC_DRV_OK) {
            LOG_E("%s mcGetSessionErrorCode: %d", __func__, errorcode);
        } else {
            LOG_E("%s mcGetSessionErrorCode: error", __func__);
        }

        ret = TCI_RET_ERR_NOTIFICATION;
        goto error;
    }

    /* Verify that the Trusted application sent a response */
    if ((TCI_RSP_ID(cid) != tlTci->rsp.id)) {
        LOG_E("%s Trusted application did not send a response:"
                "0x%02X 0x%02X", __func__, tlTci->rsp.id, TCI_RSP_ID(cid));

        ret = TCI_RET_ERR_NOTIFICATION;
        goto error;
    }

    /* Check the Trustlet return code */
    if (TCI_RET_OK != tlTci->rsp.code) {
        LOG_E("%s Trusted Application did not send a valid return code: %d",
                __func__, tlTci->rsp.code);

        ret = TCI_RET_ERR_RETURN_CODE;
        goto error;
    }

error:

    focal_hal_standby();
    if (ret != MC_DRV_OK) {
        LOG_E("<--%s, send command failed with 0x%02X.",
                __func__ , ret);
        return FOCAL_DEF_ERROR_GENERAL;
    } else {
        LOG_I("<--%s returns %d", __func__, tlTci->rsp.cmd_ret);
        return (focal_def_return_t) tlTci->rsp.cmd_ret;
    }
}

/******************************************************************************/
static focal_def_return_t _focal_tac_transfer_data(
        tci_focal_cmd_t fid,
        void* data_struct,
        uint32_t* data_size) {

    mcBulkMap_t map_info;
    mcResult_t ret = MC_DRV_OK;
    focal_def_return_t cmd_ret = FOCAL_DEF_OK;

    LOG_I("-->%s command 0x%02X", __func__, fid);

    if (NULL == tlTci) {
        LOG_E("%s TCI has not been set up properly - exiting", __func__);
        ret = FOCAL_DEF_ERROR_TAC;
        goto error;
    }

    if (data_size == 0) {
        ret = FOCAL_DEF_ERROR_SIZE;
        goto error;
    }

    ret = mcMap(&tlSessionHandle, data_struct, *data_size, &map_info);
    if (MC_DRV_OK != ret) {
        LOG_E("Mapping of data failed failed 0x%08X", ret);
        goto error;
    }

    tlTci->data_struct = map_info;

    cmd_ret = _focal_tac_send_cmd(TCI_CMD_TRANSFER_DATA, fid);
    if (FOCAL_DEF_OK != cmd_ret){
        LOG_E("Sending command failed, cmd %d, ret %d", fid, cmd_ret);
    }

    ret = mcUnmap(&tlSessionHandle, data_struct, &map_info);
    if (MC_DRV_OK != ret) {
        LOG_E("UnMapping image failed 0x%08X", ret);
        goto error;
    }

    *data_size = tlTci->data_size;
    LOG_I("Returning from transfer command 0x%d", fid);
    if (MC_DRV_OK != cmd_ret) {
        ret = cmd_ret;
        goto error;
    }

error:
    if (ret != MC_DRV_OK) {
        LOG_E("%s, send command failed with 0x%02X, closing TAC",
                __func__, ret);
        return FOCAL_DEF_ERROR_GENERAL;
    } else {
        LOG_I("<--%s returns tlTci->rsp.cmd_ret", __func__);
        return (focal_def_return_t) tlTci->rsp.cmd_ret;
    }
}

/******************************************************************************/
focal_def_return_t focal_tac_spidrv_open(void) {

    mcResult_t ret;

    LOG_I("%s", __func__);

    drTci = (tci_message_t *)malloc(sizeof(tci_message_t));
    if (drTci == NULL) {
        LOG_E("Allocation of Driver TCI failed");

        return FOCAL_DEF_ERROR_MEM;
    }
    memset(drTci, 0, sizeof(tci_message_t));

    LOG_I("Opening driver session");
    memset(&drSessionHandle, 0, sizeof(drSessionHandle));

    drSessionHandle.deviceId = DEVICE_ID;
    ret = mcOpenSession(&drSessionHandle,
                        &drUuid,
                        (uint8_t*)drTci,
                        sizeof(tci_message_t));

    if (MC_DRV_OK != ret) {
        LOG_E("Driver session open failed: %d", ret);

        return FOCAL_DEF_ERROR_MEM;
    }
    return FOCAL_DEF_OK;
}

/******************************************************************************/
void focal_tac_spidrv_close(void) {

    mcResult_t ret = MC_DRV_OK;

    LOG_I("%s", __func__);

    ret = mcCloseSession(&drSessionHandle);
    if (MC_DRV_OK != ret) {
        LOG_E("Closing session failed: %d", ret);
        // continue even in case of error
    }
    free(drTci);
    drTci = NULL;
}

focal_def_return_t focal_ft_get_image_buf(uint8_t *image)
{
	focal_def_return_t ret = 0;
	int size = 88 * 88;

	LOG_I("===========================1");
	ret = _focal_tac_transfer_data(TCI_FOCAL_CMD_GET_IMAGE_BUF, image, &size);
	if (ret != FOCAL_DEF_OK)
	{
		LOG_E("transfer TCI_FOCAL_CMD_GET_IMAGE_BUF error!");
	}
	LOG_I("===========================2");

	return FOCAL_DEF_OK;
}

int focal_ft_lib_init()
{
    int ret = 0;
    ret = _focal_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FOCAL_CMD_INIT);
    return ret;
}

int focal_ft_detectfinger()
{
    int ret = 0;
    int state;
    int size = 4;

    //LOG_LINE
    ret = _focal_tac_transfer_data(TCI_FOCAL_CMD_CHECK_FINGER_PRESENT, &state, &size);
    //LOG_LINE
    //usleep(500000);

        
    return state;
}

focal_def_return_t focal_ft_enroll_finger(focal_lib_enroll_data_t *data)
{
	focal_def_return_t ret = 0;
	int size = sizeof(focal_lib_enroll_data_t);

	ret = _focal_tac_transfer_data(TCI_FOCAL_CMD_ENROLL_FINGER, data, &size);
	if (ret != FOCAL_DEF_OK)
	{
		LOG_E("transfer TCI_FOCAL_CMD_GET_IMAGE_BUF error!");
	}

	return FOCAL_DEF_OK;
}

focal_def_return_t focal_ft_end_enroll(focal_lib_enroll_data_t *data)
{
    focal_def_return_t ret;
    int size = sizeof(focal_lib_enroll_data_t);

    LOG_LINE
    ret = _focal_tac_transfer_data(TCI_FOCAL_CMD_END_ENROLL, data, &size);
    LOG_LINE
    if (ret != FOCAL_DEF_OK)
    {
    	LOG_E("transfer TCI_FOCAL_CMD_GET_IMAGE_BUF error!");
    }
    
    return FOCAL_DEF_OK;
}

focal_lib_identify_result_t focal_ft_match_finger(focal_lib_identify_data_t *data)
{
    focal_def_return_t ret = 0;
    int size = sizeof(focal_lib_identify_data_t);

    ret = _focal_tac_transfer_data(TCI_FOCAL_CMD_MATCH_FINGER, data, &size);
    if (ret != FOCAL_DEF_OK)
    {
    	LOG_E("transfer TCI_FOCAL_CMD_GET_IMAGE_BUF error!");
    }
         
    return data->result;
}

#define FILE_PATH			"/data/focalfinger.data"
int focal_save_data_to_file(focal_lib_enroll_data_t *data)
{
    FILE *fp = NULL;
    if (-1 == access(FILE_PATH, F_OK))
    {
        printf("focalfp %s is not exist and create it!", FILE_PATH);
        fp = fopen(FILE_PATH, "wb+");
        if (NULL == fp)
        {
        	printf("focalfp fopen file error!");
        	fclose(fp);
        	return -1;
        }
        fclose(fp);
    }

    fp = fopen(FILE_PATH, "rb+");
    if (NULL == fp)
    {
        printf("focalfp fopen file error!");
        fclose(fp);
        return -1;
    }

    if(0 != fseek(fp, (sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint32_t)
        + FT_ALG_TPL_MAX_SIZE * sizeof(uint8_t)) * data->finger_id, SEEK_SET))
    {
        fclose(fp);
        printf("focalfp fseek file error!");
        return -1;
    }

    fwrite(&data->finger_id, 4, 1, fp);
    fwrite(&data->type, 1, 1, fp);
    fwrite(&data->size, 4, 1, fp);
    fwrite(data->tpl, FT_ALG_TPL_MAX_SIZE, 1, fp);
    fclose(fp);

    printf("save %d %d %d", data->finger_id, data->type, data->size);
    return 0;
}

int get_data_from_file(focal_lib_enroll_data_t *data, int index)
{
    FILE *fp = NULL;
    fp = fopen( FILE_PATH, "rb+");
    if (NULL == fp)
    {
        fclose(fp);
        return -1;
    }
     if(0 != fseek(fp, (sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint32_t)
        + FT_ALG_TPL_MAX_SIZE * sizeof(uint8_t)) * index, SEEK_SET))
    {
        fclose(fp);
        printf("focalfp fseek file error!");
        return -1;
    }
    fread(&data->finger_id, 4, 1, fp);
    fread(&data->type, 1, 1, fp);
    fread(&data->size, 4, 1, fp);
    fread(data->tpl, FT_ALG_TPL_MAX_SIZE, 1, fp);
    fclose(fp);
    printf("get %d %d %d", data->finger_id, data->type, data->size);
}
