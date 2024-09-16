// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * 因为unit test的需求，将后端分离以完成mock工作，提供虚函数使得gmock框架可以模拟dbus的输入与输出
 */

#ifndef CLEANERDBUSADAPTORINTERFACE_H
#define CLEANERDBUSADAPTORINTERFACE_H

#include <QStringList>

class CleanerDBusAdaptorInterface
{
public:
    virtual QStringList GetTrashInfoList() = 0;
    virtual QStringList GetCacheInfoList() = 0;
    virtual QStringList GetLogInfoList() = 0;
    virtual QStringList GetHistoryInfoList() = 0;
    virtual QString GetAppTrashInfoList() = 0;
    virtual QString GetBrowserCookiesInfoList() = 0;
    virtual void DeleteSpecifiedFiles(QStringList) = 0;
    virtual void AddSecurityLog(int nType, const QString &sInfo) = 0;
    virtual QString GetValueFromeGSettings(const QString &key) = 0;
    virtual void SetValueToGSettings(const QString &key, const QString &value) = 0;
    virtual void DeleteSpecifiedAppUninstallInfo(const QString &pkgName) = 0;

    virtual ~CleanerDBusAdaptorInterface() { }
};

#endif // CLEANERDBUSADAPTORINTERFACE_H
