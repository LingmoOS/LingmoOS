#include "vpn.h"

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/AccessPoint>
#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/Setting>
#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/Utils>
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/GsmSetting>
#include <NetworkManagerQt/WiredSetting>
#include <NetworkManagerQt/WirelessSetting>
#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/Ipv4Setting>

#include <NetworkManagerQt/VpnSetting>

VPN::VPN(QObject *parent)
    : QObject(parent)
{

}

void VPN::activateVPNConnection(const QString &connection, const QString &device, const QString &specificObject)
{
}
