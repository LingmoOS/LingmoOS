/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004, 2005 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "avahi-publicservice_p.h"

#include <QCoreApplication>
#include <QStringList>

#include "publicservice.h"

#include <config-kdnssd.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "avahi_entrygroup_interface.h"
#include "avahi_server_interface.h"
#include "servicebrowser.h"

namespace KDNSSD
{
PublicService::PublicService(const QString &name, const QString &type, unsigned int port, const QString &domain, const QStringList &subtypes)
    : QObject()
    , ServiceBase(new PublicServicePrivate(this, name, type, domain, port))
{
    KDNSSD_D;
    if (domain.isNull()) {
        d->m_domain = "local.";
    }
    d->m_subtypes = subtypes;
}

PublicService::~PublicService()
{
    stop();
}

void PublicServicePrivate::tryApply()
{
    if (fillEntryGroup()) {
        commit();
    } else {
        m_parent->stop();
        Q_EMIT m_parent->published(false);
    }
}

void PublicServicePrivate::gotGlobalStateChanged(int state, const QString &error, QDBusMessage msg)
{
    if (!isOurMsg(msg)) {
        return;
    }
    groupStateChanged(state, error);
}

void PublicService::setServiceName(const QString &serviceName)
{
    KDNSSD_D;
    d->m_serviceName = serviceName;
    if (d->m_running) {
        d->m_group->Reset();
        d->tryApply();
    }
}

void PublicService::setDomain(const QString &domain)
{
    KDNSSD_D;
    d->m_domain = domain;
    if (d->m_running) {
        d->m_group->Reset();
        d->tryApply();
    }
}

void PublicService::setType(const QString &type)
{
    KDNSSD_D;
    d->m_type = type;
    if (d->m_running) {
        d->m_group->Reset();
        d->tryApply();
    }
}

void PublicService::setSubTypes(const QStringList &subtypes)
{
    KDNSSD_D;
    d->m_subtypes = subtypes;
    if (d->m_running) {
        d->m_group->Reset();
        d->tryApply();
    }
}

QStringList PublicService::subtypes() const
{
    KDNSSD_D;
    return d->m_subtypes;
}

void PublicService::setPort(unsigned short port)
{
    KDNSSD_D;
    d->m_port = port;
    if (d->m_running) {
        d->m_group->Reset();
        d->tryApply();
    }
}

void PublicService::setTextData(const QMap<QString, QByteArray> &textData)
{
    KDNSSD_D;
    d->m_textData = textData;
    if (d->m_running) {
        d->m_group->Reset();
        d->tryApply();
    }
}

bool PublicService::isPublished() const
{
    KDNSSD_D;
    return d->m_published;
}

bool PublicService::publish()
{
    KDNSSD_D;
    publishAsync();
    while (d->m_running && !d->m_published) {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }
    return d->m_published;
}

void PublicService::stop()
{
    KDNSSD_D;
    if (d->m_group) {
        d->m_group->Reset();
    }
    d->m_running = false;
    d->m_published = false;
}
bool PublicServicePrivate::fillEntryGroup()
{
    registerTypes();
    if (!m_group) {
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
                                             "org.freedesktop.Avahi.EntryGroup",
                                             "StateChanged",
                                             this,
                                             SLOT(gotGlobalStateChanged(int, QString, QDBusMessage)));
        m_dbusObjectPath.clear();

        QDBusReply<QDBusObjectPath> rep = m_server->EntryGroupNew();
        if (!rep.isValid()) {
            return false;
        }

        m_dbusObjectPath = rep.value().path();

        m_group = new org::freedesktop::Avahi::EntryGroup("org.freedesktop.Avahi", m_dbusObjectPath, QDBusConnection::systemBus());
    }
    if (m_serviceName.isNull()) {
        QDBusReply<QString> rep = m_server->GetHostName();
        if (!rep.isValid()) {
            return false;
        }
        m_serviceName = rep.value();
    }

    QList<QByteArray> txt;
    QMap<QString, QByteArray>::ConstIterator itEnd = m_textData.constEnd();
    for (QMap<QString, QByteArray>::ConstIterator it = m_textData.constBegin(); it != itEnd; ++it)
        if (it.value().isNull()) {
            txt.append(it.key().toLatin1());
        } else {
            txt.append(it.key().toLatin1() + '=' + it.value());
        }

    for (;;) {
        QDBusReply<void> ret = m_group->AddService(-1, -1, 0, m_serviceName, m_type, domainToDNS(m_domain), m_hostName, m_port, txt);
        if (ret.isValid()) {
            break;
        }

        // serious error, bail out
        if (ret.error().name() != QLatin1String("org.freedesktop.Avahi.CollisionError")) {
            return false;
        }

        // name collision, try another
        QDBusReply<QString> rep = m_server->GetAlternativeServiceName(m_serviceName);
        if (rep.isValid()) {
            m_serviceName = rep.value();
        } else {
            return false;
        }
    }

    for (const QString &subtype : std::as_const(m_subtypes)) {
        m_group->AddServiceSubtype(-1, -1, 0, m_serviceName, m_type, domainToDNS(m_domain), subtype);
    }
    return true;
}

void PublicServicePrivate::serverStateChanged(int s, const QString &)
{
    if (!m_running) {
        return;
    }
    switch (s) {
    case AVAHI_SERVER_INVALID:
        m_parent->stop();
        Q_EMIT m_parent->published(false);
        break;
    case AVAHI_SERVER_REGISTERING:
    case AVAHI_SERVER_COLLISION:
        if (m_group) {
            m_group->Reset();
        }
        m_collision = true;
        break;
    case AVAHI_SERVER_RUNNING:
        if (m_collision) {
            m_collision = false;
            tryApply();
        }
    }
}

void PublicService::publishAsync()
{
    KDNSSD_D;
    if (d->m_running) {
        stop();
    }

    if (!d->m_server) {
        d->m_server = new org::freedesktop::Avahi::Server(QStringLiteral("org.freedesktop.Avahi"), QStringLiteral("/"), QDBusConnection::systemBus());
        connect(d->m_server, SIGNAL(StateChanged(int, QString)), d, SLOT(serverStateChanged(int, QString)));
    }

    int state = AVAHI_SERVER_INVALID;
    QDBusReply<int> rep = d->m_server->GetState();

    if (rep.isValid()) {
        state = rep.value();
    }
    d->m_running = true;
    d->m_collision = true; // make it look like server is getting out of collision to force registering
    d->serverStateChanged(state, QString());
}

void PublicServicePrivate::groupStateChanged(int s, const QString &reason)
{
    switch (s) {
    case AVAHI_ENTRY_GROUP_COLLISION: {
        QDBusReply<QString> rep = m_server->GetAlternativeServiceName(m_serviceName);
        if (rep.isValid()) {
            m_parent->setServiceName(rep.value());
        } else {
            serverStateChanged(AVAHI_SERVER_INVALID, reason);
        }
        break;
    }
    case AVAHI_ENTRY_GROUP_ESTABLISHED:
        m_published = true;
        Q_EMIT m_parent->published(true);
        break;
    case AVAHI_ENTRY_GROUP_FAILURE:
        serverStateChanged(AVAHI_SERVER_INVALID, reason);
        break;
    }
}

void PublicService::virtual_hook(int, void *)
{
}

}

#include "moc_avahi-publicservice_p.cpp"
#include "moc_publicservice.cpp"
