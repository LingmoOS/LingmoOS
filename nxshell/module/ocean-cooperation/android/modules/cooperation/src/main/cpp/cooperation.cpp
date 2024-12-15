// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <jni.h>
#include <string>

#include "sessionhelper.h"
#include "manager/log.h"
#include "utils.h"

#define LIBCOOPERATION_VERSION "0.1.0"

// 将jstring转换为std::string的辅助函数
std::string jstringToString(JNIEnv *env, jstring jStr) {
    const char *chars = env->GetStringUTFChars(jStr, nullptr);
    std::string str(chars);
    env->ReleaseStringUTFChars(jStr, chars);
    return str;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_lingmo_cooperation_JniCooperation_nativeVersion(
        JNIEnv* env,
        jobject /* this */) {
    std::string version = LIBCOOPERATION_VERSION;
    return env->NewStringUTF(version.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_lingmo_cooperation_JniCooperation_initNative(JNIEnv *env, jobject thiz, jstring ip) {
    SessionHelper::getInstance().init(env, thiz, ip);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_lingmo_cooperation_JniCooperation_setStorageFolder(JNIEnv *env, jobject thiz, jstring root_path,
                                                            jstring dir_name) {
    std::string storagePath = jstringToString(env, root_path);  
    std::string storageFolder = jstringToString(env, dir_name);
    SessionHelper::getInstance().setStoragePath(storagePath, storageFolder);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_lingmo_cooperation_JniCooperation_setDeviceName(JNIEnv *env, jobject thiz,
                                                            jstring nick_name) {
    std::string deviceName = jstringToString(env, nick_name);
    SessionHelper::getInstance().setDeviceName(deviceName);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_lingmo_cooperation_JniCooperation_connectRemote(JNIEnv *env, jobject thiz, jstring ip,
                                                         jint port, jstring pin) {
    std::string ipStr = jstringToString(env, ip);
    std::string pinStr = jstringToString(env, pin);
    return SessionHelper::getInstance().asyncConnect(ipStr, port, pinStr);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_lingmo_cooperation_JniCooperation_disconnectRemote(JNIEnv *env, jobject thiz) {
    SessionHelper::getInstance().asyncDisconnect();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_lingmo_cooperation_JniCooperation_scanConnect(JNIEnv *env, jobject thiz,
                                                       jint w, jint h) {
    std::string resolution = std::to_string(w) + "x" + std::to_string(h);
    SessionHelper::getInstance().requestConnect(resolution);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_lingmo_cooperation_JniCooperation_sendProjection(JNIEnv *env, jobject thiz,
                                                          jstring nice_name, jint vnc_port) {
    std::string nickName = jstringToString(env, nice_name);
    SessionHelper::getInstance().requestProjection(nickName, vnc_port);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_lingmo_cooperation_JniCooperation_stopProjection(JNIEnv *env, jobject thiz,
                                                          jstring nice_name) {
    std::string nickName = jstringToString(env, nice_name);
    SessionHelper::getInstance().requestStopProjection(nickName);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_lingmo_cooperation_JniCooperation_getStatus(JNIEnv *env, jobject thiz) {
    jint result = 0;
    return result;
}
