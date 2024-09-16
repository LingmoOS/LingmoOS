// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef OWNERNETCONTROLLER_H
#define OWNERNETCONTROLLER_H

#include <QObject>

namespace accountnetwork {
namespace systemservice {

class InterfaceServer;
class NetworkHandler;
class AccountManager;
class NetworkConfig;

class OwnerNetController : public QObject
{
    Q_OBJECT

public:
    OwnerNetController(QObject *parent = Q_NULLPTR);
    ~OwnerNetController();
    QVariantMap authenInfo() const;
    QMap<QString, QString> accountNetwork() const;

signals:
    void requestAuthen(const QVariantMap &);

private slots:
    void onAccountChanged(const QString &accountname);
    void onAccountAdded(const QString &accountname, bool isIam);
    void onAccountRemoved(const QString &accountname);
    void onRequestAuthen(const QVariantMap &authenInfo);

private:
    NetworkConfig *m_networkConfig;
    AccountManager *m_accountManager;
    InterfaceServer *m_server;
    NetworkHandler *m_networkHandler;
};

}
}

#endif // NETWORKSERVICE_H
