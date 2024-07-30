/*
 *   SPDX-FileCopyrightText: 2007-2009 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "unit.h"
#include "unit_p.h"
#include "unitcategory.h"

#include <KLocalizedString>

namespace KUnitConversion
{
UnitPrivate::UnitPrivate()
    : m_categoryId(InvalidCategory)
    , m_id(InvalidUnit)
    , m_multiplier(1.0)
{
}

UnitPrivate::UnitPrivate(CategoryId categoryId,
                         UnitId id,
                         qreal multiplier,
                         const QString &symbol,
                         const QString &description,
                         const QString &matchString,
                         const KLocalizedString &symbolString,
                         const KLocalizedString &realString,
                         const KLocalizedString &integerString)
    : m_categoryId(categoryId)
    , m_id(id)
    , m_multiplier(multiplier)
    , m_symbol(symbol)
    , m_description(description)
    , m_matchString(matchString)
    , m_symbolString(symbolString)
    , m_realString(realString)
    , m_integerString(integerString)
{
}

UnitPrivate::~UnitPrivate()
{
}

UnitPrivate *UnitPrivate::clone()
{
    return new UnitPrivate(*this);
}

bool UnitPrivate::operator==(const UnitPrivate &other) const
{
    return (m_id == other.m_id && m_symbol == other.m_symbol);
}

bool UnitPrivate::operator!=(const UnitPrivate &other) const
{
    return !(*this == other);
}

void UnitPrivate::setUnitMultiplier(qreal multiplier)
{
    m_multiplier = multiplier;
}

qreal UnitPrivate::unitMultiplier() const
{
    return m_multiplier;
}

qreal UnitPrivate::toDefault(qreal value) const
{
    return value * m_multiplier;
}

qreal UnitPrivate::fromDefault(qreal value) const
{
    return value / m_multiplier;
}

Unit::Unit()
    : d(nullptr)
{
}

Unit::Unit(UnitPrivate *dd)
    : d(dd)
{
}

Unit::Unit(const Unit &other)
    : d(other.d)
{
}

Unit::~Unit()
{
}

Unit &Unit::operator=(const Unit &other)
{
    d = other.d;
    return *this;
}

Unit &Unit::operator=(Unit &&other)
{
    d.swap(other.d);
    return *this;
}

bool Unit::operator==(const Unit &other) const
{
    if (d && other.d) {
        return (*d == *other.d);
    } else {
        return (d == other.d);
    }
}

bool Unit::operator!=(const Unit &other) const
{
    if (d && other.d) {
        return (*d != *other.d);
    } else {
        return (d != other.d);
    }
}

bool Unit::isNull() const
{
    return !d;
}

bool Unit::isValid() const
{
    return (d && !d->m_symbol.isEmpty());
}

UnitId Unit::id() const
{
    if (d) {
        return d->m_id;
    }
    return InvalidUnit;
}

CategoryId Unit::categoryId() const
{
    if (d) {
        return d->m_categoryId;
    }
    return InvalidCategory;
}

UnitCategory Unit::category() const
{
    if (d) {
        return UnitCategory(d->m_category);
    }
    return UnitCategory();
}

QString Unit::description() const
{
    if (d) {
        return d->m_description;
    }
    return QString();
}

QString Unit::symbol() const
{
    if (d) {
        return d->m_symbol;
    }
    return QString();
}

void Unit::setUnitMultiplier(qreal multiplier)
{
    if (d) {
        d->setUnitMultiplier(multiplier);
    }
}

qreal Unit::toDefault(qreal value) const
{
    if (d) {
        return d->toDefault(value);
    }
    return 0;
}

qreal Unit::fromDefault(qreal value) const
{
    if (d) {
        return d->fromDefault(value);
    }
    return 0;
}

QString Unit::toString(qreal value, int fieldWidth, char format, int precision, const QChar &fillChar) const
{
    if (isNull()) {
        return QString();
    }
    if ((int)value == value && precision < 1) {
        return d->m_integerString.subs((int)value).toString();
    }
    return d->m_realString.subs(value, fieldWidth, format, precision, fillChar).toString();
}

QString Unit::toSymbolString(qreal value, int fieldWidth, char format, int precision, const QChar &fillChar) const
{
    if (d) {
        return d->m_symbolString.subs(value, fieldWidth, format, precision, fillChar).subs(d->m_symbol).toString();
    }
    return QString();
}

}
