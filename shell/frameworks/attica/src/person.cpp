/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "person.h"

using namespace Attica;

class Q_DECL_HIDDEN Person::Private : public QSharedData
{
public:
    QString m_id;
    QString m_firstName;
    QString m_lastName;
    QDate m_birthday;
    QString m_country;
    qreal m_latitude;
    qreal m_longitude;
    QUrl m_avatarUrl;
    QString m_homepage;
    QString m_city;

    QMap<QString, QString> m_extendedAttributes;

    Private()
        : m_latitude(0)
        , m_longitude(0)
    {
    }
};

Person::Person()
    : d(new Private)
{
}

Person::Person(const Person &other)
    : d(other.d)
{
}

Person &Person::operator=(const Attica::Person &other)
{
    d = other.d;
    return *this;
}

Person::~Person()
{
}

void Person::setId(const QString &u)
{
    d->m_id = u;
}

QString Person::id() const
{
    return d->m_id;
}

void Person::setFirstName(const QString &name)
{
    d->m_firstName = name;
}

QString Person::firstName() const
{
    return d->m_firstName;
}

void Person::setLastName(const QString &name)
{
    d->m_lastName = name;
}

QString Person::lastName() const
{
    return d->m_lastName;
}

void Person::setBirthday(const QDate &date)
{
    d->m_birthday = date;
}

QDate Person::birthday() const
{
    return d->m_birthday;
}

void Person::setCountry(const QString &c)
{
    d->m_country = c;
}

QString Person::country() const
{
    return d->m_country;
}

void Person::setLatitude(qreal l)
{
    d->m_latitude = l;
}

qreal Person::latitude() const
{
    return d->m_latitude;
}

void Person::setLongitude(qreal l)
{
    d->m_longitude = l;
}

qreal Person::longitude() const
{
    return d->m_longitude;
}

void Person::setAvatarUrl(const QUrl &url)
{
    d->m_avatarUrl = url;
}

QUrl Person::avatarUrl() const
{
    return d->m_avatarUrl;
}

void Person::setHomepage(const QString &h)
{
    d->m_homepage = h;
}

QString Person::homepage() const
{
    return d->m_homepage;
}

void Person::setCity(const QString &h)
{
    d->m_city = h;
}

QString Person::city() const
{
    return d->m_city;
}
void Person::addExtendedAttribute(const QString &key, const QString &value)
{
    d->m_extendedAttributes.insert(key, value);
}

QString Person::extendedAttribute(const QString &key) const
{
    return d->m_extendedAttributes.value(key);
}

QMap<QString, QString> Person::extendedAttributes() const
{
    return d->m_extendedAttributes;
}

bool Person::isValid() const
{
    return !(d->m_id.isEmpty());
}
