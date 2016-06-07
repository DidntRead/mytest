/*
 * Copyright (c) 2013-2014 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

/**
 * @file   drIpcHandler.c
 * @brief  Implements IPC handler of the driver.
 *
 */
#include "drStd.h"
#include "DrApi/DrApi.h"

#include "drApiMarshal.h"
#include "drCommon.h"
#include "drSmgmt.h"
#include "drTemplate_Api.h"
#include "drfocal_Ali.h"
#include "drFocalExec.h"

DECLARE_STACK(drIpchStack, 2048);

static struct tlAliApiDevice aliData;
/**
 * IPC handler loop. this is the function where IPC messages are handled
 */
_NORETURN void drIpchLoop( void )
{
    /* Set IPC parameters for initial MSG_RD to IPCH */
    threadid_t     ipcClient = NILTHREAD;
    message_t      ipcMsg    = MSG_RD;
    uint32_t       ipcData   = 0;
    uint32_t       ipcMsgId  = 0;
    tlApiResult_t  tlRet     = E_TLAPI_DRV_UNKNOWN;
    drApiResult_t   drRet     = DRAPI_OK;
    drMarshalingParamTemplate_ptr pMarshal;

    struct tlfocalApiDevice *focalData=NULL;
    struct tlfocalApiDevice size_focalData;
    struct tlfocalApiDevice _focalData;
    struct tlfocalApiDevice *fdp = &_focalData;
    struct tlfocalTpl _focalTpl;
    struct tlfocalTpl *ftpl = &_focalTpl;
	struct tlfocalTpl size_focalTpl;
	char tplName[MAX_ID_NAME_SIZE];
	char *tplName_p = tplName;
	uint32_t ids[MAX_ID_LIST_SIZE], i;
	uint32_t *ids_t = &ids;

    for (;;)
    {
        /*
         * When called first time sends ready message to IPC server and
         * then waits for IPC requests
         */
        if (DRAPI_OK != drApiIpcCallToIPCH(&ipcClient, &ipcMsg, &ipcData))
        {
            drDbgPrintLnf("[Driver DrTemplate] drIpchLoop(): drApiIpcCallToIPCH failed");
            continue;
        }

        drApiLogPrintf("[DRFOCAL] Received command from trustlet (MSG_RQ_EX, API LEVEL:%d)\n", TBASE_API_LEVEL);
        ipcMsgId=drApiExtractMsgCmd(ipcMsg);

        /* Dispatch request */
        switch (ipcMsgId)
        {
            case MSG_CLOSE_TRUSTLET:
                /* Close active sessions owned by trustlet, which is being shutdown */
                //For MSG_CLOSE_TRUSTLET message threadId is in ipcData parameter
                drSmgmtCloseSessionForThread(ipcData);

                ipcMsg = MSG_CLOSE_TRUSTLET_ACK;
                ipcData = 0;
                break;
            case MSG_CLOSE_DRIVER:
                /* Acknowledge */
                ipcMsg = MSG_CLOSE_DRIVER_ACK;
                ipcData = 0;
                break;
            case MSG_RQ:
           case MSG_RQ_EX:

//            #ifdef DR_FEATURE_TL_API
                /**
                 * Handle incoming IPC requests via TL API.
                 * Map the caller trustlet to access to the marshaling data
                 */
                 drApiLogPrintf("[DRFOCAL] ===>drApiMapClientAndParams\n");

				drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
                                           (addr_t)ipcData,
                                           sizeof(drMarshalingParamTemplate_ptr),
                                           MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
                                           (void**)&pMarshal);
                if(drRet != DRAPI_OK) {
					tlRet = E_DRAPI_CANNOT_MAP;
                    drApiLogPrintf("[DRFOCAL] drIpchLoop(): map task buffer failed");
					break;
                }

                if (pMarshal)
                {
                    /* Process the request */
                    switch (pMarshal->functionId)
                    {
                    case FID_DR_OPEN_SESSION:
                        /**
                         * Handle open session request
                         */
                        pMarshal->sid = drSmgmtOpenSession(ipcClient);

                        /**
                         * Update IPC status as success. If there is an error with opening session
                         * invalid sid will indicatethat opening session failed
                         */
                        tlRet = TLAPI_OK;
                        break;
                    case FID_DR_CLOSE_SESSION:
                        /**
                         * Handle close session request based on the sid provided by the client
                         */
                        drSmgmtCloseSession(pMarshal->sid);
                        tlRet = TLAPI_OK;
                        break;
                    case FID_DR_INIT_DATA:
                        /**
                         * Initialize sesion data
                         */
                        if (E_DR_SMGMT_OK == drSmgmtSetSessionData(
                                pMarshal->sid,
                                ipcClient,
                                &(pMarshal->payload.devData)))
                        {
                            tlRet = TLAPI_OK;
                        }
                        break;
                    case FID_DR_EXECUTE:
                        /**
                         * TODO: Read registry data compare threadids to make sure that client is allowed to use
                         * registry data. Then execute the command and update tlRet accordingly
                         */
                         drApiLogPrintf("[DRFOCAL] ===>FID_DR_EXECUTE\n");

                        drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
										           (addr_t)pMarshal->payload.devData,
												    sizeof(size_focalData),
													MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
													(void**)&focalData);
					    if(drRet != DRAPI_OK) {
							    drApiLogPrintf("[DRFOCAL] FID_DR_EXECUTE: map spiData task buffer failed\n");
                                tlRet = E_DRAPI_CANNOT_MAP;
							    break;
					    } 

                         switch(focalData->cmd) {
							 case FOCAL_INIT_DATA:
								drApiLogPrintf("[DRFOCAL] FOCAL_INIT_DATA\n");
								drFocal_init(&aliData);
								break;

							case FOCAL_ADD_TPL:
								drApiLogPrintf("[DRFOCAL] ===>FOCAL_ADD_TPL.\n");
								drApiLogPrintf("[DRFOCAL] <===FOCAL_ADD_TPL.\n");
								break;

							case FOCAL_DEL_TPL:
								drApiLogPrintf("[DRFOCAL] ===>FOCAL_DEL_TPL.\n");
								drFocal_del_tpl(focalData->index,&aliData);

								drApiLogPrintf("[DRFOCAL] <===FOCAL_DEL_TPL.");
								break;

							case FOCAL_SET_LASTPASSID:
								drApiLogPrintf("[DRFOCAL] ===>FOCAL_SET_LASTPASSID.\n");
								drRet = drFocal_set_lastpassID(focalData, &aliData);
								if (drRet != DRAPI_OK) {
									drApiLogPrintf("[DRFOCAL] get timestamp fail\n");
									tlRet = E_DRAPI_CANNOT_MAP;
									break;
								}
								break;

							case FOCAL_SET_IDS:

								drApiLogPrintf("[DRFOCAL] ===>FOCAL_SET_IDS. idsCount: %d\n",focalData->idsCount);
								drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
										           (addr_t)focalData->ids,
												    (sizeof(uint32_t)*MAX_ID_LIST_SIZE),
													MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
													(void**)&(ids_t));
								if(drRet != DRAPI_OK) {
									drApiLogPrintf("[DRFOCAL] ==FOCAL_SET_IDS==: map task buffer failed\n");
									tlRet = E_DRAPI_CANNOT_MAP;
									break;
								}
								aliData.idsCount = focalData->idsCount;
								memcpy(aliData.ids,ids_t,sizeof(uint32_t)*MAX_ID_LIST_SIZE);
								for (i=0;i<focalData->idsCount;i++) drApiLogPrintf("[DRFOCAL]: ids[%d]==%d\n",i,aliData.ids[i]);

								drApiLogPrintf("[DRFOCAL] <===FOCAL_SET_IDS. aliData.idsCount: %d\n",aliData.idsCount);
								break;

							case FOCAL_SET_IDNAME:

								drApiLogPrintf("[DRFOCAL]FOCAL_SET_IDNAME\n");
								drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
										           (addr_t)focalData->focalTpl,
												    sizeof(size_focalTpl),
													MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
													(void**)&(ftpl));
								if(drRet != DRAPI_OK) {
										drApiLogPrintf("[DRFOCAL] ==FOCAL_SET_IDNAME==: map task buffer failed\n");
										tlRet = E_DRAPI_CANNOT_MAP;
										break;
								}
								drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
										           (addr_t)ftpl->name,
												    sizeof(char)*MAX_ID_NAME_SIZE,
													MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
													(void**)&(tplName_p));
								if(drRet != DRAPI_OK) {
										drApiLogPrintf("[DRFOCAL] ==FOCAL_SET_IDNAME 22==: map task buffer failed\n");
										tlRet = E_DRAPI_CANNOT_MAP;
										break;
								}

								if (focalData->index < MAX_ID_LIST_SIZE && tplName_p != NULL) {
									aliData.index = focalData->index;
									if (aliData.focalTpl[aliData.index].name == NULL)
										aliData.focalTpl[aliData.index].name = (char *)drApiMalloc(sizeof(char)*MAX_ID_NAME_SIZE,0);
									if (aliData.focalTpl[aliData.index].name != NULL)
										memcpy(aliData.focalTpl[aliData.index].name,tplName_p,MAX_ID_NAME_SIZE);
										aliData.focalTpl[aliData.index].id = ftpl->id;
								}

								drApiLogPrintf("[DRFOCAL] ===>ali id: %d, name:%s\n",aliData.focalTpl[aliData.index].id,aliData.focalTpl[aliData.index].name);
								break;

							case FOCAL_SET_VERSION:
								drApiLogPrintf("[DRFOCAL] FOCAL_SET_VERSION\n");
								aliData.version = focalData->version;
								drApiLogPrintf("[DRFOCAL] ===>ali version: %x\n",aliData.version);
								break;

							case ALI_GET_IDENTIFY_RESULT:
								drApiLogPrintf("[DRFOCAL] ALI_GET_IDENTIFY_RESULT\n");
								//focalData->lastPassid = aliData.lastPassid;
								drFocal_get_lastpassID(focalData,&aliData);
								drApiLogPrintf("[DRFOCAL] ===>ali lastPassid: %x\n",aliData.lastPassid);
								break;

							case ALI_GET_VERSION:
								drApiLogPrintf("[DRFOCAL] ALI_GET_VERSION\n");
								focalData->version = aliData.version;
								drApiLogPrintf("[DRFOCAL] ===>ali version: %x\n",aliData.version);
								break;

							case ALI_GET_IDS:
								drApiLogPrintf("[DRFOCAL] ALI_GET_IDS\n");
								drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
										           (addr_t)focalData->ids,
												    (sizeof(uint32_t)*MAX_ID_LIST_SIZE),
													MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
													(void**)&(ids_t));
								if(drRet != DRAPI_OK) {
										drApiLogPrintf("[DRFOCAL] ==FOCAL_SET_IDS==: map task buffer failed\n");
										tlRet = E_DRAPI_CANNOT_MAP;
										break;
								}
								memcpy(ids_t, aliData.ids, sizeof(uint32_t)*MAX_ID_LIST_SIZE);
								//focalData->ids = aliData.ids;
								focalData->idsCount = aliData.idsCount;
								drApiLogPrintf("[DRFOCAL] ===>ali idsCount: %x\n",focalData->idsCount);
								break;

							case ALI_GET_NAME:
								drApiLogPrintf("[DRFOCAL] ALI_GET_NAME\n");
								drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
										           (addr_t)focalData->focalTpl,
												    (sizeof(size_focalTpl)),
													MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
													(void**)&(fdp->focalTpl));
								if(drRet != DRAPI_OK) {
										drApiLogPrintf("[DRFOCAL] ==ALI_GET_NAME==: map task buffer failed\n");
										tlRet = E_DRAPI_CANNOT_MAP;
										break;
								}
								drRet = drApiMapTaskBuffer(THREADID_TO_TASKID(ipcClient),
										           (addr_t)fdp->focalTpl->name,
												    sizeof(char)*MAX_ID_NAME_SIZE,
													MAP_READABLE|MAP_WRITABLE|MAP_ALLOW_NONSECURE,
													(void**)&(tplName_p));
								if(drRet != DRAPI_OK) {
										drApiLogPrintf("[DRFOCAL] ==ALI_GET_NAME 22==: map task buffer failed\n");
										tlRet = E_DRAPI_CANNOT_MAP;
										break;
								}

								for (i=0;i<aliData.idsCount;i++) {
									if (aliData.focalTpl[i].id == fdp->focalTpl->id) {
										//memcpy(tplName_p, aliData.focalTpl[i].name,MAX_ID_NAME_SIZE);
										strcpy(tplName_p,aliData.focalTpl[i].name);
										drApiLogPrintf("[DRFOCAL] found tpl id: %d, index: %d\n",aliData.focalTpl[i].id,i);
										drApiLogPrintf("[DRFOCAL] name: %s\n",tplName_p);
										break;
									}
								}
								drApiLogPrintf("[DRFOCAL] tpl id: %d\n",fdp->focalTpl->id);
								break;

							 default:
								drApiLogPrintf("[DRFOCAL] Unknown cmd\n");
								break;
						 }

                        break;
                    default:
						drApiLogPrintf("[DRFOCAL] ===>Unknown pMarshal.\n");
                        /* Unknown message has been received*/
                        break;
                    }
                    //pMarshal->header.retVal = tlRet;
                     pMarshal->payload.retVal = tlRet;
					drApiLogPrintf("[DRFOCAL] drApiUnmapTaskBuffers\n");
					drApiUnmapTaskBuffers(THREADID_TO_TASKID(ipcClient));
                } else drApiLogPrintf("[DRFOCAL] ===>ELSE pMarshal.\n");

                /* Update response data */
                ipcMsg  = MSG_RS;
                ipcData = 0;

  //          #endif // DR_FEATURE_TL_API
                break;
            default:
                drApiIpcUnknownMessage(&ipcClient, &ipcMsg, &ipcData);
                /* Unknown message has been received*/
                ipcMsg = (message_t) E_TLAPI_DRV_UNKNOWN;
                ipcData = 0;
                drApiLogPrintf("[DRFOCAL] IPC unknown\n");
                break;
        }
    }
}


/**
 * IPC handler thread entry point
 */
_THREAD_ENTRY void drIpch( void )
{
    drIpchLoop();
}


/**
 * IPC handler init
 */
void drIpchInit(void)
{
    /* ensure thread stack is clean */
    clearStack(drIpchStack);

    /**
     * Start IPC handler thread. Exception handler thread becomes local
     * exception handler of IPC handler thread
     */
    if (DRAPI_OK != drApiStartThread(
                    DRIVER_THREAD_NO_IPCH,
                    FUNC_PTR(drIpch),
                    getStackTop(drIpchStack),
                    IPCH_PRIORITY,
                    DRIVER_THREAD_NO_EXCH))

    {
            drDbgPrintLnf("[Driver DrTemplate] drIpchInit(): drApiStartThread failed");
    }
}
