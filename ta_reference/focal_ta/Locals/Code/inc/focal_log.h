/**
 * Copyright 2011-2013 Fingerprint Cards AB
 * LOGD(ARG) : Debug warning print, where ARG is the same syntax of printf(ARG)
 * LOGE(ARG) : Error print, where ARG is the same syntax of printf(ARG). The program will exit when using this with error code 1
 * 
 */
#ifndef _FOCAL_LOG_H_
#define _FOCAL_LOG_H_

    void focal_tee_debug(const char* fmt, ...);
    #define LOGD(...)   focal_tee_debug("[focal_ta] " __VA_ARGS__)
    #define LOGE(...)   focal_tee_debug("[focal_ta] " __VA_ARGS__)
    #define LOGI(...)   focal_tee_debug("[focal_ta] " __VA_ARGS__)
    #define LOGS(...)   focal_tee_debug("[focal_ta] " __VA_ARGS__)


#endif /* _FOCAL_LOG_H_ */
  
  
