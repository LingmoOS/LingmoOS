/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2012 Laszlo Papp <lpapp@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "cloud.h"

using namespace Attica;

class Q_DECL_HIDDEN Cloud::Private : public QSharedData
{
public:
    QString m_name;
    QString m_url;
    QUrl m_icon;
    qulonglong m_quota;
    qulonglong m_free;
    qulonglong m_used;
    float m_relative;
    QString m_key;
};

Cloud::Cloud()
    : d(new Private)
{
}

Cloud::Cloud(const Attica::Cloud &other)
    : d(other.d)
{
}

Cloud &Cloud::operator=(const Attica::Cloud &other)
{
    d = other.d;
    return *this;
}

Cloud::~Cloud()
{
}

void Cloud::setName(const QString &name)
{
    d->m_name = name;
}

QString Cloud::name() const
{
    return d->m_name;
}

void Cloud::setUrl(const QString &url)
{
    d->m_url = url;
}

QString Cloud::url() const
{
    return d->m_url;
}

void Cloud::setIcon(const QUrl &icon)
{
    d->m_icon = icon;
}

QUrl Cloud::icon() const
{
    return d->m_icon;
}

void Cloud::setQuota(qulonglong quota)
{
    d->m_quota = quota;
}

qulonglong Cloud::quota() const
{
    return d->m_quota;
}

void Cloud::setFree(qulonglong free)
{
    d->m_free = free;
}

qulonglong Cloud::free() const
{
    return d->m_free;
}

void Cloud::setUsed(qulonglong used)
{
    d->m_used = used;
}

qulonglong Cloud::used() const
{
    return d->m_used;
}

void Cloud::setRelative(float relative)
{
    d->m_relative = relative;
}

float Cloud::relative() const
{
    return d->m_relative;
}

void Cloud::setKey(const QString &key)
{
    d->m_key = key;
}

QString Cloud::key() const
{
    return d->m_key;
}
