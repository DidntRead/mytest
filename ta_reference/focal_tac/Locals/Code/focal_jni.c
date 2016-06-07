#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#include "focal_lib.h"
#include "focal_def.h"

#include "focal_def.h"
#include "focal_ialgorithm.h"

#include "focal_tac.h"
#include "focal_hal.h"

#define LOG_TAG "focal_lib_jni"
#include "Logwrapper/log.h"

#define _FOCAL_LIB_MAX_FINGER_PRESENT_TRIES   ((uint32_t) 20)
#define _FOCAL_LIB_MAX_FINGER_LOST_TRIES      ((uint32_t) 60)

#define FOCAL_GENERAL_ERROR 1000

static uint32_t _focal_lib_suggested_direction_of_next_touch;
static uint32_t _focal_lib_enrol_main_cluster_identified = 0;
static uint32_t _focal_lib_current_progress;
static FocalRect _focal_lib_last_touch;
static FocalRect _focal_lib_next_touch;
static FocalMaskList _focal_lib_mask_list;
static void _load_extended_enroll(focal_lib_extended_enroll_t* ext);

static int _focal_jni_touches_very_immobile = 0;
static focal_lib_extended_enroll_t _focal_lib_current_extended_data;

/******************************************************************************/
static void enrollStart(void) {
    focal_def_return_t ret = FOCAL_DEF_OK;

    ret = focal_tac_begin_enroll();
    if (ret != FOCAL_DEF_OK) {
        LOG_E("focal_tac_begin_enroll() failed with %d", ret);
    } else {
        _focal_lib_current_progress = 0;
    }
}

