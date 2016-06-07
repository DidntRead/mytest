/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

/**
 * @file   tlDriverApi.c
 * @brief  Implements driver APIs
 *
 * The APIs allow trustlets to make requests to the driver
 *
 */

//#include "tlDriverApi.h"
#include "drfocalApi.h"
#include "drApiMarshal.h"


#define DRFOCALMSG(fmt, args...) tlDbgPrintf("[DRAPIFOCAL]"fmt, ##args)
//#define DRFOCALMSG(fmt, args...) tlApiLogPrintf("[DRAPIFOCAL]"fmt, ##args)

/* Sends a MSG_RQ message via IPC to a MobiCore driver.
 *
 * @param driverID The driver to send the IPC to.
 * @param payload MPointer to marshaling parameters to send to the driver.
 * @param payloadSize the size of the payload that will be sent to the driver
 *
 * @return TLAPI_OK
 * @return E_TLAPI_COM_ERROR in case of an IPC error.
 */
_TLAPI_EXTERN_C tlApiResult_t tlApi_callDriverEx(
        uint32_t driver_ID,
        void* payload,
        uint32_t payloadSize);

_TLAPI_EXTERN_C tlApiResult_t tlApi_callDriver(
        uint32_t driver_ID,
        void* pMarParam);

/**
 * Open session to the driver with given data
 *
 * @return  session id
 */
_TLAPI_EXTERN_C uint32_t tlfocalApiOpenSession()
{
    uint32_t sid = DR_SID_INVALID;
	tlApiResult_t tlret;

	drMarshalingParamTemplate_t marParam = {
			.functionId = FID_DR_OPEN_SESSION,
	};

	tlret = tlApi_callDriver(FOCAL_DR_ID, &marParam);

	if (TLAPI_OK == tlret)
	{
	    /* Retrieve session id provided by the driver */
	    sid = marParam.sid;
	} else DRFOCALMSG("tlfocalApiOpenSession: ret %x\n",tlret);

	return sid;
}

