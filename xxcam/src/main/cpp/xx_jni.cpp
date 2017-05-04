//
// Created by muto on 17-3-26.
//
#include <jni.h>
#include <android/log.h>

#include <sys/types.h>
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
Java_com_mut0_xxcam_XXRtmpPublish_native_1addTarget(JNIEnv *env, jobject instance, jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);


    env->ReleaseStringUTFChars(url_, url);
}

JNIEXPORT void JNICALL
Java_com_mut0_xxcam_XXRtmpPublish_native_1eatVideo(JNIEnv *env, jobject instance,
                                                   jobject byteBuffer, jint pos, jint remaining,
                                                   jint offset,
                                                   jint flags,
                                                   jlong presentationTimeUs) {

    uint8_t *data = (uint8_t *) env->GetDirectBufferAddress(byteBuffer);
    LOGI("data %p, pos %ld, len %ld, offset %d, flags %x, pts %ld", data, pos, remaining, offset,
         flags,
         presentationTimeUs);
    rtmp->video(data + pos, remaining, flags, presentationTimeUs);
}


JNIEXPORT void JNICALL
Java_com_mut0_xxcam_XXRtmpPublish_native_1connect(JNIEnv *env, jobject instance, jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);

    rtmp->CreateSession(url);

    env->ReleaseStringUTFChars(url_, url);
}

}