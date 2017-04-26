//
// Created by muto on 17-3-26.
//
#include <assert.h>
#include <jni.h>
#include <string.h>
#include <android/log.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "xx_rtmp.h"

static XXRtmp *rtmp = new XXRtmp();

extern "C" {

JNIEXPORT jint JNICALL
Java_com_mut0_xxcam_XXFlvMux_nativeTest(JNIEnv *env, jclass type) {
    return 0;
}


JNIEXPORT void JNICALL
Java_com_mut0_xxcam_XXRtmpPublish_native_1disconnect(JNIEnv *env, jobject instance) {

    // TODO

}

JNIEXPORT void JNICALL
Java_com_mut0_xxcam_XXFlvMux_nativeEatVideo(JNIEnv *env, jobject instance, jobject byteBuffer) {

    uint8_t *data = (uint8_t *) env->GetDirectBufferAddress(byteBuffer);

    rtmp->video(data);
}

JNIEXPORT void JNICALL
Java_com_mut0_xxcam_XXRtmpPublish_native_1addTarget(JNIEnv *env, jobject instance, jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);


    env->ReleaseStringUTFChars(url_, url);
}

JNIEXPORT void JNICALL
Java_com_mut0_xxcam_XXRtmpPublish_native_1eatVideo(JNIEnv *env, jobject instance,
                                                   jobject byteBuffer) {
    uint8_t *data = (uint8_t *) env->GetDirectBufferAddress(byteBuffer);

    rtmp->video(data);
}

JNIEXPORT void JNICALL
Java_com_mut0_xxcam_XXRtmpPublish_native_1connect(JNIEnv *env, jobject instance) {
    rtmp->Connect();
}

}