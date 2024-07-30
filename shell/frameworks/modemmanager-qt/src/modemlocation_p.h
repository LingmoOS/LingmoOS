/*
    SPDX-FileCopyrightText: 2008 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2010 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEMLOCATION_P_H
#define MODEMMANAGERQT_MODEMLOCATION_P_H

#include "dbus/locationinterface.h"
#include "interface_p.h"
#include "modemlocation.h"

namespace ModemManager
{
class ModemLocationPrivate : public InterfacePrivate
{
public:
    explicit ModemLocationPrivate(const QString &path, ModemLocation *q);
    OrgFreedesktopModemManager1ModemLocationInterface modemLocationIface;

    QFlags<MMModemLocationSource> capabilities;
    QFlags<MMModemLocationSource> enabledCapabilities;
    bool signalsLocation;
    LocationInformationMap location;

    Q_DECLARE_PUBLIC(ModemLocation)
    ModemLocation *q_ptr;
private Q_SLOTS:
    void onPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProps) override;
};

} // namespace ModemManager

#endif
