#include "tlStd.h"
#include "TlApi/TlApi.h"
#include <stdint.h>
#include "tci.h"
#include "focal_db.h"
#include "focal_lib.h"
#include "focal_def.h"

/* add for alipay begin*/
#include "drfocalApi.h"
static void focal_db_update_ids_dr(void);
/* add for alipay end*/

#define _FOCAL_DB_MAX_SIZE 5
#define _FOCAL_TEMPLATE_NAME_SIZE 128

#define _FOCAL_DB_TEMPLATE_SIZE_INDEX     0
#define _FOCAL_DB_TEMPLATE_ID_INDEX       1
#define _FOCAL_DB_TEMPLATE_DATA_INDEX     2
#define _FOCAL_DB_TEMPLATE_HEADER_SIZE    2

typedef struct {
    /* Id of the template */
    uint32_t id;
    uint32_t size;
    uint8_t* data;
} focal_db_template_t;

typedef struct {
       /* Number of templates in the database */
    uint32_t size;
   /* List of all template information */
    focal_db_template_t templates[_FOCAL_DB_MAX_SIZE];
} focal_db_template_db_t;

static focal_db_template_db_t _focal_db_template_db;
static uint32_t _focal_db_highest_id = 42;

/******************************************************************************/
void focal_db_init(void)
{
    uint32_t i;
    _focal_db_highest_id = 42;
    _focal_db_template_db.size= 0;

    for (i = 0; i < _FOCAL_DB_MAX_SIZE; i++) {
        _focal_db_template_db.templates[i] = (focal_db_template_t) {
                .id = 0,
                .size = 0,
                .data = NULL,
            };
    }
}

/******************************************************************************/
int32_t focal_db_load_one_template(
        uint32_t* raw_db, uint32_t buffer_size, uint32_t index)
{
    uint32_t size, id;
    focal_db_template_t* template;

    focal_debug("%s: Loading templates count: %d",
            __func__, _focal_db_template_db.size);

    size = raw_db[_FOCAL_DB_TEMPLATE_SIZE_INDEX];
    if (buffer_size < (size + _FOCAL_DB_TEMPLATE_HEADER_SIZE)) {
        focal_debug("%s, buffer is too small to contain the template "
                "buffer_size %d template size %d", __func__, buffer_size,
                size + _FOCAL_DB_TEMPLATE_HEADER_SIZE);
        return FOCAL_DEF_ERROR_SIZE;
    }

    id = raw_db[_FOCAL_DB_TEMPLATE_ID_INDEX];

    focal_debug("%s: packed template size %d, tid = %d ",__func__, size, id);

    template = &_focal_db_template_db.templates[index];
    if (template->data != NULL) {
        focal_debug("<--%s template index %d not empty", __func__, index)
        return FOCAL_DEF_ERROR_SIZE;
    }

    template->data = (uint8_t*) malloc(size);
    if (template->data == NULL) {
        focal_debug("<--%s: Failed to allocate memory for template data",
                __func__);

        return FOCAL_DEF_ERROR_MEM;
    }
        template->size = size;
    template->id = id;
    if (id > _focal_db_highest_id) {
        _focal_db_highest_id = id;
    }
    memcpy(template->data, &raw_db[_FOCAL_DB_TEMPLATE_DATA_INDEX], size);

    _focal_db_template_db.size++;

	/* add for alipay begin*/
	focal_db_update_ids_dr();
	/* add for alipay end*/

    return FOCAL_DEF_OK;
}

