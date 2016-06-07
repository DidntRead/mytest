#include <stdint.h>
#include <stdlib.h>

#include "focal_tac.h"
#include "focal_def.h"
#include "focal_lib.h"
#include "focal_hal.h"

#include "focal_ialgorithm.h"

#define LOG_TAG "focal_test"
#include "log.h"

//#include "spi_slsi.h"

#define IMAGE_FILE_SIZE (208 * 80) 
static void returnExitCode(int exitCode);
static uint32_t _focal_lib_suggested_direction_of_next_touch;
static uint32_t _focal_lib_enrol_main_cluster_identified = 0;
static uint32_t _focal_lib_current_progress;
static FocalRect _focal_lib_last_touch;
static FocalRect _focal_lib_next_touch;
static FocalMaskList _focal_lib_mask_list;
static void _load_extended_enroll(focal_lib_extended_enroll_t* ext);


/******************************************************************************/
static void _load_extended_enroll(focal_lib_extended_enroll_t* ext) {
    uint32_t* raw_data; 

    if (ext->data == NULL || ext->size == 0) {
        return;
    }

    raw_data = (uint32_t*) ext->data;

    _focal_lib_suggested_direction_of_next_touch =
        raw_data[FOCAL_DEF_EXT_DATA_DIRECTION_OF_NEXT_TOUCH];

    _focal_lib_enrol_main_cluster_identified =
        raw_data[FOCAL_DEF_EXT_DATA_MAIN_CLUSTER_IDENTIFIED];
    LOG_I("memcpy next touch");
    memcpy(&_focal_lib_next_touch,
            &raw_data[FOCAL_DEF_EXT_DATA_NEXT_TOUCH], sizeof(FocalRect));

    LOG_I("memcpy last touch");
    memcpy(&_focal_lib_last_touch,
            &raw_data[FOCAL_DEF_EXT_DATA_LAST_TOUCH], sizeof(FocalRect));

    if (_focal_lib_mask_list.masks != NULL) {
        free(_focal_lib_mask_list.masks);
        _focal_lib_mask_list.masks = NULL;
    }
    _focal_lib_mask_list.nr_of_masks = raw_data[FOCAL_DEF_EXT_DATA_NR_OF_MASKS];
    
    LOG_E("%s next_touch %d  main cluster %d number of masks %d\n",
                __func__, _focal_lib_suggested_direction_of_next_touch,
                _focal_lib_enrol_main_cluster_identified,
                _focal_lib_mask_list.nr_of_masks);
       
 
    LOG_I("%s mask list %d", __func__, _focal_lib_mask_list.nr_of_masks);
    _focal_lib_mask_list.masks = 
        (FocalRect*) malloc(_focal_lib_mask_list.nr_of_masks * sizeof(FocalRect));

    if (_focal_lib_mask_list.masks == NULL) {
        LOG_E("%s Failed to allocate mask list", __func__);
        return;
    }

    memcpy(_focal_lib_mask_list.masks, &raw_data[FOCAL_DEF_EXT_DATA_MASK_LIST],
            _focal_lib_mask_list.nr_of_masks * sizeof(FocalRect));
 
}

/******************************************************************************/
void print_event(uint32_t event) {
    switch (event) {
        case FOCAL_DEF_EVENT_FINGER_DOWN:
            LOG_I("Finger down");
            break;
        case FOCAL_DEF_EVENT_WAITING_FINGER:
            LOG_I("Waiting for finger");
            break;
        case FOCAL_DEF_EVENT_FINGER_UP:
            LOG_I("Finger up");
            break;
        case FOCAL_DEF_EVENT_IDENTIFY_SUCCESS:
            LOG_I("Identify success");
            break;
        case FOCAL_DEF_EVENT_IDENTIFY_FAILED:
            LOG_I("Identify failed");
            break;
        case FOCAL_DEF_EVENT_ENROLLED_SUCCESS:
            LOG_I("Enrol success");
            break;
        case FOCAL_DEF_EVENT_ENROLLED_FAILED:
            LOG_I("Enrol failed");
            break;
        case FOCAL_DEF_EVENT_PROGRESS:
            LOG_I("progress");
            break;
        default :
            LOG_I("Unknown event");
    }

}

