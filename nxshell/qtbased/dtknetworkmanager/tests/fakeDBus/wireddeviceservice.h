// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WIREDDEVICESERVICE_H
#define WIREDDEVICESERVICE_H

#include <QDBusObjectPath>
#include "dnetworkmanagertypes.h"

DNETWORKMANAGER_USE_NAMESPACE

class FakeWiredDeviceService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.FakeNetworkManager.Device.Wired")
public:
    explicit FakeWiredDeviceService(QObject *parent = nullptr);
    ~FakeWiredDeviceService() override;

    Q_PROPERTY(QString HwAddress READ HwAddress)
    Q_PROPERTY(QString PermHwAddress READ permHwAddress)
    Q_PROPERTY(quint32 Speed READ speed)
    Q_PROPERTY(QStringList S390Subchannels READ S390Subchannels)
    Q_PROPERTY(bool Carrier READ carrier)

    QString m_HwAddress{"127.0.0.1"};
    QString m_permHwAddress{"127.0.0.1"};
    quint32 m_speed{200};
    QStringList m_S390Subchannels{QString{"subchannel"}};
    bool m_carrier{true};

    QString HwAddress() const { return m_HwAddress; };
    QString permHwAddress() const { return m_permHwAddress; };
    quint32 speed() const { return m_speed; };
    QStringList S390Subchannels() const { return m_S390Subchannels; };
    bool carrier() const { return m_carrier; };

private:
    void registerService();
    void unregisterService();
    const QString Service{"com.deepin.FakeNetworkManager"};
    const QString Path{"/com/deepin/FakeNetworkManager/Devices/2"};
};

#endif
