/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "note.h"
#include "parametermap_p.h"

#include <QDataStream>
#include <QStringList>

using namespace KContacts;

class Q_DECL_HIDDEN Note::Private : public QSharedData
{
public:
    Private()
    {
    }

    Private(const Private &other)
        : QSharedData(other)
    {
        mParamMap = other.mParamMap;
        note = other.note;
    }

    ParameterMap mParamMap;
    QString note;
};

Note::Note()
    : d(new Private)
{
}

Note::Note(const Note &other)
    : d(other.d)
{
}

Note::Note(const QString &note)
    : d(new Private)
{
    d->note = note;
}

Note::~Note()
{
}

void Note::setNote(const QString &note)
{
    d->note = note;
}

QString Note::note() const
{
    return d->note;
}

bool Note::isValid() const
{
    return !d->note.isEmpty();
}

void Note::setParams(const ParameterMap &params)
{
    d->mParamMap = params;
}

ParameterMap Note::params() const
{
    return d->mParamMap;
}

bool Note::operator==(const Note &other) const
{
    return (d->mParamMap == other.d->mParamMap) && (d->note == other.note());
}

bool Note::operator!=(const Note &other) const
{
    return !(other == *this);
}

Note &Note::operator=(const Note &other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

QString Note::toString() const
{
    QString str = QLatin1String("Note {\n");
    str += QStringLiteral("    note: %1\n").arg(d->note);
    str += d->mParamMap.toString();
    str += QLatin1String("}\n");
    return str;
}

QDataStream &KContacts::operator<<(QDataStream &s, const Note &note)
{
    return s << note.d->mParamMap << note.d->note;
}

QDataStream &KContacts::operator>>(QDataStream &s, Note &note)
{
    s >> note.d->mParamMap >> note.d->note;
    return s;
}
