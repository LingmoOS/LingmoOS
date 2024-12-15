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
#include "load_libs.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <dlfcn.h>
void PrintError(){
    char *error;
    if ((error = dlerror()) != NULL)  {
        fprintf (stderr, "%s ", error);
    }
}
static LoadLibNames g_ldnames;
static LoadLibs *pLibs = NULL;
static LoadLibs *newClass(void)
{
    pLibs = (LoadLibs *)malloc(sizeof(LoadLibs));
    pLibs->m_document_clip_copy = NULL;
    pLibs->m_document_close = NULL;
//    RTLD_NOW：在dlopen返回前，解析出全部没有定义的符号，解析不出来返回NULL。
//    RTLD_LAZY：暂缓决定，等有需要时再解出符号
    void *handle = NULL;
    if (g_ldnames.chDocumentPr != NULL) {
        handle = dlopen(g_ldnames.chDocumentPr, RTLD_LAZY);
        if (handle == NULL) {
            PrintError();
        }
    } else {
        fprintf(stderr, "Error: Library path is NULL\n");
    }
    if (handle == NULL) {
        return pLibs;
    }

    pLibs->m_document_clip_copy = (uos_document_clip_copy)dlsym(handle, "document_clip_copy");
    PrintError();
    pLibs->m_document_close = (uos_document_close)dlsym(handle, "document_close");
    PrintError();

    assert(pLibs != NULL);
    return pLibs;
}

/**
 * 饿汉式
 * 支持延迟加载，但是为了多线程安全，性能有所降低
 * 注意：方法内部要加锁，防止多线程多次创建
 * */
LoadLibs *getLoadLibsInstance()
{    
    static pthread_mutex_t mutex;
    //双检锁
    if (pLibs == NULL) {
        // 这里要对pLibs加锁
        pthread_mutex_lock(&mutex);
        if (pLibs == NULL)
            pLibs = newClass();

        //退出时解锁
        pthread_mutex_unlock(&mutex);
    }

    return pLibs;
}

void setLibNames(LoadLibNames tmp)
{
    if(tmp.chDocumentPr == NULL) {
        g_ldnames.chDocumentPr = NULL;
    } else {
        g_ldnames.chDocumentPr = ( char*)malloc(strlen(tmp.chDocumentPr)+1);
        strcpy(g_ldnames.chDocumentPr,tmp.chDocumentPr);
    }
}

