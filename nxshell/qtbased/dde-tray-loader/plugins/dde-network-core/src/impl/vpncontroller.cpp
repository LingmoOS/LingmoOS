// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vpncontroller.h"
#include "networkconst.h"

#include "networkmanagerqt/manager.h"

using namespace dde::network;
using namespace NetworkManager;

VPNController::VPNController(QObject *parent)
    : QObject(parent)
{
}

VPNController::~VPNController()
{
}

/**
 * @brief UVPNItem详细项
 */

VPNItem::VPNItem()
    : ControllItems()
    , m_connectionStatus(ConnectionStatus::Deactivated)
{
}

VPNItem::~VPNItem()
{
}

void VPNItem::setConnectionStatus(ConnectionStatus connectionStatus)
{
    m_connectionStatus = connectionStatus;
}

ConnectionStatus VPNItem::status() const
{
    return m_connectionStatus;
}
