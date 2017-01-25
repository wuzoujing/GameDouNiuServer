LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_LDLIBS :=-llog
LOCAL_CFLAGS := -DUSE_IN_ANDROID

LOCAL_MODULE    := douniuserver_jni
LOCAL_C_INCLUDES := douniuserver_jni.h DouniuServer.h GameLogic.h DouniuStruct.h msg.h
LOCAL_SRC_FILES := douniuserver_jni.cpp DouniuServer.c GameLogic.c

include $(BUILD_SHARED_LIBRARY)
