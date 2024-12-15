// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFENDERUPDATESTATE_H
#define DEFENDERUPDATESTATE_H

#include "defenderversioninfo.h"

#include <QtCore/QString>
#include <QtDBus/QtDBus>
#include <QDBusMetaType>

struct DefenderUpdateState
{
    double progress;
    DefenderVersionInfo info;

    bool operator!=(const DefenderUpdateState &updateState);
};

Q_DECLARE_METATYPE(DefenderUpdateState)

QDBusArgument &operator<<(QDBusArgument &argument, const DefenderUpdateState &updateState);
const QDBusArgument &operator>>(const QDBusArgument &argument, DefenderUpdateState &updateState);

void registerDefenderUpdateStateMetaType();

#endif // DEFENDERUPDATESTATE_H
