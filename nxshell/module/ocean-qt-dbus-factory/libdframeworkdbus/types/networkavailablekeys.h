// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETWORKAVAILABLEKEYS_H
#define NETWORKAVAILABLEKEYS_H

#include <QList>
#include <QVariant>
#include <QDBusMetaType>

typedef QMap<QString, QStringList> NetworkAvailableKeys;

void registerNetworkAvailableKeysMetaType();

#endif // NETWORKAVAILABLEKEYS_H
