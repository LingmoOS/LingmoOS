// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETUTILS_H
#define NETUTILS_H

#include "networkconst.h"

#include <NetworkManagerQt/ActiveConnection>

#include <QDebug>
#include <QJsonObject>

#include "com_deepin_daemon_network.h"

namespace dde {
namespace network {

// an alias for numeric zero, no flags set.
#define DEVICE_INTERFACE_FLAG_NONE 0
// the interface is enabled from the administrative point of view. Corresponds to kernel IFF_UP.
#define DEVICE_INTERFACE_FLAG_UP 0x1
// the physical link is up. Corresponds to kernel IFF_LOWER_UP.
#define DEVICE_INTERFACE_FLAG_LOWER_UP 0x2
// the interface has carrier. In most cases this is equal to the value of @NM_DEVICE_INTERFACE_FLAG_LOWER_UP
#define DEVICE_INTERFACE_FLAG_CARRIER 0x10000

// wifi6的标记
#define AP_FLAGS_HE 0x10

using NetworkInter = com::deepin::daemon::Network;

Connectivity connectivityValue(uint sourceConnectivity);
DeviceStatus convertDeviceStatus(int sourceDeviceStatus);
ConnectionStatus convertConnectionStatus(int sourceConnectionStatus);
ConnectionStatus convertStateFromNetworkManager(NetworkManager::ActiveConnection::State state);
}
}
#endif  // NETUTILS_H
