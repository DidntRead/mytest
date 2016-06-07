
#ifndef __SENSOR_H__
#define __SENSOR_H__

#include "focal_hal_extension.h"

int sensor_detect(const sensor_env_t *env);

int sensor_disconnect(void);

int sensor_call_isr(const sensor_env_t *env);

#endif

