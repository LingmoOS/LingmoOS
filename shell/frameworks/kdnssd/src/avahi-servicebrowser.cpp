/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 Jakub Stachowski <qbast@go2.pl>
    SPDX-FileCopyrightText: 2018 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "avahi-servicebrowser_p.h"
#include "avahi_server_interface.h"
#include "avahi_servicebrowser_interface.h"
#include "servicebrowser.h"
#include <QHash>
#include <QHostAddress>
#include <QStringList>

namespace KDNSSD
{
ServiceBrowser::ServiceBrowser(const QString &type, bool autoResolve, const QString &domain, const QString &subtype)
    : d(new ServiceBrowserPrivate(this))
{
    Q_D(ServiceBrowser);
    d->m_type = type;
    d->m_subtype = subtype;
    d->m_autoResolve = autoResolve;
    d->m_domain = domain;
    d->m_timer.setSingleShot(true);
}

ServiceBrowser::State ServiceBrowser::isAvailable()
{
    org::freedesktop::Avahi::Server s(QStringLiteral("org.freedesktop.Avahi"), QStringLiteral("/"), QDBusConnection::systemBus());
    QDBusReply<int> rep = s.GetState();
    return (rep.isValid() && rep.value() == 2) ? Working : Stopped;
}

ServiceBrowser::~ServiceBrowser() = default;

bool ServiceBrowser::isAutoResolving() const
{
    Q_D(const ServiceBrowser);
    return d->m_autoResolve;
}

void ServiceBrowser::startBrowse()
{
    Q_D(ServiceBrowser);
    if (d->m_running) {
        return;
    }

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
    QDBusConnection::systemBus().connect("org.freedesktop.Avahi",
                                         "",
                                         "org.freedesktop.Avahi.ServiceBrowser",
                                         "ItemNew",
                                         d,
                                         SLOT(gotGlobalItemNew(int, int, QString, QString, QString, uint, QDBusMessage)));
    QDBusConnection::systemBus().connect("org.freedesktop.Avahi",
                                         "",
                                         "org.freedesktop.Avahi.ServiceBrowser",
                                         "ItemRemove",
                                         d,
                                         SLOT(gotGlobalItemRemove(int, int, QString, QString, QString, uint, QDBusMessage)));
    QDBusConnection::systemBus()
        .connect("org.freedesktop.Avahi", "", "org.freedesktop.Avahi.ServiceBrowser", "AllForNow", d, SLOT(gotGlobalAllForNow(QDBusMessage)));
    d->m_dbusObjectPath.clear();

    org::freedesktop::Avahi::Server s(QStringLiteral("org.freedesktop.Avahi"), QStringLiteral("/"), QDBusConnection::systemBus());

    QString fullType = d->m_type;
    if (!d->m_subtype.isEmpty()) {
        fullType = d->m_subtype + QStringLiteral("._sub.") + d->m_type;
    }
    QDBusReply<QDBusObjectPath> rep = s.ServiceBrowserNew(-1, -1, fullType, domainToDNS(d->m_domain), 0);
    if (!rep.isValid()) {
        Q_EMIT finished();
        return;
    }

    d->m_dbusObjectPath = rep.value().path();
    d->m_running = true;
    d->m_browserFinished = true;

    // This is held because we need to explicitly Free it!
    d->m_browser = new org::freedesktop::Avahi::ServiceBrowser(s.service(), d->m_dbusObjectPath, s.connection());

    connect(&d->m_timer, &QTimer::timeout, d, &ServiceBrowserPrivate::browserFinished);
    d->m_timer.start(domainIsLocal(d->m_domain) ? TIMEOUT_LAST_SERVICE : TIMEOUT_START_WAN);
}

void ServiceBrowserPrivate::serviceResolved(bool success)
{
    QObject *sender_obj = const_cast<QObject *>(sender());
    RemoteService *svr = static_cast<RemoteService *>(sender_obj);
    disconnect(svr, SIGNAL(resolved(bool)), this, SLOT(serviceResolved(bool)));
    QList<RemoteService::Ptr>::Iterator it = m_duringResolve.begin();
    QList<RemoteService::Ptr>::Iterator itEnd = m_duringResolve.end();
    while (it != itEnd && svr != (*it).data()) {
        ++it;
    }
    if (it != itEnd) {
        if (success) {
            m_services += (*it);
            Q_EMIT m_parent->serviceAdded(RemoteService::Ptr(svr));
        }
        m_duringResolve.erase(it);
        queryFinished();
    }
}

void ServiceBrowserPrivate::gotGlobalItemNew(int interface,
                                             int protocol,
                                             const QString &name,
                                             const QString &type,
                                             const QString &domain,
                                             uint flags,
                                             QDBusMessage msg)
{
    if (!isOurMsg(msg)) {
        return;
    }
    gotNewService(interface, protocol, name, type, domain, flags);
}

void ServiceBrowserPrivate::gotGlobalItemRemove(int interface,
                                                int protocol,
                                                const QString &name,
                                                const QString &type,
                                                const QString &domain,
                                                uint flags,
                                                QDBusMessage msg)
{
    if (!isOurMsg(msg)) {
        return;
    }
    gotRemoveService(interface, protocol, name, type, domain, flags);
}

void ServiceBrowserPrivate::gotGlobalAllForNow(QDBusMessage msg)
{
    if (!isOurMsg(msg)) {
        return;
    }
    browserFinished();
}

RemoteService::Ptr ServiceBrowserPrivate::find(RemoteService::Ptr s, const QList<RemoteService::Ptr> &where) const
{
    for (const RemoteService::Ptr &i : where)
        if (*s == *i) {
            return i;
        }
    return RemoteService::Ptr();
}

void ServiceBrowserPrivate::gotNewService(int, int, const QString &name, const QString &type, const QString &domain, uint)
{
    m_timer.start(TIMEOUT_LAST_SERVICE);
    RemoteService::Ptr svr(new RemoteService(name, type, domain));
    if (m_autoResolve) {
        connect(svr.data(), SIGNAL(resolved(bool)), this, SLOT(serviceResolved(bool)));
        m_duringResolve += svr;
        svr->resolveAsync();
    } else {
        m_services += svr;
        Q_EMIT m_parent->serviceAdded(svr);
    }
}

void ServiceBrowserPrivate::gotRemoveService(int, int, const QString &name, const QString &type, const QString &domain, uint)
{
    m_timer.start(TIMEOUT_LAST_SERVICE);
    RemoteService::Ptr tmpl(new RemoteService(name, type, domain));
    RemoteService::Ptr found = find(tmpl, m_duringResolve);
    if (found) {
        m_duringResolve.removeAll(found);
        return;
    }
    found = find(tmpl, m_services);
    if (!found) {
        return;
    }

    Q_EMIT m_parent->serviceRemoved(found);
    m_services.removeAll(found);
}
void ServiceBrowserPrivate::browserFinished()
{
    m_timer.stop();
    m_browserFinished = true;
    queryFinished();
}

void ServiceBrowserPrivate::queryFinished()
{
    if (!m_duringResolve.count() && m_browserFinished) {
        Q_EMIT m_parent->finished();
    }
}

QList<RemoteService::Ptr> ServiceBrowser::services() const
{
    Q_D(const ServiceBrowser);
    return d->m_services;
}

void ServiceBrowser::virtual_hook(int, void *)
{
}

QHostAddress ServiceBrowser::resolveHostName(const QString &hostname)
{
    org::freedesktop::Avahi::Server s(QStringLiteral("org.freedesktop.Avahi"), QStringLiteral("/"), QDBusConnection::systemBus());

    int protocol = 0;
    QString name;
    int aprotocol = 0;
    QString address;
    uint flags = 0;

    QDBusReply<int> reply = s.ResolveHostName(-1, -1, hostname, 0, (unsigned int)0, protocol, name, aprotocol, address, flags);

    if (reply.isValid()) {
        return QHostAddress(address);
    } else {
        return QHostAddress();
    }
}

QString ServiceBrowser::getLocalHostName()
{
    org::freedesktop::Avahi::Server s(QStringLiteral("org.freedesktop.Avahi"), QStringLiteral("/"), QDBusConnection::systemBus());

    QDBusReply<QString> reply = s.GetHostName();

    if (reply.isValid()) {
        return reply.value();
    } else {
        return QString();
    }
}

}

#include "moc_avahi-servicebrowser_p.cpp"
#include "moc_servicebrowser.cpp"
