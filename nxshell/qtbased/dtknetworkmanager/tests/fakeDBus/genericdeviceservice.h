// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef GENERICDEVICESERVICE_H
#define GENERICDEVICESERVICE_H

#include <QDBusObjectPath>

class FakeGenericDeviceService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.FakeNetworkManager.Device.Generic")
public:
    explicit FakeGenericDeviceService(QObject *parent = nullptr);
    ~FakeGenericDeviceService() override;

    Q_PROPERTY(QString HwAddress READ HwAddress)
    Q_PROPERTY(QString TypeDescription READ typeDescription)

    QString m_HwAddress{"127.0.0.1"};
    QString m_typeDescription{"generic"};

    QString HwAddress() const { return m_HwAddress; };
    QString typeDescription() const { return m_typeDescription; };

private:
    void registerService();
    void unregisterService();
    const QString Service{"com.deepin.FakeNetworkManager"};
    const QString Path{"/com/deepin/FakeNetworkManager/Devices/2"};
};

#endif
