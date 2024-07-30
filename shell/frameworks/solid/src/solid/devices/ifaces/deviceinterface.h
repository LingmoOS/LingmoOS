/*
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_IFACES_DEVICEINTERFACE_H
#define SOLID_IFACES_DEVICEINTERFACE_H

#include <QObject>

namespace Solid
{
namespace Ifaces
{
/**
 * Base interface of all the device interfaces.
 *
 * A device interface describes what a device can do. A device generally has
 * a set of device interfaces.
 */
class DeviceInterface
{
public:
    /**
     * Destroys a DeviceInterface object.
     */
    virtual ~DeviceInterface();
};
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::DeviceInterface, "org.kde.Solid.Ifaces.DeviceInterface/0.1")

#endif
