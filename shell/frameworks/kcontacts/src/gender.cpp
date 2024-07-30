/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "gender.h"
#include "parametermap_p.h"

using namespace KContacts;

class Q_DECL_HIDDEN Gender::Private : public QSharedData
{
public:
    Private()
    {
    }

    Private(const Private &other)
        : QSharedData(other)
    {
        comment = other.comment;
        gender = other.gender;
    }

    QString gender;
    QString comment;
};

Gender::Gender()
    : d(new Private)
{
}

Gender::Gender(const QString &gender)
    : d(new Private)
{
    d->gender = gender;
}

Gender::Gender(const Gender &other)
    : d(other.d)
{
}

Gender::~Gender()
{
}

bool Gender::operator==(const Gender &other) const
{
    return (d->comment == other.comment()) && (d->gender == other.gender());
}

bool Gender::operator!=(const Gender &other) const
{
    return !(other == *this);
}

Gender &Gender::operator=(const Gender &other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

QString Gender::toString() const
{
    QString str = QLatin1String("Gender {\n");
    str += QStringLiteral("    gender: %1\n").arg(d->gender);
    str += QStringLiteral("    comment: %1\n").arg(d->comment);
    str += QLatin1String("}\n");
    return str;
}

void Gender::setGender(const QString &gender)
{
    d->gender = gender;
}

QString Gender::gender() const
{
    return d->gender;
}

void Gender::setComment(const QString &comment)
{
    d->comment = comment;
}

QString Gender::comment() const
{
    return d->comment;
}

bool Gender::isValid() const
{
    return !d->gender.isEmpty() || !d->comment.isEmpty();
}

QDataStream &KContacts::operator<<(QDataStream &s, const Gender &gender)
{
    return s << gender.d->comment << gender.d->gender;
}

QDataStream &KContacts::operator>>(QDataStream &s, Gender &gender)
{
    s >> gender.d->comment >> gender.d->gender;
    return s;
}
