// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2011 Craig Drummond <craig.p.drummond@gmail.com>
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>
/*
 * UFW KControl Module
 */

#include "rule.h"
#include "appprofiles.h"
#include "firewallclient.h"
#include <KLocalizedString>
#include <QByteArray>
#include <QMap>
#include <QTextStream>
#include <arpa/inet.h>
#include <netdb.h>

#include "firewallclient.h"

// Keep in sync with kcm_ufw_helper.py
static const char *ANY_ADDR = "0.0.0.0/0";
static const char *ANY_ADDR_V6 = "::/0";
static const char *ANY_PORT = "any";

// Shorten an IPv6 address (if applicable)
static QString shortenAddress(const QString &addr)
{
    if (!addr.contains(":")) {
        return addr;
    }
    QByteArray bytes(addr.toLatin1());
    unsigned char num[16];

    if (inet_pton(AF_INET6, bytes.constData(), num) > 0) {
        char conv[41];
        if (NULL != inet_ntop(AF_INET6, num, conv, 41)) {
            return QLatin1String(conv);
        }
    }
    return addr;
}

static QString addIface(const QString &orig, const QString &iface)
{
    return iface.isEmpty() ? orig : i18nc("address on interface", "%1 on %2", orig, iface);
}

static QString serviceName(short port)
{
    static QMap<int, QString> serviceMap;

    if (serviceMap.contains(port)) {
        return serviceMap[port];
    }

    struct servent *ent = getservbyport(htons(port), 0L);

    if (ent && ent->s_name) {
        serviceMap[port] = ent->s_name;
        return serviceMap[port];
    }

    return {};
}

static QString formatPort(const QString &port, int prot)
{
    return port.isEmpty() ? Rule::protocolSuffix(prot, QString()) : port + Rule::protocolSuffix(prot);
}

static QString modifyAddress(const QString &addr, const QString &port)
{
    if (addr.isEmpty() || ANY_ADDR == addr || ANY_ADDR_V6 == addr) {
        return port.isEmpty() ? i18n("Anywhere") : QString();
    }

    return shortenAddress(addr);
}

static QString modifyPort(const QString &port, int prot, bool matchPortNoProto = false)
{
    if (port.isEmpty()) {
        return port;
    }
    // Does it match a pre-configured application?
    Types::PredefinedPort pp = Types::toPredefinedPort(port + Rule::protocolSuffix(prot));

    // When matching glog lines, the protocol is *always* specified - but don't always want this when
    // matching names...
    if (matchPortNoProto && Types::PP_COUNT == pp) {
        pp = Types::toPredefinedPort(port);
    }

    if (Types::PP_COUNT != pp) {
        return i18nc("service/application name (port numbers)", "%1 (%2)", Types::toString(pp, true), port + Rule::protocolSuffix(prot));
    }

    // Is it a service known to /etc/services ???
    bool ok(false);
    QString service;
    short portNum = port.toShort(&ok);

    if (ok) {
        service = serviceName(portNum);
    }

    if (!service.isEmpty()) {
        return i18nc("service/application name (port numbers)", "%1 (%2)", service, formatPort(port, prot));
    }

    // Just return port/servicename and protocol
    return formatPort(port, prot);
}

static QString modifyApp(const QString &app, const QString &port, int prot)
{
    if (app.isEmpty()) {
        return port;
    }

    // TODO: Send the profile, not the app name.
    Entry profile({});
    //     Entry profile(get(app));

    return i18nc("service/application name (port numbers)", "%1 (%2)", app, profile.name.isEmpty() ? formatPort(port, prot) : profile.ports);
}

int Rule::servicePort(const QString &name)
{
    static QMap<QString, int> serviceMap;

    if (serviceMap.contains(name)) {
        return serviceMap[name];
    }

    QByteArray l1 = name.toLatin1();
    struct servent *ent = getservbyname(l1.constData(), 0L);

    if (ent && ent->s_name) {
        serviceMap[name] = ntohs(ent->s_port);
        return serviceMap[name];
    }

    return 0;
}

QString Rule::protocolSuffix(int prot, const QString &sep)
{
    if (FirewallClient::isTcpAndUdp(prot)) {
        return {};
    }

    if (prot == -1) {
        prot = 0;
        qWarning() << "Invalid protocol -1, defaulting to" << FirewallClient::knownProtocols().at(prot);
    }

    return sep + FirewallClient::knownProtocols().at(prot);
}

QString Rule::modify(const QString &address, const QString &port, const QString &application, const QString &iface, int protocol, bool matchPortNoProto)
{
    if ((port == ANY_PORT || port.isEmpty()) && (address.isEmpty() || ANY_ADDR == address || ANY_ADDR_V6 == address))
        return addIface(i18n("Anywhere"), iface);

    bool isAnyAddress = address.isEmpty() || ANY_ADDR == address || ANY_ADDR_V6 == address, isAnyPort = port.isEmpty() || ANY_PORT == port;
    QString bPort = application.isEmpty() ? modifyPort(port, protocol, matchPortNoProto) : modifyApp(application, port, protocol),
            bAddr = modifyAddress(address, port);

    return addIface(isAnyAddress ? isAnyPort ? i18n("Anywhere") : bPort : bAddr.isEmpty() ? bPort : bAddr + QChar(' ') + bPort, iface);
}

