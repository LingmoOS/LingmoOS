// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtCore/QString>
#include <QtDBus/QtDBus>
#include <QtCore/QList>
#include <QDBusMetaType>

struct LauncherItemInfo {
    QString Path;
    QString Name;
    QString ID;
    QString Icon;
    qint64 CategoryID;
    qint64 TimeInstalled;
    bool operator!=(const LauncherItemInfo &versionInfo);
};

Q_DECLARE_METATYPE(LauncherItemInfo)

QDBusArgument &operator<<(QDBusArgument &argument, const LauncherItemInfo &versionInfo);
const QDBusArgument &operator>>(const QDBusArgument &argument, LauncherItemInfo &versionInfo);
void registerLauncherItemInfoMetaType();
