#ifndef _FOCAL_MAIN_H_
#define _FOCAL_MAIN_H_

#include <tlStd.h>
#include "focal_lib.h"

focal_lib_return_t focal_main_init(void);
focal_lib_return_t focal_main_init_mem(unsigned long addr);
focal_lib_return_t focal_main_begin_identify(void* data, uint32_t size);
focal_lib_return_t focal_main_identify(void* data, uint32_t size);
focal_lib_return_t focal_main_end_identify();
focal_lib_return_t focal_main_load_image(void* data, uint32_t size);
focal_lib_return_t focal_main_debug_inject_image(void* data, uint32_t size);
focal_lib_return_t focal_main_debug_retrieve_image(void* data, uint32_t size);
focal_lib_return_t focal_main_get_ids(void* data, uint32_t* size);
focal_lib_return_t focal_main_begin_enroll(uint32_t id);
focal_lib_return_t focal_main_enroll(void* data, uint32_t size);
focal_lib_return_t focal_main_get_extended_data(void* data, uint32_t size);
focal_lib_return_t focal_main_end_enroll(uint32_t* id);
focal_lib_return_t focal_main_load_template_db(void* data, uint32_t size); 
focal_lib_return_t focal_main_store_template_db(void* data, uint32_t size);
uint32_t focal_main_get_template_db_size(void); 
focal_lib_return_t focal_main_abort(void);
//uint32_t focal_main_progress;
//uint32_t focal_main_event;

#endif /* _FOCAL_MAIN_H_ */
