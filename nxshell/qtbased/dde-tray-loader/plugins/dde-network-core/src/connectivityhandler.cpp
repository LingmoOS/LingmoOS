// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "connectivityhandler.h"
#include "netinterface.h"
#include "wirelessdevice.h"

#include "networkdevicebase.h"
#include "netutils.h"
#include "configsetting.h"

using namespace dde::network;

ConnectivityHandler::ConnectivityHandler(QObject *parent)
    : QObject(parent)
    , m_connectivity(Connectivity::Full)
{
    initConnectivity();
    initConnection();
}

ConnectivityHandler::~ConnectivityHandler()
{
}

Connectivity ConnectivityHandler::connectivity() const
{
    return m_connectivity;
}

void ConnectivityHandler::initConnectivity()
{
    QDBusInterface dbusInter("org.deepin.service.SystemNetwork", "/org/deepin/service/SystemNetwork", "org.deepin.service.SystemNetwork", QDBusConnection::systemBus());
    m_connectivity = static_cast<Connectivity>(dbusInter.property("Connectivity").toInt());
}

void ConnectivityHandler::initConnection()
{
    QDBusConnection::systemBus().connect("org.deepin.service.SystemNetwork", "/org/deepin/service/SystemNetwork",
                                         "org.deepin.service.SystemNetwork", "ConnectivityChanged", this, SLOT(onConnectivityChanged(int)));
}

void ConnectivityHandler::onConnectivityChanged(int connectivity)
{
    qDebug() << "connecviticy changed" << connectivity;
    m_connectivity = static_cast<Connectivity>(connectivity);
    emit connectivityChanged(m_connectivity);
}
