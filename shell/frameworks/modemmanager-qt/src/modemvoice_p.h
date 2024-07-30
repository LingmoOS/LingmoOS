/*
    SPDX-FileCopyrightText: 2018 Aleksander Morgado <aleksander@aleksander.es>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGER_MODEMVOICE_P_H
#define MODEMMANAGER_MODEMVOICE_P_H

#include "call.h"
#include "dbus/voiceinterface.h"
#include "interface_p.h"
#include "modemvoice.h"

namespace ModemManager
{
class ModemVoicePrivate : public InterfacePrivate
{
public:
    explicit ModemVoicePrivate(const QString &path, ModemVoice *q);
    OrgFreedesktopModemManager1ModemVoiceInterface modemVoiceIface;

    QMap<QString, ModemManager::Call::Ptr> callList;
    ModemManager::Call::Ptr findCall(const QString &uni);
    ModemManager::Call::List calls();

    Q_DECLARE_PUBLIC(ModemVoice)
    ModemVoice *q_ptr;
private Q_SLOTS:
    void onCallAdded(const QDBusObjectPath &path);
    void onCallDeleted(const QDBusObjectPath &path);
};

} // namespace ModemManager

#endif
