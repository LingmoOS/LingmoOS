/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef DBUSHELPER_H_
#define DBUSHELPER_H_

#include <QDBusAbstractInterface>
#include <QDBusConnection>

namespace KBolt
{
class Device;
}

namespace DBusHelper
{
QDBusConnection connection();
QString serviceName();

using CallErrorCallback = std::function<void(const QString &)>;
using CallOkCallback = std::function<void()>;
void handleCall(QDBusPendingCall call, CallOkCallback &&okCb, CallErrorCallback &&errCb, QObject *parent);

template<typename... V>
void call(QDBusAbstractInterface *iface, const QString &method, const V &... args, CallOkCallback &&okCb, CallErrorCallback &&errCb, QObject *parent = nullptr)
{
    handleCall(iface->asyncCall(method, args...), std::move(okCb), std::move(errCb), parent);
}

} // namespace

#endif
