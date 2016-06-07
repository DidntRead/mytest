
#include <sys/types.h>

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "sensor_irq.h"
#include "util.h"

#include "log.h"


// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
static sig_callback_t sig_func = NULL;

static void sensor_irq_sighandler(int n, siginfo_t *info, void *unused);


// -----------------------------------------------------------------------------
// Function definitions
// -----------------------------------------------------------------------------
int sensor_irq_detect(const sensor_env_t *env)
{
	int ret = 0;
	int val;

	ret = util_read_int(env->device_irq, "setup/dst_pid", &val);

	if(ret > 0)
		ret = 0;

	return ret;
}


// -----------------------------------------------------------------------------
int sensor_irq_disconnect(const sensor_env_t *env)
{
	int ret = 0;
	int val = 0;


	ret = util_read_int(env->device_irq, "setup/dst_signo", &val);

	if (ret > 0)
	{
		signal(val, SIG_DFL);

		ret = util_write_int(env->device_irq, "setup/dst_pid", -1);
	}
	if (ret == 0)
		ret = util_write_int(env->device_irq, "setup/enabled", 0);

	return ret;
}


// -----------------------------------------------------------------------------
int sensor_irq_register_sighandler(const sensor_env_t *env, sig_callback_t callback, int signo)
{
	int ret = 0;
	struct sigaction sig;

	pid_t my_pid  = getpid();

	memset (&sig, 0, sizeof(struct sigaction));
	sig.sa_sigaction = sensor_irq_sighandler;
	sig.sa_flags = SA_SIGINFO;
	ret = sigaction(signo, &sig, NULL);

	if(ret == 0)
		ret = util_write_int(env->device_irq, "setup/dst_pid", (int)my_pid);

	if(ret == 0)
		ret = util_write_int(env->device_irq, "setup/dst_signo", signo);

	if (ret == 0)
		ret = util_write_int(env->device_irq, "setup/enabled", 0);

	if(ret == 0)
		sig_func = callback;

	return ret;
}


// -----------------------------------------------------------------------------
int sensor_irq_unregister_sighandler(void)
{
	sig_func = NULL;
	return 0;
}


// -----------------------------------------------------------------------------
int sensor_irq_notify_irq_enable(const sensor_env_t *env, bool new_state)
{
	return util_write_int(env->device_irq, "setup/enabled", (new_state)? 1 : 0);
}


// -----------------------------------------------------------------------------
int sensor_irq_notify_pm_enable(const sensor_env_t *env, bool new_state)
{
	return util_write_int(env->device_irq, "pm/notify_enabled", (new_state)? 1 : 0);
}


// -----------------------------------------------------------------------------
int sensor_irq_ack_pm_notify(const sensor_env_t *env, int val)
{
	return util_write_int(env->device_irq, "pm/notify_ack", val);
}


// -----------------------------------------------------------------------------
int sensor_irq_get_pm_state(const sensor_env_t *env)
{
	int ret, temp;

	ret = util_read_int(env->device_irq, "pm/state", &temp);

	return (ret < 0)? ret : temp;
}


// -----------------------------------------------------------------------------
static void sensor_irq_sighandler(int n, siginfo_t *info, void *unused)
{
	int val = info->si_int;
	LOGD("%s : received value %i\n", __func__, val);

	if(sig_func != NULL)
	{
		sig_func(val);
	}
}


// -----------------------------------------------------------------------------


