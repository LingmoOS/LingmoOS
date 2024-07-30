/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004, 2005 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mdnsd-responder.h"
#include "mdnsd-sdevent.h"
#include "publicservice.h"
#include "servicebase_p.h"
#include <QCoreApplication>
#include <QStringList>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#define KDNSSD_D PublicServicePrivate *d = static_cast<PublicServicePrivate *>(this->d.operator->())

namespace KDNSSD
{
void publish_callback(DNSServiceRef, DNSServiceFlags, DNSServiceErrorType errorCode, const char *name, const char *, const char *, void *context);
class PublicServicePrivate : public Responder, public ServiceBasePrivate
{
public:
    PublicServicePrivate(PublicService *parent, const QString &name, const QString &type, unsigned int port, const QString &domain)
        : Responder()
        , ServiceBasePrivate(name, type, domain, QString(), port)
        , m_published(false)
        , m_parent(parent)
    {
    }
    bool m_published;
    PublicService *m_parent;
    QStringList m_subtypes;
    virtual void customEvent(QEvent *event);
};

PublicService::PublicService(const QString &name, const QString &type, unsigned int port, const QString &domain, const QStringList &subtypes)
    : QObject()
    , ServiceBase(new PublicServicePrivate(this, name, type, port, domain))
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

void PublicService::setServiceName(const QString &serviceName)
{
    KDNSSD_D;
    d->m_serviceName = serviceName;
    if (d->isRunning()) {
        stop();
        publishAsync();
    }
}

void PublicService::setDomain(const QString &domain)
{
    KDNSSD_D;
    d->m_domain = domain;
    if (d->isRunning()) {
        stop();
        publishAsync();
    }
}

QStringList PublicService::subtypes() const
{
    KDNSSD_D;
    return d->m_subtypes;
}

void PublicService::setType(const QString &type)
{
    KDNSSD_D;
    d->m_type = type;
    if (d->isRunning()) {
        stop();
        publishAsync();
    }
}

void PublicService::setSubTypes(const QStringList &subtypes)
{
    KDNSSD_D;
    d->m_subtypes = subtypes;
    if (d->isRunning()) {
        stop();
        publishAsync();
    }
}

void PublicService::setPort(unsigned short port)
{
    KDNSSD_D;
    d->m_port = port;
    if (d->isRunning()) {
        stop();
        publishAsync();
    }
}

bool PublicService::isPublished() const
{
    KDNSSD_D;
    return d->m_published;
}

void PublicService::setTextData(const QMap<QString, QByteArray> &textData)
{
    KDNSSD_D;
    d->m_textData = textData;
    if (d->isRunning()) {
        stop();
        publishAsync();
    }
}

bool PublicService::publish()
{
    KDNSSD_D;
    publishAsync();
    while (d->isRunning() && !d->m_published) {
        d->process();
    }
    return d->m_published;
}

void PublicService::stop()
{
    KDNSSD_D;
    d->stop();
    d->m_published = false;
}

void PublicService::publishAsync()
{
    KDNSSD_D;
    if (d->isRunning()) {
        stop();
    }
    TXTRecordRef txt;
    TXTRecordCreate(&txt, 0, 0);
    QMap<QString, QByteArray>::ConstIterator itEnd = d->m_textData.cend();
    for (QMap<QString, QByteArray>::ConstIterator it = d->m_textData.cbegin(); it != itEnd; ++it) {
        if (TXTRecordSetValue(&txt, it.key().toUtf8().constData(), it.value().length(), it.value().constData()) != kDNSServiceErr_NoError) {
            TXTRecordDeallocate(&txt);
            Q_EMIT published(false);
            return;
        }
    }
    DNSServiceRef ref;
    QString fullType = d->m_type;
    for (const QString &subtype : std::as_const(d->m_subtypes)) {
        fullType += ',' + subtype;
    }
    if (DNSServiceRegister(&ref,
                           0,
                           0,
                           d->m_serviceName.toUtf8().constData(),
                           fullType.toLatin1().constData(),
                           domainToDNS(d->m_domain).constData(),
                           NULL,
                           htons(d->m_port),
                           TXTRecordGetLength(&txt),
                           TXTRecordGetBytesPtr(&txt),
                           publish_callback,
                           reinterpret_cast<void *>(d))
        == kDNSServiceErr_NoError) {
        d->setRef(ref);
    }
    TXTRecordDeallocate(&txt);
    if (!d->isRunning()) {
        Q_EMIT published(false);
    }
}

void publish_callback(DNSServiceRef, DNSServiceFlags, DNSServiceErrorType errorCode, const char *name, const char *, const char *, void *context)
{
    QObject *obj = reinterpret_cast<QObject *>(context);
    if (errorCode != kDNSServiceErr_NoError) {
        ErrorEvent err;
        QCoreApplication::sendEvent(obj, &err);
    } else {
        PublishEvent pev(QString::fromUtf8(name));
        QCoreApplication::sendEvent(obj, &pev);
    }
}

void PublicServicePrivate::customEvent(QEvent *event)
{
    if (event->type() == QEvent::User + SD_ERROR) {
        m_parent->stop();
        Q_EMIT m_parent->published(false);
    }
    if (event->type() == QEvent::User + SD_PUBLISH) {
        m_published = true;
        Q_EMIT m_parent->published(true);
        m_serviceName = static_cast<PublishEvent *>(event)->m_name;
    }
}

void PublicService::virtual_hook(int, void *)
{
}

}

#include "moc_publicservice.cpp"
