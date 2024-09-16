// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFENDERDISSECTION_H
#define DEFENDERDISSECTION_H

#include "defenderfileinfo.h"

#include <QtCore/QString>
#include <QtDBus/QtDBus>
#include <QDBusMetaType>

struct DefenderDissection
{
    int taskId;
    double progress;
    DefenderFileInfo fileInfo;

    bool operator!=(const DefenderDissection &dissectionInfo);
};

Q_DECLARE_METATYPE(DefenderDissection)

QDBusArgument &operator<<(QDBusArgument &argument, const DefenderDissection &dissectionInfo);
const QDBusArgument &operator>>(const QDBusArgument &argument, DefenderDissection &dissectionInfo);

void registerDefenderDissectionMetaType();

#endif // DEFENDERDISSECTION_H
