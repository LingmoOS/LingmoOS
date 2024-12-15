// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "systemservice.h"
#include "systemcontainer.h"
#include "systemipconfilct.h"
#include "connectivityprocesser.h"

using namespace network::systemservice;

SystemService::SystemService(SystemContainer *network, QObject *parent)
    : QObject(parent)
    , m_network(network)
{
    connect(m_network->ipConfilctedChecker(), &SystemIPConflict::ipConflictChanged, this, &SystemService::IpConflictChanged);
    connect(m_network->connectivityProcesser(), &ConnectivityProcesser::connectivityChanged, this, [ this ](const network::service::Connectivity &connectivity) {
        emit ConnectivityChanged(static_cast<int>(connectivity));
    });
    connect(m_network->connectivityProcesser(), &ConnectivityProcesser::portalDetected, this, &SystemService::PortalDetected);
}

bool SystemService::IpConflicted(const QString &devicePath)
{
    return m_network->ipConfilctedChecker()->ipConflicted(devicePath);
}

void SystemService::CheckConnectivity()
{
    m_network->connectivityProcesser()->checkConnectivity();
}

int SystemService::Connectivity()
{
    return static_cast<int>(m_network->connectivityProcesser()->connectivity());
}

QString SystemService::PortalUrl()
{
    return m_network->connectivityProcesser()->portalUrl();
}
