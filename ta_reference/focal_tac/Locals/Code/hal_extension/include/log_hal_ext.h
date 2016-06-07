
#ifndef __LOG_H__
#define __LOG_H__

#ifndef NO_ANDROID_LOG
#include <android/log.h>
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#endif // NO_ANDROID_LOG

#ifdef NO_ANDROID_LOG
#define LOGE(...) {}
#define LOGD(...) {}
#endif // NO_ANDROID_LOG

#endif

