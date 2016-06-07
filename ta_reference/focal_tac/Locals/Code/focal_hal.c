#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>

#include "focal_tac.h"
#include "focal_def.h"
#include "focal_lib.h"
#include "focal_hal.h"

#define LOG_TAG "focal_hal"
#include "Logwrapper/log.h"

//#include "spi_slsi.h"
#include "focal_hal_extension.h"
#include "pthread.h"
#include "focal_ialgorithm.h"

#define _WAITING_FOR_FINGER_DETECT  0x00
#define _START_FINGER_DETECT        0x01
#define _WAITING_FOR_FINGER_PRESENT 0x02
#define _WAITING_FOR_FINGER_LOST    0x03

/* Wait for 10 ms between finger lost/detect checks */
#define _FOCAL_HAL_WAIT_TIME 100


const sensor_env_t sensor_env =
{
 .device_file =  "/dev/focal1020",
 .device_spi =  "/sys/bus/spi/devices/spi0.1",
 .device_irq =  "/sys/bus/platform/devices/focal_irq.0"
};

const uint32_t spi_port = 4;
const uint32_t spi_clk = 1E6;

volatile uint32_t irq_counter = 0;
static volatile uint32_t _focal_hal_abort = 0;
static uint32_t _focal_hal_exit = 0;
static volatile uint32_t  finger_present_detecting = 0;
static volatile uint32_t  finger_lost_detecting = 0;

pthread_t _focal_hal_main_pthread;
pthread_attr_t _focal_hal_thread_attribute;

pthread_cond_t _focal_hal_irq_sem = PTHREAD_COND_INITIALIZER;
pthread_cond_t _focal_hal_driver_sem = PTHREAD_COND_INITIALIZER;

pthread_mutex_t _focal_hal_irq_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t _focal_hal_driver_mutex = PTHREAD_MUTEX_INITIALIZER;

focal_exit_condition_t* _focal_hal_should_exit = NULL;
void* _focal_hal_user;
int _focal_hal_woken = 0;
int _focal_hal_wakeup = 0;

void* _focal_hal_irq_thread(void* _pvArg);
focal_def_return_t focal_hal_init_irq();
uint32_t _focal_hal_irq_type;

/******************************************************************************/
focal_def_return_t focal_hal_wakeup() {
    focal_def_return_t ret;
    uint32_t spi_ret;
    uint32_t counter = 0;

    ret = focal_tac_wakeup_setup();
    if (FOCAL_LIB_OK != ret) {
        LOG_E("Wakup failed");
        return ret;
    }

    spi_ret = focal_hal_extension_enable_irq();
    if (spi_ret != 0) {
        LOG_I("irq enable error %d", spi_ret);
        return FOCAL_DEF_ERROR_SENSOR;
    }

    pthread_mutex_lock(&_focal_hal_irq_mutex); 
    pthread_cond_wait(&_focal_hal_irq_sem, &_focal_hal_irq_mutex);
    LOG_I("Detected Wakeup IRQ (%d)!", irq_counter);
    pthread_mutex_unlock(&_focal_hal_irq_mutex); 

    spi_ret = focal_hal_extension_irq_disable();
    if (0 != spi_ret) {
       LOG_I("irq disable error %d", spi_ret);
       return FOCAL_DEF_ERROR_GENERAL;
    }            

    while (ret != FOCAL_DEF_FINGER_PRESENT && counter++ < 100) {
        ret = focal_tac_wakeup_qualification();
        LOG_I("Wakeup returns %d", ret);
        usleep(100000);
    }

    return ret;
}

/******************************************************************************/
focal_def_return_t focal_hal_standby() {
    uint32_t irq_ret;

    irq_ret = focal_hal_extension_enable_pm_notify();
    if (0 != irq_ret) {
       LOG_I("pm notify enable error %d", irq_ret);
       return FOCAL_DEF_ERROR_GENERAL;
    }
}

