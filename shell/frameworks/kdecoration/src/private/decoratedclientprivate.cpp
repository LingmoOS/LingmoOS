/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#include "decoratedclientprivate.h"

#include <QColor>

namespace KDecoration2
{
class Q_DECL_HIDDEN DecoratedClientPrivate::Private
{
public:
    explicit Private(DecoratedClient *client, Decoration *decoration);
    DecoratedClient *client;
    Decoration *decoration;
};

DecoratedClientPrivate::Private::Private(DecoratedClient *client, Decoration *decoration)
    : client(client)
    , decoration(decoration)
{
}

DecoratedClientPrivate::DecoratedClientPrivate(DecoratedClient *client, Decoration *decoration)
    : d(new Private(client, decoration))
{
}

DecoratedClientPrivate::~DecoratedClientPrivate() = default;

Decoration *DecoratedClientPrivate::decoration()
{
    return d->decoration;
}

Decoration *DecoratedClientPrivate::decoration() const
{
    return d->decoration;
}

DecoratedClient *DecoratedClientPrivate::client()
{
    return d->client;
}

QColor DecoratedClientPrivate::color(ColorGroup group, ColorRole role) const
{
    Q_UNUSED(role)
    Q_UNUSED(group)

    return QColor();
}

ApplicationMenuEnabledDecoratedClientPrivate::ApplicationMenuEnabledDecoratedClientPrivate(DecoratedClient *client, Decoration *decoration)
    : DecoratedClientPrivate(client, decoration)
{
}

ApplicationMenuEnabledDecoratedClientPrivate::~ApplicationMenuEnabledDecoratedClientPrivate() = default;

}
