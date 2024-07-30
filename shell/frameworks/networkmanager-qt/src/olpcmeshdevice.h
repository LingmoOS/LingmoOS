/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2011-2013 Lamarque Souza <lamarque@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_OLPCMESHDEVICE_H
#define NETWORKMANAGERQT_OLPCMESHDEVICE_H

#include "device.h"

#include <networkmanagerqt/networkmanagerqt_export.h>

namespace NetworkManager
{
class OlpcMeshDevicePrivate;

/**
 * A OLPC mesh interface
 */
class NETWORKMANAGERQT_EXPORT OlpcMeshDevice : public Device
{
    Q_OBJECT

public:
    typedef QSharedPointer<OlpcMeshDevice> Ptr;
    typedef QList<Ptr> List;
    /**
     * Creates a new OlpcMeshDevice object.
     *
     * @param path the DBus path of the device
     */
    explicit OlpcMeshDevice(const QString &path, QObject *parent = nullptr);
    /**
     * Destroys a OlpcMeshDevice object.
     */
    ~OlpcMeshDevice() override;
    /**
     * Return the type
     */
    Type type() const override;
    /**
     * Currently active channel.
     */
    uint activeChannel() const;
    /**
     * The hardware address currently used by the network interface
     */
    QString hardwareAddress() const;
    /**
     * The current companion device.
     */
    Device::Ptr companionDevice() const;

Q_SIGNALS:
    /**
     * The active channel changed.
     */
    void activeChannelChanged(uint);
    /**
     * The companion changed.
     */
    void companionChanged(const Device::Ptr &device);
    /**
     * The device changed its hardware address
     */
    void hardwareAddressChanged(const QString &);

private:
    Q_DECLARE_PRIVATE(OlpcMeshDevice)
};

} // namespace NetworkManager
#endif // NETWORKMANAGERQT_OLPCMESHDEVICE_H
