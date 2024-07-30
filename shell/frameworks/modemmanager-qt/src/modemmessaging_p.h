/*
    SPDX-FileCopyrightText: 2013 Anant Kamath <kamathanant@gmail.com>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGER_MODEMMESSAGING_P_H
#define MODEMMANAGER_MODEMMESSAGING_P_H

#include "dbus/messaginginterface.h"
#include "interface_p.h"
#include "modemmessaging.h"
#include "sms.h"

namespace ModemManager
{
class ModemMessagingPrivate : public InterfacePrivate
{
public:
    explicit ModemMessagingPrivate(const QString &path, ModemMessaging *q);
    OrgFreedesktopModemManager1ModemMessagingInterface modemMessagingIface;

    QMap<QString, ModemManager::Sms::Ptr> messageList;
    QList<MMSmsStorage> supportedStorages;
    MMSmsStorage defaultStorage;
    ModemManager::Sms::Ptr findMessage(const QString &uni);
    ModemManager::Sms::List messages();

    Q_DECLARE_PUBLIC(ModemMessaging)
    ModemMessaging *q_ptr;
private Q_SLOTS:
    void onPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProps) override;
    void onMessageAdded(const QDBusObjectPath &path, bool received);
    void onMessageDeleted(const QDBusObjectPath &path);
};

} // namespace ModemManager

#endif
