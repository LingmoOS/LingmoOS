/*
    SPDX-FileCopyrightText: 2008 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2010 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEM3GPPUSSD_P_H
#define MODEMMANAGERQT_MODEM3GPPUSSD_P_H

#include "dbus/ussdinterface.h"
#include "interface_p.h"
#include "modem3gppussd.h"

namespace ModemManager
{
class Modem3gppUssdPrivate : public InterfacePrivate
{
    Q_OBJECT
public:
    explicit Modem3gppUssdPrivate(const QString &path, Modem3gppUssd *q);
    OrgFreedesktopModemManager1ModemModem3gppUssdInterface ussdIface;

    MMModem3gppUssdSessionState state;
    QString networkNotification;
    QString networkRequest;

    Q_DECLARE_PUBLIC(Modem3gppUssd)
    Modem3gppUssd *q_ptr;
private Q_SLOTS:
    void onPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProps) override;
};

} // namespace ModemManager

#endif
