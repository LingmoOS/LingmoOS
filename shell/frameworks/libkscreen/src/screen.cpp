/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2014 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "screen.h"

using namespace KScreen;

class Q_DECL_HIDDEN Screen::Private
{
public:
    Private()
        : id(0)
        , maxActiveOutputsCount(0)
    {
    }

    Private(const Private &other)
        : id(other.id)
        , maxActiveOutputsCount(other.maxActiveOutputsCount)
        , currentSize(other.currentSize)
        , minSize(other.minSize)
        , maxSize(other.maxSize)
    {
    }

    int id;
    int maxActiveOutputsCount;
    QSize currentSize;
    QSize minSize;
    QSize maxSize;
};

Screen::Screen()
    : QObject(nullptr)
    , d(new Private())
{
}

Screen::Screen(Screen::Private *dd)
    : QObject()
    , d(dd)
{
}

Screen::~Screen()
{
    delete d;
}

ScreenPtr Screen::clone() const
{
    return ScreenPtr(new Screen(new Private(*d)));
}

int Screen::id() const
{
    return d->id;
}

void Screen::setId(int id)
{
    d->id = id;
}

QSize Screen::currentSize() const
{
    return d->currentSize;
}

void Screen::setCurrentSize(const QSize &currentSize)
{
    if (d->currentSize == currentSize) {
        return;
    }

    d->currentSize = currentSize;

    Q_EMIT currentSizeChanged();
}

QSize Screen::maxSize() const
{
    return d->maxSize;
}

void Screen::setMaxSize(const QSize &maxSize)
{
    d->maxSize = maxSize;
}

QSize Screen::minSize() const
{
    return d->minSize;
}

void Screen::setMinSize(const QSize &minSize)
{
    d->minSize = minSize;
}

int Screen::maxActiveOutputsCount() const
{
    return d->maxActiveOutputsCount;
}

void Screen::setMaxActiveOutputsCount(int maxActiveOutputsCount)
{
    d->maxActiveOutputsCount = maxActiveOutputsCount;
}

void Screen::apply(const ScreenPtr &other)
{
    // Only set values that can change
    setMaxActiveOutputsCount(other->d->maxActiveOutputsCount);
    setCurrentSize(other->d->currentSize);
}

#include "moc_screen.cpp"
