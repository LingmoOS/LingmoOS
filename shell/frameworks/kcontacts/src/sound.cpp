/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2002 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "sound.h"

#include <QDataStream>
#include <QSharedData>

using namespace KContacts;

class Q_DECL_HIDDEN Sound::Private : public QSharedData
{
public:
    Private()
        : mIntern(false)
    {
    }

    Private(const Private &other)
        : QSharedData(other)
    {
        mUrl = other.mUrl;
        mData = other.mData;
        mIntern = other.mIntern;
    }

    QString mUrl;
    QByteArray mData;

    bool mIntern;
};

Sound::Sound()
    : d(new Private)
{
}

Sound::Sound(const QString &url)
    : d(new Private)
{
    d->mUrl = url;
}

Sound::Sound(const QByteArray &data)
    : d(new Private)
{
    d->mIntern = true;
    d->mData = data;
}

Sound::Sound(const Sound &other)
    : d(other.d)
{
}

Sound::~Sound()
{
}

Sound &Sound::operator=(const Sound &other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

bool Sound::operator==(const Sound &other) const
{
    if (d->mIntern != other.d->mIntern) {
        return false;
    }

    if (d->mIntern) {
        if (d->mData != other.d->mData) {
            return false;
        }
    } else {
        if (d->mUrl != other.d->mUrl) {
            return false;
        }
    }

    return true;
}

bool Sound::operator!=(const Sound &other) const
{
    return !(other == *this);
}

void Sound::setUrl(const QString &url)
{
    d->mIntern = false;
    d->mUrl = url;
}

void Sound::setData(const QByteArray &data)
{
    d->mIntern = true;
    d->mData = data;
}

bool Sound::isIntern() const
{
    return d->mIntern;
}

bool Sound::isEmpty() const
{
    return (d->mIntern && d->mData.isEmpty()) || (!d->mIntern && d->mUrl.isEmpty());
}

QString Sound::url() const
{
    return d->mUrl;
}

QByteArray Sound::data() const
{
    return d->mData;
}

QString Sound::toString() const
{
    QString str = QLatin1String("Sound {\n");
    str += QStringLiteral("  IsIntern: %1\n").arg(d->mIntern ? QStringLiteral("true") : QStringLiteral("false"));
    if (d->mIntern) {
        str += QStringLiteral("  Data: %1\n").arg(QString::fromLatin1(d->mData.toBase64()));
    } else {
        str += QStringLiteral("  Url: %1\n").arg(d->mUrl);
    }
    str += QLatin1String("}\n");

    return str;
}

QDataStream &KContacts::operator<<(QDataStream &s, const Sound &sound)
{
    return s << sound.d->mIntern << sound.d->mUrl << sound.d->mData;
}

QDataStream &KContacts::operator>>(QDataStream &s, Sound &sound)
{
    s >> sound.d->mIntern >> sound.d->mUrl >> sound.d->mData;

    return s;
}
