
#include <stdio.h>

#include "sensor.h"
#include "util.h"

#include "log.h"

// -----------------------------------------------------------------------------
// Function definitions
// -----------------------------------------------------------------------------
int sensor_detect(const sensor_env_t *env)
{
	int ret = 0;
	int val;

	LOGD("%s\n", __func__);

	ret = util_read_int(env->device_spi, "setup/capture_mode", &val);

	if(ret > 0)
		ret = 0;

	return ret;
}


// -----------------------------------------------------------------------------
int sensor_disconnect(void)
{
	return 0;
}


// -----------------------------------------------------------------------------
int sensor_call_isr(const sensor_env_t *env)
{
	int ret = 0;

	LOGD("%s\n", __func__);

	ret = util_write_int(env->device_spi, "diag/fake_irq", 1);

	return ret;
}


// -----------------------------------------------------------------------------


