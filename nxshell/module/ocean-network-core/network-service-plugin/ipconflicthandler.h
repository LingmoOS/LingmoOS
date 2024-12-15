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
    virtual bool ipConflicted(const QString &devicePath, const QString &ip) const;
    virtual void ipv4Changed(const NetworkManager::Device::Ptr &device) {}

private:
    void initConnection();
    void initDevice();
    void initDeviceConnection(const NetworkManager::Device::Ptr &device);

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
    bool isConflicted(const QString &devicePath, const QString &ip) const;
    QString devicePathOfMac(const QString &devicePath) const;
    QSharedPointer<NetworkManager::Device> getDevicePathByMac(const QString &mac) const;
    void updateIpv4(const NetworkManager::Device::Ptr &device, const QStringList &ips, QStringList &resloveIps);

private:
    QString formatMac(const QString &macAddress) const;

private:
    typedef struct DeviceIpData
    {
        QString m_ip;
        QString m_localMac;
        QString m_remoteMac;
        QString m_uni;
        bool operator==(const struct DeviceIpData &data) {
            return this->m_ip == data.m_ip
                    && this->m_uni == data.m_uni
                    && this->m_localMac == data.m_localMac
                    && this->m_remoteMac == data.m_remoteMac;
        }
    } DeviceIpData;

    QMap<QString, QList<DeviceIpData>> m_conflictData;
};

#endif // IPCONFLICTHANDLER_H