/******************************************************************************/
int32_t focal_db_store_one_template(
        uint32_t* raw_db, uint32_t len,  uint32_t db_index)
{
    uint32_t size, id;
    focal_db_template_t* template;

    focal_debug("-->%s, templates count %d with dbIndex %d",
            __func__, _focal_db_template_db.size, db_index);

    if (raw_db == NULL) {
        focal_debug("%s, no buffer to store template.", __func__);
        return FOCAL_DEF_ERROR_MEM;
    }

    if (db_index >= _FOCAL_DB_MAX_SIZE) {
        focal_debug("%s, index out of bounds.", __func__);
        return FOCAL_DEF_ERROR_SIZE;
    }

    template = &_focal_db_template_db.templates[db_index];
    if (template->data != NULL) {
        id = template->id;
        size = template->size;
        focal_debug("%s, index: %d size: %d tid: %d",
                __func__, db_index, size, id);

        if ((_FOCAL_DB_TEMPLATE_HEADER_SIZE + size) <= len) {
            raw_db[_FOCAL_DB_TEMPLATE_SIZE_INDEX] = size;
            raw_db[_FOCAL_DB_TEMPLATE_ID_INDEX] = id;
            memcpy(&raw_db[_FOCAL_DB_TEMPLATE_DATA_INDEX], template->data, size);
        } else {
            focal_debug("%s, Buffer too small for template. buffer size %d "
                    "template size %d", __func__,
                    len, size + _FOCAL_DB_TEMPLATE_HEADER_SIZE);

            focal_debug("<--%s, size error", __func__);
            return FOCAL_DEF_ERROR_SIZE;
        }
    } else {
        focal_debug("<--%s, memory error", __func__);
        return FOCAL_DEF_ERROR_MEM;
    }

    focal_debug("%s data 0x%02X 0x%02X",
            __func__, raw_db[_FOCAL_DB_TEMPLATE_SIZE_INDEX],
            raw_db[_FOCAL_DB_TEMPLATE_ID_INDEX]);


    focal_debug("<--%s, ok", __func__);
    return FOCAL_LIB_OK;
}

/******************************************************************************/
int32_t focal_db_delete_template(uint32_t tid)
{
    uint32_t i;
    focal_db_template_t* template;
    focal_debug("-->%s", __func__);

    for (i = 0; i < _FOCAL_DB_MAX_SIZE; i++) {
        template = &_focal_db_template_db.templates[i];
        if (template->data != NULL) {
            if (template->id == tid) {
                focal_debug("%s, Deleted template, id = %d", __func__, tid);
                free(template->data);
                template->data = NULL;
                template->size = 0;
                template->id = 0;
                _focal_db_template_db.size--;
                /* Break after removing the first template with the right id. */
                break;
            }
        }
    }

    focal_debug("<--%s ended at index = %d", __func__, i);

    if (i == _FOCAL_DB_MAX_SIZE) {
        return -1;
    } else {
	/* add for alipay begin*/
    	focal_db_update_ids_dr();
	/* add for alipay end*/
        return i;
    }
}

void focal_db_delete_template_db()
{
    uint32_t tid;
    focal_db_template_db_t* db;

    db = &_focal_db_template_db;
    focal_debug("Delete template db, cnt=%d", db->size);

    if (db->templates != NULL) {
        for (tid = 0; tid < _FOCAL_DB_MAX_SIZE; tid++) {
            if (db->templates[tid].data != NULL) {
                free(db->templates[tid].data);
                db->templates[tid].data = NULL;
            }
        }
        db->size = 0;
    }
}

/******************************************************************************/
focal_def_return_t focal_db_update_template(uint32_t tid, focal_lib_template_t* tpl) {

    int index;
    focal_db_template_t* template;

    focal_debug("-->%s", __func__);
    index = focal_db_get_db_index_by_id(tid);
    if (index < 0 || index > _FOCAL_DB_MAX_SIZE) {
        focal_debug("<--%s id %d not existing", __func__, tid);
        return FOCAL_LIB_ERROR_PARAMETER;
    }

    if (tpl == NULL) {
        focal_debug("<--%s template is null", __func__);
        return FOCAL_LIB_ERROR_PARAMETER;
    }

    if (tpl->tpl == NULL) {
        focal_debug("<--%s template buffer is null", __func__);
        return FOCAL_LIB_ERROR_PARAMETER;
    }

    template = &_focal_db_template_db.templates[index];
    if (template != NULL) {
        if(template->data != NULL) {
            free(template->data);
        }
    }

    template->data = malloc(tpl->size);
    if (template->data == NULL) {
        focal_debug("%s, no memory for new template", __func__);
    }
    template->size = tpl->size;

    memcpy(template->data, tpl->tpl, tpl->size);

    focal_debug("<--%s", __func__);
    return FOCAL_DEF_OK;
}

