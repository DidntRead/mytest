#ifndef __DRFOCALEXEC__
#define __DRFOCALEXEC__

#include "drfocal.h"
#include "drfocal_Ali.h"
drApiResult_t drFocal_get_lastpassID(struct tlfocalApiDevice *focalData, struct tlAliApiDevice *aliData);
drApiResult_t drFocal_set_lastpassID(struct tlfocalApiDevice *focalData, struct tlAliApiDevice *aliData);
void drFocal_init(struct tlAliApiDevice *aliData);
void drFocal_del_tpl(int tid,struct tlAliApiDevice *aliData);
#endif
