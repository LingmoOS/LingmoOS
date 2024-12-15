// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOG_H
#define LOG_H

#include <android/log.h>

#define LOG_TAG "cooperarion-native" // 自定义日志标签

#define ELOG(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__) // 错误日志
#define DLOG(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__) // 调试日志
#define WLOG(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__) // 调试日志

#endif // LOG_H
