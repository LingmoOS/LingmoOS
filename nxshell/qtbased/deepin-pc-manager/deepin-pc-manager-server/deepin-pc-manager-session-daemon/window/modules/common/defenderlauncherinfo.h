// Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SOCKETLAUNCHERINFO_H
#define SOCKETLAUNCHERINFO_H

#include <QtCore/QString>
#include <QtDBus/QtDBus>
#include <QDBusMetaType>
#include <QtCore/QList>

// com.deepin.dde.daemon.Launcher dbus GetAllItemInfos获取的结构体信息
struct LauncherItemInfo {
    QString Path;
    QString Name;
    QString ID;
    QString Icon;
    qint64 CategoryID;
    qint64 TimeInstalled;
};
Q_DECLARE_METATYPE(LauncherItemInfo);

typedef QList<LauncherItemInfo> LauncherItemInfoList;
Q_DECLARE_METATYPE(LauncherItemInfoList)

// 重载launcher dbus GetAllItemInfos获取的结构体操作符函数
const QDBusArgument &operator>>(const QDBusArgument &argument, LauncherItemInfo &iteminfos);
const QDBusArgument &operator<<(QDBusArgument &argument, const LauncherItemInfo &iteminfos);
void registerLauncherItemInfoMetaType();
void registerLauncherItemInfoListMetaType();

#endif // SOCKETLAUNCHERINFO_H
