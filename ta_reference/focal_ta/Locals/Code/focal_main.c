/**
 * @file   focal_main.c
 *
 */
#include "focal_main.h"
#include "focal_lib.h"
#include "focal_db.h"
#include "tci.h"
#include "TlApi/TlApi.h"

#include "focal_def.h"
#include "focal_lib.h"

/* add for alipay begin*/
#include "drfocalApi.h"
/* add for alipay end*/

/* The main thread where all tasks are run. */
_THREAD_ENTRY void focal_main_thread(void);
focal_lib_enroll_data_t _focal_main_current_enroll_data;
static uint32_t _focal_main_current_enroll_id;
uint32_t* _focal_main_identify_id_list;
uint32_t _focal_main_identify_id_list_count;
int32_t _focal_main_identify_updated_index;
uint32_t _focal_main_identify_updated_template_size;

/******************************************************************************/
focal_lib_return_t focal_main_init(void) {
    focal_lib_return_t ret;
    uint8_t* data;
    uint32_t size;

    focal_debug("init database");
    focal_db_init();

    focal_debug("init focal_lib");

#if 0	//focal
    size = focal_lib_init_data_size();
#endif
    data = malloc(size);
    if (data == NULL) {
        focal_debug("failed to allocate init data mem.");
        return FOCAL_LIB_ERROR_MEMORY;
    }
#if 0	//focal
    ret = focal_lib_init(data, &size);
#endif
    /* Should be stored */
    free(data);
    data = NULL;

    _focal_main_current_enroll_id = 0;
    _focal_main_identify_id_list = NULL;
    _focal_main_identify_id_list_count = 0;
    _focal_main_identify_updated_index = -1;

    /* add for alipay begin*/
//    drfocal_init();	//wenfs
    /* add for alipay end*/  

    focal_debug("Lib init returned %d", ret);
    return ret;
}

/******************************************************************************/
focal_lib_return_t focal_main_begin_identify(void* buffer, uint32_t size)
{
    focal_lib_return_t ret;
    uint32_t i, id_count;
    uint32_t* ids;
    focal_lib_template_t* list;

    /* Set the update index to a non valid value */
    _focal_main_identify_updated_index = -1;

    id_count = size / sizeof(focal_template_id_t);
    focal_debug("-->%s idcount %d", __func__, id_count);

    if (NULL == buffer) {
        focal_debug("<--%s id buffer is null", __func__);
        return FOCAL_LIB_ERROR_PARAMETER;
    }

    if (id_count == 0) {
        focal_debug("<--%s id count is 0, should be at least 1", __func__);
    }

    if (_focal_main_identify_id_list != NULL) {
        free(_focal_main_identify_id_list);
    }

    /* Store the id list until end identify */
    _focal_main_identify_id_list =
        (focal_template_id_t*) malloc(id_count * sizeof(focal_template_id_t));

    if (_focal_main_identify_id_list == NULL) {
        return FOCAL_LIB_ERROR_MEMORY;
    }

    _focal_main_identify_id_list_count = id_count;

    memcpy(_focal_main_identify_id_list, buffer, size);
    
    /* Get the template list before end identify */
    list = (focal_lib_template_t*) malloc(id_count * sizeof(focal_lib_template_t));
    if (list == NULL) {
        focal_debug("no mem for template list");
        return FOCAL_LIB_ERROR_MEMORY;
    }

    ids = (uint32_t*) buffer;
    for (i = 0; i < id_count; i++) {
        focal_debug("template id %i is %d", i, ids[i]);
    }

    ret = focal_db_get_templates(ids, id_count, list);
    if (ret != FOCAL_LIB_OK) {
        focal_debug("Failed to find any matching templates. %d", ret);
        return FOCAL_LIB_ERROR_PARAMETER;
    }

    focal_debug("Got template list 0x%08X size %d\n", list[0].tpl, list[0].size);
//wenfs    ret = focal_lib_begin_identify(list, id_count, FOCAL_LIB_SECURITY_HIGH);
    focal_debug("Begin identify returns %d", ret);

    free(list);
    list = NULL;
    return ret;
}

