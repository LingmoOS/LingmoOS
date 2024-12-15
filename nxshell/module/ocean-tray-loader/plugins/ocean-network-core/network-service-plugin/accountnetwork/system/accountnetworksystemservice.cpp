// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "accountnetworksystemservice.h"
#include "accountnetworksystemcontainer.h"
#include "interfaceserver.h"
#include "ownernetcontroller.h"

#include <QDBusMetaType>

using namespace accountnetwork::systemservice;
using namespace network::systemservice;

AccountNetworkSystemService::AccountNetworkSystemService(AccountNetworkSystemContainer *network, QObject *parent)
    : network::systemservice::SystemService(network, parent)
    , m_network(network)
{
    qDBusRegisterMetaType<QMap<QString, QString>>();
    connect(m_network->authenInterface(), &OwnerNetController::requestAuthen, this, &AccountNetworkSystemService::requestAuthen);
}

QVariantMap AccountNetworkSystemService::getAuthenInfo() const
{
    return m_network->authenInterface()->authenInfo();
}

QMap<QString, QString> AccountNetworkSystemService::accountNetwork() const
{
    return m_network->authenInterface()->accountNetwork();
}
