#include "drStd.h"
#include "DrApi/DrApi.h"
#include "drFocalExec.h"

#define ONE_SEC 1*1000*1000

drApiResult_t drFocal_get_lastpassID(struct tlfocalApiDevice *focalData, struct tlAliApiDevice *aliData) {
	drApiResult_t drRet = DRAPI_OK;
	uint64_t time_stamp1;
	uint32_t timeHigh;
	uint32_t timeLow;

	drRet = drApiGetSecureTimestamp(&time_stamp1);
	if (drRet != DRAPI_OK) {
		drApiLogPrintf("[DRFOCAL] get timestamp fail\n");
		return drRet;
	}

	timeHigh = (time_stamp1 >> 32);
	if ((timeHigh - aliData->idStampHigh) > 0) drApiLogPrintf("[DRFOCAL] ==>ALI_GET_IDENTIFY_RESULT: timestamp high diff more than 1 sec\n");
	timeLow = (uint32_t) time_stamp1;
    if ((timeLow - aliData->idStampLow) > ONE_SEC || (timeHigh - aliData->idStampHigh) > 0) 
	{
		drApiLogPrintf("[DRFOCAL] ==>ALI_GET_IDENTIFY_RESULT: lucky timestamp low diff more than 1 sec\n");
		focalData->lastPassid = 0;
	}else
	{
		drApiLogPrintf("[DRFOCAL] ==>ALI_GET_IDENTIFY_RESULT: lastPassid valid \n");
		focalData->lastPassid = aliData->lastPassid;
	}
		//focalData->lastPassid = aliData->lastPassid;


	focalData->lastPassid = aliData->lastPassid;
	drApiLogPrintf("[DRFOCAL] <===ALI_GET_IDENTIFY_RESULT. last passid %d\n",focalData->lastPassid);
	drApiLogPrintf("[DRFOCAL] <===ALI_GET_IDENTIFY_RESULT. idStampLow: 0x%x, idStampLow: 0x%x\n",aliData->idStampLow,aliData->idStampHigh);
	return drRet;
}

drApiResult_t drFocal_set_lastpassID(struct tlfocalApiDevice *focalData, struct tlAliApiDevice *aliData) {
	drApiResult_t drRet = DRAPI_OK;
	uint64_t time_stamp1;

	drRet = drApiGetSecureTimestamp(&time_stamp1);
	if (drRet != DRAPI_OK) {
		drApiLogPrintf("[DRFOCAL] get timestamp fail\n");
		return drRet;
	}

	aliData->idStampLow = time_stamp1;
	aliData->idStampHigh = (unsigned int)(time_stamp1 >> 32);
	aliData->lastPassid = focalData->lastPassid;
	drApiLogPrintf("[DRFOCAL] <===FOCAL_SET_LASTPASSID. last passid %d\n",aliData->lastPassid);
	drApiLogPrintf("[DRFOCAL] <===FOCAL_SET_LASTPASSID. idStampLow: 0x%x, idStampLow: 0x%x\n",aliData->idStampLow,aliData->idStampHigh);
	return drRet;
}

void drFocal_init(struct tlAliApiDevice *aliData) {
	int i;
	
	aliData->version = FOCAL_VERSION;
	aliData->authenticatorVersion = FOCAL_AUTHENTICATOR_VERSION;
	aliData->idsCount = 0;
	aliData->index = 0;
	aliData->lastPassid = 0;
	aliData->idStampLow = 0;
	aliData->idStampHigh = 0;
	for (i = 0;i< MAX_ID_LIST_SIZE; i++) aliData->focalTpl[i].name = NULL;
}


void drFocal_del_tpl(int tid,struct tlAliApiDevice *aliData) {
	int dbidx = -1, i;
	
	drApiLogPrintf("==>[DRFOCAL] drFocal_del_tpl\n");
	
	for (i =0; i<aliData->idsCount; i++) {
		if (tid == aliData->focalTpl[i].id) {
			dbidx = i;
			break;
		}
	}
	
	if (dbidx > 0) {
		drApiFree(aliData->focalTpl[i].name);
		for (i = dbidx; i<aliData->idsCount;i++) {
			aliData->focalTpl[i].id = aliData->focalTpl[i+1].id;
			aliData->focalTpl[i].name = aliData->focalTpl[i+1].name;
		}
	}
	
	aliData->idsCount--;
	
	for (i=0;i<aliData->idsCount;i++) drApiLogPrintf("==>[DRFOCAL] drFocal_del_tpl: tpl[%d]: tid: %d, name: %s\n",i,
														aliData->focalTpl[i].id, aliData->focalTpl[i].name);
}
