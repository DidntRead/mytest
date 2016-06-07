#ifndef _TAC_FOCAL_H_
#define _TAC_FOCAL_H_

#include "focal_def.h"
#include "focal_lib.h"

#ifdef __cplusplus
extern "C" {
#endif
focal_def_return_t focal_tac_open(void);

focal_def_return_t focal_tac_init(void);
focal_def_return_t focal_tac_deinit(void);
focal_def_return_t focal_tac_begin_identify(uint32_t* ids, uint32_t length);
focal_def_return_t focal_tac_end_identify();

/*
 * Identify a fingerprint with one of the template id on the list of id
 * associated with the client id.
 *
 * @param client_id - the id of the caller
 * @param id - an array of candidate ids
 * @param length - the length of the candidate list
 *
 * @return
 */
focal_def_return_t focal_tac_identify(focal_lib_identify_data_t* data);

/*
 * Begin the enrol process.
 *
 * @param tid - the id of the template to enroll 
 */
focal_def_return_t focal_tac_begin_enroll();

/*
 * Starts the enrol process.
 *
 */
focal_def_return_t focal_tac_enroll(focal_lib_enroll_data_t* data);

focal_def_return_t focal_tac_get_extended_enroll(focal_lib_extended_enroll_t* data);

/*
 * End the enrol process.
 *
 */
focal_def_return_t focal_tac_end_enroll(uint32_t* id);

/*
 * Gets the enrolled ids from the driver.
 *
 * @param ids - an array to fill with ids  
 * @param len - length of the array 
 */
focal_def_return_t focal_tac_get_ids(uint32_t* ids, uint32_t* len);

focal_def_return_t focal_tac_get_ids_count(uint32_t* len);
focal_def_return_t focal_tac_deadpixel_test(int* nDeadPixels);

focal_def_return_t focal_tac_debug_inject_image(uint8_t* image, uint32_t size);
focal_def_return_t focal_tac_debug_retrieve_image(uint8_t* image, uint32_t* size);
focal_def_return_t focal_tac_init_finger_detect(uint32_t* data);
focal_def_return_t focal_tac_start_finger_detect(uint32_t* data);
focal_def_return_t focal_tac_check_finger_present(uint32_t* data);
focal_def_return_t focal_tac_check_finger_lost(uint32_t* data);
focal_def_return_t focal_tac_capture_image();
focal_def_return_t focal_tac_remove_template(uint32_t id);
focal_def_return_t focal_tac_spidrv_open(void);

focal_def_return_t focal_tac_get_image_buf(uint8_t *image);

void focal_tac_spidrv_close(void);

void focal_tac_close(void);
#ifdef __cplusplus
}
#endif
#endif // _TAC_FOCAL_H_
