// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETWORKCONNECTIONDATA_H
#define NETWORKCONNECTIONDATA_H

#include <QList>
#include <QVariant>
#include <QDBusMetaType>

typedef QMap<QString, QMap<QString, QVariant>> NetworkConnectionData;

void registerNetworkConnectionDataMetaType();

#endif // NETWORKCONNECTIONDATA_H