Rule::Rule()
    : m_position(0)
    , m_action(Types::POLICY_REJECT)
    , m_incoming(true)
    , m_ipv6(false)
    , m_simplified(true)
    , m_protocol(0)
    , m_logtype(Types::LOGGING_OFF)
    , m_interface(0)
{
}

QString Rule::fromStr() const
{
    /* qDebug() << "Before Crashing" << m_protocol; */
    return modify(m_sourceAddress, m_sourcePort, m_sourceApplication, m_interfaceIn, m_protocol);
}

QString Rule::toStr() const
{
    return modify(m_destAddress, m_destPort, m_destApplication, m_interfaceOut, m_protocol);
}

QString Rule::actionStr() const
{
    return m_incoming ? i18nc("firewallAction incoming", "%1 incoming", Types::toString(m_action, true))
                      : i18nc("firewallAction outgoing", "%1 outgoing", Types::toString(m_action, true));
}

QString Rule::ipV6Str() const
{
    return m_ipv6 ? i18n("Yes") : QString();
}

QString Rule::loggingStr() const
{
    return Types::toString(m_logtype, false);
}

void Rule::setPolicy(const QString &policy)
{
    auto policy_t = Types::toPolicy(policy);

    if (policy_t == action()) {
        return;
    }

    m_action = policy_t;
    Q_EMIT policyChanged(policy);
}

void Rule::setIncoming(bool incoming)
{
    if (m_incoming == incoming) {
        return;
    }

    m_incoming = incoming;
    Q_EMIT incomingChanged(incoming);
}

void Rule::setSourceAddress(const QString &sourceAddress)
{
    if (m_sourceAddress == sourceAddress) {
        return;
    }
    m_sourceAddress = sourceAddress;
    Q_EMIT sourceAddressChanged(sourceAddress);
}

void Rule::setSourcePort(const QString &sourcePort)
{
    if (m_sourcePort == sourcePort) {
        return;
    }

    m_sourcePort = sourcePort;
    Q_EMIT sourcePortChanged(sourcePort);
}

void Rule::setDestinationAddress(const QString &destinationAddress)
{
    if (m_destAddress == destinationAddress) {
        return;
    }
    m_destAddress = destinationAddress;
    Q_EMIT destinationAddressChanged(destinationAddress);
}

void Rule::setDestinationPort(const QString &destinationPort)
{
    if (m_destPort == destinationPort) {
        return;
    }

    m_destPort = destinationPort;
    Q_EMIT destinationPortChanged(destinationPort);
}

void Rule::setIpv6(bool ipv6)
{
    if (m_ipv6 == ipv6) {
        return;
    }

    m_ipv6 = ipv6;
    Q_EMIT ipv6Changed(ipv6);
}

void Rule::setProtocol(int v)
{
    Q_ASSERT(v != -1);
    m_protocol = v;
}

void Rule::setInterface(int interface)
{
    if (m_interface == interface) {
        return;
    }

    m_interfaceStr = interface != 0 ? FirewallClient::knownInterfaces().at(interface) : QString();
    m_interface = interface;

    Q_EMIT interfaceChanged(interface);
}

void Rule::setLogging(const QString &logging)
{
    auto logging_t = Types::toLogging(logging);
    if (m_logtype == logging_t) {
        return;
    }

    m_logtype = logging_t;
    Q_EMIT loggingChanged(logging);
}

void Rule::setPosition(int position)
{
    if (m_position == position) {
        return;
    }

    m_position = position;
    Q_EMIT positionChanged(position);
}

QString Rule::policy() const
{
    return Types::toString(action());
}

QString Rule::destinationAddress() const
{
    return m_destAddress;
}

QString Rule::interfaceStr() const
{
    return m_interfaceStr;
}

int Rule::interface() const
{
    return m_interface;
}

QString Rule::destinationPort() const
{
    return m_destPort;
}

int Rule::position() const
{
    return m_position;
}
Types::Policy Rule::action() const
{
    return m_action;
}
bool Rule::incoming() const
{
    return m_incoming;
}
bool Rule::ipv6() const
{
    return m_ipv6;
}

int Rule::protocol() const
{
    return m_protocol;
}

QString Rule::sourceApplication() const
{
    return m_sourceApplication;
}

QString Rule::sourceAddress() const
{
    return m_sourceAddress;
}
QString Rule::sourcePort() const
{
    return m_sourcePort;
}
QString Rule::interfaceIn() const
{
    return m_interfaceIn;
}
QString Rule::interfaceOut() const
{
    return m_interfaceOut;
}

Types::Logging Rule::logging() const
{
    return m_logtype;
}

void Rule::setV6(const bool v)
{
    m_ipv6 = v;
}

QString Rule::destinationApplication() const
{
    return m_destApplication;
}

void Rule::setSimplified(bool value)
{
    if (m_simplified == value) {
        return;
    }
    m_simplified = value;
    Q_EMIT simplifiedChanged(value);
}

void Rule::setSourceApplication(const QString &app)
{
    if (m_sourceApplication == app) {
        return;
    }
    m_sourceApplication = app;
    Q_EMIT sourceApplicationChanged(app);
}
bool Rule::simplified() const
{
    return m_simplified;
}
