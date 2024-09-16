// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dyncreatobject.h"
QMap<QString, CREATE_FUNC> *ClassObjectFactory::s_mapNameClass = /*QMap<QString, CREATE_FUNC>()*/nullptr;
void *ClassObjectFactory::creatClassObject(const QString &name)
{
    if (s_mapNameClass == nullptr) {
        s_mapNameClass = new QMap<QString, CREATE_FUNC>;
    }
    auto it = s_mapNameClass->find(name);
    if (it != s_mapNameClass->end()) {
        CREATE_FUNC f = it.value();
        return f();
    }
    return nullptr;
}

void ClassObjectFactory::registerClass(const QString &name, CREATE_FUNC func)
{
    if (s_mapNameClass == nullptr) {
        s_mapNameClass = new QMap<QString, CREATE_FUNC>;
    }
    (*s_mapNameClass)[name] = func;
}