/******************************************************************************/
focal_def_return_t focal_hal_stop_standby() {
    uint32_t irq_ret;

    irq_ret = focal_hal_extension_pm_notify_disable();
    if (0 != irq_ret) {
       LOG_I("pm notify disable error %d", irq_ret);
       return FOCAL_DEF_ERROR_GENERAL;
    }
}

/******************************************************************************/
void focal_hal_set_exit_callback(focal_exit_condition_t* shouldExit, void* user) {
    _focal_hal_should_exit = shouldExit;
    _focal_hal_user = user;
}

/******************************************************************************/
focal_def_return_t focal_hal_wait_for_finger_present(uint32_t limit) {
    int32_t spi_ret;
    uint32_t ret;
    uint32_t data;
    bool wait_for_irq;
    bool done;

    LOG_I("Starting finger detect.");
    
    finger_present_detecting = 1;
    ret = focal_tac_init_finger_detect(&data);
    if (FOCAL_LIB_ERROR_SENSOR == ret) {
        LOG_I("Sensor error %d", ret);
        usleep(_FOCAL_HAL_WAIT_TIME);
        goto finger_detect_error;
    }
    
    if (FOCAL_LIB_ENABLE_EVENT_FINGER_PRESENT != ret) {
        LOG_I("Init finger detect out of sync %d", data);
        usleep(_FOCAL_HAL_WAIT_TIME);
        goto finger_detect_error;
    }
    
    spi_ret = focal_hal_extension_enable_irq();
    if (spi_ret != 0) {
        LOG_I("irq enable error %d", spi_ret);
        ret = FOCAL_DEF_ERROR_GENERAL;
        goto finger_detect_error;
    }

    wait_for_irq = true;
    done = false;
    
    LOG_I("Finger detect initialized.");
    ret = focal_tac_start_finger_detect(&data);
    if (FOCAL_LIB_WAIT_EVENT_FINGER_PRESENT != ret) {
        LOG_I("Start finger detect out of sync %d", ret);
        goto finger_detect_error;
    }

    _focal_hal_abort = false;
    while(!_focal_hal_abort && !done) { 

        if(_focal_hal_should_exit != NULL) {
            if(_focal_hal_should_exit(_focal_hal_user)) {
                break;
            }
        }

        ret = focal_tac_check_finger_present(&data);
        
        switch (ret) {
            case FOCAL_DEF_FINGER_PRESENT:
                LOG_I("Finger detected");
                done = true;
                wait_for_irq = false;
                break;
            
            case FOCAL_DEF_WAIT_TIME:
                LOG_I("Waiting as result %d", data);
                done = false;
                wait_for_irq = false;
                usleep(_FOCAL_HAL_WAIT_TIME);
                break;

            case FOCAL_DEF_FINGER_LOST:
                LOG_I("Finger lost (touch too fast?)");
                done = true;
                wait_for_irq = false;
                break;

            case FOCAL_DEF_WAIT_EVENT_FINGER_PRESENT:
                LOG_I("unexpected return value FOCAL_DEF_WAIT_EVENT_FINGER_PRESENT");
                spi_ret = focal_hal_extension_enable_irq();
                if (spi_ret != 0) {
                    LOG_I("irq enable error %d", spi_ret);
                    ret = FOCAL_DEF_ERROR_GENERAL;
                    break;
                }
                done = false;
                wait_for_irq = false; //samuel test

                usleep(_FOCAL_HAL_WAIT_TIME);//samuel test
                break;

            default:
                LOG_E("Unexpected return value from finger present %d.\n",
                        ret);
                ret = FOCAL_DEF_ERROR_GENERAL;
                done = true;
                wait_for_irq = false;
                break;
        }
        if (wait_for_irq) {
            pthread_mutex_lock(&_focal_hal_irq_mutex); 
            pthread_cond_wait(&_focal_hal_irq_sem, &_focal_hal_irq_mutex);
            LOG_I("Detected IRQ (%d)!", irq_counter);
            pthread_mutex_unlock(&_focal_hal_irq_mutex); 
#if 0
            spi_ret = focal_hal_extension_irq_disable();
            if (0 != spi_ret) {
               LOG_I("irq disable error %d", spi_ret);
               return FOCAL_DEF_ERROR_GENERAL;
            }
#endif
            wait_for_irq = false;
        }
    }

finger_detect_error:
#if 0
    spi_ret = focal_hal_extension_irq_disable();
    if (0 != spi_ret) {
        LOG_I("irq disable error %d", spi_ret);
        return FOCAL_DEF_ERROR_GENERAL;
    }
#endif
    /* Reset finger detect sequence */
    focal_tac_check_finger_lost(&data);

    finger_present_detecting = 0;
    return ret;
}

