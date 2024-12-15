// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef IPCONFLICTHANDLER_H
#define IPCONFLICTHANDLER_H

#include <QObject>

#include <NetworkManagerQt/Settings>

namespace NetworkManager {
class Device;
class ActiveConnection;
}

class IPConflictHandler : public QObject
{
    Q_OBJECT

public:
    IPConflictHandler(QObject *parent);
    ~IPConflictHandler() = default;

protected:
    NetworkManager::Setting::SecretFlags secretFlags(const NetworkManager::ActiveConnection::Ptr &activeConnection) const;
    void requestReConnect(const QSharedPointer<NetworkManager::Device> &device);
    virtual bool canReconnect(const QSharedPointer<NetworkManager::ActiveConnection> &activeConnection);
    virtual void onIPConflicted(const QString &ip, const QString &localMac, const QString &remoteMac) = 0;
    virtual void onIPConflictReslove(const QString &ip, const QString &localMac, const QString &remoteMac) = 0;

private slots:
    void onReceiveIPConflict(const QString &ip, const QString &localMac, const QString &remoteMac);
    void onReceiveIPConflictReslove(const QString &ip, const QString &localMac, const QString &remoteMac);
};

class IPConflictStore
{
public:
    IPConflictStore();
    ~IPConflictStore();
    bool doConfilcted(const QString &ip, const QString &localMac, const QString &remoteMac, bool &needReconnect);
    bool doConflictReslove(const QString &ip, const QString &localMac, const QString &remoteMac);
    bool isConflicted(const QString &devicePath) const;
    QString devicePathOfMac(const QString &devicePath) const;
    QSharedPointer<NetworkManager::Device> getDevicePathByMac(const QString &mac) const;

private:
    QString formatMac(const QString &macAddress) const;

private:
    typedef struct
    {
        QString m_ip;
        QString m_localMac;
        QString m_remoteMac;
        QString m_uni;
    } DeviceIpData;

    QMap<QString, QList<DeviceIpData>> m_conflictData;
};

#endif // IPCONFLICTHANDLER_H
