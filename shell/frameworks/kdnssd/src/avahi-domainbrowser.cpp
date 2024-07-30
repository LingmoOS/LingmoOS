/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 Jakub Stachowski <qbast@go2.pl>
    SPDX-FileCopyrightText: 2018 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "avahi-domainbrowser_p.h"
#include "avahi_domainbrowser_interface.h"
#include "avahi_server_interface.h"
#include "domainbrowser.h"
#include <QFile>
#include <QIODevice>
#include <QSet>
#include <QStandardPaths>
#include <avahi-common/defs.h>

namespace KDNSSD
{
DomainBrowser::DomainBrowser(DomainType type, QObject *parent)
    : QObject(parent)
    , d(new DomainBrowserPrivate(type, this))
{
}

DomainBrowser::~DomainBrowser() = default;

void DomainBrowser::startBrowse()
{
    Q_D(DomainBrowser);
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
                                         "org.freedesktop.Avahi.DomainBrowser",
                                         "ItemNew",
                                         d,
                                         SLOT(gotGlobalItemNew(int, int, QString, uint, QDBusMessage)));
    QDBusConnection::systemBus().connect("org.freedesktop.Avahi",
                                         "",
                                         "org.freedesktop.Avahi.DomainBrowser",
                                         "ItemRemove",
                                         d,
                                         SLOT(gotGlobalItemRemove(int, int, QString, uint, QDBusMessage)));
    QDBusConnection::systemBus()
        .connect("org.freedesktop.Avahi", "", "org.freedesktop.Avahi.DomainBrowser", "AllForNow", d, SLOT(gotGlobalAllForNow(QDBusMessage)));
    d->m_dbusObjectPath.clear();

    org::freedesktop::Avahi::Server s(QStringLiteral("org.freedesktop.Avahi"), QStringLiteral("/"), QDBusConnection::systemBus());
    QDBusReply<QDBusObjectPath> rep =
        s.DomainBrowserNew(-1, -1, QString(), (d->m_type == Browsing) ? AVAHI_DOMAIN_BROWSER_BROWSE : AVAHI_DOMAIN_BROWSER_REGISTER, 0);
    if (!rep.isValid()) {
        return;
    }

    d->m_dbusObjectPath = rep.value().path();

    // This is held because we need to explicitly Free it!
    d->m_browser = new org::freedesktop::Avahi::DomainBrowser(s.service(), d->m_dbusObjectPath, s.connection());

    if (d->m_type == Browsing) {
        QString domains_evar = QString::fromLocal8Bit(qgetenv("AVAHI_BROWSE_DOMAINS"));
        if (!domains_evar.isEmpty()) {
            const QStringList edomains = domains_evar.split(QLatin1Char(':'));
            for (const QString &s : edomains) {
                d->gotNewDomain(-1, -1, s, 0);
            }
        }
        // FIXME: watch this file and restart browser if it changes
        QString confDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
        QFile domains_cfg(confDir + QStringLiteral("/avahi/browse-domains"));
        if (domains_cfg.open(QIODevice::ReadOnly | QIODevice::Text))
            while (!domains_cfg.atEnd()) {
                d->gotNewDomain(-1, -1, QString::fromUtf8(domains_cfg.readLine().data()).trimmed(), 0);
            }
    }
}

void DomainBrowserPrivate::gotGlobalItemNew(int interface, int protocol, const QString &domain, uint flags, QDBusMessage msg)
{
    if (!isOurMsg(msg)) {
        return;
    }
    gotNewDomain(interface, protocol, domain, flags);
}

void DomainBrowserPrivate::gotGlobalItemRemove(int interface, int protocol, const QString &domain, uint flags, QDBusMessage msg)
{
    if (!isOurMsg(msg)) {
        return;
    }
    gotRemoveDomain(interface, protocol, domain, flags);
}

void DomainBrowserPrivate::gotGlobalAllForNow(QDBusMessage msg)
{
    if (!isOurMsg(msg)) {
        return;
    }
}

void DomainBrowserPrivate::gotNewDomain(int, int, const QString &domain, uint)
{
    QString decoded = DNSToDomain(domain);
    if (m_domains.contains(decoded)) {
        return;
    }
    m_domains += decoded;
    Q_EMIT m_parent->domainAdded(decoded);
}

void DomainBrowserPrivate::gotRemoveDomain(int, int, const QString &domain, uint)
{
    QString decoded = DNSToDomain(domain);
    if (!m_domains.contains(decoded)) {
        return;
    }
    Q_EMIT m_parent->domainRemoved(decoded);
    m_domains.remove(decoded);
}

QStringList DomainBrowser::domains() const
{
    Q_D(const DomainBrowser);
    return d->m_domains.values();
}

bool DomainBrowser::isRunning() const
{
    Q_D(const DomainBrowser);
    return d->m_started;
}

}
#include "moc_avahi-domainbrowser_p.cpp"
#include "moc_domainbrowser.cpp"
