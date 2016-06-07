
#include <sys/types.h>

#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "sensor.h"
#include "sensor_ctrl.h"
#include "sensor_irq.h"

#include "focal_hal_extension.h"
#include "log.h"

// -----------------------------------------------------------------------------
// Function prototypes
// -----------------------------------------------------------------------------
static int register_sigint(void);
static void sigint_handler(int n, siginfo_t *info, void *unused);
static void sig_handler_default(int);


// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
const sensor_env_t sensor_env_default =
{
	.device_file = "/dev/focal1020",
	.device_spi = "/sys/bus/spi/devices/spi0.1",
	.device_irq = "/sys/bus/platform/devices/focal_irq.0",
};

sensor_env_t const *sensor_env_ptr;

pthread_t execute_thread;

int irq_signal = -1;

volatile sig_atomic_t stop_req = 0;
volatile sig_atomic_t running  = 0;


// -----------------------------------------------------------------------------
// Function definitions
// -----------------------------------------------------------------------------
int focal_hal_extension_enter(sensor_env_t const *env, req_callback_t req_callback)
{
	int ret = 0;

	req_callback_t handler;

	handler = (req_callback)? req_callback : &sig_handler_default;

	sensor_env_ptr = (env)? env : &sensor_env_default;

	LOGD("%s\n", __func__);

	stop_req = 0;

	ret = register_sigint();

	if(ret == 0)
	{
		ret = sensor_irq_detect(sensor_env_ptr);
	}

	if(ret == 0)
	{
		irq_signal = SIGRTMIN;
		ret = sensor_irq_register_sighandler(sensor_env_ptr, handler, irq_signal);
	}

	if(ret != 0)
	{
		LOGE("%s : Error %d\n", __func__, ret);
	}

	return ret;
}


// -----------------------------------------------------------------------------
void *execute_function(void *par)
{
	volatile int x = 0;

	LOGD ("%s : enter, pid=%d, ppid=%d, thread=%d \n", __func__,
			(int)getpid(),
			(int)getppid(),
			(int)pthread_self());

	while (stop_req == 0)
	{
		x++;
		sleep(1); // Yield, just wait for signal !
	};

	LOGD ("%s : exit\n", __func__);

	running = 0;

	return NULL;
}


// -----------------------------------------------------------------------------
int focal_hal_extension_run(void)
{
	int ret = 0;

	LOGD("%s\n", __func__);

	if (running)
	{
		LOGE ("%s : Already running\n", __func__);
		return -1;
	}

	ret = pthread_create(&execute_thread, NULL, execute_function, NULL);

	if (ret)
	{
		LOGE ("%s : Error creating thread (%d)\n",__func__, ret);
	}
	else
	{
		LOGD ("%s : Started thread\n",__func__);
		running = 1;
	}

	return ret;
}


// -----------------------------------------------------------------------------
extern int focal_hal_extension_enable_irq(void)
{
	LOGD("%s\n", __func__);
	return sensor_irq_notify_irq_enable(sensor_env_ptr, true);
}


// -----------------------------------------------------------------------------
extern int focal_hal_extension_irq_disable(void)
{
	LOGD("%s, dummy operation\n", __func__);
	return sensor_irq_notify_irq_enable(sensor_env_ptr, false);
}


// -----------------------------------------------------------------------------
int focal_hal_extension_exit(void)
{
	LOGD("%s\n", __func__);

	stop_req = 1;

	if(pthread_join(execute_thread, NULL))
	{
		LOGE ("%s : Error stopping thread\n",__func__);
	}

	sensor_irq_unregister_sighandler();
	sensor_irq_disconnect(sensor_env_ptr);
	sensor_disconnect();

	return 0;
}


// -----------------------------------------------------------------------------
int focal_hal_extension_check_running(void)
{
	return (running)? 1 : 0;
}


// -----------------------------------------------------------------------------
int focal_hal_extension_enable_supply(void)
{
	LOGD("%s\n", __func__);
	return sensor_ctrl_supply_set(sensor_env_ptr, true);
}


// -----------------------------------------------------------------------------
int focal_hal_extension_supply_disable(void)
{
	LOGD("%s\n", __func__);
	return sensor_ctrl_supply_set(sensor_env_ptr, false);
}


// -----------------------------------------------------------------------------
int focal_hal_extension_hw_reset(void)
{
	LOGD("%s\n", __func__);
	return sensor_ctrl_hw_reset(sensor_env_ptr);
}


// -----------------------------------------------------------------------------
int focal_hal_extension_pm_check_state(void)
{
	return sensor_irq_get_pm_state(sensor_env_ptr);
}


// -----------------------------------------------------------------------------
int focal_hal_extension_enable_pm_notify(void)
{
	LOGD("%s\n", __func__);
	return sensor_irq_notify_pm_enable(sensor_env_ptr, true);
}


// -----------------------------------------------------------------------------
int focal_hal_extension_pm_notify_disable(void)
{
	LOGD("%s\n", __func__);
	return sensor_irq_notify_pm_enable(sensor_env_ptr, false);
}

// -----------------------------------------------------------------------------
int focal_hal_extension_suspend_ack(int type)
{
	LOGD("%s\n", __func__);
	return sensor_irq_ack_pm_notify(sensor_env_ptr, type);
}


// -----------------------------------------------------------------------------
int focal_hal_extension_resume_ack(int type)
{
	LOGD("%s\n", __func__);
	return sensor_irq_ack_pm_notify(sensor_env_ptr, type);
}


// -----------------------------------------------------------------------------
int focal_hal_extension_wakeup_req(void)
{
	LOGD("%s\n", __func__);
	return sensor_ctrl_issue_wakeup(sensor_env_ptr);
}


// -----------------------------------------------------------------------------
static int register_sigint(void)
{
	int ret = 0;
	struct sigaction sig;

	memset (&sig, 0, sizeof(struct sigaction));
	sig.sa_sigaction = sigint_handler;
	ret = sigaction(SIGTERM, &sig, NULL);
	
	return ret;
}


// -----------------------------------------------------------------------------
static void sigint_handler(int n, siginfo_t *info, void *unused)
{
	LOGD("%s : stop req.\n", __func__);
	stop_req = 1;
}


// -----------------------------------------------------------------------------
static void sig_handler_default(int val)
{
	static int sensor_detected = 0;
	int ret = 0;

	LOGD("%s\n", __func__);

	if (!sensor_detected)
	{
		sensor_detected = (sensor_detect(sensor_env_ptr) == 0) ? 1 : 0;
	}

	if (sensor_detected)
	{
		ret = sensor_call_isr(sensor_env_ptr);

		if (ret != 0)
		{
			LOGE("%s : Unable to call ISR\n", __func__);
		}
	}
}


// -----------------------------------------------------------------------------


