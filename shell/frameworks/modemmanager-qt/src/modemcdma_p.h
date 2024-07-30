/*
    SPDX-FileCopyrightText: 2008 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2010 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEMCDMA_P_H
#define MODEMMANAGERQT_MODEMCDMA_P_H

#include "dbus/modemcdmainterface.h"
#include "interface_p.h"
#include "modemcdma.h"

namespace ModemManager
{
class ModemCdmaPrivate : public InterfacePrivate
{
public:
    explicit ModemCdmaPrivate(const QString &path, ModemCdma *q);
    OrgFreedesktopModemManager1ModemModemCdmaInterface modemCdmaIface;

    MMModemCdmaActivationState activationState;
    QString meid;
    QString esn;
    uint sid;
    uint nid;
    MMModemCdmaRegistrationState cdma1xRegistrationState;
    MMModemCdmaRegistrationState evdoRegistrationState;

    Q_DECLARE_PUBLIC(ModemCdma)
    ModemCdma *q_ptr;
private Q_SLOTS:
    void onActivationStateChanged(uint activation_state, uint activation_error, const QVariantMap &status_changes);
    void onPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProps) override;
};

} // namespace ModemManager

#endif
