
#ifndef __SENSOR_CTRL_H__
#define __SENSOR_CTRL_H__

#include <stdbool.h>

#include "focal_hal_extension.h"

extern int sensor_ctrl_supply_set(const sensor_env_t *env, bool new_state);

extern int sensor_ctrl_hw_reset(const sensor_env_t *env);

extern int sensor_ctrl_issue_wakeup(const sensor_env_t *env);

#endif


