/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEM_SIMPLE_P_H
#define MODEMMANAGERQT_MODEM_SIMPLE_P_H

#include "dbus/simpleinterface.h"

#include "interface_p.h"
#include "modemsimple.h"

namespace ModemManager
{
class ModemSimplePrivate : public InterfacePrivate
{
    Q_OBJECT
public:
    explicit ModemSimplePrivate(const QString &path, ModemSimple *q);
    OrgFreedesktopModemManager1ModemSimpleInterface modemSimpleIface;

    Q_DECLARE_PUBLIC(ModemSimple)
    ModemSimple *q_ptr;
};

} // namespace ModemManager

#endif
