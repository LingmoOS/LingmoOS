// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef NETWORKDETAILNMREALIZE_H
#define NETWORKDETAILNMREALIZE_H

#include "netinterface.h"

#include <NetworkManagerQt/Device>

namespace dde {
namespace network {

class IpManager;

class NetworkDetailNMRealize : public NetworkDetailRealize
{
    Q_OBJECT

public:
    explicit NetworkDetailNMRealize(NetworkManager::Device::Ptr device, NetworkManager::ActiveConnection::Ptr activeConnection, QObject *parent = nullptr);
    ~NetworkDetailNMRealize();

    QString name();
    QList<QPair<QString, QString>> items();

private:
    void initProperties();
    void appendInfo(const QString &title, const QString &value);
    QString getSecurity(const NetworkManager::Device::Ptr &device) const;
    QString macAddress() const;
    QString prefixToNetMask(int prefixLength) const;
    QString getSpeedStr() const;
    void initConnection();

private Q_SLOTS:
     void onUpdateInfo();

private:
    NetworkManager::Device::Ptr m_device;
    NetworkManager::ActiveConnection::Ptr m_activeConnection;
    QString m_name;
    QList<QPair<QString, QString>> m_items;
    IpManager *m_ipConfig;
};

}
}

#endif // NETWORKDETAILNMREALIZE_H
