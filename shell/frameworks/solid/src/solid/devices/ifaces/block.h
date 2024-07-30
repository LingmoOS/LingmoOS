/*
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_IFACES_BLOCK_H
#define SOLID_IFACES_BLOCK_H

#include <solid/devices/ifaces/deviceinterface.h>

namespace Solid
{
namespace Ifaces
{
/**
 * This device interface is available on block devices.
 *
 * A block device is an addressable device such as drive or partition.
 * It is possible to interact with such a device using a special file
 * in the system.
 */
class Block : virtual public DeviceInterface
{
public:
    /**
     * Destroys a Block object.
     */
    ~Block() override;

    /**
     * Retrieves the major number of the node file to interact with
     * the device.
     *
     * @return the device major number
     */
    virtual int deviceMajor() const = 0;

    /**
     * Retrieves the minor number of the node file to interact with
     * the device.
     *
     * @return the device minor number
     */
    virtual int deviceMinor() const = 0;

    /**
     * Retrieves the absolute path of the special file to interact
     * with the device.
     *
     * @return the absolute path of the special file to interact with
     * the device
     */
    virtual QString device() const = 0;
};
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::Block, "org.kde.Solid.Ifaces.Block/0.1")

#endif