/**
 * Close session
 *
 * @param sid  session id
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C tlApiResult_t tlfocalApiCloseSession(
        uint32_t sid)
{
    drMarshalingParamTemplate_t marParam = {
            .functionId = FID_DR_CLOSE_SESSION,
            .sid = sid
    };

    tlApiResult_t ret = tlApi_callDriver(FOCAL_DR_ID, (void*)&marParam);

    return ret;
}

/**
 * Executes command
 *
 * @param sid        session id
 * @param commandId  command id
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C tlApiResult_t tlfocalApiExecute(
        uint32_t sid,
        struct tlfocalApiDevice *focaldev)
{
    drMarshalingParamTemplate_t marParam = {
            .functionId = FID_DR_EXECUTE,
            .sid        = sid,
            .payload    = {
                    .devData = focaldev
            }
    };

    tlApiResult_t ret = tlApi_callDriver(FOCAL_DR_ID, (void*)&marParam);
    return ret;
}

_TLAPI_EXTERN_C int drfp_setPassId(uint32_t passid)
{
	int iRet = 0;
	struct tlfocalApiDevice focaldev;
	uint32_t  crSession;
	tlApiResult_t tlRet = TLAPI_OK;
	int i =0;

	DRFOCALMSG("===>drfp_setPassId: last pass id: %d\n",passid);
	focaldev.cmd = FOCAL_SET_LASTPASSID;
	focaldev.lastPassid = passid;

	do
    {
        crSession = tlfocalApiOpenSession();
		tlRet = tlfocalApiExecute(crSession, &focaldev);
		tlfocalApiCloseSession(crSession);

		break;
    }while(false);

	return iRet;
}

_TLAPI_EXTERN_C int drfp_setids(struct tlfocalApiDevice *focaldev)
{
	int iRet = 0;
	uint32_t  crSession;
	tlApiResult_t tlRet = TLAPI_OK;
	int i =0;

	DRFOCALMSG("===>drfp_setids. focaldev->idsCount: %d\n",focaldev->idsCount);
	focaldev->cmd = FOCAL_SET_IDS;

	for (i=0;i<focaldev->idsCount;i++) DRFOCALMSG("===>drfp_setids: ids[%d]: %d\n",i,focaldev->ids[i]);

	do
    {
        crSession = tlfocalApiOpenSession();
		tlRet = tlfocalApiExecute(crSession, focaldev);
		tlfocalApiCloseSession(crSession);

		break;
    }while(false);

	return iRet;
}

_TLAPI_EXTERN_C int drfp_updateTpl(unsigned int tid, char isadd)
{
	int iRet = 0;
	struct tlfocalApiDevice focaldev;
	uint32_t  crSession;
	tlApiResult_t  tlRet = TLAPI_OK;

	focaldev.cmd = isadd ? FOCAL_ADD_TPL : FOCAL_DEL_TPL;
	focaldev.index = tid;

	DRFOCALMSG("===>drfp_updateTpl: tid[%d]\n",tid);
	 do
    {
        crSession = tlfocalApiOpenSession();
		tlRet = tlfocalApiExecute(crSession, &focaldev);
		tlfocalApiCloseSession(crSession);

        //DRFOCALMSG("===>drfp_setIdName: authenticator %x\n",focaldev.authenticatorVersion);

		break;
    }while(false);

	return iRet;
}

_TLAPI_EXTERN_C int drfp_setIdName(unsigned int index, struct tlfocalApiDevice *focaldev)
{
	int iRet = 0;
	//struct tlfocalApiDevice focaldev;
	uint32_t  crSession;
	tlApiResult_t  tlRet = TLAPI_OK;

	focaldev->cmd = FOCAL_SET_IDNAME;
	focaldev->index = index;

	DRFOCALMSG("===>drfp_setIdName: index[%d] name %s\n",index,focaldev->focalTpl->name);
	 do
    {
        crSession = tlfocalApiOpenSession();
		tlRet = tlfocalApiExecute(crSession, focaldev);
		tlfocalApiCloseSession(crSession);

        //DRFOCALMSG("===>drfp_setIdName: authenticator %x\n",focaldev.authenticatorVersion);

		break;
    }while(false);

	return iRet;
}

_TLAPI_EXTERN_C int drfp_setversion(unsigned int ver)
{
	int iRet = 0;
	struct tlfocalApiDevice focaldev;
	uint32_t  crSession;
	tlApiResult_t  tlRet = TLAPI_OK;

	focaldev.version = ver;
	focaldev.cmd = FOCAL_SET_VERSION;

	focaldev.authenticatorVersion = 0x100;

	DRFOCALMSG("===>drfp_setversion: ver %x\n",ver);

	 do
    {
        crSession = tlfocalApiOpenSession();
		tlRet = tlfocalApiExecute(crSession, &focaldev);
		tlfocalApiCloseSession(crSession);

        DRFOCALMSG("===>drfp_setversion: authenticator %x\n",focaldev.authenticatorVersion);

		break;
    }while(false);

	return iRet;
}



_TLAPI_EXTERN_C tlApiResult_t drfocal_init(void)
{
    uint32_t  crSession;
    tlApiResult_t  tlRet = TLAPI_OK;
    struct tlfocalApiDevice focaldev;

    DRFOCALMSG("drfocal_init in SWd\n");
    focaldev.cmd = FOCAL_INIT_DATA;

    do
    {
        crSession = tlfocalApiOpenSession();
		tlRet = tlfocalApiExecute(crSession, &focaldev);
		tlfocalApiCloseSession(crSession);

		break;
    }while(false);

    return tlRet;
}

_TLAPI_EXTERN_C int alipay_sec_fp_get_ids(int* ids, int* idsCount)
{
	int iRet = 0;
	int i;

	struct tlfocalApiDevice focaldev;
	uint32_t  crSession;
	tlApiResult_t  tlRet = TLAPI_OK;

	if (ids == NULL || idsCount == NULL) return E_TLAPI_INVALID_PARAMETER;
	DRFOCALMSG("==>alipay_sec_fp_get_ids\n");

	focaldev.cmd = ALI_GET_IDS;
	focaldev.ids = ids;

	 do
    {
        crSession = tlfocalApiOpenSession();

		tlRet = tlfocalApiExecute(crSession, &focaldev); /*this will trigger SpiIpc thead*/

        tlfocalApiCloseSession(crSession);

		break;
    }while(false);

    DRFOCALMSG("alipay_sec_fp_get_ids: idsCount: %d\n",focaldev.idsCount);

    if (focaldev.idsCount <= MAX_ID_LIST_SIZE && focaldev.ids != NULL) {
		*idsCount = focaldev.idsCount;
		for(i = 0; i< *idsCount; i++) DRFOCALMSG("alipay_sec_fp_get_ids: ids[%d] == %d\n",i, ids[i]);
		//memcpy(ids,focaldev.ids,sizeof(uint32_t)*5);
	}
	return iRet;
}

