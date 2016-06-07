
#include <sys/types.h>

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "sensor.h"
#include "sensor_irq.h"
#include "sensor_ctrl.h"

#include "log.h"


// -----------------------------------------------------------------------------
const sensor_env_t sensor_env =
{
	.device_file = "/dev/focal1020",
	.device_spi  = "/sys/bus/spi/devices/spi4.0",
	.device_irq  = "/sys/bus/platform/devices/focal_interrupt.21"
};

int irq_signal = -1;

volatile sig_atomic_t stop_req = 0;

const char *str_test    = "SIGNAL_TEST";
const char *str_irq     = "IRQ";
const char *str_suspend = "SUSPEND REQUEST";
const char *str_resume  = "RESUME REQUEST";
const char *str_unknown = "UNKNOWN";


// -----------------------------------------------------------------------------
void sig_handler(int val)
{
	// static int sensor_detected = 0;
	int ret = 0;
	char *str;

	str = (val == FOCAL_IRQ_SIGNAL_TEST) ? 		str_test :
		(val == FOCAL_IRQ_SIGNAL_INTERRUPT_REQ) ? str_irq :
		(val == FOCAL_IRQ_SIGNAL_SUSPEND_REQ) ?	str_suspend :
		(val == FOCAL_IRQ_SIGNAL_RESUME_REQ) ?	str_resume :
							str_unknown;

	LOGD("%s val = %d (%s)\n", __func__, val, str);

	switch (val)
	{
		case FOCAL_IRQ_SIGNAL_TEST:
		case FOCAL_IRQ_SIGNAL_INTERRUPT_REQ:
			break;

		case FOCAL_IRQ_SIGNAL_SUSPEND_REQ:
			ret = sensor_irq_ack_pm_notify(&sensor_env, FOCAL_IRQ_SIGNAL_SUSPEND_REQ);
			break;

		case FOCAL_IRQ_SIGNAL_RESUME_REQ:
			ret = sensor_irq_ack_pm_notify(&sensor_env, FOCAL_IRQ_SIGNAL_RESUME_REQ);
			break;

		default:
			break;
	}

	LOGD("%s ret = %d\n", __func__, ret);

/*
	if (!sensor_detected)
	{
		sensor_detected = (sensor_detect(&sensor_env) == 0) ? 1 : 0;
	}

	if (sensor_detected)
	{
		ret = sensor_call_isr(&sensor_env);

		if (ret != 0)
		{
			LOGE("%s : Unable to call ISR\n", __func__);
		}
	}
*/
}


// -----------------------------------------------------------------------------
static void sigint_handler(int n, siginfo_t *info, void *unused)
{
	LOGD("%s Terminate !\n", __func__);
	stop_req = 1;
}


// -----------------------------------------------------------------------------
int main(void)
{
	int ret = 0;
	struct sigaction sig;

	LOGD("FOCAL fake_hal, enter\n");

	ret = sensor_irq_detect(&sensor_env);

	irq_signal = SIGRTMIN;
	ret = sensor_irq_register_sighandler(&sensor_env, &sig_handler, irq_signal);

	memset (&sig, 0, sizeof(struct sigaction));
	sig.sa_sigaction = sigint_handler;
//	sig.sa_flags = SA_SIGINFO;
	ret = sigaction(SIGTERM, &sig, NULL);

	ret = sensor_ctrl_supply_set(&sensor_env, true);
	LOGD("Enable supply (%d)\n", ret);

	ret = sensor_ctrl_hw_reset(&sensor_env);
	LOGD("Sensor reset (%d)\n", ret);

	stop_req = 0;
	while (!stop_req)
	{
	}

	// cleanup
	ret = sensor_ctrl_supply_set(&sensor_env, false);
	sensor_irq_unregister_sighandler();
	sensor_irq_disconnect(&sensor_env);
	sensor_disconnect();

	LOGD("FOCAL fake_hal, exit! \n");

	return ret;
}


// -----------------------------------------------------------------------------


