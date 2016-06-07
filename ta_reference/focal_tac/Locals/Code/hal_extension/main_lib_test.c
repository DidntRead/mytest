
#include <unistd.h>

#include "focal_hal_extension.h"

void irq_callback(int val)
{
}

const sensor_env_t sensor_env =
{
	.device_file = "/dev/focal1020",
	.device_spi  = "/sys/bus/spi/devices/spi4.0",
	.device_irq  ="/sys/bus/platform/devices/focal_interrupt.21"
};

int main(void)
{
	focal_hal_extension_enter(&sensor_env, &irq_callback);
	//focal_hal_extension_enter(&sensor_env, NULL);

	focal_hal_extension_run();

	while (focal_hal_extension_check_running())
	{
		sleep(1);
	}

	focal_hal_extension_exit();

	return 0;
}