/******************************************************************************/
focal_lib_return_t focal_main_identify(void* data, uint32_t size) {
    focal_lib_return_t ret;
    focal_lib_identify_data_t ident_data;

    if (size < sizeof(focal_lib_identify_data_t)) {
        return FOCAL_LIB_ERROR_PARAMETER;
    }
//wenfs    ret = focal_lib_identify(&ident_data);
    focal_debug("Identify returns %d, ident_data:result=%d,score=%d,index=%d,updated_size=%d ", ret,ident_data.result,ident_data.score, ident_data.index, ident_data.updated_template_size);

    if (FOCAL_LIB_IDENTIFY_MATCH_UPDATED_TEMPLATE == ident_data.result) {
        _focal_main_identify_updated_index = ident_data.index;
        _focal_main_identify_updated_template_size =
            ident_data.updated_template_size;
    } 

    if (FOCAL_LIB_IDENTIFY_NO_MATCH != ident_data.result) {
        ident_data.index = _focal_main_identify_id_list[ident_data.index]; 
        focal_debug("%s id used %d", __func__, ident_data.index);
    }

    memcpy(data, &ident_data, sizeof(focal_lib_identify_data_t));

	/* add for alipay begin*/
    if (FOCAL_LIB_IDENTIFY_NO_MATCH != ident_data.result) {
	    drfp_setPassId(ident_data.index);
    }
	/* add for alipay end*/

    return ret;
}

/******************************************************************************/
focal_lib_return_t focal_main_end_identify() {
    focal_lib_return_t ret;
    focal_lib_template_t tpl;
    int32_t updated_index;
    updated_index = _focal_main_identify_updated_index;

    tpl.tpl = NULL;
    tpl.size = 0;

    if (updated_index > -1 &&
        updated_index < (int32_t) _focal_main_identify_id_list_count) {

        focal_debug("%s update template index %d id %d", __func__, updated_index,
                _focal_main_identify_id_list[updated_index]);
        tpl.tpl = (uint8_t*) malloc(_focal_main_identify_updated_template_size);
        if (tpl.tpl == NULL) {
            ret = FOCAL_LIB_ERROR_MEMORY;
            goto error;
        }
        tpl.size = _focal_main_identify_updated_template_size;
    }

//wenfs    ret = focal_lib_end_identify(&tpl);
    focal_debug("%s end identify returns %d ", __func__,  ret);
    /* FOCAL_LIB_ERROR_MEMORY means the buffer was too small */
    if (FOCAL_LIB_ERROR_MEMORY == ret) {
        /* Try to allocate a buffer of the correct size */
        if (tpl.size > _focal_main_identify_updated_template_size) {
            free(tpl.tpl);  
            tpl.tpl = NULL;

            tpl.tpl = (uint8_t*) malloc(tpl.size);
            if (tpl.tpl == NULL) {
                ret = FOCAL_LIB_ERROR_MEMORY;
                goto error;
            }

//wenfs            ret = focal_lib_end_identify(&tpl);
        }
    } 
    
    if (ret != FOCAL_LIB_OK) {
        goto error;
    }

    if (updated_index > -1 &&
        updated_index < (int32_t) _focal_main_identify_id_list_count) {
       
        focal_db_update_template(_focal_main_identify_id_list[updated_index], &tpl);
    }

error:
    if (_focal_main_identify_id_list != NULL) {
        free(_focal_main_identify_id_list);
        _focal_main_identify_id_list = NULL;
    }

    if (NULL != tpl.tpl) {
        free(tpl.tpl);
        tpl.tpl = NULL;
    }

    _focal_main_identify_id_list_count = 0;

    focal_debug("<--%s returns %d",__func__, ret);
    return ret;
}

/******************************************************************************/
focal_lib_return_t focal_main_get_extended_data(void* data, uint32_t size) {
    focal_lib_return_t ret;
    focal_lib_extended_enroll_t ext_data;
    uint32_t l_size = size;
    focal_debug("-->%s size %d", __func__, size);

    ext_data.data = data;
    ext_data.size = size;
//wenfs    ret = focal_lib_get_extended_enroll_data(&ext_data, &l_size);
    if (FOCAL_LIB_OK != ret) {
        focal_debug("<--%s extended enroll failed %d", __func__, ret);
        return ret;
    }

    if (l_size > size) {
        focal_debug("<--%s actual size %d, buffer size %d",
                __func__, l_size, size);

        return FOCAL_LIB_ERROR_PARAMETER;
    }

    focal_debug("<--%s return %d", __func__, ret);
    return ret;
}

