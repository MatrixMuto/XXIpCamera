//
// Created by muto on 17-4-26.
//

#ifndef PROJECT_XX_LOG_H
#define PROJECT_XX_LOG_H

#ifdef ANDROID
#include <android/log.h>
#define  LOG_TAG    "xxio"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#else
#define  LOGI(...)  printf(__VA_ARGS__);printf("\n")
#define  LOGE(...)  printf(__VA_ARGS__);printf("\n")
#endif


#endif //PROJECT_XX_LOG_H
