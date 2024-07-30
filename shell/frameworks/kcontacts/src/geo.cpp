/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "geo.h"

#include <QDataStream>
#include <QSharedData>

using namespace KContacts;

class Q_DECL_HIDDEN Geo::Private : public QSharedData
{
public:
    Private()
        : mLatitude(91)
        , mLongitude(181)
        , mValidLatitude(false)
        , mValidLongitude(false)
    {
    }

    Private(const Private &other)
        : QSharedData(other)
    {
        mLatitude = other.mLatitude;
        mLongitude = other.mLongitude;
        mValidLatitude = other.mValidLatitude;
        mValidLongitude = other.mValidLongitude;
    }

    float mLatitude;
    float mLongitude;

    bool mValidLatitude;
    bool mValidLongitude;
};

Geo::Geo()
    : d(new Private)
{
}

Geo::Geo(float latitude, float longitude)
    : d(new Private)
{
    setLatitude(latitude);
    setLongitude(longitude);
}

Geo::Geo(const Geo &other)
    : d(other.d)
{
}

Geo::~Geo()
{
}

void Geo::setLatitude(float latitude)
{
    if (latitude >= -90 && latitude <= 90) {
        d->mLatitude = latitude;
        d->mValidLatitude = true;
    } else {
        d->mLatitude = 91;
        d->mValidLatitude = false;
    }
}

float Geo::latitude() const
{
    return d->mLatitude;
}

void Geo::setLongitude(float longitude)
{
    if (longitude >= -180 && longitude <= 180) {
        d->mLongitude = longitude;
        d->mValidLongitude = true;
    } else {
        d->mLongitude = 181;
        d->mValidLongitude = false;
    }
}

float Geo::longitude() const
{
    return d->mLongitude;
}

bool Geo::isValid() const
{
    return d->mValidLatitude && d->mValidLongitude;
}

bool Geo::operator==(const Geo &other) const
{
    if (!other.isValid() && !isValid()) {
        return true;
    }

    if (!other.isValid() || !isValid()) {
        return false;
    }

    if (other.d->mLatitude == d->mLatitude && other.d->mLongitude == d->mLongitude) {
        return true;
    }

    return false;
}

bool Geo::operator!=(const Geo &other) const
{
    return !(*this == other);
}

Geo &Geo::operator=(const Geo &other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

QString Geo::toString() const
{
    QString str = QLatin1String("Geo {\n");
    str += QStringLiteral("  Valid: %1\n").arg(isValid() ? QStringLiteral("true") : QStringLiteral("false"));
    str += QStringLiteral("  Latitude: %1\n").arg(d->mLatitude);
    str += QStringLiteral("  Longitude: %1\n").arg(d->mLongitude);
    str += QLatin1String("}\n");

    return str;
}

void Geo::clear()
{
    d->mValidLatitude = false;
    d->mValidLongitude = false;
}

// clang-format off
QDataStream &KContacts::operator<<(QDataStream &s, const Geo &geo)
{
    return s << geo.d->mLatitude << geo.d->mValidLatitude
             << geo.d->mLongitude << geo.d->mValidLongitude;
}

QDataStream &KContacts::operator>>(QDataStream &s, Geo &geo)
{
    s >> geo.d->mLatitude >> geo.d->mValidLatitude
    >> geo.d->mLongitude >> geo.d->mValidLongitude;

    return s;
}
// clang-format on

#include "moc_geo.cpp"
