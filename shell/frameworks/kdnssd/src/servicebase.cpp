/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "servicebase.h"
#include "servicebase_p.h"
#include <QDataStream>
#include <QUrl>

namespace KDNSSD
{
ServiceBase::ServiceBase(const QString &name, const QString &type, const QString &domain, const QString &host, unsigned short port)
    : d(new ServiceBasePrivate(name, type, domain, host, port))
{
}

ServiceBase::ServiceBase(ServiceBasePrivate *const _d)
    : d(_d)
{
}

ServiceBase::~ServiceBase() = default;

QString ServiceBase::serviceName() const
{
    return d->m_serviceName;
}

QString ServiceBase::type() const
{
    return d->m_type;
}

QString ServiceBase::domain() const
{
    return d->m_domain;
}

QString ServiceBase::hostName() const
{
    return d->m_hostName;
}

unsigned short ServiceBase::port() const
{
    return d->m_port;
}
QMap<QString, QByteArray> ServiceBase::textData() const
{
    return d->m_textData;
}

bool ServiceBase::operator==(const ServiceBase &o) const
{
    return d->m_domain == o.d->m_domain && d->m_serviceName == o.d->m_serviceName && d->m_type == o.d->m_type;
}

bool ServiceBase::operator!=(const ServiceBase &o) const
{
    return !(*this == o);
}

void ServiceBase::virtual_hook(int, void *)
{
}

bool domainIsLocal(const QString &domain)
{
    return domain.section(QLatin1Char('.'), -1, -1).toLower() == QLatin1String("local");
}

}
