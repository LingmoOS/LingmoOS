// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef INTERFACESERVER_H
#define INTERFACESERVER_H

#include <QObject>
#include <QVariantMap>

class QLocalServer;
class QLocalSocket;

namespace accountnetwork {
namespace systemservice {

class SystemIPConflict;
class ConnectivityProcesser;
class NetworkConfig;
class AccountManager;

class InterfaceServer : public QObject
{
    Q_OBJECT

public:
    InterfaceServer(AccountManager *accountManager, NetworkConfig *conf, QObject *parent = Q_NULLPTR);
    ~InterfaceServer();
    void setNetworkConfig();
    QVariantMap authenInfo(const QString &accountname) const;
    void updateIamAuthen(const QString &accountname);

signals:
    void requestAuthen(const QVariantMap &);

private slots:
    void newConnectionHandler();
    void readyReadHandler();
    void disconnectedHandler();

private:
    QLocalServer *m_localServer;
    QList<QLocalSocket *> m_clients;
    QMap<QString, QVariantMap> m_authenInfo;
    AccountManager *m_accountManager;
    NetworkConfig *m_networkConfig;
    QVariantMap m_authen;
    bool m_needAuthen;
};

}
}

#endif // NETWORKSERVICE_H
