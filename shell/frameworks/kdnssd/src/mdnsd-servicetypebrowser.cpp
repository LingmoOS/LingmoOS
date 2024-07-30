/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mdnsd-servicetypebrowser_p.h"
#include "servicebrowser.h"
#include "servicetypebrowser.h"

namespace KDNSSD
{
ServiceTypeBrowser::ServiceTypeBrowser(const QString &domain, QObject *parent)
    : QObject(parent)
    , d(new ServiceTypeBrowserPrivate(this))
{
    Q_D(ServiceTypeBrowser);
    d->m_browser = new ServiceBrowser("_services._dns-sd._udp", false, domain);
    connect(d->m_browser, SIGNAL(serviceAdded(KDNSSD::RemoteService::Ptr)), d, SLOT(newService(KDNSSD::RemoteService::Ptr)));
    connect(d->m_browser, SIGNAL(serviceRemoved(KDNSSD::RemoteService::Ptr)), d, SLOT(removeService(KDNSSD::RemoteService::Ptr)));
    connect(d->m_browser, SIGNAL(finished()), this, SIGNAL(finished()));
}

ServiceTypeBrowser::~ServiceTypeBrowser() = default;

QStringList ServiceTypeBrowser::serviceTypes() const
{
    Q_D(const ServiceTypeBrowser);
    return d->m_servicetypes;
}

void ServiceTypeBrowser::startBrowse()
{
    Q_D(ServiceTypeBrowser);
    d->m_browser->startBrowse();
}

void ServiceTypeBrowserPrivate::newService(KDNSSD::RemoteService::Ptr srv)
{
    QString type = srv->serviceName() + '.' + srv->type();
    m_servicetypes += type;
    Q_EMIT m_parent->serviceTypeAdded(type);
}

void ServiceTypeBrowserPrivate::removeService(KDNSSD::RemoteService::Ptr srv)
{
    QString type = srv->serviceName() + '.' + srv->type();
    m_servicetypes.removeAll(type);
    Q_EMIT m_parent->serviceTypeRemoved(type);
}

}

#include "moc_mdnsd-servicetypebrowser_p.cpp"
#include "moc_servicetypebrowser.cpp"
