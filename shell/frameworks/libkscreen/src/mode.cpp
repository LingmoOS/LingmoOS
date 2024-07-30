/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2014 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "mode.h"

using namespace KScreen;
class Q_DECL_HIDDEN Mode::Private
{
public:
    Private()
        : rate(0)
    {
    }

    Private(const Private &other)
        : id(other.id)
        , name(other.name)
        , size(other.size)
        , rate(other.rate)
    {
    }

    QString id;
    QString name;
    QSize size;
    float rate;
};

Mode::Mode()
    : QObject(nullptr)
    , d(new Private())
{
}

Mode::Mode(Mode::Private *dd)
    : QObject()
    , d(dd)
{
}

Mode::~Mode()
{
    delete d;
}

ModePtr Mode::clone() const
{
    return ModePtr(new Mode(new Private(*d)));
}

const QString Mode::id() const
{
    return d->id;
}

void Mode::setId(const QString &id)
{
    if (d->id == id) {
        return;
    }

    d->id = id;

    Q_EMIT modeChanged();
}

QString Mode::name() const
{
    return d->name;
}

void Mode::setName(const QString &name)
{
    if (d->name == name) {
        return;
    }

    d->name = name;

    Q_EMIT modeChanged();
}

QSize Mode::size() const
{
    return d->size;
}

void Mode::setSize(const QSize &size)
{
    if (d->size == size) {
        return;
    }

    d->size = size;

    Q_EMIT modeChanged();
}

float Mode::refreshRate() const
{
    return d->rate;
}

void Mode::setRefreshRate(float refresh)
{
    if (qFuzzyCompare(d->rate, refresh)) {
        return;
    }

    d->rate = refresh;

    Q_EMIT modeChanged();
}

QDebug operator<<(QDebug dbg, const KScreen::ModePtr &mode)
{
    if (mode) {
        dbg << "KScreen::Mode(Id:" << mode->id() << ", Size:" << mode->size() << "@" << mode->refreshRate() << ")";
    } else {
        dbg << "KScreen::Mode(NULL)";
    }
    return dbg;
}

#include "moc_mode.cpp"
