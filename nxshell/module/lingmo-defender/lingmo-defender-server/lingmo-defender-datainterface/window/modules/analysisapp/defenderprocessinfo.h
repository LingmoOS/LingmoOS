// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtCore/QString>
#include <QtDBus/QtDBus>
#include <QDBusMetaType>

struct DefenderProcessInfo
{
    int proc;
    QString title;
    QString desktop;

    bool operator!=(const DefenderProcessInfo &procInfo);
};

Q_DECLARE_METATYPE(DefenderProcessInfo)

QDBusArgument &operator<<(QDBusArgument &argument, const DefenderProcessInfo &procInfo);
const QDBusArgument &operator>>(const QDBusArgument &argument, DefenderProcessInfo &procInfo);

void registerDefenderProcessInfoMetaType();
