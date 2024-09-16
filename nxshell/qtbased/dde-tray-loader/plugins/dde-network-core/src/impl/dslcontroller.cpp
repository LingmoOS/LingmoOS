// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dslcontroller.h"
#include "networkdevicebase.h"

#include <networkmanagerqt/manager.h>

using namespace dde::network;
using namespace NetworkManager;

DSLController::DSLController(QObject *parent)
    : QObject(parent)
{
}

DSLController::~DSLController()
{
}

/**
 * @brief UDSLItem::connection
 * @return
 */
DSLItem::DSLItem()
    : ControllItems()
    , m_connectStatus(ConnectionStatus::Unknown)
{
}

DSLItem::~DSLItem()
{
}

void DSLItem::setConnectionStatus(ConnectionStatus connectStatus)
{
    m_connectStatus = connectStatus;
}

ConnectionStatus DSLItem::status() const
{
    return m_connectStatus;
}
