/*
* Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
*
* Author:     shicetu <shicetu@uniontech.com>
*             hujianbo <hujianbo@uniontech.com>
* Maintainer: shicetu <shicetu@uniontech.com>
*             hujianbo <hujianbo@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LOAD_LIBS_H
#define LOAD_LIBS_H
#include <pthread.h>

typedef void (*uos_document_clip_copy)(const char *path, int *intercept);
typedef void (*uos_document_close)(const char *path);

typedef struct _LoadLibNames {
    char *chDocumentPr;
} LoadLibNames;

void setLibNames(LoadLibNames tmp);

typedef struct _LoadLibs {
    uos_document_clip_copy m_document_clip_copy;
    uos_document_close m_document_close;
} LoadLibs;

LoadLibs *getLoadLibsInstance();//饿汉式不支持延迟加载


#endif//LOAD_LIBS_H
