/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "domainbrowser.h"
#include "mdnsd-domainbrowser_p.h"
#include "mdnsd-responder.h"
#include "mdnsd-sdevent.h"
#include "remoteservice.h"

#include <QCoreApplication>
#include <QHash>
#include <QStringList>

namespace KDNSSD
{
void domain_callback(DNSServiceRef, DNSServiceFlags flags, uint32_t, DNSServiceErrorType errorCode, const char *replyDomain, void *context);

DomainBrowser::DomainBrowser(DomainType type, QObject *parent)
    : QObject(parent)
    , d(new DomainBrowserPrivate(type, this))
{
}

DomainBrowser::~DomainBrowser() = default;

void DomainBrowser::startBrowse()
{
    Q_D(DomainBrowser);
    if (d->isRunning()) {
        return;
    }
    DNSServiceRef ref;
    if (DNSServiceEnumerateDomains(&ref,
                                   (d->m_type == Browsing) ? kDNSServiceFlagsBrowseDomains : kDNSServiceFlagsBrowseDomains,
                                   0,
                                   domain_callback,
                                   reinterpret_cast<void *>(d))
        == kDNSServiceErr_NoError) {
        d->setRef(ref);
    }
}

void DomainBrowserPrivate::customEvent(QEvent *event)
{
    if (event->type() == QEvent::User + SD_ERROR) {
        stop();
    }
    if (event->type() == QEvent::User + SD_ADDREMOVE) {
        AddRemoveEvent *aev = static_cast<AddRemoveEvent *>(event);
        if (aev->m_op == AddRemoveEvent::Add) {
            // FIXME: check if domain name is not name+domain (there was some mdnsd weirdness)
            if (m_domains.contains(aev->m_domain)) {
                return;
            }
            m_domains.append(aev->m_domain);
            Q_EMIT m_parent->domainAdded(aev->m_domain);
        } else {
            m_domains.removeAll(aev->m_domain);
            Q_EMIT m_parent->domainRemoved(aev->m_domain);
        }
    }
}

QStringList DomainBrowser::domains() const
{
    Q_D(const DomainBrowser);
    return d->m_domains;
}

bool DomainBrowser::isRunning() const
{
    Q_D(const DomainBrowser);
    return d->isRunning();
}

void domain_callback(DNSServiceRef, DNSServiceFlags flags, uint32_t, DNSServiceErrorType errorCode, const char *replyDomain, void *context)
{
    QObject *obj = reinterpret_cast<QObject *>(context);
    if (errorCode != kDNSServiceErr_NoError) {
        ErrorEvent err;
        QCoreApplication::sendEvent(obj, &err);
    } else {
        // domain browser is supposed to return only _additional_ domains
        if (flags & kDNSServiceFlagsDefault) {
            return;
        }
        AddRemoveEvent arev((flags & kDNSServiceFlagsAdd) ? AddRemoveEvent::Add : AddRemoveEvent::Remove,
                            QString(),
                            QString(),
                            DNSToDomain(replyDomain),
                            !(flags & kDNSServiceFlagsMoreComing));
        QCoreApplication::sendEvent(obj, &arev);
    }
}

}

#include "moc_domainbrowser.cpp"
#include "moc_mdnsd-domainbrowser_p.cpp"