/******************************************************************************/
static int32_t enrollAddImage(focal_image_quality_t* quality)
{
    focal_def_return_t ret;
    focal_lib_enroll_data_t data;
    focal_lib_extended_enroll_t* ext_data;
    ext_data = &_focal_lib_current_extended_data; 

    ret = focal_tac_enroll(&data);
    if (ret != FOCAL_DEF_OK) {
        return ret;
    }

    ext_data->size = data.extended_enroll_size;
    if (ext_data->size != 0) {
        ext_data->data = (uint8_t*) malloc(ext_data->size);
        if (ext_data->data != NULL) {
            focal_tac_get_extended_enroll(ext_data);
            _load_extended_enroll(ext_data);
        }
    } else {
       _focal_lib_suggested_direction_of_next_touch = 0; 
       _focal_lib_enrol_main_cluster_identified = 0; 
    }

    quality->coverage = data.coverage;
    quality->quality = data.quality;
    quality->stitched = data.extended_enroll_size != 0;
    _focal_jni_touches_very_immobile = data.user_touches_too_immobile;

    if (data.result == 0) {
        quality->reject_reason = 0;
        quality->acceptance = 1;
    } else {
        quality->acceptance = 0;

        if (data.result == FOCAL_LIB_ENROLL_FAIL_LOW_QUALITY) {
            quality->reject_reason = 1; // TODO: fix constants from Algo
        } else if (data.result == FOCAL_LIB_ENROLL_FAIL_LOW_COVERAGE) {
            quality->reject_reason = 2; // TODO: fix constants from Algo
        } else {
            quality->reject_reason = 3; // TODO: fix constants from Algo
        }
    }


    if (data.progress > 100) {
        data.progress = 100;
    }

    _focal_lib_current_progress = data.progress;

    return ret;
}

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
    
    LOG_I("%s next_touch %d  main cluster %d number of masks %d\n",
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
static int8_t enrollGetProgress(int8_t* touches_very_immobile)
{
    *touches_very_immobile = _focal_jni_touches_very_immobile;
    return _focal_lib_current_progress;
}

/******************************************************************************/
static int32_t enrollGetSuggestedDirectionOfNextTouch()
{
    return _focal_lib_suggested_direction_of_next_touch;
}

/******************************************************************************/
static int8_t enrollMainClusterIdentified()
{
    return _focal_lib_enrol_main_cluster_identified;
}

/******************************************************************************/
static bool enrollGetLastTouch(FocalRect* rect) {
    memcpy(rect, &_focal_lib_last_touch, sizeof(FocalRect));
    return true;
}

/******************************************************************************/
static bool enrollGetNextTouch(FocalRect* rect) {
    memcpy(rect, &_focal_lib_next_touch, sizeof(FocalRect));
    return true;
}

/******************************************************************************/
static int enrollGetStitchedMaskListSize() {
    return _focal_lib_mask_list.nr_of_masks;
}

/******************************************************************************/
static bool enrollGetStitchedMaskList(FocalRect* rectList)
{
    memcpy(rectList, _focal_lib_mask_list.masks,
            sizeof(FocalRect) * _focal_lib_mask_list.nr_of_masks);

    return true;
}

/******************************************************************************/
static int32_t enrollFinish()
{
    uint32_t id;
    focal_def_return_t ret;

    ret = focal_tac_end_enroll(&id);
    if (ret != FOCAL_DEF_OK) {
        LOG_E("End enroll failed %d", ret);
        return -1;
    }

    return id;
}

/******************************************************************************/
static int32_t identifyStart(uint32_t* ids, uint32_t id_count) 
{
    focal_tac_begin_identify(ids, id_count);
}

/******************************************************************************/
static int32_t identifyFinish(void) 
{
    focal_tac_end_identify();
}

/******************************************************************************/
static int32_t identifyImage(int16_t* result,
                               int32_t* score,
                               focal_image_quality_t* quality)
{
    int32_t retval = 0; 
    focal_lib_identify_data_t data;

    retval = focal_tac_identify(&data);
    quality->coverage = 0;
    quality->quality = 0;
    if (retval != 0) {
        *result = -1;
        *score = 0;
        retval = -1;
    } else {
        *result = data.index;
        *score = data.score;
        retval = data.result;
    }

    LOG_I("Index ident %d", data.index);
    return retval;
}

/******************************************************************************/
static uint32_t removeTemplate(uint32_t id)
{
    return focal_tac_remove_template(id);
}

static uint32_t removeAllTemplates()
{
    focal_def_return_t ret;
    ret = focal_tac_remove_all_templates();
    return ret;
}

/******************************************************************************/
static uint32_t captureImage()
{
    focal_def_return_t ret;
    ret = focal_tac_capture_image();
    return ret;
}

/******************************************************************************/
static int32_t debugInjectImage(uint8_t* image, uint32_t size) {
    focal_tac_debug_inject_image(image, size);
}

/******************************************************************************/
static int32_t debugRetrieveImage(uint8_t* image, uint32_t* size ) {
    focal_tac_debug_retrieve_image(image, size);
}

/******************************************************************************/
static int8_t enrolEstimateFingerSize() {
    return 0;
}

/******************************************************************************/
static uint32_t getIdsCount() {
    uint32_t id;
    uint32_t len = 1;
    focal_tac_get_ids_count(&len);
    return len;
}


/******************************************************************************/
static void getIds(uint32_t* ids, uint32_t* len) {
    focal_tac_get_ids(ids, len);
}

/******************************************************************************/
static uint32_t waitForFingerPresent() {
    return focal_hal_wait_for_finger_present(_FOCAL_LIB_MAX_FINGER_PRESENT_TRIES);
}

/******************************************************************************/
static uint32_t waitForFingerLost() {
    return focal_hal_wait_for_finger_lost(_FOCAL_LIB_MAX_FINGER_LOST_TRIES);
}

/******************************************************************************/
static int32_t deadPixelTest(int32_t* nDeadPixels) {
    return focal_tac_deadpixel_test(nDeadPixels); 
}

/******************************************************************************/
static int32_t wakeupSetup(void) {
    return  focal_hal_suspend();
}
#if 0
/******************************************************************************/
static uint32_t set_callback(focal_exit_condition_t* shouldExit, void* user) {
   focal_hal_set_exit_callback(shouldExit, user); 
}
#endif
/******************************************************************************/
static void _abort(void) {
    focal_hal_abort();
}
/******************************************************************************/
void getIalgorithm(ialgorithm_t* ialgorithm)
{
    ialgorithm->enrolStart = enrollStart;
    ialgorithm->enrolAddImage = enrollAddImage;
    ialgorithm->enrolGetProgress = enrollGetProgress;
    ialgorithm->enrolGetSuggestedDirectionOfNextTouch = 
        enrollGetSuggestedDirectionOfNextTouch;

    ialgorithm->enrolMainClusterIdentified = enrollMainClusterIdentified;
    ialgorithm->enrolGetLastTouch = enrollGetLastTouch;
    ialgorithm->enrolGetNextTouch = enrollGetNextTouch;
    ialgorithm->enrolGetStitchedMaskList = enrollGetStitchedMaskList;
    ialgorithm->enrolGetStitchedMaskListSize = enrollGetStitchedMaskListSize;
    ialgorithm->enrolFinish = enrollFinish;
    ialgorithm->identifyStart= identifyStart;
    ialgorithm->identifyImage = identifyImage;
    ialgorithm->identifyFinish = identifyFinish;
    ialgorithm->removeTemplate= removeTemplate;
    ialgorithm->removeAllTemplates = removeAllTemplates;
    ialgorithm->debugInjectImage = debugInjectImage;
    ialgorithm->debugRetrieveImage = debugRetrieveImage;
    ialgorithm->waitForFingerPresent = waitForFingerPresent;
    ialgorithm->waitForFingerLost = waitForFingerLost;
    ialgorithm->enrolEstimateFingerSize = enrolEstimateFingerSize;
    ialgorithm->getIds = getIds;
    ialgorithm->getIdsCount = getIdsCount;
    ialgorithm->captureImage = captureImage;
    ialgorithm->deadPixelTest = deadPixelTest;
    ialgorithm->abort = _abort;
    ialgorithm->wakeupSetup = wakeupSetup;
    focal_hal_init();
}

/******************************************************************************/
void getIalgorithmClose() {
    focal_hal_deinit();
}

/******************************************************************************/
int32_t getIalgorithmVersion(void)
{
    return FOCAL_IALGORITHM_VERSION;
}
