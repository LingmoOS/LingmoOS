/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "domainbrowser.h"
#include "mdnsd-responder.h"
#include "mdnsd-sdevent.h"
#include "mdnsd-servicebrowser_p.h"
#include "remoteservice.h"
#include "servicebrowser.h"
#include <QCoreApplication>
#include <QHash>
#include <QHostInfo>
#include <QStringList>
#include <QTimer>
#include <dns_sd.h>

#define TIMEOUT_WAN 2000
#define TIMEOUT_LAN 200

namespace KDNSSD
{
void query_callback(DNSServiceRef,
                    DNSServiceFlags flags,
                    uint32_t,
                    DNSServiceErrorType errorCode,
                    const char *serviceName,
                    const char *regtype,
                    const char *replyDomain,
                    void *context);

ServiceBrowser::ServiceBrowser(const QString &type, bool autoResolve, const QString &domain, const QString &subtype)
    : d(new ServiceBrowserPrivate(this))
{
    Q_D(ServiceBrowser);
    d->m_type = type;
    d->m_autoResolve = autoResolve;
    d->m_domain = domain;
    d->m_subtype = subtype;
    d->timeout.setSingleShot(true);
    connect(&d->timeout, SIGNAL(timeout()), d, SLOT(onTimeout()));
}

ServiceBrowser::State ServiceBrowser::isAvailable()
{
    //  DNSServiceRef ref;
    //  bool ok (DNSServiceCreateConnection(&ref)==kDNSServiceErr_NoError);
    //  if (ok) DNSServiceRefDeallocate(ref);
    //  return (ok) ? Working : Stopped;
    return Working;
}

ServiceBrowser::~ServiceBrowser() = default;

bool ServiceBrowser::isAutoResolving() const
{
    Q_D(const ServiceBrowser);
    return d->m_autoResolve;
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

void ServiceBrowser::startBrowse()
{
    Q_D(ServiceBrowser);
    if (d->isRunning()) {
        return;
    }
    d->m_finished = false;
    DNSServiceRef ref;
    QString fullType = d->m_type;
    if (!d->m_subtype.isEmpty()) {
        fullType = d->m_subtype + "._sub." + d->m_type;
    }
    if (DNSServiceBrowse(&ref, 0, 0, fullType.toLatin1().constData(), domainToDNS(d->m_domain).constData(), query_callback, reinterpret_cast<void *>(d))
        == kDNSServiceErr_NoError) {
        d->setRef(ref);
    }
    if (!d->isRunning()) {
        Q_EMIT finished();
    } else {
        d->timeout.start(domainIsLocal(d->m_domain) ? TIMEOUT_LAN : TIMEOUT_WAN);
    }
}

void ServiceBrowserPrivate::queryFinished()
{
    if (!m_duringResolve.count() && m_finished) {
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

RemoteService::Ptr ServiceBrowserPrivate::find(RemoteService::Ptr s, const QList<RemoteService::Ptr> &where) const
{
    for (const RemoteService::Ptr &i : where)
        if (*s == *i) {
            return i;
        }
    return RemoteService::Ptr();
}

void ServiceBrowserPrivate::customEvent(QEvent *event)
{
    if (event->type() == QEvent::User + SD_ERROR) {
        stop();
        m_finished = false;
        queryFinished();
    }
    if (event->type() == QEvent::User + SD_ADDREMOVE) {
        AddRemoveEvent *aev = static_cast<AddRemoveEvent *>(event);
        // m_type has useless trailing dot
        RemoteService::Ptr svr(new RemoteService(aev->m_name, aev->m_type.left(aev->m_type.length() - 1), aev->m_domain));
        if (aev->m_op == AddRemoveEvent::Add) {
            if (m_autoResolve) {
                connect(svr.data(), SIGNAL(resolved(bool)), this, SLOT(serviceResolved(bool)));
                m_duringResolve += svr;
                svr->resolveAsync();
            } else {
                m_services += svr;
                Q_EMIT m_parent->serviceAdded(svr);
            }
        } else {
            RemoteService::Ptr found = find(svr, m_duringResolve);
            if (found) {
                m_duringResolve.removeAll(found);
            } else {
                found = find(svr, m_services);
                if (found) {
                    Q_EMIT m_parent->serviceRemoved(found);
                    m_services.removeAll(found);
                }
            }
        }
        m_finished = aev->m_last;
        if (m_finished) {
            queryFinished();
        }
    }
}

void ServiceBrowserPrivate::onTimeout()
{
    m_finished = true;
    queryFinished();
}

void query_callback(DNSServiceRef,
                    DNSServiceFlags flags,
                    uint32_t,
                    DNSServiceErrorType errorCode,
                    const char *serviceName,
                    const char *regtype,
                    const char *replyDomain,
                    void *context)
{
    QObject *obj = reinterpret_cast<QObject *>(context);
    if (errorCode != kDNSServiceErr_NoError) {
        ErrorEvent err;
        QCoreApplication::sendEvent(obj, &err);
    } else {
        AddRemoveEvent arev((flags & kDNSServiceFlagsAdd) ? AddRemoveEvent::Add : AddRemoveEvent::Remove,
                            QString::fromUtf8(serviceName),
                            regtype,
                            DNSToDomain(replyDomain),
                            !(flags & kDNSServiceFlagsMoreComing));
        QCoreApplication::sendEvent(obj, &arev);
    }
}

// TODO: Please Implement Me - Using a KResolver (if not natively)
QHostAddress ServiceBrowser::resolveHostName(const QString &hostname)
{
    return QHostAddress();
}

QString ServiceBrowser::getLocalHostName()
{
    return QHostInfo::localHostName();
}

}

#include "moc_mdnsd-servicebrowser_p.cpp"
#include "moc_servicebrowser.cpp"
