/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004, 2005 Jakub Stachowski <qbast@go2.pl>
    SPDX-FileCopyrightText: 2018 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "avahi-remoteservice_p.h"
#include "avahi_server_interface.h"
#include "avahi_serviceresolver_interface.h"
#include "remoteservice.h"
#include <QCoreApplication>
#include <QDebug>
#include <QEventLoop>
#include <netinet/in.h>
namespace KDNSSD
{
RemoteService::RemoteService(const QString &name, const QString &type, const QString &domain)
    : ServiceBase(new RemoteServicePrivate(this, name, type, domain))
{
}

RemoteService::~RemoteService()
{
}

bool RemoteService::resolve()
{
    KDNSSD_D;
    resolveAsync();
    while (d->m_running && !d->m_resolved) {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }
    return d->m_resolved;
}

void RemoteService::resolveAsync()
{
    KDNSSD_D;
    if (d->m_running) {
        return;
    }
    d->m_resolved = false;
    registerTypes();

    // Do not race!
    // https://github.com/lathiat/avahi/issues/9
    // Avahi's DBus API is incredibly racey with signals getting fired
    // immediately after a request was made even though we may not yet be
    // listening. In lieu of a proper upstream fix for this we'll unfortunately
    // have to resort to this hack:
    // We register to all signals regardless of path and then filter them once
    // we know what "our" path is. This is much more fragile than a proper
    // QDBusInterface assisted signal connection but unfortunately the only way
    // we can reliably prevent signals getting lost in the race.
    // This uses a fancy trick whereby using QDBusMessage as last argument will
    // give us the correct signal argument types as well as the underlying
    // message so that we may check the message path.
    QDBusConnection::systemBus().connect(
        "org.freedesktop.Avahi",
        "",
        "org.freedesktop.Avahi.ServiceResolver",
        "Found",
        d,
        SLOT(gotGlobalFound(int, int, QString, QString, QString, QString, int, QString, ushort, QList<QByteArray>, uint, QDBusMessage)));
    QDBusConnection::systemBus()
        .connect("org.freedesktop.Avahi", "", "org.freedesktop.Avahi.ServiceResolver", "Failure", d, SLOT(gotGlobalError(QDBusMessage)));
    d->m_dbusObjectPath.clear();

    // qDebug() << this << ":Starting resolve of : " << d->m_serviceName << " " << d->m_type << " " << d->m_domain << "\n";
    org::freedesktop::Avahi::Server s(QStringLiteral("org.freedesktop.Avahi"), QStringLiteral("/"), QDBusConnection::systemBus());
    // FIXME: don't use LOOKUP_NO_ADDRESS if NSS unavailable
    QDBusReply<QDBusObjectPath> rep = s.ServiceResolverNew(-1, -1, d->m_serviceName, d->m_type, domainToDNS(d->m_domain), -1, 8 /*AVAHI_LOOKUP_NO_ADDRESS*/);
    if (!rep.isValid()) {
        Q_EMIT resolved(false);
        return;
    }

    d->m_dbusObjectPath = rep.value().path();

    // This is held because we need to explicitly Free it!
    d->m_resolver = new org::freedesktop::Avahi::ServiceResolver(s.service(), d->m_dbusObjectPath, s.connection());
    d->m_running = true;
}

bool RemoteService::isResolved() const
{
    KDNSSD_D;
    return d->m_resolved;
}

void RemoteServicePrivate::gotError()
{
    m_resolved = false;
    stop();

    Q_EMIT m_parent->resolved(false);
}

void RemoteServicePrivate::gotGlobalFound(int interface,
                                          int protocol,
                                          const QString &name,
                                          const QString &type,
                                          const QString &domain,
                                          const QString &host,
                                          int aprotocol,
                                          const QString &address,
                                          ushort port,
                                          const QList<QByteArray> &txt,
                                          uint flags,
                                          QDBusMessage msg)
{
    if (!isOurMsg(msg)) {
        return;
    }
    gotFound(interface, protocol, name, type, domain, host, aprotocol, address, port, txt, flags);
}

void RemoteServicePrivate::gotGlobalError(QDBusMessage msg)
{
    if (!isOurMsg(msg)) {
        return;
    }
    gotError();
}

void RemoteServicePrivate::gotFound(int,
                                    int,
                                    const QString &name,
                                    const QString &,
                                    const QString &domain,
                                    const QString &host,
                                    int,
                                    const QString &,
                                    ushort port,
                                    const QList<QByteArray> &txt,
                                    uint)
{
    m_serviceName = name;
    m_hostName = host;
    m_port = port;
    m_domain = DNSToDomain(domain);
    for (const QByteArray &x : txt) {
        int pos = x.indexOf("=");
        if (pos == -1) {
            m_textData[x] = QByteArray();
        } else {
            m_textData[x.mid(0, pos)] = x.mid(pos + 1, x.size() - pos);
        }
    }
    m_resolved = true;
    Q_EMIT m_parent->resolved(true);
}

void RemoteServicePrivate::stop()
{
    if (m_resolver) {
        m_resolver->Free();
    }
    delete m_resolver;
    m_resolver = nullptr;
    m_running = false;
}

void RemoteService::virtual_hook(int, void *)
{
    // BASE::virtual_hook(int, void*);
}

}

#include "moc_avahi-remoteservice_p.cpp"
#include "moc_remoteservice.cpp"
