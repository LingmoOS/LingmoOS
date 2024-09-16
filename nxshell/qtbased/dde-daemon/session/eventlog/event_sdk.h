// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENT_SDK_H
#define EVENT_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

int InitEventSDK();
void writeEventLog(const char *log, int isDebug);
void CloseEventLog();


#ifdef __cplusplus
}
#endif
#endif // EVENT_SDK_H