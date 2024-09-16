// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ACCESSPOINTSERVICE_H
#define ACCESSPOINTSERVICE_H

#include <QDBusObjectPath>

class FakeAccessPointService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.FakeNetworkManager.AccessPoint")
public:
    explicit FakeAccessPointService(QObject *parent = nullptr);
    ~FakeAccessPointService() override;

    Q_PROPERTY(QByteArray Ssid READ Ssid)
    Q_PROPERTY(quint8 Strength READ strength)
    Q_PROPERTY(quint32 Flags READ flags)
    Q_PROPERTY(quint32 Frequency READ frequency)
    Q_PROPERTY(quint32 RsnFlags READ rsnFlags)
    Q_PROPERTY(quint32 WpaFlags READ wpaFlags)

    QByteArray m_Ssid{"uniontech"};
    quint8 m_strength{20};
    quint32 m_flags{3};
    quint32 m_frequency{3152};
    quint32 m_rsnFlags{3};
    quint32 m_wpaFlags{3};

    QByteArray Ssid() const { return m_Ssid; }
    quint8 strength() const { return m_strength; }
    quint32 flags() const { return m_flags; }
    quint32 frequency() const { return m_frequency; }
    quint32 rsnFlags() const { return m_rsnFlags; }
    quint32 wpaFlags() const { return m_wpaFlags; }

private:
    void registerService();
    void unregisterService();
    const QString Service{"com.deepin.FakeNetworkManager"};
    const QString Path{"/com/deepin/FakeNetworkManager/AccessPoint/15"};
};

#endif