_TLAPI_EXTERN_C int alipay_sec_fp_getname(int id, char *name, int *length)
{
	int iRet = 0;
	int i;
	struct tlfocalApiDevice focaldev;
	struct tlfocalTpl focaltpl;
	uint32_t  crSession;
	tlApiResult_t  tlRet = TLAPI_OK;

	if (name == NULL || length == NULL) return E_TLAPI_INVALID_PARAMETER;
	DRFOCALMSG("==>alipay_sec_fp_getname\n");

	focaldev.cmd = ALI_GET_NAME;
	focaldev.focalTpl = &focaltpl;
	focaldev.focalTpl->name = name;
	focaldev.focalTpl->id = id;

	 do
    {
        crSession = tlfocalApiOpenSession();

		tlRet = tlfocalApiExecute(crSession, &focaldev); /*this will trigger SpiIpc thead*/

        tlfocalApiCloseSession(crSession);

		break;
    }while(false);

    if (name != NULL) {
    
		DRFOCALMSG("alipay_sec_fp_getname: name %s\n",name);
		*length = strlen(name);
	}
	DRFOCALMSG("<==alipay_sec_fp_getname\n");
	return iRet;
}

_TLAPI_EXTERN_C int alipay_sec_fp_get_identify_result(int *id)
{
	int iRet = 0;
	struct tlfocalApiDevice focaldev;
	uint32_t  crSession;
	tlApiResult_t  tlRet = TLAPI_OK;

	if (id == NULL) return E_TLAPI_INVALID_PARAMETER;

	focaldev.cmd = ALI_GET_IDENTIFY_RESULT;

	 do
    {
        crSession = tlfocalApiOpenSession();

		tlRet = tlfocalApiExecute(crSession, &focaldev); /*this will trigger SpiIpc thead*/

        tlfocalApiCloseSession(crSession);

		break;
    }while(false);

    *id = focaldev.lastPassid;

    DRFOCALMSG("<==alipay_sec_fp_get_identify_result. id:%x\n",*id);

	return iRet;
}

_TLAPI_EXTERN_C int alipay_sec_fp_getversion(unsigned int* ver)
{
	int iRet = 0;

	struct tlfocalApiDevice focaldev;
	uint32_t  crSession;
	tlApiResult_t  tlRet = TLAPI_OK;

	if (ver == NULL) return E_TLAPI_INVALID_PARAMETER;

	focaldev.cmd = ALI_GET_VERSION;

	 do
    {
        crSession = tlfocalApiOpenSession();

		tlRet = tlfocalApiExecute(crSession, &focaldev); /*this will trigger SpiIpc thead*/

        tlfocalApiCloseSession(crSession);

		break;
    }while(false);

    *ver = focaldev.version;

    DRFOCALMSG("<==alipay_sec_fp_getversion. ver:%x\n",*ver);
	return iRet;
}