/******************************************************************************/
focal_lib_return_t focal_db_get_templates(uint32_t* ids, uint32_t size,
         focal_lib_template_t* list)
{
    uint32_t i,id;
    bool found;
    focal_db_template_t* template;
    focal_debug("-->%s %d", __func__, size);

    if (list == NULL) {
        focal_debug("<--%s no id list", __func__);
        return FOCAL_DEF_ERROR_PARAMETER;
    }
    if (size > _focal_db_template_db.size) {
        focal_debug("<--%s asked for more ids than the database can contain."
                "asked for %d ids but db size is %d",
                __func__, size, _focal_db_template_db.size);

        return FOCAL_DEF_ERROR_MEM;
    }

    for (id = 0; id < size; id++) {
        found = false;
	/* modify the max index for searching begin*/
        for (i = 0; i < _FOCAL_DB_MAX_SIZE; i++) {
	/* modify the max index for searching end*/
            template = &_focal_db_template_db.templates[i];
            if (template != NULL) {
                if (template->id == ids[id] && template->data != NULL) {
                    list[id].tpl = template->data;
                    list[id].size = template->size;
                    found = true;
                }
            }
        }
        if (!found) {
            focal_debug("%s id %d not found in database", __func__, ids[id]);
        }
    }

    focal_debug("<--%s", __func__);
    return FOCAL_LIB_OK;
}

/******************************************************************************/
int32_t focal_db_get_ids(uint32_t* ids, uint32_t* size)
{
    uint32_t i = 0;
    uint32_t c = 0;
    focal_db_template_t* template = NULL;

    if (*size < _focal_db_template_db.size) {
        return FOCAL_DEF_ERROR_SIZE;
    }

    for (i = 0; i < _FOCAL_DB_MAX_SIZE; i++) {
        template = &_focal_db_template_db.templates[i];
        if (template->data != NULL) {
            if (c >= *size) {
                focal_debug("Ids buffer too small.");
            }
            ids[c] = template->id;
            ++c;
        }
    }

    focal_debug("Number of templates is %d", c);

    *size = c;
    return FOCAL_LIB_OK;
}

/******************************************************************************/
uint32_t focal_db_get_id_count() {
    return _focal_db_template_db.size;
}

/******************************************************************************/
int32_t focal_db_add_template(focal_lib_template_t* tpl, uint32_t* id)
{
    uint32_t i;
    uint32_t size = _focal_db_template_db.size;
    focal_db_template_t* template;

    focal_debug("-->%s Highest id %d", __func__, _focal_db_highest_id);

    if (size == _FOCAL_DB_MAX_SIZE) {
        focal_debug("<--%s, database is full.", __func__);
        return FOCAL_DEF_ERROR_SIZE;
    }

    if (tpl == NULL) {
        focal_debug("<--%s, Template struct parameter is null.", __func__);
        return FOCAL_DEF_ERROR_MEM;
    }

    if (tpl->tpl == NULL) {
        focal_debug("<--%s, Template buffer is null.", __func__);
        return FOCAL_DEF_ERROR_MEM;
    }

    for (i = 0; i < _FOCAL_DB_MAX_SIZE; i++) {
        template = &_focal_db_template_db.templates[i];
        if (template->data == NULL) {
            focal_debug("<--%s, template added at %d %d", __func__, i, _focal_db_highest_id);

            template->data = (uint8_t*) malloc(tpl->size);
            if (template->data == NULL) {
                focal_debug("<--%s, Template allocation fail.", __func__);
                return FOCAL_LIB_ERROR_MEMORY;
            }

            memcpy(template->data, tpl->tpl, tpl->size);
            /* Please overflow me */
            ++_focal_db_highest_id;
            template->id = _focal_db_highest_id;
            *id = _focal_db_highest_id;
            template->size = tpl->size;
            _focal_db_template_db.size++;
            break;
        }
    }

    focal_debug("%s data pointer 0x%08X, data 0x%02X 0x%02X 0x%02X 0x%02X \n"
            "0x%02X 0x%02X 0x%02X 0x%02X \n"
            "0x%02X 0x%02X 0x%02X 0x%02X \n",
            __func__, template->data, template->data[0],
            template->data[1], template->data[2], template->data[3],
            template->data[4], template->data[5], template->data[6],
            template->data[7], template->data[8], template->data[9],
            template->data[10], template->data[11]);


    if (i == _FOCAL_DB_MAX_SIZE) {
        focal_debug("<--%s, database was full.", __func__);
        return FOCAL_DEF_ERROR_SIZE;
    }
    
	/* add for alipay begin*/
	focal_db_update_ids_dr();
	/* add for alipay end*/

    focal_debug("<--%s, Added template with id %d .", __func__, *id);
    return FOCAL_DEF_OK;
}

