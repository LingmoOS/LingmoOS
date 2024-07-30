/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_ADSL_DEVICE_H
#define NETWORKMANAGERQT_ADSL_DEVICE_H

#include "device.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

namespace NetworkManager
{
class AdslDevicePrivate;

/**
 * An adsl device interface
 */
class NETWORKMANAGERQT_EXPORT AdslDevice : public Device
{
    Q_OBJECT
    Q_PROPERTY(bool carrier READ carrier NOTIFY carrierChanged)
public:
    typedef QSharedPointer<AdslDevice> Ptr;
    typedef QList<Ptr> List;
    explicit AdslDevice(const QString &path, QObject *parent = nullptr);
    ~AdslDevice() override;

    Type type() const override;
    /**
     * Indicates whether the physical carrier is found
     */
    bool carrier() const;

Q_SIGNALS:
    /**
     * Emitted when the carrier of this device has changed
     */
    void carrierChanged(bool plugged);

private:
    Q_DECLARE_PRIVATE(AdslDevice)
};

}

#endif
