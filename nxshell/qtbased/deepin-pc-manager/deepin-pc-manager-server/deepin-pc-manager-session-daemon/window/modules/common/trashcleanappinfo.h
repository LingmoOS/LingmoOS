// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHCLEANAPPINFO_H
#define TRASHCLEANAPPINFO_H

#include <QtCore/QString>
#include <QtDBus/QtDBus>
#include <QDBusMetaType>
#include <QtCore/QList>

struct TrashCleanAppInfo {
    bool isUnistalled;
    QString appPkgName;
    QString appDisplayName;
    QString appConfigPaths;
    QString appCachePaths;
    TrashCleanAppInfo()
    {
        isUnistalled = false;
        appPkgName = "";
        appDisplayName = "";
        appConfigPaths = "";
        appCachePaths = "";
    }
    bool operator!=(TrashCleanAppInfo &other)
    {
        return this->appPkgName != other.appPkgName;
    }
};

QDBusArgument &operator<<(QDBusArgument &argument, const TrashCleanAppInfo &procInfo);
const QDBusArgument &operator>>(const QDBusArgument &argument, TrashCleanAppInfo &procInfo);
Q_DECLARE_METATYPE(TrashCleanAppInfo);
void registerTrashCleanAppInfoMetaType();

typedef QList<TrashCleanAppInfo> TrashCleanAppInfoList;
Q_DECLARE_METATYPE(TrashCleanAppInfoList);
void registerTrashCleanAppInfoListMetaType();

#endif // TRASHCLEANAPPINFO_H
