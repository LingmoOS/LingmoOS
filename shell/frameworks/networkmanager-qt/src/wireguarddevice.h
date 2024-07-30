/*
    SPDX-FileCopyrightText: 2019 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_WIREGUARD_DEVICE_H
#define NETWORKMANAGERQT_WIREGUARD_DEVICE_H

#include "device.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

namespace NetworkManager
{
class WireGuardDevicePrivate;

/**
 * A WireGuard device interface
 */
class NETWORKMANAGERQT_EXPORT WireGuardDevice : public Device
{
    Q_OBJECT
    Q_PROPERTY(QByteArray publicKey READ publicKey NOTIFY publicKeyChanged)
    Q_PROPERTY(uint listenPort READ listenPort NOTIFY listenPortChanged)
    Q_PROPERTY(uint fwMark READ fwMark NOTIFY fwMarkChanged)

public:
    typedef QSharedPointer<WireGuardDevice> Ptr;
    typedef QList<Ptr> List;
    explicit WireGuardDevice(const QString &path, QObject *parent = nullptr);
    ~WireGuardDevice() override;

    Type type() const override;

    /**
     * 32-byte public WireGuard key.
     */
    QByteArray publicKey() const;
    /**
     * Local UDP listening port.
     */
    uint listenPort() const;
    /**
     * Optional 32-bit mark used to set routing policy for outgoing encrypted packets. See: ip-rule(8)
     */
    uint fwMark() const;

Q_SIGNALS:
    /**
     * Emitted when the public key of this device has changed
     */
    void publicKeyChanged(const QByteArray &publicKey);
    /**
     * Emitted when the listen port of this device has changed
     */
    void listenPortChanged(uint listenPort);
    /**
     * Emitted when the fwmark of this device have changed
     */
    void fwMarkChanged(uint fwMark);

private:
    Q_DECLARE_PRIVATE(WireGuardDevice)
};

}

#endif
