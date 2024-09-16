// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QString>
#include <QDBusArgument>

struct DockItemInfo
{
    QString name;
    QString displayName;
    QString itemKey;
    QString settingKey;
    QString dccIcon;
    bool visible;
};

QDebug operator<<(QDebug argument, const DockItemInfo &info);
QDBusArgument &operator<<(QDBusArgument &arg, const DockItemInfo &info);
const QDBusArgument &operator>>(const QDBusArgument &arg, DockItemInfo &info);

Q_DECLARE_METATYPE(DockItemInfo)

typedef QList<DockItemInfo> DockItemInfos;

Q_DECLARE_METATYPE(DockItemInfos)

void registerPluginInfoMetaType();
