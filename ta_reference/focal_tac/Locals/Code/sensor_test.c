#include <stdint.h>
#include <stdlib.h>

#include "focal_tac.h"
#include "focal_def.h"
#include "focal_lib.h"
#include "focal_hal.h"
#include "tee_type.h"
#define LOG_TAG "sensor_test"
#include "log.h"

#define _WAITING_FOR_FINGER_DETECT  0x00
#define _START_FINGER_DETECT        0x01
#define _WAITING_FOR_FINGER_PRESENT 0x02
#define _WAITING_FOR_FINGER_LOST    0x03

#define FINGER_ON       1
#define FINGER_LOST      0    

static void returnExitCode(int exitCode);
focal_def_return_t store_image();

uint16_t g_sensor_width = 88;
uint16_t g_sensor_height = 88;
/******************************************************************************/
focal_def_return_t enroll_finger(int limit) {
    focal_lib_enroll_data_t enroll_data;
    focal_lib_extended_enroll_t ext_data;
    int ret;

    memset(&enroll_data, 0, sizeof(enroll_data));
    memset(&ext_data, 0, sizeof(ext_data));
    int images = 0;

    while ((images < limit) && (100 != enroll_data.progress)) {
        if (focal_hal_wait_for_finger_present(60) != FOCAL_DEF_FINGER_PRESENT) {
           return FOCAL_DEF_ERROR_GENERAL;
        }

        LOG_I("Capture image.");
        ret = focal_tac_capture_image();
        if (ret != 0) {
            LOG_I("Image capture error %d", ret);
            return FOCAL_DEF_ERROR_GENERAL;
        }

        store_image();
        LOG_I("Image capture");
        printf("Image captured\n");

        ret = focal_tac_enroll(&enroll_data);
        if (ret != 0) {
            LOG_I("Enroll error %d", ret);
            return FOCAL_DEF_ERROR_GENERAL;
        }
        printf("Enroll Image %d %d %d %d %d %d %d\n",
                enroll_data.progress,
                enroll_data.quality,
                enroll_data.result,
                enroll_data.nr_successful,
                enroll_data.nr_failed,
                enroll_data.enrolled_template_size,
                enroll_data.extended_enroll_size);


        LOG_I("Enroll Image %d %d %d %d %d %d %d",
                enroll_data.progress,
                enroll_data.quality,
                enroll_data.result,
                enroll_data.nr_successful,
                enroll_data.nr_failed,
                enroll_data.enrolled_template_size,
                enroll_data.extended_enroll_size);

   /*     if (enroll_data.extended_enroll_size != 0) {
            ext_data.data = (uint8_t*) malloc(enroll_data.extended_enroll_size);
            ext_data.size = enroll_data.extended_enroll_size;
            ret = focal_tac_get_extended_enroll(&ext_data);
        }*/

        images++; 
        if (focal_hal_wait_for_finger_lost(60) != FOCAL_DEF_FINGER_LOST) {
            return FOCAL_DEF_ERROR_GENERAL;
        }
    }

    return (100 == enroll_data.progress) ? FOCAL_DEF_OK : FOCAL_DEF_ERROR_GENERAL;
}

/******************************************************************************/
focal_def_return_t identify(int attempts) {
    focal_def_return_t ret;
    focal_lib_identify_data_t identify_data;
    uint32_t *ids = NULL;
    uint32_t i, id_count, event;
    uint32_t error;
    error = 0;

    id_count = 5;

    ids = (uint32_t*) malloc(5 * sizeof(uint32_t));
    for (i = 0; i < 5; i++) {
        ids[i] = 255;
    }

    LOG_I("Start get ids");
    ret = focal_tac_get_ids(ids, &id_count);
    if (ret != FOCAL_DEF_OK || id_count == 0) {
        LOG_E("Get ids fail");
        return FOCAL_DEF_ERROR_GENERAL;
    }

    LOG_I("Start identify");
    ret = focal_tac_begin_identify(ids, 1);
    if (ret != FOCAL_DEF_OK) {
        LOG_I("Begin identify error %d", ret);
        return FOCAL_DEF_ERROR_GENERAL;
    }

    for (i = 0; i < attempts;) {

        if (focal_hal_wait_for_finger_lost(10) != FOCAL_DEF_FINGER_LOST) {
            return FOCAL_DEF_ERROR_GENERAL;
        }

        if (focal_hal_wait_for_finger_present(10) != FOCAL_DEF_FINGER_PRESENT) {
            LOG_E("Finger present error %d", ret);
            return FOCAL_DEF_ERROR_GENERAL;
        }

        LOG_I("Capture image.");
        ret = focal_tac_capture_image();
        if (ret != 0) {
            LOG_I("Image capture error %d error count %d", ret, error);
            if (error > 10) {
                return FOCAL_DEF_ERROR_GENERAL;
            }
            ++error;
            continue;
        }

        error = 0;

        ret = focal_tac_identify(&identify_data);
        if (ret != 0) {
            LOG_I("Identify error %d", ret);
            return FOCAL_DEF_ERROR_GENERAL;
        }

        i++;
        LOG_I("Identify Attempt %d Image %d %d %d",
                i,
                identify_data.result,
                identify_data.score,
                identify_data.index);

            }

    return FOCAL_DEF_OK;
}

