#include "douniuserver_jni.h"
#include "DouniuServer.h"

jstring Java_com_game_douniuserver_jni_DouniuServerInterface_stringFromJNI( JNIEnv* env,
                                                  jobject thiz )
{
    return env->NewStringUTF("Hello from JNI! WZJ!");
}

JNIEXPORT jint JNICALL Java_com_game_douniuserver_jni_DouniuServerInterface_nativeInitAndAccept( JNIEnv* env,
                                                  jobject thiz )
{
	int ret = initAndAccept();
	LOGD("initAndAccept's ret:%d",ret);
	return ret;
}

JNIEXPORT void JNICALL Java_com_game_douniuserver_jni_DouniuServerInterface_nativeDisconnected( JNIEnv* env,
                                                  jobject thiz )
{
	disconnect();
}
