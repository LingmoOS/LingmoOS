/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_GENERICDEVICE_H
#define NETWORKMANAGERQT_GENERICDEVICE_H

#include <networkmanagerqt/networkmanagerqt_export.h>

#include "device.h"

namespace NetworkManager
{
class GenericDevicePrivate;

/**
 * A generic device interface
 */
class NETWORKMANAGERQT_EXPORT GenericDevice : public Device
{
    Q_OBJECT
    Q_PROPERTY(QString hardwareAddress READ hardwareAddress)
    Q_PROPERTY(QString typeDescription READ typeDescription)

public:
    typedef QSharedPointer<GenericDevice> Ptr;
    typedef QList<Ptr> List;
    explicit GenericDevice(const QString &path, QObject *parent = nullptr);
    ~GenericDevice() override;
    /**
     * Return the type
     */
    Type type() const override;
    /**
     * Active hardware address of the device
     */
    QString hardwareAddress() const;
    /**
     * A (non-localized) description of the interface type, if known.
     */
    QString typeDescription() const;

Q_SIGNALS:
    /**
     * Emitted when the hardware address of this device has changed
     */
    void hardwareAddressChanged(const QString &hwAddress);
    /**
     * Emitted when the type description this device has changed
     */
    void permanentHardwareAddressChanged(const QString &permHwAddress);

private:
    Q_DECLARE_PRIVATE(GenericDevice)
};

}

#endif // NETWORKMANAGERQT_GENERICDEVICE_H
