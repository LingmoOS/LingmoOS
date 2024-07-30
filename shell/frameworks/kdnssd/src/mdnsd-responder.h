/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MDNSD_RESPONDER_H
#define MDNSD_RESPONDER_H

#include <QObject>
#include <QSocketNotifier>
#include <dns_sd.h>

namespace KDNSSD
{
/**
This class should not be used directly.

@author Jakub Stachowski
@short Internal class wrapping dns_sd.h interface
 */
class Responder : public QObject
{
    Q_OBJECT

public:
    explicit Responder(DNSServiceRef ref = 0, QObject *parent = nullptr);

    ~Responder();

    /**
    Returns true if it is possible to use mDNS service publishing and discovery.
    It needs mDNSResponder running.
     */
    bool isRunning() const;
    void setRef(DNSServiceRef ref);
    void stop();
public Q_SLOTS:
    void process();

protected:
    DNSServiceRef m_ref;
    bool m_running;
    QSocketNotifier *m_socket;
};

/* Utils functions */

// Encodes domain name using utf8() or IDN
QByteArray domainToDNS(const QString &domain);
QString DNSToDomain(const char *domain);

}

#endif
