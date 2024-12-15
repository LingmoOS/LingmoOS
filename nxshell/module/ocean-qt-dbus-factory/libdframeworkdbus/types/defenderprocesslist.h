// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFENDERPROCESSLIST_H
#define DEFENDERPROCESSLIST_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtDBus/QtDBus>
#include <QDBusMetaType>

//save all process's pid
typedef QList<int> DefenderProcessList;
Q_DECLARE_METATYPE(DefenderProcessList);

void registerDefenderProcessListMetaType();

#endif // DEFENDERPROCESSLIST_H
