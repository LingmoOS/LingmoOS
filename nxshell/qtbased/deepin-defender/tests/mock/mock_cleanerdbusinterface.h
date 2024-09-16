// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOCK_CLEANER_DBUS_INTERFACE_H
#define MOCK_CLEANER_DBUS_INTERFACE_H

#include "../../deepin-defender/src/window/modules/cleaner/cleanerdbusadaptorinterface.h"

#include <QObject>
#include <QFile>

#include "gmock/gmock.h"

class CleanerDbusInterface : public QObject
    , public CleanerDBusAdaptorInterface
{
    Q_OBJECT

public:
    explicit CleanerDbusInterface(QObject *parent = nullptr)
        : QObject(parent)
    {
        m_deletedFiles.clear();
    }

    virtual ~CleanerDbusInterface() {}

    // function from CleanerDBusAdaptorInterface
    virtual QStringList GetTrashInfoList()
    {
        return QStringList();
    }

    virtual QStringList GetCacheInfoList()
    {
        return QStringList();
    }

    virtual QStringList GetHistoryInfoList()
    {
        return QStringList();
    }

    virtual QStringList GetLogInfoList()
    {
        return QStringList();
    }

    virtual QString GetAppTrashInfoList()
    {
        return QString();
    }

    virtual QString GetBrowserCookiesInfoList()
    {
        return QString();
    }

    virtual void DeleteSpecifiedFiles(QStringList deleteList)
    {
        m_deletedFiles.append(deleteList);
    }

    virtual void AddSecurityLog(int nType, const QString &sInfo)
    {
        Q_UNUSED(nType);
        Q_UNUSED(sInfo);
    }

    virtual QString GetValueFromeGSettings(const QString &key)
    {
        // 仅使用一个字段，所以不需要使用入参
        Q_UNUSED(key);
        return m_lastCleanedFilesSize;
    }

    virtual void SetValueToGSettings(const QString &key, const QString &value)
    {
        Q_UNUSED(key);
        Q_UNUSED(value);
        m_lastCleanedFilesSize = value;
    }

    QStringList m_deletedFiles;
    QString m_lastCleanedFilesSize;
};

class MockCleanerDbusInterface : public CleanerDbusInterface
{
public:
    MOCK_METHOD0(GetTrashInfoList, QStringList());
    MOCK_METHOD0(GetCacheInfoList, QStringList());
    MOCK_METHOD0(GetLogInfoList, QStringList());
    MOCK_METHOD0(GetHistoryInfoList, QStringList());
    MOCK_METHOD0(GetAppTrashInfoList, QString());
    MOCK_METHOD0(GetBrowserCookiesInfoList, QString());
    // 不需要对gsettings使用mock形式，依照原逻辑，仅检测输入与读出
};

#endif // MOCK_CLEANER_DBUS_INTERFACE_H
