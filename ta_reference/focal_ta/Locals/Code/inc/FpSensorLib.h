#ifndef _FP_SENSOR_LIB_H_
#define _FP_SENSOR_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define FOCAL_EVENT_PROMPT					0x1000

/*Move finger more*/
#define FOCAL_EVENT_MOVE_FINGER_MORE		(FOCAL_EVENT_PROMPT | 0x0007)

/*Do not move finger too much*/
#define FOCAL_EVENT_MOVE_FINGER_LESS		(FOCAL_EVENT_PROMPT | 0x0008)


/*
 * get fp image
 * @param[out] - where to store fp image buffer
 *
 * @return 0 - ok
 * 		other - error
 */
unsigned char focal_GetFpImage(unsigned char *pFpImage);

/*
 * detect finger
 * @return 0 - finger on
 * 		other - finger off
 */
unsigned char focal_DetectFinger(void);


/*
 * get focal lib version
 * @param1[out] - version 20bytes
 * 
 *
 * @return - none
 */
void focal_GetLibVersion(unsigned char *pstrBuf);

/*
 * set fp sensor col and row
 * @param1[in] - fp sensor col
 * @param2[in] = fp sensor row
 *
 * @return - none
 */
void focal_SetFpSensorColsRows(unsigned char cols, unsigned char rows);
/*
 * write sfr register
 * @param 1[in]: write data 
 * @param 2[in]: data length
 *
 * @return 0:OK
 * 		   other: error code
*/
typedef int (*FtFpSpiWriteFunc)(unsigned char *, unsigned int);

/*
 * read sfr register
 * @param 1[in]: write data
 * @param 2[out]: where to store data read from fp sensor sram
 * @param 3[in]: data lenght
 *
 * @return 0:OK
 * 		   other: error code
*/
typedef int (*FtFpSpiReadFunc)(unsigned char *, unsigned char *, unsigned int);

/*
 * usleep
 * @param 1[in]: time of microsecond
 *
 * @return none
 */
typedef void (*FtFpUsleepFunc)(unsigned int);

/*the function list as follow must be called at first*/

/*init write sfr register function point*/
void focal_InitFuncSpiWrite(FtFpSpiWriteFunc func);

/*init read sfr register function point*/
void focal_InitFuncSpiRead(FtFpSpiReadFunc func);

/*init usleep function*/
void focal_InitFuncUsleep(FtFpUsleepFunc func);


/* extern function interface*/

/*
 * set enroll max template number. default value is 8
 * @param [in] - max template
 *
 * @return - none
 */
void focal_SetEnrollMaxTpl(int maxtpl);

/*
 * set alg max template number. default value is 32
 * @param [in] - max alg template number
 *
 * @return - none
 */
void focal_SetFpAlgMaxTpl(int maxtpl);


/*
 * init alg resource
 *
 * @return 0 - OK
 * 		   -1 - Error
 */
int focal_InitFpAlg(void);

/*
 * get event during enroll
 * @param[out] - enroll event
 */
void focal_GetEnrollingEvent(unsigned short *pEnrollEvent);


/*
 * init alg template
 * @param[in] - finger id
 * @param[in] - template type
 * @param[in] - template size
 * @param[in] - template data
 *
 * @return 0 - OK
 * 		   -1 - Error
 */
int focal_InitAlgTplData(unsigned short fingerId, unsigned char tpl_type,
			unsigned int tpl_size, unsigned char * tpl_data);

/*
 * alg save template and return template to host
 * @param[in] - finger id
 * @param[out] - template type
 * @param[out] - template size
 * @param[out] - template data
 *
 * @return 0 - OK
 * 		   -1 - Error
 */
int focal_SaveAlgTplData(unsigned short fingerId, unsigned char *tpl_type,
			unsigned int *tpl_size, unsigned char * tpl_data);


/*
 * host get template, generally used to update the template.
 * @param[in] - finger id
 * @param[out] - template type
 * @param[out] - template size
 * @param[out] - template data
 *
 * @return 0 - OK
 * 		   -1 - Error
 */
int focal_GetAlgTplData(unsigned short fingerId, unsigned char *tpl_type,
			unsigned int *tpl_size, unsigned char * tpl_data);


/*
 * finger enroll
 * @param[in] - fingerId
 * @param[in] - index of enroll
 * @param[out] - finger coverage during enroll
 *
 * @return 0 - OK
 * 		   other - Error
 */
int focal_Enroll(unsigned short fingerId, unsigned char enroll_index,
			unsigned char *penroll_coverage);


/*
 * cancel enroll this time
 *
 * @return 0 - ok
 * 		   -1 - Error
 */
int focal_CancelEnroll(void);

/*
 * finger identify
 * @param[out] - match of finger id
 * @param[out] - update tpl or no
 *
 * @return 0 - OK
 * 		   other - Error
 */
int focal_Identify(unsigned char *pfingerId, unsigned char *pupdateTpl);

#ifdef __cplusplus
}
#endif


#endif