/******************************************************************************/
focal_def_return_t focal_hal_wait_for_finger_lost(uint32_t limit) {
    focal_def_return_t ret;
    uint32_t i, data;
    LOG_I("Waiting for finger lost.");
    _focal_hal_abort = false;

    finger_lost_detecting = 1;
#if 1
    while(!_focal_hal_abort) {

        if (_focal_hal_should_exit != NULL) {
            if(_focal_hal_should_exit(_focal_hal_user)) {
                break;
            }
        }

        ret = focal_tac_check_finger_lost(&data);
        if (ret == FOCAL_DEF_FINGER_LOST) {
            printf("Finger Lost\n");
            LOG_I("Finger Lost %d", ret);
            finger_lost_detecting = 0;
            return FOCAL_DEF_FINGER_LOST;
        } else if (ret == FOCAL_DEF_ERROR_SENSOR) {
            LOG_I("Error when waiting for finger lost %d", ret);
            usleep(_FOCAL_HAL_WAIT_TIME);
        } else {
            usleep(_FOCAL_HAL_WAIT_TIME);
        }
    }
#else
        ret = focal_tac_check_finger_lost(&data);
        if (ret == FOCAL_DEF_FINGER_LOST) {
            printf("Finger Lost\n");
            LOG_I("Finger Lost %d", ret);
            return FOCAL_DEF_FINGER_LOST;
        } else 
            return FOCAL_DEF_WAIT_TIME;

#endif
    LOG_E("Finger lost %d s limit is reached!", limit);
    finger_lost_detecting = 0;
    return FOCAL_DEF_OK;
}

/******************************************************************************/
focal_def_return_t focal_hal_suspend() {
    focal_def_return_t def_ret;
    LOG_I("%s, Suspend call recieved", __func__);
    
    def_ret = focal_hal_abort();
    if (FOCAL_LIB_OK != def_ret) {
        LOG_E("focal_hal_abort with %d", def_ret);
    }

    _focal_hal_wakeup = 1;
    def_ret = focal_tac_wakeup_setup();
    if (FOCAL_LIB_OK != def_ret) {
        LOG_E("Wake up setup failed with %d", def_ret);
    }
    return def_ret;
}

