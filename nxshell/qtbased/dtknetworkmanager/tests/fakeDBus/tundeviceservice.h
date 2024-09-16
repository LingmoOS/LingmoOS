// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TUNDEVICESERVICE_H
#define TUNDEVICESERVICE_H

#include <QDBusObjectPath>

class FakeTunDeviceService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.FakeNetworkManager.Device.Tun")
public:
    explicit FakeTunDeviceService(QObject *parent = nullptr);
    ~FakeTunDeviceService() override;

    Q_PROPERTY(QString HwAddress READ HwAddress)

    QString m_HwAddress{"127.0.0.1"};

    QString HwAddress() const { return m_HwAddress; };

private:
    void registerService();
    void unregisterService();
    const QString Service{"com.deepin.FakeNetworkManager"};
    const QString Path{"/com/deepin/FakeNetworkManager/Devices/2"};
};

#endif
