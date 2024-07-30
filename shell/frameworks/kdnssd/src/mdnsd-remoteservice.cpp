/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004, 2005 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mdnsd-responder.h"
#include "mdnsd-sdevent.h"
#include "remoteservice.h"
#include "servicebase_p.h"
#include <QCoreApplication>
#include <QDebug>
#include <QEventLoop>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

namespace KDNSSD
{
void resolve_callback(DNSServiceRef,
                      DNSServiceFlags,
                      uint32_t,
                      DNSServiceErrorType errorCode,
                      const char *,
                      const char *hosttarget,
                      uint16_t port,
                      uint16_t txtLen,
                      const unsigned char *txtRecord,
                      void *context);

#define KDNSSD_D RemoteServicePrivate *d = static_cast<RemoteServicePrivate *>(this->d.operator->())

class RemoteServicePrivate : public Responder, public ServiceBasePrivate
{
public:
    RemoteServicePrivate(RemoteService *parent, const QString &name, const QString &type, const QString &domain)
        : Responder()
        , ServiceBasePrivate(name, type, domain, QString(), 0)
        , m_resolved(false)
        , m_parent(parent)
    {
    }
    bool m_resolved;
    RemoteService *m_parent;
    virtual void customEvent(QEvent *event);
};

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
    while (d->isRunning() && !d->m_resolved) {
        d->process();
    }
    d->stop();
    return d->m_resolved;
}

void RemoteService::resolveAsync()
{
    KDNSSD_D;
    if (d->isRunning()) {
        return;
    }
    d->m_resolved = false;
    // qDebug() << this << ":Starting resolve of : " << d->m_serviceName << " " << d->m_type << " " << d->m_domain << "\n";
    DNSServiceRef ref;
    if (DNSServiceResolve(&ref,
                          0,
                          0,
                          d->m_serviceName.toUtf8().constData(),
                          d->m_type.toLatin1().constData(),
                          domainToDNS(d->m_domain).constData(),
                          (DNSServiceResolveReply)resolve_callback,
                          reinterpret_cast<void *>(d))
        == kDNSServiceErr_NoError) {
        d->setRef(ref);
    }
    if (!d->isRunning()) {
        Q_EMIT resolved(false);
    }
}

bool RemoteService::isResolved() const
{
    KDNSSD_D;
    return d->m_resolved;
}

void RemoteServicePrivate::customEvent(QEvent *event)
{
    if (event->type() == QEvent::User + SD_ERROR) {
        stop();
        m_resolved = false;
        Q_EMIT m_parent->resolved(false);
    }
    if (event->type() == QEvent::User + SD_RESOLVE) {
        ResolveEvent *rev = static_cast<ResolveEvent *>(event);
        m_hostName = rev->m_hostname;
        m_port = rev->m_port;
        m_textData = rev->m_txtdata;
        m_resolved = true;
        Q_EMIT m_parent->resolved(true);
    }
}

void RemoteService::virtual_hook(int, void *)
{
    // BASE::virtual_hook(int, void*);
}

void resolve_callback(DNSServiceRef,
                      DNSServiceFlags,
                      uint32_t,
                      DNSServiceErrorType errorCode,
                      const char *,
                      const char *hosttarget,
                      uint16_t port,
                      uint16_t txtLen,
                      const unsigned char *txtRecord,
                      void *context)
{
    QObject *obj = reinterpret_cast<QObject *>(context);
    if (errorCode != kDNSServiceErr_NoError) {
        ErrorEvent err;
        QCoreApplication::sendEvent(obj, &err);
        return;
    }
    char key[256];
    int index = 0;
    unsigned char valueLen;
    // qDebug() << "Resolve callback\n";
    QMap<QString, QByteArray> map;
    const void *voidValue = 0;
    while (TXTRecordGetItemAtIndex(txtLen, txtRecord, index++, 256, key, &valueLen, &voidValue) == kDNSServiceErr_NoError) {
        if (voidValue) {
            map[QString::fromUtf8(key)] = QByteArray((const char *)voidValue, valueLen);
        } else {
            map[QString::fromUtf8(key)].clear();
        }
    }
    ResolveEvent rev(DNSToDomain(hosttarget), ntohs(port), map);
    QCoreApplication::sendEvent(obj, &rev);
}

}

#include "moc_remoteservice.cpp"
