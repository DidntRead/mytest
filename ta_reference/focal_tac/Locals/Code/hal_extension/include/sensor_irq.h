
#ifndef __SENSOR_IRQ_H__
#define __SENSOR_IRQ_H__

#include <stdbool.h>
#include <linux/stddef.h>

#include "focal_hal_extension.h"

typedef void (*sig_callback_t)(int);

int sensor_irq_detect(const sensor_env_t *env);

int sensor_irq_disconnect(const sensor_env_t *env);

int sensor_irq_register_sighandler(const sensor_env_t *env, sig_callback_t callback, int signo);

int sensor_irq_unregister_sighandler(void);

int sensor_irq_notify_irq_enable(const sensor_env_t *env, bool new_state);

int sensor_irq_notify_pm_enable(const sensor_env_t *env, bool new_state);

int sensor_irq_ack_pm_notify(const sensor_env_t *env, int val);

int sensor_irq_get_pm_state(const sensor_env_t *env);

#endif

