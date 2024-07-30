/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Intel Corporation
    SPDX-FileContributor: Mateu Batle Sastre <mbatle@collabora.co.uk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "icon.h"

using namespace Attica;

class Q_DECL_HIDDEN Icon::Private : public QSharedData
{
public:
    QUrl url;
    uint width;
    uint height;

    Private()
        : width(0)
        , height(0)
    {
    }
};

Icon::Icon()
    : d(new Private)
{
}

Icon::Icon(const Attica::Icon &other)
    : d(other.d)
{
}

Icon &Icon::operator=(const Attica::Icon &other)
{
    d = other.d;
    return *this;
}

Icon::~Icon()
{
}

QUrl Icon::url() const
{
    return d->url;
}

void Icon::setUrl(const QUrl &url)
{
    d->url = url;
}

uint Icon::width() const
{
    return d->width;
}

void Icon::setWidth(uint width)
{
    d->width = width;
}

uint Icon::height() const
{
    return d->height;
}

void Icon::setHeight(uint height)
{
    d->height = height;
}