/******************************************************************************/
focal_lib_return_t focal_main_begin_enroll(uint32_t id) {
    focal_lib_return_t ret;
    focal_debug("-->%s", __func__);
    _focal_main_current_enroll_id = id;
//wenfs    ret = focal_lib_begin_enroll();
    focal_debug("<--%s", __func__);
    return ret;
}

/******************************************************************************/
focal_lib_return_t focal_main_enroll(void* data, uint32_t size) {
    focal_lib_enroll_data_t* enroll_data = &_focal_main_current_enroll_data;
    focal_lib_return_t ret;

    focal_debug("-->%s", __func__);
//wenfs    ret = focal_lib_enroll(enroll_data);
    if (FOCAL_LIB_OK != ret) {
        focal_debug("%s enroll failed", __func__);
        return ret;
    }

    if (size < sizeof(focal_lib_enroll_data_t)) {
        focal_debug("<--%s size fault %d", __func__, size);
        return FOCAL_LIB_ERROR_MEMORY;
    }

    memcpy(data, enroll_data, sizeof(focal_lib_enroll_data_t));
    focal_debug("done data enroll. %d %d %d",
            enroll_data->result, enroll_data->quality, enroll_data->progress);

    focal_debug("<--%s", __func__);
    return FOCAL_LIB_OK;
}

/******************************************************************************/
focal_lib_return_t focal_main_end_enroll(uint32_t* id) {
    focal_lib_return_t ret;
    focal_lib_template_t tpl;

    focal_debug("-->%s, end enroll", __func__);

    tpl.size = _focal_main_current_enroll_data.enrolled_template_size;
    tpl.tpl = malloc(tpl.size);
    if (tpl.tpl == NULL) {
        focal_debug("<--%s, no memory for template.", __func__);
        return FOCAL_LIB_ERROR_MEMORY;
    }

//wenfs    ret = focal_lib_end_enroll(&tpl);
#if 0
    if (ret == FOCAL_LIB_ERROR_MEMORY) {
        free(tpl.tpl);
        tpl.tpl = malloc(tpl.size);
        if (tpl.tpl == NULL) {
            focal_debug("<--%s, no memory for template.", __func__);
            goto error;
        }
        ret = focal_lib_end_enroll(&tpl);
    }
#endif
    if (ret != FOCAL_LIB_OK) {
        focal_debug("%s, failed to end enroll %d.", __func__, ret);
        goto error;
    }

   if (_focal_main_current_enroll_data.progress < 100) {
        focal_debug("<--%s, Enroll not complete.", __func__);
        return FOCAL_LIB_ERROR_GENERAL;
    }

    ret = focal_db_add_template(&tpl, id);
    if (ret != FOCAL_LIB_OK) {
        focal_debug("%s, failed to add template data %d", __func__, ret);
        goto error;
    }

error:
    free(tpl.tpl);

    focal_debug("<--%s, returns %d", __func__, ret);
    return ret;
}

/******************************************************************************/
focal_lib_return_t focal_main_get_ids(void* ids, uint32_t* size) {
    uint32_t i;

    focal_db_get_ids((uint32_t*) ids, size);
    for (i = 0; i < *size; i++) {
        focal_debug("%s, template tid %d is %d",
                __func__, i, ((uint32_t*)ids)[i]);
    }

    return FOCAL_LIB_OK;
}

/******************************************************************************/
focal_lib_return_t focal_main_debug_inject_image(void* image, uint32_t len) {
    focal_lib_return_t ret;

    if (len != FOCAL_DEF_IMAGE_SIZE) {
        focal_debug("len not the same , image 0x%02X, "
                " len %d ", image, len);
    }

    focal_debug("inject with 0x%02X size %d", image, len);
//wenfs    ret = focal_lib_debug_inject_image((uint8_t*) image, len);
    focal_debug("inject returns %d size %d", ret, len);
    return FOCAL_LIB_OK;
}

/******************************************************************************/
focal_lib_return_t focal_main_debug_retrieve_image(void* image, uint32_t len) {

    if (len != FOCAL_DEF_IMAGE_SIZE) {
        focal_debug("len not the same image 0x%02X, "
                "addr 0x%02X len %d ",  image, len);
    }

//wenfs    focal_lib_debug_retrieve_image((uint8_t*) image, len);

    return FOCAL_LIB_OK;
}

/******************************************************************************/
uint32_t focal_main_get_template_db_size() {
   return focal_db_get_id_count();
}
