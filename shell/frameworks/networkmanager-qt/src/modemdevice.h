/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2011-2013 Lamarque V. Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_MODEMDEVICE_H
#define NETWORKMANAGERQT_MODEMDEVICE_H

#include "device.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

namespace NetworkManager
{
class ModemDevicePrivate;

/**
 * Represents a generic modem device, generally defined by the modemCapabilities() it exposes and at
 * the current point in time by the currentCapabilities().
 */
class NETWORKMANAGERQT_EXPORT ModemDevice : public Device
{
    Q_OBJECT
public:
    typedef QSharedPointer<ModemDevice> Ptr;
    typedef QList<Ptr> List;
    enum Capability {
        NoCapability = 0x0,
        Pots = 0x1,
        CdmaEvdo = 0x2,
        GsmUmts = 0x4,
        Lte = 0x8,
    };
    Q_DECLARE_FLAGS(Capabilities, Capability)
    explicit ModemDevice(const QString &path, QObject *parent = nullptr);
    ~ModemDevice() override;
    /**
     * Return the type
     */
    Type type() const override;
    /**
     * The generic family of access technologies the modem supports. Not all capabilities are
     * available at the same time however; some modems require a firmware reload or other
     * reinitialization to switch between eg CDMA/EVDO and GSM/UMTS.
     */
    Capabilities modemCapabilities() const;
    /**
     * The generic family of access technologies the modem currently supports without a firmware
     * reload or reinitialization.
     */
    Capabilities currentCapabilities() const;

Q_SIGNALS:
    /**
     * This signal is emitted when the capabilities of the device change
     */
    void currentCapabilitiesChanged(Capabilities);

protected:
    NETWORKMANAGERQT_NO_EXPORT explicit ModemDevice(ModemDevicePrivate &dd, QObject *parent = nullptr);

private:
    Q_DECLARE_PRIVATE(ModemDevice)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ModemDevice::Capabilities)

} // namespace NetworkManager

#endif