/******************************************************************************/
void* _focal_hal_irq_thread(void* _pvArg) {
    int irq_type = _focal_hal_irq_type;
    int _focal_hal_state = 0;
    int ret = 0;
    focal_def_return_t def_ret;

    ret = focal_hal_init_irq();
    if (ret != FOCAL_DEF_OK) {
        LOG_E("Init REE irq failed %d", ret);
    }

    pthread_mutex_unlock(&_focal_hal_driver_mutex); 
    pthread_mutex_unlock(&_focal_hal_irq_mutex);
    while (!_focal_hal_exit) {
        /* wait for irq enable*/
        pthread_mutex_lock(&_focal_hal_driver_mutex); 
        LOG_I("Waiting for sem!");
        pthread_cond_wait(&_focal_hal_driver_sem, &_focal_hal_driver_mutex); 
        LOG_I("Got sem for %d!", _focal_hal_irq_type);
        switch(_focal_hal_irq_type) {

            case FOCAL_IRQ_SIGNAL_TEST:
#if 0
                if (_focal_hal_wakeup) {
                    LOG_I("%s, Wakup call recieved", __func__);
                    def_ret = focal_tac_wakeup_qualification();
                    switch (def_ret) {
                        case FOCAL_DEF_FINGER_PRESENT:
                            /* Tell driver to wakeup */
                            _focal_hal_woken = 1;
                            _focal_hal_wakeup = 0; 
                            focal_hal_extension_wakeup_req();
                            break;

                        case FOCAL_DEF_FINGER_LOST:
                            LOG_E("Driver should wait again.");
                            /* Tell driver wait for wake-up */
                            break;

                        default:
                            LOG_E("Wake qualification returns %d", def_ret);
                            /* Error assume no wake-up */
                            break;
                    }
                } else {
#endif
                    _focal_hal_woken = 1;
                    LOG_I("%s, IRQ call recieved", __func__);
                    pthread_mutex_lock(&_focal_hal_irq_mutex);
                    pthread_cond_signal(&_focal_hal_irq_sem);
                    pthread_mutex_unlock(&_focal_hal_irq_mutex);
              //  }
                break;

            case FOCAL_IRQ_SIGNAL_SUSPEND_EARLY_REQ:
            case FOCAL_IRQ_SIGNAL_SUSPEND_REQ:
                LOG_I("%s, Suspend call recieved", __func__);
                def_ret = focal_hal_suspend();
                if (FOCAL_LIB_OK != def_ret) {
                    LOG_E("Wake up setup failed with %d", def_ret);
                } else {
                    /* Tell the driver to wait for wake-up */
                   focal_hal_extension_suspend_ack(_focal_hal_irq_type);
                }
                break;

            case FOCAL_IRQ_SIGNAL_RESUME_LATE_REQ:
            case FOCAL_IRQ_SIGNAL_RESUME_REQ:
                LOG_I("Resume call!");
                _focal_hal_wakeup = 0;
                focal_hal_extension_resume_ack(_focal_hal_irq_type);
                /* Resume */
                break;

            default:
                LOG_E("Error state %d", _focal_hal_irq_type);
                break;
        }
        pthread_mutex_unlock(&_focal_hal_driver_mutex); 
    }
}

//only used by sensor_test
/******************************************************************************/
int focal_hal_finger_woken() {
    if (_focal_hal_woken == 1) {
        _focal_hal_woken = 0;
        return 1;
    } else {
        return _focal_hal_woken;
    }
}

/******************************************************************************/
void irq_callback(int type)
{
    LOG_I("%s start %d", __func__, type);
    if(pthread_mutex_trylock(&_focal_hal_driver_mutex)) {
        LOG_I("%s failed %d", __func__, type);
        return;
    }
    _focal_hal_irq_type = type;
    pthread_cond_signal(&_focal_hal_driver_sem);
    pthread_mutex_unlock(&_focal_hal_driver_mutex); 
    LOG_I("%s %d", __func__, irq_counter);
}

/******************************************************************************/
focal_def_return_t focal_hal_init_irq() {
    int spi_ret;

    LOG_I("Start HAL extension, enter");
    spi_ret = focal_hal_extension_enter(&sensor_env, &irq_callback);
    if (spi_ret != 0) {
        LOG_E("HAL extension enter failed, error %d", spi_ret);
        goto error;
    }

    LOG_I("Start HAL extension, run");
    spi_ret = focal_hal_extension_run();
    if (spi_ret != 0) {
        LOG_E("HAL extension run failed, error %d", spi_ret);
        goto error;
    }

    if (focal_hal_extension_check_running() != 1) {
        LOG_E("Error, HAL extension  not running");
        goto error;
    }

error:

    if (spi_ret != 0) {
        return FOCAL_DEF_ERROR_GENERAL;
    } else {
        return FOCAL_DEF_OK;
    }

}

/******************************************************************************/
focal_def_return_t focal_hal_deinit_irq() {
    int ret;
    ret = focal_hal_extension_exit();
    if (ret != 0) {
        return FOCAL_DEF_ERROR_GENERAL;
    }
    return FOCAL_DEF_OK; 
}

/******************************************************************************/
focal_def_return_t focal_hal_spi_prepare() {
   /* int spi_ret;

    spi_ret = spi_prepare(4, 1E6);
    if (spi_ret != 0) {
        LOG_E("spi_prepare() failed error %d", spi_ret);
        return FOCAL_DEF_ERROR_SENSOR;
        
    }*/
    LOG_I("[focal hal]hal_spi_prepare:nothing to do!");
    return FOCAL_DEF_OK;
}

/******************************************************************************/
focal_def_return_t focal_hal_spi_unprepare() {
   /* int spi_ret;

    spi_ret = spi_unprepare(4, 1E6);
    if (spi_ret != 0) {
        LOG_E("spi_prepare() failed error %d", spi_ret);
        return FOCAL_DEF_ERROR_SENSOR;
        
    }*/
    return FOCAL_DEF_OK;
}

/******************************************************************************/
focal_def_return_t focal_hal_init() {

    focal_def_return_t ret;
    int r;

	LOG_I("[focal hal]hal init!");
#if 1
    r = pthread_attr_init(&_focal_hal_thread_attribute);
    if (r < 0) {
        LOG_E("Failed to set mask");
    }

	r = pthread_attr_setdetachstate(
            &_focal_hal_thread_attribute, PTHREAD_CREATE_JOINABLE);

    if (r < 0) {
        LOG_E("Failed to set mask");
    }

	r = pthread_create(&_focal_hal_main_pthread,
            &_focal_hal_thread_attribute, &_focal_hal_irq_thread, NULL);

    if (r < 0) {
        LOG_E("Failed to set mask");
    }

    ret = focal_hal_spi_prepare();
    if (ret != FOCAL_DEF_OK) {
        LOG_E("Prepare spi %d", ret);
        goto error_init_irq;
    }

#endif
    ret = focal_tac_open();
    if (ret != FOCAL_DEF_OK) {
        LOG_E("tac open failed error %d", ret);
        goto error_open_tac;
    } 
#if 0
    ret = focal_tac_spidrv_open();
    if (ret != FOCAL_DEF_OK) {
        LOG_E("spi_prepare() failed error %d", ret);
        goto error_open_drv;
    } 
#endif
#if 1
    ret = focal_tac_init();
    if (ret != FOCAL_DEF_OK) {
        LOG_E("tac init() failed error %d", ret);
        goto error_init;
    }
#endif	
    return FOCAL_DEF_OK;

error_init:
    if (FOCAL_DEF_OK != ret) {
        focal_tac_deinit();    
    }
//error_init:
#if 0
    if (FOCAL_DEF_OK != ret) {
        focal_tac_spidrv_close(); 
    }
#endif

error_open_drv:
    if (FOCAL_DEF_OK != ret) {
        focal_tac_close();
    }

error_open_tac:

    if (FOCAL_DEF_OK != ret) {
        focal_hal_spi_unprepare();
    }
error_init_irq:
    if (FOCAL_DEF_OK != ret) {
        focal_hal_deinit_irq();
    }

    return ret;
}

/******************************************************************************/
focal_def_return_t focal_hal_abort() {
    _focal_hal_abort = true;
    LOG_I("%s: aborting", __func__);
    pthread_cond_signal(&_focal_hal_irq_sem);
#if 0
    while (finger_present_detecting || finger_lost_detecting) {
        LOG_E("%s: waiting for finger detect or lost to finish %d:%d", __func__, finger_present_detecting, finger_lost_detecting);
       usleep(1000);
    }
#endif
    return FOCAL_DEF_OK;
}

/******************************************************************************/
focal_def_return_t focal_hal_deinit() {
    focal_def_return_t ret;

    LOG_I("Deinit irq");
    focal_hal_deinit_irq();

    LOG_I("Deinit tac");
    focal_tac_deinit();
#if 0
    focal_tac_spidrv_close();
#endif

    LOG_I("Close tac");
    focal_tac_close();

    LOG_I("Deinit spi");
    focal_hal_spi_unprepare();

    LOG_I("Close thread");
    _focal_hal_abort = true;
    _focal_hal_exit = true;
    pthread_cond_signal(&_focal_hal_driver_sem);

    pthread_join(_focal_hal_main_pthread, NULL);
	pthread_attr_destroy(&_focal_hal_thread_attribute);
    LOG_I("Hal deinit done");
}
