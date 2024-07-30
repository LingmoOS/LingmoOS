/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "simpleextractionresult.h"

using namespace KFileMetaData;

class KFileMetaData::SimpleExtractionResultPrivate
{
public:
    PropertyMultiMap m_properties;
    QString m_text;
    QList<Type::Type> m_types;
};

SimpleExtractionResult::SimpleExtractionResult(const QString& url, const QString& mimetype, const Flags& flags)
    : ExtractionResult(url, mimetype, flags)
    , d(new SimpleExtractionResultPrivate)
{
}

SimpleExtractionResult::SimpleExtractionResult(const SimpleExtractionResult& rhs)
    : ExtractionResult(*this)
    , d(new SimpleExtractionResultPrivate(*rhs.d))
{
}

SimpleExtractionResult::~SimpleExtractionResult() = default;

SimpleExtractionResult& SimpleExtractionResult::operator=(const SimpleExtractionResult& rhs)
{
    *d = *rhs.d;
    return *this;
}

bool SimpleExtractionResult::operator==(const SimpleExtractionResult& rhs) const
{
    return d->m_properties == rhs.d->m_properties && d->m_text == rhs.d->m_text
           && d->m_types == rhs.d->m_types;
}

void SimpleExtractionResult::add(Property::Property property, const QVariant& value)
{
    d->m_properties.insert(property, value);
}

void SimpleExtractionResult::addType(Type::Type type)
{
    d->m_types << type;
}

void SimpleExtractionResult::append(const QString& text)
{
    d->m_text.append(text);
    d->m_text.append(QLatin1Char(' '));
}

PropertyMultiMap SimpleExtractionResult::properties() const
{
    return d->m_properties;
}

QString SimpleExtractionResult::text() const
{
    return d->m_text;
}

QList<Type::Type> SimpleExtractionResult::types() const
{
    return d->m_types;
}
