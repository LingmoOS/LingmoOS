/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004, 2007 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SERVICEBASE_P_H
#define SERVICEBASE_P_H

#include <QMap>
#include <QString>

namespace KDNSSD
{
class ServiceBasePrivate
{
public:
    ServiceBasePrivate(const QString &name, const QString &type, const QString &domain, const QString &host, unsigned short port)
        : m_serviceName(name)
        , m_type(type)
        , m_domain(domain)
        , m_hostName(host)
        , m_port(port)
    {
    }

    virtual ~ServiceBasePrivate()
    {
    }

    QString m_serviceName;
    QString m_type;
    QString m_domain;
    QString m_hostName;
    unsigned short m_port;

    /**
    Map of TXT properties
     */
    QMap<QString, QByteArray> m_textData;
};
}
#endif