/******************************************************************************/
int _load_image(const char* filename) {
    FILE* pStream;
    uint32_t filesize;
    uint32_t read_result = 0;
    uint8_t file_buffer[IMAGE_FILE_SIZE];
    int32_t i = 0;

    LOG_I("Loading image called.");

    pStream = fopen(filename, "r");
    if (NULL == pStream) {
        LOG_E("Error: Cannot open file: %s.\n", filename);
        return -1;
    }

    if (fseek(pStream, 0L, SEEK_END) != 0) {
        LOG_E("Error: Cannot read file: %s.\n", filename);
        fclose(pStream); 
        return -1;
    }

    filesize = ftell(pStream);
    if (filesize < 0) {
        LOG_E("Error: Cannot get the file size: %s.\n", filename);
        fclose(pStream); 
        return -1;
    }

    if (filesize == 0) {
        LOG_E("Error: Empty file: %s.\n", filename);
        fclose(pStream); 
        return -1;
    }
    
    if (fseek(pStream, 0L, SEEK_SET) != 0) {
        LOG_E("Error: Cannot return to beginning: %s.\n", filename);
        fclose(pStream); 
        return -1;
    }
    
    read_result = fread(file_buffer, 1, filesize, pStream);
    if (read_result != filesize) {
        LOG_E("Entire file not read");
        fclose(pStream); 
        return -1;
    }
    fclose(pStream); 

    return focal_tac_debug_inject_image(file_buffer, filesize);
}

/******************************************************************************/
focal_def_return_t enroll(uint32_t tid) {
    focal_def_return_t ret;
    int file_ret;
    uint32_t i, id_count;
    int32_t id;
    int32_t progress = 0;
    char filename[100];
    focal_lib_enroll_data_t enroll_data;
    focal_lib_extended_enroll_t ext_data;
    int current_file;
   

    LOG_I("Start enroll");
    ret = focal_tac_begin_enroll();
    if (FOCAL_DEF_OK != ret) {
		LOG_E("begin enroll failed!");
        fprintf(stderr, "Command enroll failed.\n");
		returnExitCode(2);
	}
    
    current_file = 0;

    memset(&enroll_data, 0, sizeof(enroll_data));
    LOG_I("Staring enroll ");
	//enroll_data.tid = 1002;
    while (progress >= 0 && current_file < 31) {
        
        sprintf(filename, "/data/images/20140512200700_00_0%02d_1.raw", current_file);
        LOG_I("Image file used %s", filename);
        file_ret = _load_image(filename);
        if (file_ret != 0) {
            LOG_I("Soo much reading fail! %d ", ret);
            break;
        }
        current_file ++;
        ret = focal_tac_enroll(&enroll_data);

        LOG_I("enrol data %d %d %d %d", enroll_data.result,
                enroll_data.quality, enroll_data.progress,
                enroll_data.extended_enroll_size);

        if (FOCAL_DEF_OK != ret) {
            LOG_E("Call to cmd enroll failed!");
            fprintf(stderr, "Command enroll failed.\n");
            return FOCAL_DEF_ERROR_TAC;
        }

        
        if (enroll_data.extended_enroll_size != 0) {
            ext_data.data = (uint8_t*) malloc(enroll_data.extended_enroll_size);
            if (ext_data.data == NULL) {
                LOG_E("Allocation for extended data failed");
                returnExitCode(2);
            }
            ext_data.size = enroll_data.extended_enroll_size;
            ret = focal_tac_get_extended_enroll(&ext_data);
            _load_extended_enroll(&ext_data);
        }

        if (FOCAL_DEF_OK != ret) {
            LOG_E("Call to cmd get extened enroll failed!");
            fprintf(stderr, "Command get extened enroll failed.\n");
            returnExitCode(2);
        }

        if (enroll_data.progress < 0) {
           LOG_E("ERROR"); 
        } else if (enroll_data.progress == 100) {
            LOG_I("Enrol complete");
            break;
        } else {
            progress = enroll_data.progress;
            LOG_I("Progress! %d", progress);
        } 

    }

    ret = focal_tac_end_enroll(&id);
    if (ret != FOCAL_DEF_OK) {
        LOG_E("End enroll fail");
        printf("End enroll fail.\n");
        return FOCAL_DEF_ERROR_TAC;
    }

    return FOCAL_DEF_OK;

}

