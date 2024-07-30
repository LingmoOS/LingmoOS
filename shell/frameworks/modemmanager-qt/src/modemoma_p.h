/*
    SPDX-FileCopyrightText: 2014 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEMOMA_P_H
#define MODEMMANAGERQT_MODEMOMA_P_H

#include "dbus/omainterface.h"
#include "interface_p.h"
#include "modemoma.h"

namespace ModemManager
{
class ModemOmaPrivate : public InterfacePrivate
{
public:
    explicit ModemOmaPrivate(const QString &path, ModemOma *q);
    OrgFreedesktopModemManager1ModemOmaInterface omaIface;

    QFlags<MMOmaFeature> features;
    OmaSessionTypes pendingNetworkInitiatedSessions;
    MMOmaSessionType sessionType;
    MMOmaSessionState sessionState;

    Q_DECLARE_PUBLIC(ModemOma)
    ModemOma *q_ptr;
private Q_SLOTS:
    void onSessionStateChanged(int oldState, int newState, uint failedReason);
    void onPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProps) override;
};

} // namespace ModemManager

#endif
