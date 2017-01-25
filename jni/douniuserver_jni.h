#include <string.h>
#include <jni.h>
#include <android/log.h>

#define TAG "[wzj]douniuserver_jni"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG , TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO  , TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN  , TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , TAG, __VA_ARGS__)

#ifndef _Included_DOUNIUSERVER_H
#define _Included_DOUNIUSERVER_H

#ifdef __cplusplus
extern "C" {
#endif

jstring Java_com_game_douniuserver_jni_DouniuServerInterface_stringFromJNI( JNIEnv* env,
                                                  jobject thiz );
JNIEXPORT jint JNICALL Java_com_game_douniuserver_jni_DouniuServerInterface_nativeInitAndAccept( JNIEnv* env,
                                                  jobject thiz );
JNIEXPORT void JNICALL Java_com_game_douniuserver_jni_DouniuServerInterface_nativeDisconnected( JNIEnv* env,
                                                  jobject thiz );

extern int initAndAccept();
extern void disconnect();

#ifdef __cplusplus
}
#endif

#endif //_Included_DOUNIUSERVER_H
