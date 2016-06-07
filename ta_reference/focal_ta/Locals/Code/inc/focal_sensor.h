#ifndef __FOCAL_SENSOR_H__
#define __FOCAL_SENSOR_H__

#define RET_OK					0
#define RET_ERROR				1
#define RET_ERROR_CHECKSUM		2
#define RET_ERROR_RW			3
#define RET_ERROR_PARAM			4


#define FP_RING_FINGER_OFF			0
#define FP_RING_FINGER_ON			1
#define FP_RING_FINGER_DETECTING	2
#define FP_RING_FINGER_ON_FIRSTTIME 3


#define FP_RING_EXIT_THRES 			100
#define FP_RING_BASE_REFRESH_THRES 	300

unsigned char GetFpImage(unsigned char *pFpImgData);

unsigned char DetectFinger(void);

int GetFpImageDataFromSRAM(unsigned char * pImage);
#endif/*__FOCAL_SENSOR_H__*/
