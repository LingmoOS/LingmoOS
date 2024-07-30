/*
    SPDX-FileCopyrightText: 2014 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEMSIGNAL_P_H
#define MODEMMANAGERQT_MODEMSIGNAL_P_H

#include "dbus/signalinterface.h"
#include "interface_p.h"
#include "modemsignal.h"

namespace ModemManager
{
class ModemSignalPrivate : public InterfacePrivate
{
public:
    explicit ModemSignalPrivate(const QString &path, ModemSignal *q);
    OrgFreedesktopModemManager1ModemSignalInterface modemSignalIface;

    uint rate;
    QVariantMap cdma;
    QVariantMap evdo;
    QVariantMap gsm;
    QVariantMap umts;
    QVariantMap lte;

    Q_DECLARE_PUBLIC(ModemSignal)
    ModemSignal *q_ptr;
private Q_SLOTS:
    void onPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProps) override;
};

} // namespace ModemManager

#endif
