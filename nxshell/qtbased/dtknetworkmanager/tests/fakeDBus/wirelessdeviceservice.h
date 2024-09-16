// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WIRELESSDEVICESERVICE_H
#define WIRELESSDEVICESERVICE_H

#include <QDBusObjectPath>
#include "dnetworkmanagertypes.h"

DNETWORKMANAGER_USE_NAMESPACE

class FakeWirelessDeviceService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.FakeNetworkManager.Device.Wireless")
public:
    explicit FakeWirelessDeviceService(QObject *parent = nullptr);
    ~FakeWirelessDeviceService() override;

    Q_PROPERTY(QList<QDBusObjectPath> AccessPoints READ accessPoints)
    Q_PROPERTY(QString HwAddress READ HwAddress)
    Q_PROPERTY(QString PermHwAddress READ permHwAddress)
    Q_PROPERTY(quint32 Mode READ mode)
    Q_PROPERTY(quint32 Bitrate READ bitrate)
    Q_PROPERTY(QDBusObjectPath ActiveAccessPoint READ activeAccessPoint)
    Q_PROPERTY(quint32 WirelessCapabilities READ wirelessCapabilities)
    Q_PROPERTY(qint64 LastScan READ lastScan)

    QList<QDBusObjectPath> m_accessPoints{QDBusObjectPath{"/org/freedesktop/NetworkManager/AccessPoint/15"}};
    QString m_HwAddress{"127.0.0.1"};
    QString m_permHwAddress{"127.0.0.1"};
    quint32 m_mode{2};
    quint32 m_bitrate{200};
    QDBusObjectPath m_activeAccessPoint{"/org/freedesktop/NetworkManager/AccessPoint/15"};
    quint32 m_wirelessCapabilities{3};
    qint64 m_lastScan{1433143};

    QList<QDBusObjectPath> accessPoints() const { return m_accessPoints; };
    QString HwAddress() const { return m_HwAddress; };
    QString permHwAddress() const { return m_permHwAddress; };
    quint32 mode() const { return m_mode; };
    quint32 bitrate() const { return m_bitrate; };
    QDBusObjectPath activeAccessPoint() const { return m_activeAccessPoint; };
    quint32 wirelessCapabilities() const { return m_wirelessCapabilities; };
    qint64 lastScan() const { return m_lastScan; }

    bool m_requestScanTrigger{false};
    bool m_getAllAccesspointTrigger{false};

public Q_SLOTS:

    Q_SCRIPTABLE void RequestScan(const Config &) { m_requestScanTrigger = true; }

    Q_SCRIPTABLE QList<QDBusObjectPath> GetAllAccessPoints()
    {
        m_getAllAccesspointTrigger = true;
        return {};
    };

private:
    void registerService();
    void unregisterService();
    const QString Service{"com.deepin.FakeNetworkManager"};
    const QString Path{"/com/deepin/FakeNetworkManager/Devices/2"};
};

#endif
