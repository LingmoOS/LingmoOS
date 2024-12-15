// Copyright (C) 2020 ~ 2022 Uniontech Software Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defsecuritytoolinfo.h"

#include <QDBusMetaType>

void registerDefSecurityToolInfo()
{
    qRegisterMetaType<DEFSECURITYTOOLINFO>("DEFSECURITYTOOLINFO");
    qDBusRegisterMetaType<DEFSECURITYTOOLINFO>();
}

void registerDefSecurityToolInfoList()
{
    qRegisterMetaType<DEFSECURITYTOOLINFOLIST>("DEFSECURITYTOOLINFOLIST");
    qRegisterMetaType<QList<DEFSECURITYTOOLINFO>>("QList<DEFSECURITYTOOLINFO>");
    qDBusRegisterMetaType<DEFSECURITYTOOLINFOLIST>();
    qDBusRegisterMetaType<QList<DEFSECURITYTOOLINFO>>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const DEFSECURITYTOOLINFO &toolInfo)
{
    argument.beginStructure();
    argument << toolInfo.strPackageName;
    argument << toolInfo.strAppName;
    argument << toolInfo.strImagePath;
    argument << toolInfo.strDescription;
    argument << toolInfo.strCompany;
    argument << toolInfo.strInallModel;
    argument << toolInfo.strCallMethodName;
    argument << toolInfo.bSystemTool;
    argument << toolInfo.bHasInstalled;
    argument << toolInfo.bCanUpdate;
    argument << toolInfo.iClassify;
    argument << toolInfo.strReserve;
    argument << toolInfo.strReserve2;
    argument << toolInfo.strReserve3;
    argument << toolInfo.strReserve4;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DEFSECURITYTOOLINFO &toolInfo)
{
    argument.beginStructure();
    argument >> toolInfo.strPackageName;
    argument >> toolInfo.strAppName;
    argument >> toolInfo.strImagePath;
    argument >> toolInfo.strDescription;
    argument >> toolInfo.strCompany;
    argument >> toolInfo.strInallModel;
    argument >> toolInfo.strCallMethodName;
    argument >> toolInfo.bSystemTool;
    argument >> toolInfo.bHasInstalled;
    argument >> toolInfo.bCanUpdate;
    argument >> toolInfo.iClassify;
    argument >> toolInfo.strReserve;
    argument >> toolInfo.strReserve2;
    argument >> toolInfo.strReserve3;
    argument >> toolInfo.strReserve4;
    argument.endStructure();
    return argument;
}
