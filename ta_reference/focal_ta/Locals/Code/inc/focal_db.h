#ifndef _FOCAL_DB_H_
#define _FOCAL_DB_H_
#include <stdint.h>
#include "focal_lib.h"
#include "focal_def.h"

void focal_db_init(void);

/* 
 * Loads one template from a raw data blob into the database.
 */
int32_t focal_db_load_one_template(
        uint32_t* rawdb, uint32_t buffer_size, uint32_t len);

/*
 * Store on the template with the db_index in the binary blob.
 */
int32_t focal_db_store_one_template(
        uint32_t* buffer, uint32_t buffer_size, uint32_t db_index);

/* 
 * Returns the number of templates in the database 
 */
uint32_t focal_db_get_id_count(void);

/* 
 * Get the templates associated with the ids in the given array.
 * Templates are listed in the same order as the ids.  
 */
focal_lib_return_t focal_db_get_templates(uint32_t* ids, uint32_t size,
        focal_lib_template_t* list);
/*
 * Delete template from the database
 * @returns the index where the id was stored
 */
int32_t focal_db_delete_template(uint32_t id);

/*
 * Delete all templates from the database
 */
void focal_db_delete_template_db();

/* Get size by database index, used where storing the templates */
uint32_t focal_db_get_size_by_index(uint32_t index);

/* Get size by database index, used where storing the templates */
uint32_t focal_db_get_template_size(uint32_t id);

/*
 * Add template to the database
 */
int32_t focal_db_add_template(focal_lib_template_t* tpl, uint32_t* id);

/* 
 * Update the template with the given id. 
 */
focal_def_return_t focal_db_update_template(uint32_t tid, focal_lib_template_t* tpl);

/*
 * Get the size of the template with the given id.
 */
uint32_t focal_db_get_size_by_index(uint32_t id);

/* 
 * Returns the database index associated with the database id
 */
int focal_db_get_db_index_by_id(uint32_t id);

/*
 * Return all ids in the database. 
 */
int32_t focal_db_get_ids(uint32_t* ids, uint32_t* size);

/* 
 * Returns the database id associated with the database index
 */
uint32_t focal_db_get_id_by_db_index(uint32_t index);

#endif
