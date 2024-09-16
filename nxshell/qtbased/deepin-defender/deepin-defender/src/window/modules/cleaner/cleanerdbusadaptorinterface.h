// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    virtual ~CleanerDBusAdaptorInterface() {}
};

#endif // CLEANERDBUSADAPTORINTERFACE_H
