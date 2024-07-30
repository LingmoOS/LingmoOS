// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2011 Craig Drummond <craig.p.drummond@gmail.com>
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

#ifndef UFW_RULE_H
#define UFW_RULE_H

#include <QDebug>
#include <QObject>
#include <QString>
#include <kcm_firewall_core_export.h>
#include <sys/socket.h>

#include "types.h"

class KCM_FIREWALL_CORE_EXPORT Rule : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString policy READ policy WRITE setPolicy NOTIFY policyChanged)
    Q_PROPERTY(bool incoming READ incoming WRITE setIncoming NOTIFY incomingChanged)
    Q_PROPERTY(QString sourceAddress READ sourceAddress WRITE setSourceAddress NOTIFY sourceAddressChanged)
    Q_PROPERTY(QString sourcePort READ sourcePort WRITE setSourcePort NOTIFY sourcePortChanged)
    Q_PROPERTY(QString destinationAddress READ destinationAddress WRITE setDestinationAddress NOTIFY destinationAddressChanged)
    Q_PROPERTY(QString destinationPort READ destinationPort WRITE setDestinationPort NOTIFY destinationPortChanged)
    Q_PROPERTY(bool ipv6 READ ipv6 WRITE setIpv6 NOTIFY ipv6Changed)
    Q_PROPERTY(int protocol READ protocol WRITE setProtocol NOTIFY protocolChanged)
    Q_PROPERTY(int interface READ interface WRITE setInterface NOTIFY interfaceChanged)
    Q_PROPERTY(QString logging READ loggingStr WRITE setLogging NOTIFY loggingChanged)
    Q_PROPERTY(int position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(bool simplified READ simplified WRITE setSimplified NOTIFY simplifiedChanged)
    Q_PROPERTY(QString sourceApplication READ sourceApplication WRITE setSourceApplication NOTIFY sourceApplicationChanged)

public:
    static int servicePort(const QString &name);
    static QString protocolSuffix(int prot, const QString &sep = QString("/"));
    static QString
    modify(const QString &address, const QString &port, const QString &application, const QString &iface, int protocol, bool matchPortNoProto = false);

    Rule();
    Rule(Types::Policy pol,
         bool incomming,
         Types::Logging log,
         int protocolIdx,
         const QString &srcHost = QString(),
         const QString &srcPort = QString(),
         const QString &destHost = QString(),
         const QString &destPort = QString(),
         const QString &ifaceIn = QString(),
         const QString &ifaceOut = QString(),
         const QString &srcApp = QString(),
         const QString &destApp = QString(),
         unsigned int i = 0,
         bool ipv6 = false,
         bool simplified = true)
        : m_position(i)
        , m_action(pol)
        , m_incoming(incomming)
        , m_ipv6(ipv6)
        , m_simplified(simplified)
        , m_protocol(protocolIdx)
        , m_logtype(log)
        , m_destApplication(destApp)
        , m_sourceApplication(srcApp)
        , m_destAddress(destHost)
        , m_sourceAddress(srcHost)
        , m_destPort(destPort)
        , m_sourcePort(srcPort)
        , m_interfaceIn(ifaceIn)
        , m_interfaceOut(ifaceOut)
        , m_interface(0)
    {
    }
    Rule(const Rule &rhs, QObject *parent)
        : QObject(parent)
        , m_position(rhs.m_position)
        , m_action(rhs.m_action)
        , m_incoming(rhs.m_incoming)
        , m_ipv6(rhs.m_ipv6)
        , m_simplified(true)
        , m_protocol(rhs.m_protocol)
        , m_logtype(rhs.m_logtype)
        , m_destApplication(rhs.m_destApplication)
        , m_sourceApplication(rhs.m_sourceApplication)
        , m_destAddress(rhs.m_destAddress)
        , m_sourceAddress(rhs.m_sourceAddress)
        , m_destPort(rhs.m_destPort)
        , m_sourcePort(rhs.m_sourcePort)
        , m_interfaceIn(rhs.m_interfaceIn)
        , m_interfaceOut(rhs.m_interfaceIn)
        , m_interface(rhs.m_interface){};

    QString toStr() const;
    QString fromStr() const;
    QString actionStr() const;
    QString loggingStr() const;
    QString policy() const;
    QString destinationAddress() const;
    QString destinationPort() const;
    QString sourceApplication() const;
    QString sourceAddress() const;
    QString sourcePort() const;
    QString interfaceIn() const;
    QString interfaceOut() const;
    QString destinationApplication() const;
    Types::Logging logging() const;

    // TODO: are they really necessary?
    QString ipV6Str() const;
    QString interfaceStr() const;

    bool incoming() const;
    bool ipv6() const;
    bool simplified() const;
    int interface() const;
    int position() const;
    int protocol() const;
    Types::Policy action() const;

    // 'different' is used in the EditRule dialog to know whether the rule has actually changed...
    bool different(const Rule &o) const
    {
        return m_logtype != o.m_logtype /*|| description!=o.description*/ || !(*this == o);
    }

    bool operator==(const Rule &o) const
    {
        return m_action == o.m_action && m_incoming == o.m_incoming && m_ipv6 == o.m_ipv6 && m_protocol == o.m_protocol
            && m_destApplication == o.m_destApplication && m_sourceApplication == o.m_sourceApplication && m_destAddress == o.m_destAddress
            && m_sourceAddress == o.m_sourceAddress && (m_destApplication.isEmpty() && o.m_destApplication.isEmpty() ? m_destPort == o.m_destPort : true)
            && (m_sourceApplication.isEmpty() && o.m_sourceApplication.isEmpty() ? m_sourcePort == o.m_sourcePort : true) && m_interfaceIn == o.m_interfaceIn
            && m_interfaceOut == o.m_interfaceOut;
    }

public slots:
    void setPolicy(const QString &policy);
    void setIncoming(bool incoming);
    void setSourceAddress(const QString &sourceAddress);
    void setSourcePort(const QString &sourcePort);
    void setDestinationAddress(const QString &destinationAddress);
    void setDestinationPort(const QString &destinationPort);
    void setIpv6(bool ipv6);
    void setProtocol(int protocol);
    void setInterface(int interface);
    void setLogging(const QString &logging);
    void setV6(const bool v);
    void setPosition(int position);
    void setSimplified(bool value);
    void setSourceApplication(const QString &app);

Q_SIGNALS:
    void policyChanged(const QString &policy);
    void directionChanged(const QString &direction);
    void sourceAddressChanged(const QString &sourceAddress);
    void sourcePortChanged(const QString &sourcePort);
    void destinationAddressChanged(const QString &destinationAddress);
    void destinationPortChanged(const QString &destinationPort);
    void ipv6Changed(bool ipv6);
    void protocolChanged(int protocol);
    void interfaceChanged(int interface);
    void loggingChanged(const QString &logging);
    void incomingChanged(bool incoming);
    void positionChanged(int position);
    void simplifiedChanged(bool value);
    void sourceApplicationChanged(const QString &app);

private:
    int m_position;
    Types::Policy m_action;
    bool m_incoming, m_ipv6, m_simplified;
    int m_protocol;
    Types::Logging m_logtype;
    QString m_destApplication;
    QString m_sourceApplication;
    QString m_destAddress;
    QString m_sourceAddress;
    QString m_destPort;
    QString m_sourcePort;
    QString m_interfaceIn;
    QString m_interfaceOut;
    QString m_interfaceStr;
    int m_interface;
};

#endif
