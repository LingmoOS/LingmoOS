/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004, 2007 Jakub Stachowski <qbast@go2.pl>
    SPDX-FileCopyrightText: 2018 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "avahi-servicetypebrowser_p.h"
#include "avahi_server_interface.h"
#include "avahi_servicetypebrowser_interface.h"
#include "servicetypebrowser.h"
#include <QSet>

#define UNSPEC -1
namespace KDNSSD
{
ServiceTypeBrowser::ServiceTypeBrowser(const QString &domain, QObject *parent)
    : QObject(parent)
    , d(new ServiceTypeBrowserPrivate(this))
{
    Q_D(ServiceTypeBrowser);
    d->m_domain = domain;
    d->m_timer.setSingleShot(true);
}

ServiceTypeBrowser::~ServiceTypeBrowser() = default;

void ServiceTypeBrowser::startBrowse()
{
    Q_D(ServiceTypeBrowser);
    if (d->m_started) {
        return;
    }
    d->m_started = true;

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
                                         "org.freedesktop.Avahi.ServiceTypeBrowser",
                                         "ItemNew",
                                         d,
                                         SLOT(gotGlobalItemNew(int, int, QString, QString, uint, QDBusMessage)));
    QDBusConnection::systemBus().connect("org.freedesktop.Avahi",
                                         "",
                                         "org.freedesktop.Avahi.ServiceTypeBrowser",
                                         "ItemRemove",
                                         d,
                                         SLOT(gotGlobalItemRemove(int, int, QString, QString, uint, QDBusMessage)));
    QDBusConnection::systemBus()
        .connect("org.freedesktop.Avahi", "", "org.freedesktop.Avahi.ServiceTypeBrowser", "AllForNow", d, SLOT(gotGlobalAllForNow(QDBusMessage)));
    d->m_dbusObjectPath.clear();

    org::freedesktop::Avahi::Server s(QStringLiteral("org.freedesktop.Avahi"), QStringLiteral("/"), QDBusConnection::systemBus());

    QDBusReply<QDBusObjectPath> rep = s.ServiceTypeBrowserNew(-1, -1, d->m_domain, 0);
    if (!rep.isValid()) {
        return;
    }

    d->m_dbusObjectPath = rep.value().path();

    // This is held because we need to explicitly Free it!
    d->m_browser = new org::freedesktop::Avahi::ServiceTypeBrowser(s.service(), d->m_dbusObjectPath, s.connection());

    connect(&d->m_timer, SIGNAL(timeout()), d, SLOT(finished()));
    d->m_timer.start(domainIsLocal(d->m_domain) ? TIMEOUT_LAST_SERVICE : TIMEOUT_START_WAN);
}

void ServiceTypeBrowserPrivate::finished()
{
    m_timer.stop();
    Q_EMIT m_parent->finished();
}

void ServiceTypeBrowserPrivate::gotGlobalItemNew(int interface, int protocol, const QString &type, const QString &domain, uint flags, QDBusMessage msg)
{
    if (!isOurMsg(msg)) {
        return;
    }
    gotNewServiceType(interface, protocol, type, domain, flags);
}

void ServiceTypeBrowserPrivate::gotGlobalItemRemove(int interface, int protocol, const QString &type, const QString &domain, uint flags, QDBusMessage msg)
{
    if (!isOurMsg(msg)) {
        return;
    }
    gotRemoveServiceType(interface, protocol, type, domain, flags);
}

void ServiceTypeBrowserPrivate::gotGlobalAllForNow(QDBusMessage msg)
{
    if (!isOurMsg(msg)) {
        return;
    }
    finished();
}

void ServiceTypeBrowserPrivate::gotNewServiceType(int, int, const QString &type, const QString &, uint)
{
    m_timer.start(TIMEOUT_LAST_SERVICE);
    m_servicetypes += type;
    Q_EMIT m_parent->serviceTypeAdded(type);
}

void ServiceTypeBrowserPrivate::gotRemoveServiceType(int, int, const QString &type, const QString &, uint)
{
    m_timer.start(TIMEOUT_LAST_SERVICE);
    m_servicetypes.removeAll(type);
    Q_EMIT m_parent->serviceTypeRemoved(type);
}

QStringList ServiceTypeBrowser::serviceTypes() const
{
    Q_D(const ServiceTypeBrowser);
    return d->m_servicetypes;
}

}
#include "moc_avahi-servicetypebrowser_p.cpp"
#include "moc_servicetypebrowser.cpp"