/******************************************************************************/
uint32_t focal_db_get_size_by_index(uint32_t index) {
    focal_db_template_t* template;

    focal_debug("-->%s index %d", __func__, index);
    if (index <  _FOCAL_DB_MAX_SIZE) {
        template = &_focal_db_template_db.templates[index];
        if (template->data != NULL) {
            focal_debug("<--%s size is %d", __func__, template->size + _FOCAL_DB_TEMPLATE_HEADER_SIZE);
            return template->size + _FOCAL_DB_TEMPLATE_HEADER_SIZE;
        }
    }

    focal_debug("<--%s did not found any size.", __func__);
    return 0;
}

/******************************************************************************/
uint32_t focal_db_get_template_size(uint32_t id) {
    int32_t index;

    index = focal_db_get_db_index_by_id(id);
    if (index >= 0) {
        return focal_db_get_size_by_index((uint32_t) index);
    } else {
        return 0;
    }
}

/******************************************************************************/
int focal_db_get_db_index_by_id(uint32_t id) {
    int i = 0;
    int db_index = -1;
    focal_db_template_t* template;


    focal_debug("%s, tid = %d\n", __func__, id);

    for (i = 0; i < _FOCAL_DB_MAX_SIZE; i++) {
        template = &_focal_db_template_db.templates[i];
        if(template->data != NULL && id == template->id) {
            focal_debug("%s, tid = %d found,"
                    "dbIndex = %d\n", __func__, id, i);
            db_index = i;
            break;
        }
    }

    return db_index;
}

/******************************************************************************/
uint32_t focal_db_get_id_by_db_index(uint32_t index) {
    int tid = -1;

    if (index < _FOCAL_DB_MAX_SIZE) {
        tid = _focal_db_template_db.templates[index].id;
    }

    return tid;
}

/* add for alipay begin*/
focal_def_return_t focal_db_dradd_ids(uint32_t *ids, uint32_t idsCount)
{
    struct tlfocalApiDevice focaldev;
    int32_t i = 0;

    focal_debug("===>%s. idsCount: %d\n", __func__, idsCount);

    for (i = 0; i < idsCount; i++) {
        focal_debug("%s: ids[%i] == %d\n", __func__, i, ids[i]);
    }

    if (ids != NULL && idsCount <= _FOCAL_DB_MAX_SIZE) {
        focaldev.ids = ids;
        focaldev.idsCount = idsCount;
    }
    else {
        return FOCAL_DEF_ERROR_GENERAL;
    }

    drfp_setids(&focaldev);

    return FOCAL_DEF_OK;
}

/*Update db template to drfocal*/
static void focal_db_update_ids_dr(void)
{
    uint32_t ids[_FOCAL_DB_MAX_SIZE], id_count;

    id_count = focal_db_get_id_count();
    focal_db_get_ids(ids, &id_count);
    focal_db_dradd_ids(ids, id_count);
}
/*add for alipay end*/

