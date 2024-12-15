// SPDX-FileCopyrightText: 2022-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOAD_LIBS_H
#define LOAD_LIBS_H
#include <pthread.h>

typedef void (*uos_document_clip_copy)(const char *path, int *intercept);
typedef void (*uos_document_close)(const char *path);

typedef struct _LoadLibNames {
    char *chZPDDLL;
} LoadLibNames;

void setLibNames(LoadLibNames tmp);

typedef struct _LoadLibs {
    uos_document_clip_copy m_document_clip_copy;
    uos_document_close m_document_close;
} LoadLibs;

LoadLibs *getLoadZPDLibsInstance();//饿汉式不支持延迟加载

#endif//LOAD_LIBS_H
