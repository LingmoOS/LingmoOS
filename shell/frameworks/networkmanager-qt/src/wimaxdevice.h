/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2011-2013 Lamarque V. Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_WIMAXDEVICE_H
#define NETWORKMANAGERQT_WIMAXDEVICE_H

#include <networkmanagerqt/networkmanagerqt_export.h>

#include "device.h"
#include "wimaxnsp.h"

namespace NetworkManager
{
class WimaxDevicePrivate;

/**
 * Wimax network interface
 */
class NETWORKMANAGERQT_EXPORT WimaxDevice : public Device
{
    Q_OBJECT

public:
    typedef QSharedPointer<WimaxDevice> Ptr;
    typedef QList<Ptr> List;

    /**
     * Creates a new WimaxDevice object.
     *
     * @param path the DBus path of the device
     */
    explicit WimaxDevice(const QString &path, QObject *parent = nullptr);
    /**
     * Destroys a WimaxDevice object.
     */
    ~WimaxDevice() override;
    /**
     * Return the type
     */
    Type type() const override;
    /**
     * List of network service providers currently visible to the hardware
     */
    QStringList nsps() const;
    /**
     * Identifier of the NSP this interface is currently associated with
     */
    WimaxNsp::Ptr activeNsp() const;
    /**
     * The ID of the serving base station as received from the network.
     */
    QString bsid() const;
    /**
     * The hardware address currently used by the network interface
     */
    QString hardwareAddress() const;
    /**
     * Center frequency (in KHz) of the radio channel the device is using to communicate with the network when connected.
     */
    uint centerFrequency() const;
    /**
     * CINR (Carrier to Interference + Noise Ratio) of the current radio link in dB.
     */
    int cinr() const;
    /**
     * RSSI of the current radio link in dBm. This value indicates how strong the raw received RF signal from the base station is, but does not indicate the
     * overall quality of the radio link.
     */
    int rssi() const;
    /**
     * Average power of the last burst transmitted by the device, in units of 0.5 dBm. i.e. a TxPower of -11 represents an actual device TX power of -5.5 dBm.
     */
    int txPower() const;

    /**
     * Finds NSP object given its Unique Network Identifier.
     *
     * @param uni the identifier of the AP to find from this network interface
     * @returns a valid WimaxNsp object if a network having the given UNI for this device is known to the system, 0 otherwise
     */
    NetworkManager::WimaxNsp::Ptr findNsp(const QString &uni) const;

Q_SIGNALS:
    /**
     * This signal is emitted when the bitrate of this network has changed.
     *
     * @param bitrate the new bitrate value for this network
     */
    void bitRateChanged(int bitrate);
    /**
     * The active NSP changed.
     */
    void activeNspChanged(const QString &);
    /**
     * The BSID changed.
     */
    void bsidChanged(const QString &);
    /**
     * The device changed its hardware address
     */
    void hardwareAddressChanged(const QString &);
    /**
     * The device changed its center frequency
     */
    void centerFrequencyChanged(uint);
    /**
     * The device changed its signal/noise ratio
     */
    void cinrChanged(int);
    /**
     * The device changed its RSSI
     */
    void rssiChanged(int);
    /**
     * The device changed its TxPower.
     */
    void txPowerChanged(int);
    /**
     * A new NSP appeared
     */
    void nspAppeared(const QString &);
    /**
     * A wireless access point disappeared
     */
    void nspDisappeared(const QString &);

private:
    Q_DECLARE_PRIVATE(WimaxDevice)
};

} // namespace NetworkManager
#endif // NETWORKMANAGERQT_WIMAXDEVICE_H