/******************************************************************************/
focal_def_return_t store_image() {
    focal_def_return_t ret;
    int write_result;
    FILE* file = NULL;
    uint8_t* image;
    uint32_t image_size = 208 * 80;

    image = (uint8_t*) malloc(image_size);
    if (NULL == image) {
        ret = FOCAL_DEF_ERROR_MEM;
        goto error;
    }

    file = fopen("/data/image.raw", "w");
    if (NULL == file) {
        LOG_E("Error: Cannot open file.\n");
        ret = FOCAL_DEF_ERROR_MEM;
        goto error;
    }

    focal_tac_debug_retrieve_image(image, &image_size);

    write_result = fwrite(image, 1, image_size, file);

    if (write_result != image_size) {
        LOG_E("Entire file not written %d bytes instead of %d "
                "bytes err: %d", write_result, image_size, ferror(file));

        ret = FOCAL_DEF_ERROR_SIZE;
        goto error;
    }
    
error:
    if (file != NULL) {
        fclose(file);
        file = NULL;
    }

    LOG_I("Filesize %d", image_size);
    if (image != NULL) {
        free(image);
        file = NULL;
    }

    return ret;
}

static void save_picture(unsigned char *image_buf)
{	
	FILE *fp = NULL;
	unsigned char temp_head[] =
	{	
		0x42,0x4d,//file type 
		0x0,0x0,0x0,0x00, //file size***
		0x00,0x00, //reserved
		0x00,0x00,//reserved
		0x36,0x4,0x00,0x00,//head byte***
		//infoheader
		0x28,0x00,0x00,0x00,//struct size
		
		//0x00,0x01,0x00,0x00,//map width*** 
		0x00,0x00,0x0,0x00,//map width*** 
		//0x68,0x01,0x00,0x00,//map height***
		0x00,0x00,0x00,0x00,//map height***
		
		0x01,0x00,//must be 1
		0x08,0x00,//color count
		0x00,0x00,0x00,0x00, //compression
		//0x00,0x68,0x01,0x00,//data size***
		0x00,0x00,0x00,0x00,//data size***
		0x00,0x00,0x00,0x00, //dpix
		0x00,0x00,0x00,0x00, //dpiy
		0x00,0x00,0x00,0x00,//color used
		0x00,0x00,0x00,0x00,//color important
	};
	unsigned char head[1078] = {0};
	unsigned char newbmp[1078 + g_sensor_width * g_sensor_height];
	
	memcpy(head, temp_head, sizeof(temp_head));

	int i,j;
	long long num;
	
	fp = fopen("/data/ft_1.bmp", "wb+");

	num = g_sensor_width;
	head[18] = (unsigned char) (num & 0xFF);
	num = num>>8;  
	head[19] = (unsigned char) (num & 0xFF);
	num = num>>8;  
	head[20] = (unsigned char) (num & 0xFF);
	num = num>>8;  
	head[21] = (unsigned char) (num & 0xFF);

	num = g_sensor_height; 
	head[22] = (unsigned char) (num & 0xFF);
	num = num>>8;  
	head[23] = (unsigned char) (num & 0xFF);
	num = num>>8;  
	head[24] = (unsigned char) (num & 0xFF);
	num = num>>8; 
	head[25] = (unsigned char) (num & 0xFF);

	j = 0;
	for (i = 54; i < 1078; i = i + 4)
	{
		head[i] = head[i+1] = head[i+2] = (unsigned char) j; 
		head[i+3] = 0;
		j++;
	}

	memcpy(newbmp, head, sizeof(head));
	memcpy(newbmp + 1078, image_buf, g_sensor_width * g_sensor_height);

	fwrite(newbmp, 1078 + g_sensor_width * g_sensor_height, 1, fp);

	fclose(fp);
}

