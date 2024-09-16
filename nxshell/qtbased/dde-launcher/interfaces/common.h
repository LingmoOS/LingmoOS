// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMON_H
#define COMMON_H

#include <QDBusArgument>
#include <QDebug>
#include <QDataStream>
#include <QtDBus>

class AppInfo;
typedef QList<AppInfo> AppInfoList;

Q_DECLARE_METATYPE(AppInfo)
Q_DECLARE_METATYPE(AppInfoList)

const QStringList CompatiblePluginApiList
{
    "1.0.0",
};

class AppInfo
{
public:
    enum AppStatus {
        Normal,         // 正常状态，不展示应用的进度值
        Busy,           // 进行中的状态，展示应用的进度值
    };

    AppInfo()
    : m_status(-1)
    , m_categoryId(0)
    , m_progressValue(0)
    {
    }

    AppInfo(const AppInfo &info)
    : m_desktop(info.m_desktop)
    , m_name(info.m_name)
    , m_key(info.m_key)
    , m_iconKey(info.m_iconKey)
    , m_status(info.m_status)
    , m_categoryId(info.m_categoryId)
    , m_description(info.m_description)
    , m_progressValue(info.m_progressValue)
    {
    }

    ~AppInfo() {}

    static void registerMetaType()
    {
        qRegisterMetaType<AppInfo>("AppInfo");
        qDBusRegisterMetaType<AppInfo>();
        qRegisterMetaType<AppInfoList>("AppInfoList");
        qDBusRegisterMetaType<AppInfoList>();
    }

    inline bool operator==(const AppInfo &other) {  return m_desktop == other.m_desktop; }
    friend QDebug operator<<(QDebug argument, const AppInfo &info)
    {
        argument << info.m_desktop << info.m_name << info.m_key;
        argument << info.m_iconKey << info.m_status << info.m_categoryId;
        argument << info.m_description << info.m_progressValue;

        return argument;
    }
    friend QDBusArgument &operator<<(QDBusArgument &argument, const AppInfo &info)
    {
        argument.beginStructure();
        argument << info.m_desktop << info.m_name << info.m_key;
        argument << info.m_iconKey << info.m_status << info.m_categoryId;
        argument << info.m_description << info.m_progressValue;
        argument.endStructure();

        return argument;
    }

    friend const QDBusArgument &operator>>(const QDBusArgument &argument, AppInfo &info)
    {
        argument.beginStructure();
        argument >> info.m_desktop >> info.m_name >> info.m_key;
        argument >> info.m_iconKey >> info.m_iconKey >> info.m_categoryId;
        argument >> info.m_description >> info.m_progressValue;
        argument.endStructure();

        return argument;
    }

    friend const QDataStream &operator>>(QDataStream &argument, AppInfo &info)
    {
        argument >> info.m_desktop >> info.m_name >> info.m_key;
        argument >> info.m_iconKey >> info.m_status >> info.m_categoryId;
        argument >> info.m_categoryId >> info.m_description >> info.m_progressValue;

        return argument;
    }

public:
    QString m_desktop;              // 应用的绝对路径
    QString m_name;                 // 应用名称
    QString m_key;                  // 应用所对应的二进制名称
    QString m_iconKey;              // 图标文件名称
    int m_status;                   // 应用状态
    qlonglong m_categoryId;         // 应用分类的id,每个分类的id值不同
    QString m_description;          // 展示当前应用状态
    int m_progressValue;            // 显示应用的某种进度
};

#endif
