// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "connectivityhandler.h"
#include "netinterface.h"
#include "wirelessdevice.h"

#include "networkdevicebase.h"
#include "netutils.h"
#include "configsetting.h"

#define SYSTEM_NETWORK_SERVICE "org.lingmo.service.SystemNetwork"
#define SYSTEM_NETWORK_PATH "/org/lingmo/service/SystemNetwork"
#define SYSTEM_NETWORK_INTERFACE "org.lingmo.service.SystemNetwork"

using namespace ocean::network;

ConnectivityHandler::ConnectivityHandler(QObject *parent)
    : QObject(parent)
    , m_connectivity(Connectivity::Full)
{
    initConnection();
    init();
}

ConnectivityHandler::~ConnectivityHandler()
{
}

void ConnectivityHandler::init()
{
    int connectivity = getConnectivity();
    onConnectivityChanged(connectivity);
}

Connectivity ConnectivityHandler::connectivity() const
{
    return m_connectivity;
}

void ConnectivityHandler::initConnection()
{
    QDBusConnection::systemBus().connect(SYSTEM_NETWORK_SERVICE, SYSTEM_NETWORK_PATH,
                                         SYSTEM_NETWORK_INTERFACE, "ConnectivityChanged", this, SLOT(onConnectivityChanged(int)));
}

int ConnectivityHandler::getConnectivity()
{
    QDBusInterface dbusInter(SYSTEM_NETWORK_SERVICE, SYSTEM_NETWORK_PATH, SYSTEM_NETWORK_INTERFACE, QDBusConnection::systemBus());
    return dbusInter.property("Connectivity").toInt();
}

void ConnectivityHandler::onConnectivityChanged(int connectivity)
{
    qCWarning(DNC) << "connecviticy changed" << connectivity;
    m_connectivity = static_cast<Connectivity>(connectivity);
    emit connectivityChanged(m_connectivity);
}
