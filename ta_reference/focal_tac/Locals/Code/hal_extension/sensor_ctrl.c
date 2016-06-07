
#include <stdbool.h>

#include "sensor_ctrl.h"
#include "util.h"

#include "log.h"

// -----------------------------------------------------------------------------
// Function definitions
// -----------------------------------------------------------------------------
int sensor_ctrl_supply_set(const sensor_env_t *env, bool new_state)
{
	return util_write_int(env->device_irq,
				"pm/supply_on",
				(new_state) ? 1 : 0);
}


// -----------------------------------------------------------------------------
int sensor_ctrl_hw_reset(const sensor_env_t *env)
{
	return util_write_int(env->device_irq, "pm/hw_reset", 1);
}


// -----------------------------------------------------------------------------
extern int sensor_ctrl_issue_wakeup(const sensor_env_t *env)
{
	return util_write_int(env->device_irq, "pm/wakeup_req", 1);
}


// -----------------------------------------------------------------------------

