/*
    SPDX-FileCopyrightText: 2011-2013 Lamarque V. Souza <lamarque@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <arpa/inet.h>

#include <QTextStream>

#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/Manager>

QString typeAsString(const int type)
{
    switch (type) {
    case 0x0:
        return QString("Unknown");
    case 0x1:
        return QString("Ethernet");
    case 0x2:
        return QString("Wifi");
    case 0x3:
        return QString("Unused1");
    case 0x4:
        return QString("Unused2");
    case 0x5:
        return QString("Bluetooth");
    case 0x6:
        return QString("OlpcMesh");
    case 0x7:
        return QString("Wimax");
    case 0x8:
        return QString("Modem");
    }
    return QString("Unknown");
}

int main()
{
    QTextStream qout(stdout, QIODevice::WriteOnly);

    const NetworkManager::Device::List list = NetworkManager::networkInterfaces();

    // List device configuration, not including vpn connections, which do not
    // have a real device tied to them.
    for (const NetworkManager::Device::Ptr &dev : list) {
        qout << "\n=====\n";
        qout << dev->uni() << "\n";
        qout << "type: " << typeAsString(dev->type()) << "\n";
        qout << "managed: " << dev->managed() << "\n";
        qout << "interface name: " << dev->interfaceName() << "\n";

        NetworkManager::IpConfig ipConfig = dev->ipV4Config();
        if (ipConfig.isValid()) {
            // static IPv4 configuration.
            if (ipConfig.addresses().isEmpty()) {
                qout << "ip address: <not set>\n";
            } else {
                NetworkManager::IpAddress address = ipConfig.addresses().at(0);
                qout << "ip address: " << address.ip().toString() << "\n";
                qout << "gateway: " << address.gateway().toString() << "\n";
                qout << "ip address (raw): " << dev->ipV4Address().toString() << "\n";

                // Static routes.
                if (ipConfig.routes().isEmpty()) {
                    qout << "routers: <not set>\n";
                } else {
                    qout << "routers: " << ipConfig.routes().at(0).ip().toString() << "\n";
                }

                if (ipConfig.nameservers().isEmpty()) {
                    qout << "nameserver: <not set>\n";
                } else {
                    qout << "nameserver: " << ipConfig.nameservers().at(0).toString() << "\n";
                }
            }
            // DHCPv4 configuration.
            NetworkManager::Dhcp4Config::Ptr dhcp4Config = dev->dhcp4Config();
            if (!dhcp4Config) {
                qout << "dhcp info unavailable\n";
            } else {
                qout << "Dhcp4 options (" << dhcp4Config->path() << "): ";
                QVariantMap options = dhcp4Config->options();
                QVariantMap::ConstIterator it = options.constBegin();
                QVariantMap::ConstIterator end = options.constEnd();
                for (; it != end; ++it) {
                    qout << it.key() << "=" << it.value().toString() << " ";
                }
                qout << "\n";

                qout << "(dhcp) ip address: " << dhcp4Config->optionValue("ip_address") << "\n";
                qout << "(dhcp) network: " << dhcp4Config->optionValue("network_number") << '/' << dhcp4Config->optionValue("subnet_cidr") << " ("
                     << dhcp4Config->optionValue("subnet_mask") << ")\n";

                if (dhcp4Config->optionValue("routers").isEmpty()) {
                    qout << "(dhcp) gateway(s): <not set>\n";
                } else {
                    qout << "(dhcp) gateway(s): " << dhcp4Config->optionValue("routers") << "\n";
                }

                if (dhcp4Config->optionValue("domain_name_servers").isEmpty()) {
                    qout << "(dhcp) domain name server(s): <not set>\n";
                } else {
                    qout << "(dhcp) domain name server(s): " << dhcp4Config->optionValue("domain_name_servers") << "\n";
                }
            }
        }

        const NetworkManager::Connection::List connections = dev->availableConnections();

        qout << "available connections: ";

        for (const NetworkManager::Connection::Ptr &con : connections) {
            qout << "con";
            NetworkManager::ConnectionSettings::Ptr settings = con->settings();
            qout << "\"" << settings->id() << "\" ";
        }
    }
    qout << "\n";
}
