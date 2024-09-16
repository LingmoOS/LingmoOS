// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "hotspotcontroller.h"
#include "wirelessdevice.h"

#include <QDebug>

#include <networkmanagerqt/manager.h>

using namespace dde::network;
using namespace NetworkManager;

HotspotController::HotspotController(QObject *parent)
    : QObject(parent)
{
}

HotspotController::~HotspotController()
{
}

/**
 * @brief UHotspotItem::UHotspotItem
 */

HotspotItem::HotspotItem(WirelessDevice *device)
    : ControllItems()
    , m_device(device)
    , m_devicePath(m_device->path())
    , m_connectionStatus(ConnectionStatus::Unknown)
{
}

HotspotItem::~HotspotItem()
{
}

void HotspotItem::setConnectionStatus(const ConnectionStatus &status)
{
    m_connectionStatus = status;
}

QString HotspotItem::name() const
{
    return connection()->id();
}

WirelessDevice *HotspotItem::device() const
{
    return m_device;
}

QString HotspotItem::devicePath() const
{
    return m_devicePath;
}

ConnectionStatus HotspotItem::status() const
{
    return m_connectionStatus;
}
