// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H
#include <QDBusPendingReply>

namespace NetworkManager {
QDBusPendingReply<QDBusObjectPath> activateConnection2(const QString &connectionUni, const QString &interfaceUni, const QString &connectionParameter, const QVariantMap &options);
} // namespace NetworkManager
#endif // NETWORKMANAGER_H
