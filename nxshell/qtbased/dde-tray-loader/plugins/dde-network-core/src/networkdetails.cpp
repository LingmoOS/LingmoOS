// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "networkdetails.h"
#include "netinterface.h"

#include <networkmanagerqt/settings.h>
#include <networkmanagerqt/connection.h>
#include <networkmanagerqt/connectionsettings.h>
#include <networkmanagerqt/ipv6setting.h>

#include <QJsonObject>
#include <QJsonArray>

using namespace NetworkManager;
using namespace dde::network;

NetworkDetails::NetworkDetails(NetworkDetailRealize *realize, QObject *parent)
    : QObject(parent)
    , m_realize(realize)
{
}

NetworkDetails::~NetworkDetails()
{
    m_realize->deleteLater();
}

QString NetworkDetails::name()
{
    return m_realize->name();
}

QList<QPair<QString, QString> > NetworkDetails::items()
{
    return m_realize->items();
}
