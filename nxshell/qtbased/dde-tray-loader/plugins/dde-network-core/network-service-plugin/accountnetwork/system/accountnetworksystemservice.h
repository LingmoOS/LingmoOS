// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ACCOUNTNETWORKSYSTEMSERVICE_H
#define ACCOUNTNETWORKSYSTEMSERVICE_H

#include "systemservice.h"

#include <QVariantMap>

namespace accountnetwork {
namespace systemservice {

class AccountNetworkSystemContainer;

class AccountNetworkSystemService : public network::systemservice::SystemService
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "org.deepin.service.AccountSystemNetwork")

public:
    explicit AccountNetworkSystemService(AccountNetworkSystemContainer *network, QObject *parent = nullptr);

public slots:
    QVariantMap getAuthenInfo() const;
    QMap<QString, QString> accountNetwork() const;

signals:
    void requestAuthen(QVariantMap identity);

private:
    AccountNetworkSystemContainer *m_network;
};

}
}

#endif // SERVICE_H
