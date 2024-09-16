// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DWIRELESSDEVICEINTERFACE_H
#define DWIRELESSDEVICEINTERFACE_H

#include "ddeviceInterface.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;

class DWirelessDeviceInterface : public DDeviceInterface
{
    Q_OBJECT
public:
    explicit DWirelessDeviceInterface(const QByteArray &path, QObject *parent = nullptr);
    ~DWirelessDeviceInterface() override = default;

    Q_PROPERTY(QList<QDBusObjectPath> accessPoints READ accessPoints NOTIFY AccessPointsChanged)
    Q_PROPERTY(QString HwAddress READ HwAddress NOTIFY HwAddressChanged)
    Q_PROPERTY(QString permHwAddress READ permHwAddress NOTIFY PermHwAddressChanged)
    Q_PROPERTY(quint32 mode READ mode NOTIFY ModeChanged)
    Q_PROPERTY(quint32 bitrate READ bitrate NOTIFY BitrateChanged)
    Q_PROPERTY(QDBusObjectPath activeAccessPoint READ activeAccessPoint NOTIFY ActiveAccessPointChanged)
    Q_PROPERTY(quint32 wirelessCapabilities READ wirelessCapabilities NOTIFY WirelessCapabilitiesChanged)
    Q_PROPERTY(qint64 lastScan READ lastScan NOTIFY LastScanChanged)

    QList<QDBusObjectPath> accessPoints() const;
    QString HwAddress() const;
    QString permHwAddress() const;
    quint32 mode() const;
    quint32 bitrate() const;
    QDBusObjectPath activeAccessPoint() const;
    quint32 wirelessCapabilities() const;
    qint64 lastScan() const;

public Q_SLOTS:
    QDBusPendingReply<void> requestScan(const Config &options) const;
    QDBusPendingReply<QList<QDBusObjectPath>> getAllAccessPoints() const;

Q_SIGNALS:
    void AccessPointsChanged(const QList<QDBusObjectPath> &aps);
    void HwAddressChanged(const QString &address);
    void PermHwAddressChanged(const QString &address);
    void ModeChanged(const quint32 mode);
    void BitrateChanged(const quint32 bitrate);
    void ActiveAccessPointChanged(const QDBusObjectPath &path);
    void WirelessCapabilitiesChanged(const quint32 wirelessCapabilities);
    void LastScanChanged(const qint64 time);

    void AccessPointAdded(const QDBusObjectPath &ap);
    void AccessPointRemoved(const QDBusObjectPath &ap);

private:
    DDBusInterface *m_wirelessInter{nullptr};
};

DNETWORKMANAGER_END_NAMESPACE

#endif
