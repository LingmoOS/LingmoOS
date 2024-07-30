/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "timezone.h"

#include <QDataStream>
#include <QSharedData>

using namespace KContacts;

class Q_DECL_HIDDEN TimeZone::Private : public QSharedData
{
public:
    Private(int offset = 0, bool valid = false)
        : mOffset(offset)
        , mValid(valid)
    {
    }

    Private(const Private &other)
        : QSharedData(other)
    {
        mOffset = other.mOffset;
        mValid = other.mValid;
    }

    int mOffset;
    bool mValid;
};

TimeZone::TimeZone()
    : d(new Private)
{
}

TimeZone::TimeZone(int offset)
    : d(new Private(offset, true))
{
}

TimeZone::TimeZone(const TimeZone &other)
    : d(other.d)
{
}

TimeZone::~TimeZone()
{
}

void TimeZone::setOffset(int offset)
{
    d->mOffset = offset;
    d->mValid = true;
}

int TimeZone::offset() const
{
    return d->mOffset;
}

bool TimeZone::isValid() const
{
    return d->mValid;
}

bool TimeZone::operator==(const TimeZone &t) const
{
    if (!t.isValid() && !isValid()) {
        return true;
    }

    if (!t.isValid() || !isValid()) {
        return false;
    }

    if (t.d->mOffset == d->mOffset) {
        return true;
    }

    return false;
}

bool TimeZone::operator!=(const TimeZone &t) const
{
    return !(*this == t);
}

TimeZone &TimeZone::operator=(const TimeZone &other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

QString TimeZone::toString() const
{
    QString str = QLatin1String("TimeZone {\n");
    str += QStringLiteral("  Offset: %1\n").arg(d->mOffset);
    str += QLatin1String("}\n");

    return str;
}

QDataStream &KContacts::operator<<(QDataStream &s, const TimeZone &zone)
{
    return s << zone.d->mOffset << zone.d->mValid;
}

QDataStream &KContacts::operator>>(QDataStream &s, TimeZone &zone)
{
    s >> zone.d->mOffset >> zone.d->mValid;

    return s;
}
