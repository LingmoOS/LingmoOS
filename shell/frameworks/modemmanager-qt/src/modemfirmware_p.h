/*
    SPDX-FileCopyrightText: 2014 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEMFIRMWARE_P_H
#define MODEMMANAGERQT_MODEMFIRMWARE_P_H

#include "dbus/firmwareinterface.h"
#include "interface_p.h"

namespace ModemManager
{
class ModemFirmwarePrivate : public InterfacePrivate
{
public:
    explicit ModemFirmwarePrivate(const QString &path, ModemFirmware *q);
    OrgFreedesktopModemManager1ModemFirmwareInterface modemFirmwareIface;

    Q_DECLARE_PUBLIC(ModemFirmware)
    ModemFirmware *q_ptr;
};

} // namespace ModemManager

#endif