/******************************************************************************/
focal_def_return_t identify() {
	focal_def_return_t ret;
    uint32_t file_ret;
    uint32_t *ids = NULL;
    uint32_t i, id_count;
    char filename[100];
    focal_lib_identify_data_t identify_data;

    focal_tac_get_ids_count(&id_count);
    LOG_I("focal_tac_get_ids_count: count: %d",id_count);
    ids = (uint32_t*) malloc(id_count * sizeof(uint32_t));
    if (NULL == ids) {
        printf("Could not allocated memory for ids\n");
        return FOCAL_DEF_ERROR_MEM;
    }
    ret = focal_tac_get_ids(ids, &id_count);
    if (ret != FOCAL_DEF_OK) {
        printf("%s, focal_tac_get_ids failed %d\n", __func__, ret);
        return FOCAL_DEF_ERROR_TAC;
    }

    LOG_I("template count is = %d", id_count);
    for (i = 0; i < id_count; i++) {
        LOG_I("template id %i is %d", i, ids[i] );
    }
    focal_tac_begin_identify(ids, id_count);
    
    for (i = 0; i < 5; i++) {
        sprintf(filename, "/data/images/20140512200700_00_0%02d_1.raw", i);
        LOG_I("Image file used %s", filename);
        file_ret = _load_image(filename);
        if (file_ret != 0) {
            printf("Load image failed\n");
            return FOCAL_DEF_ERROR_GENERAL;
        }
        memset(&identify_data, 0, sizeof(focal_lib_identify_data_t));

        ret = focal_tac_identify(&identify_data); 
        if (ret != FOCAL_DEF_OK) {
            printf("Identify failed. %d\n", ret);
            return FOCAL_DEF_ERROR_GENERAL;
        }
        if ((identify_data.result == FOCAL_LIB_IDENTIFY_MATCH ||
            identify_data.result ==  FOCAL_LIB_IDENTIFY_MATCH_UPDATED_TEMPLATE) && 
            identify_data.index == 0) {
            LOG_I("Identify %d as expected", i);
        } else {
            LOG_I("Unexpected identify result %d %d",
                    identify_data.result, identify_data.index);
            printf("Unexpected identify result %d %d\n",
                    identify_data.result, identify_data.index);
         }
    }

    focal_tac_end_identify();

    return FOCAL_DEF_OK;
}

/******************************************************************************/
int remove_all_templates() {
    uint32_t* ids;
    uint32_t i, id_count, ret;

    printf("-->%s \n", __func__);
    ret = focal_tac_get_ids_count(&id_count);
    if (0 == id_count) {
        printf("%s no templates no db\n", __func__);
        return 0;
    }
    printf("%s, focal_tac_get_ids_count: count: %d\n", __func__,id_count);

    ids = (uint32_t*) malloc(id_count * sizeof(uint32_t));
    if (NULL == ids) {
        LOG_E("Could not allocated memory for ids.");
        printf("%s Could not allocated memory for ids.\n", __func__);
        return -1;
    }

    ret = focal_tac_get_ids(ids, &id_count);
    if (ret != FOCAL_DEF_OK) {
        LOG_E("Remove templates, focal_tac_get_ids failed");
        printf("%s, Remove templates focal_tac_get_ids failed.\n", __func__);
        free(ids);
        return -1;
    }

    for (i = 0; i < id_count; i++) {
        ret = focal_tac_remove_template(ids[i]);
        if (ret != FOCAL_DEF_OK) {
            LOG_E("Remove template failed");
            printf("%s, Remove template failed.\n", __func__);
            free(ids);
            return -1;
        }
    }

    free(ids);
    return 0;
}

/******************************************************************************/
int main(int argc, char *args[]) {
	focal_def_return_t ret;
    int int_ret;
    int spi_ret;

    __android_log_write(ANDROID_LOG_INFO, LOG_TAG, "fingerprints test started ");
    printf("Start %s %s!\n", __DATE__, __TIME__);

    ret = focal_hal_init(); 
    if (FOCAL_DEF_OK != ret) {
        returnExitCode(ret);
    }

    ret = enroll(5);
    if (ret != FOCAL_DEF_OK) {
        printf("Enroll failed \n");
    }
 
    ret = identify();
    if (ret != FOCAL_DEF_OK) {
        printf("Identify failed \n");
    }        

    focal_hal_deinit();

    returnExitCode(ret);
    return 0;
}

/******************************************************************************/
static void returnExitCode(int exitCode) {
    if (0 != exitCode) {
        __android_log_write(ANDROID_LOG_ERROR, LOG_TAG, "Failure");
    }
    else {
        __android_log_write(ANDROID_LOG_INFO, LOG_TAG, "Success");
    }
    fprintf(stderr, "TLC exit code: %08x\n", exitCode);
    exit(exitCode);
}
