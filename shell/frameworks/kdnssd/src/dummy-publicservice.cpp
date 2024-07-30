/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004, 2005 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "publicservice.h"
#include "servicebase_p.h"
#include <QStringList>

namespace KDNSSD
{
PublicService::PublicService(const QString &name, const QString &type, unsigned int port, const QString &domain, const QStringList &)
    : QObject()
    , ServiceBase(name, type, QString(), domain, port)
{
    if (domain.isNull()) {
        d->m_domain = QLatin1String("local.");
    }
}

PublicService::~PublicService()
{
}

void PublicService::setServiceName(const QString &serviceName)
{
    d->m_serviceName = serviceName;
}

void PublicService::setDomain(const QString &domain)
{
    d->m_domain = domain;
}

void PublicService::setTextData(const QMap<QString, QByteArray> &textData)
{
    d->m_textData = textData;
}

void PublicService::setType(const QString &type)
{
    d->m_type = type;
}

void PublicService::setSubTypes(const QStringList &)
{
    // dummy and empty
}

void PublicService::setPort(unsigned short port)
{
    d->m_port = port;
}

QStringList PublicService::subtypes() const
{
    return QStringList();
}

bool PublicService::isPublished() const
{
    return false;
}

bool PublicService::publish()
{
    return false;
}

void PublicService::stop()
{
}

void PublicService::publishAsync()
{
    Q_EMIT published(false);
}

void PublicService::virtual_hook(int, void *)
{
}

}

#include "moc_publicservice.cpp"
