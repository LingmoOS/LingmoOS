/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2018 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "config.h"

using namespace Attica;

class Q_DECL_HIDDEN Config::Private : public QSharedData
{
public:
    QString m_version;
    QString m_website;
    QString m_host;
    QString m_contact;
    bool m_ssl;

    Private()
        : m_ssl(false)
    {
    }
};

Config::Config()
    : d(new Private)
{
}

Config::Config(const Attica::Config &other)
    : d(other.d)
{
}

Config &Config::operator=(const Attica::Config &other)
{
    d = other.d;
    return *this;
}

Config::~Config()
{
}

QString Attica::Config::version() const
{
    return d->m_version;
}

QString Config::website() const
{
    return d->m_website;
}

QString Config::host() const
{
    return d->m_host;
}

QString Config::contact() const
{
    return d->m_contact;
}

bool Config::ssl() const
{
    return d->m_ssl;
}

bool Config::isValid() const
{
    return !(d->m_version.isEmpty());
}

void Config::setContact(const QString &contact)
{
    d->m_contact = contact;
}

void Config::setVersion(const QString &version)
{
    d->m_version = version;
}

void Config::setWebsite(const QString &website)
{
    d->m_website = website;
}

void Config::setHost(const QString &host)
{
    d->m_host = host;
}

void Config::setSsl(bool ssl)
{
    d->m_ssl = ssl;
}
