/*
    This file is part of KDE.

    SPDX-FileCopyrightText: Martin Sandsmark <martin.sandsmark@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "privatedata.h"

#include <QStringList>

using namespace Attica;

class Q_DECL_HIDDEN PrivateData::Private : public QSharedData
{
public:
    QMap<QString, QString> m_attributes;
    QMap<QString, QDateTime> m_attributesTimestamp;

    Provider *m_provider;

    Private()
        : m_provider(nullptr)
    {
    }
};

PrivateData::PrivateData()
    : d(new Private)
{
}

PrivateData::PrivateData(const PrivateData &other)
    : d(other.d)
{
}

PrivateData &PrivateData::operator=(const Attica::PrivateData &other)
{
    d = other.d;
    return *this;
}

PrivateData::~PrivateData()
{
}

void PrivateData::setAttribute(const QString &key, const QString &value)
{
    d->m_attributes[key] = value;
    d->m_attributesTimestamp[key] = QDateTime::currentDateTime();
}

QString PrivateData::attribute(const QString &key) const
{
    return d->m_attributes[key];
}

QDateTime PrivateData::timestamp(const QString &key) const
{
    return d->m_attributesTimestamp[key];
}

QStringList PrivateData::keys() const
{
    return d->m_attributes.keys();
}

void PrivateData::setTimestamp(const QString &key, const QDateTime &when)
{
    d->m_attributesTimestamp[key] = when;
}
