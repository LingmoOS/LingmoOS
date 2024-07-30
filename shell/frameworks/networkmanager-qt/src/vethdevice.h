/*
    SPDX-FileCopyrightText: 2013 Lukáš Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_VETH_DEVICE_H
#define NETWORKMANAGERQT_VETH_DEVICE_H

#include "device.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

namespace NetworkManager
{
class VethDevicePrivate;

/**
 * A veth device interface
 */
class NETWORKMANAGERQT_EXPORT VethDevice : public Device
{
    Q_OBJECT
    Q_PROPERTY(QString peer READ peer NOTIFY peerChanged)

public:
    typedef QSharedPointer<VethDevice> Ptr;
    typedef QList<Ptr> List;

    explicit VethDevice(const QString &path, QObject *parent = nullptr);
    ~VethDevice() override;

    Type type() const override;

    QString peer() const;

Q_SIGNALS:
    void peerChanged(const QString &peer);

private:
    Q_DECLARE_PRIVATE(VethDevice)
};

}

#endif
