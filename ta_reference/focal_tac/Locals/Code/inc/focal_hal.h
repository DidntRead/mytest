#ifndef __FOCAL_HAL_H_DEFINED__
#define __FOCAL_HAL_H_DEFINED__

focal_def_return_t focal_hal_wait_for_finger_present(uint32_t limit);
focal_def_return_t focal_hal_wait_for_finger_lost(uint32_t limit);
focal_def_return_t focal_hal_standby();
focal_def_return_t focal_hal_stop_standby();
focal_def_return_t focal_hal_suspend();
focal_def_return_t focal_hal_abort();
focal_def_return_t focal_hal_init();
focal_def_return_t focal_hal_deinit();

#endif /* __FOCAL_HAL_H_DEFINED__ */
