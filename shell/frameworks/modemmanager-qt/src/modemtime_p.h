/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGER_MODEMTIME_P_H
#define MODEMMANAGER_MODEMTIME_P_H

#include "dbus/timeinterface.h"
#include "interface_p.h"
#include "modemtime.h"

namespace ModemManager
{
class ModemTimePrivate : public InterfacePrivate
{
public:
    explicit ModemTimePrivate(const QString &path, ModemTime *q);
    OrgFreedesktopModemManager1ModemTimeInterface modemTimeIface;
    ModemManager::NetworkTimezone networkTimezone;

    ModemManager::NetworkTimezone variantMapToTimezone(const QVariantMap &map);

    Q_DECLARE_PUBLIC(ModemTime)
    ModemTime *q_ptr;
private Q_SLOTS:
    void onNetworkTimeChanged(const QString &isoDateTime);
    void onPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProps) override;
};

} // namespace ModemManager

#endif
