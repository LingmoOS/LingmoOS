// SPDX-FileCopyrightText: 2022 -2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DWIRELESSDEVICE_H
#define DWIRELESSDEVICE_H

#include "ddevice.h"
#include <QFlags>
#include <DExpected>

DNETWORKMANAGER_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DExpected;

class DWirelessDevicePrivate;

class DWirelessDevice : public DDevice
{
    Q_OBJECT
    Q_PROPERTY(QList<quint64> accessPoints READ accessPoints NOTIFY accessPointsChanged)
    Q_PROPERTY(QByteArray HwAddress READ HwAddress NOTIFY HwAddressChanged)
    Q_PROPERTY(QByteArray permHwAddress READ permHwAddress NOTIFY permHwAddressChanged)
    Q_PROPERTY(NM80211Mode mode READ mode NOTIFY modeChanged)
    Q_PROPERTY(quint32 bitrate READ bitrate NOTIFY bitrateChanged)
    Q_PROPERTY(quint64 activeAccessPoint READ activeAccessPoint NOTIFY activeAccessPointChanged)
    Q_PROPERTY(NMWifiCap wirelessCapabilities READ wirelessCapabilities NOTIFY wirelessCapabilitiesChanged)
    Q_PROPERTY(qint64 lastScan READ lastScan NOTIFY lastScanChanged)
public:
    Q_DECLARE_FLAGS(NMWifiCap, NMDeviceWiFiCapabilities);

    explicit DWirelessDevice(const quint64 id, QObject *parent = nullptr);
    ~DWirelessDevice() override = default;

    QList<quint64> accessPoints() const;
    QByteArray HwAddress() const;
    QByteArray permHwAddress() const;
    NM80211Mode mode() const;
    quint32 bitrate() const;
    quint64 activeAccessPoint() const;
    NMWifiCap wirelessCapabilities() const;
    qint64 lastScan() const;

public Q_SLOTS:
    DExpected<void> requestScan(const Config &options = {}) const;
    DExpected<QList<quint64>> getAllAccessPoints() const;

Q_SIGNALS:
    void accessPointsChanged(const QList<quint64> &aps);
    void HwAddressChanged(const QByteArray &address);
    void permHwAddressChanged(const QByteArray &address);
    void modeChanged(const NM80211Mode mode);
    void bitrateChanged(const quint32 bitrate);
    void activeAccessPointChanged(const quint64 &path);
    void wirelessCapabilitiesChanged(const NMWifiCap wirelessCapabilities);
    void lastScanChanged(const qint64 time);

    void AccessPointAdded(const quint64 &ap);
    void AccessPointRemoved(const quint64 &ap);

private:
    Q_DECLARE_PRIVATE(DWirelessDevice)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DWirelessDevice::NMWifiCap)

DNETWORKMANAGER_END_NAMESPACE

#endif