/******************************************************************************/
int main(int argc, char *args[]) {
    focal_def_return_t ret;
    focal_lib_identify_result_t identity_ret;
    int spi_ret;
    int i = 0;
    int ret1 = 0;
    uint32_t data;
    static int count = 0;
    int counter;
    int fingerstate = FINGER_LOST;
    int nDeadPixels=100;
    bool finger_should_up = false;
    bool match_finger = false;
    bool enroll_end = false;
    bool enroll_success = false;
    unsigned char buf[88 * 88] = {0};
    focal_lib_enroll_data_t enroll_data;
    focal_lib_identify_data_t identity_data;
    printf("Sensor Test %s %s\n", __DATE__, __TIME__);

    printf("[focal test]Start init & test\n");
    ret = focal_hal_init();
    if (FOCAL_DEF_OK != ret) {
        goto error;
    }

    //ret1 = focal_ft_lib_init();
    //printf("[focal test]Start dead pixel test\n");
#if 1
    ret1 = focal_ft_lib_init();
#if 0
    for (i = 0; i < 2; i++)
    {
        get_data_from_file(&enroll_data, i);    
    }
#endif
    enroll_data.finger_id = 0;
    enroll_data.finger_index = 0;

    printf("init ret = %d\n", ret1);
    while(1)
    {
        if (fingerstate == FINGER_LOST)
        {
            if (focal_ft_detectfinger() != 0)
            {
                count++;
                //printf("finger lost!\n");
                finger_should_up = false;
                fingerstate = FINGER_LOST;
                continue;
            }
        }
        if (finger_should_up == true)
            continue;
        fingerstate = FINGER_ON;
        if (enroll_data.finger_index != 8 && match_finger == false)
        {
            printf("finger on and enroll finger!\n");
            focal_ft_enroll_finger(&enroll_data);
            enroll_data.finger_index++;
            printf("_____id = %d  index + 1 = %d\n", enroll_data.finger_id, enroll_data.finger_index);
            finger_should_up = true;
            fingerstate = FINGER_LOST;
            if (enroll_data.finger_index == 8)
            {
                enroll_end = true;
                //if (enroll_data.finger_id == 1)
                //{
                    enroll_success = true;
                //}
                //else
                //{
                    //enroll_data.finger_index = 0;
                //}
            }
        }
        if (enroll_end == true)
        {
            printf("__enroll end! \n");
            focal_ft_end_enroll(&enroll_data);
            printf("type = %d size = %d\n", enroll_data.type, enroll_data.size);
            //focal_save_data_to_file(&enroll_data);//save data to file

            enroll_end = false;
            //if (enroll_data.finger_id  == 1 && enroll_success == true)
                match_finger = true;
            finger_should_up = true;
            //enroll_data.finger_id += 1;
            continue;
        }
        //match 
        if (match_finger == true)
        {
            printf("search finger!\n");
            identity_ret = focal_ft_match_finger(&identity_data);
            if (identity_ret == FOCAL_LIB_IDENTIFY_MATCH )
                printf("match id = %d updata = %d\n", identity_data.index, identity_data.score);
            else
                printf("finger don't match!\n");
            finger_should_up = true;
            fingerstate = FINGER_LOST;
        }
    }
#else   //add for test
    while(1)
    {
        if (focal_ft_detectfinger() != 0)
        {
            printf("finger lost!\n");
            continue;
        }
        break;
    }
    focal_ft_get_image_buf(buf);
    for (i = 0; i < 15; i++)
    {
	   printf("___1_%d__\n", buf[i]);
    }

    save_picture(buf);
    printf("save picture!!!!\n");
#endif    
    
#if 0
    ret = focal_tac_deadpixel_test(&nDeadPixels); 
    if (FOCAL_DEF_OK != ret) {
        printf("Deadpixel test failed,found %d dead pixels \n", nDeadPixels);
    }

    printf("Passed Deadpixel test,found %d dead pixels \n", nDeadPixels);
#endif

#if 0

    counter = 0;
    while (counter < 10) {
        printf("Waiting for stuff!\n");
        counter++;
        sleep(1);
        if(focal_hal_finger_woken() == 1) {
            printf("Woke me up!\n"); 
            break;
        }
    }
#endif
#if 0
    for (counter = 0; counter < 10; counter++) {
        printf("Present finger.\n");
        ret = focal_hal_wakeup(); 
        if (ret != FOCAL_DEF_FINGER_PRESENT) {
            printf("Wakup failed %d.\n", ret);
            goto error;
        }

        printf("Capture image.\n");
        ret = focal_tac_capture_image();
        if (ret != 0) {
            LOG_I("Image capture error %d", ret);
            goto error;
        }

        printf("Wait for finger lost.\n");
        if (focal_hal_wait_for_finger_lost(60) != FOCAL_DEF_FINGER_LOST) {
            ret = FOCAL_DEF_ERROR_GENERAL;
            goto error;
        } 
    }

    ret = focal_tac_begin_enroll();
    if (ret != FOCAL_DEF_OK) {
        printf("Start enroll failed \n");
    }

    ret = enroll_finger(1000);
    if (ret != FOCAL_DEF_OK) {
        printf("Enroll failed \n");
    }
    
    ret = focal_tac_end_enroll(&data);
    if (ret != FOCAL_DEF_OK) {
        printf("End enroll failed \n");
    }

    ret = identify(10);
    if (ret != FOCAL_DEF_OK) {
        printf("Identify failed \n");
    }        
#endif
    
error:
    focal_hal_deinit();

    returnExitCode(ret);

    return 0;
}

/******************************************************************************/
static void returnExitCode(int exitCode) {
    if (0 != exitCode) {
        __android_log_write(ANDROID_LOG_ERROR, LOG_TAG, "Failure");
    } else {
        __android_log_write(ANDROID_LOG_INFO, LOG_TAG, "Success");
    }
    fprintf(stderr, "TLC exit code: %08x\n", exitCode);
    exit(exitCode);
}

