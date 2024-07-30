/*
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_IFACES_CAMERA_H
#define SOLID_IFACES_CAMERA_H

#include <solid/devices/ifaces/deviceinterface.h>

#include <QStringList>

namespace Solid
{
namespace Ifaces
{
/**
 * This device interface is available on digital camera devices.
 *
 * A digital camera is a device used to transform images into
 * data. Nowaday most digital cameras are multifunctional and
 * able to take photographs, video or sound. On the system side
 * they are a particular type of device holding data, the access
 * method can be different from the typical storage device, hence
 * why it's a separate device interface.
 */
class Camera : virtual public DeviceInterface
{
public:
    /**
     * Destroys a Camera object.
     */
    ~Camera() override;

    /**
     * Retrieves known protocols this device can speak.  This list may be dependent
     * on installed device driver libraries.
     *
     * @return a list of known protocols this device can speak
     */
    virtual QStringList supportedProtocols() const = 0;

    /**
     * Retrieves known installed device drivers that claim to handle this device
     * using the requested protocol.
     *
     * @param protocol The protocol to get drivers for.
     * @return a list of known device drivers that can handle this device
     */
    virtual QStringList supportedDrivers(QString protocol = QString()) const = 0;

    /**
     * Retrieves a driver specific string allowing to access the device.
     *
     * For example for the "gphoto" driver it will return a list of the
     * form '("usb", vendor_id, product_id)'.
     *
     * @return the driver specific data
     */
    virtual QVariant driverHandle(const QString &driver) const = 0;
};
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::Camera, "org.kde.Solid.Ifaces.Camera/0.1")

#endif
