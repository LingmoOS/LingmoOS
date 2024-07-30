/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mdnsd-responder.h"
#include "servicebase.h"
#include <QCoreApplication>
#include <QUrl>

namespace KDNSSD
{
Responder::Responder(DNSServiceRef ref, QObject *parent)
    : QObject(parent)
    , m_ref(0)
    , m_socket(0)
{
    setRef(ref);
}

void Responder::setRef(DNSServiceRef ref)
{
    if (m_socket || m_ref) {
        stop();
    }
    m_running = false;
    m_ref = ref;
    if (m_ref == 0) {
        return;
    }
    int fd = DNSServiceRefSockFD(ref);
    if (fd == -1) {
        return;
    }
    m_socket = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(m_socket, SIGNAL(activated(int)), this, SLOT(process()));
    m_running = true;
}
Responder::~Responder()
{
    stop();
}

void Responder::stop()
{
    delete m_socket;
    m_socket = 0;
    if (m_ref) {
        DNSServiceRefDeallocate(m_ref);
    }
    m_ref = 0;
    m_running = false;
}

void Responder::process()
{
    if (DNSServiceProcessResult(m_ref) != kDNSServiceErr_NoError) {
        stop();
    }
}

bool Responder::isRunning() const
{
    return m_running;
}

QByteArray domainToDNS(const QString &domain)
{
    if (domainIsLocal(domain)) {
        return domain.toUtf8();
    } else {
        return QUrl::toAce(domain);
    }
}

QString DNSToDomain(const char *domain)
{
    if (domainIsLocal(domain)) {
        return QString::fromUtf8(domain);
    } else {
        return QUrl::fromAce(domain);
    }
}

}

#include "moc_mdnsd-responder.cpp"
