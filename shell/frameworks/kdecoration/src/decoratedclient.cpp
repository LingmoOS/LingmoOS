/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#include "decoratedclient.h"
#include "private/decoratedclientprivate.h"
#include "private/decorationbridge.h"

#include <QColor>

namespace KDecoration2
{
DecoratedClient::DecoratedClient(Decoration *parent, DecorationBridge *bridge)
    : QObject()
    , d(bridge->createClient(this, parent))
{
}

DecoratedClient::~DecoratedClient() = default;

bool DecoratedClient::isActive() const
{
    return d->isActive();
}

QString DecoratedClient::caption() const
{
    return d->caption();
}

bool DecoratedClient::isOnAllDesktops() const
{
    return d->isOnAllDesktops();
}

bool DecoratedClient::isShaded() const
{
    return d->isShaded();
}

QIcon DecoratedClient::icon() const
{
    return d->icon();
}

bool DecoratedClient::isMaximized() const
{
    return d->isMaximized();
}

bool DecoratedClient::isMaximizedHorizontally() const
{
    return d->isMaximizedHorizontally();
}

bool DecoratedClient::isMaximizedVertically() const
{
    return d->isMaximizedVertically();
}

bool DecoratedClient::isKeepAbove() const
{
    return d->isKeepAbove();
}

bool DecoratedClient::isKeepBelow() const
{
    return d->isKeepBelow();
}

bool DecoratedClient::isCloseable() const
{
    return d->isCloseable();
}

bool DecoratedClient::isMaximizeable() const
{
    return d->isMaximizeable();
}

bool DecoratedClient::isMinimizeable() const
{
    return d->isMinimizeable();
}

bool DecoratedClient::providesContextHelp() const
{
    return d->providesContextHelp();
}

bool DecoratedClient::isModal() const
{
    return d->isModal();
}

bool DecoratedClient::isShadeable() const
{
    return d->isShadeable();
}

bool DecoratedClient::isMoveable() const
{
    return d->isMoveable();
}

bool DecoratedClient::isResizeable() const
{
    return d->isResizeable();
}

WId DecoratedClient::windowId() const
{
    return d->windowId();
}

WId DecoratedClient::decorationId() const
{
    return d->decorationId();
}

int DecoratedClient::width() const
{
    return d->width();
}

int DecoratedClient::height() const
{
    return d->height();
}

QSize DecoratedClient::size() const
{
    return d->size();
}

QPalette DecoratedClient::palette() const
{
    return d->palette();
}

Qt::Edges DecoratedClient::adjacentScreenEdges() const
{
    return d->adjacentScreenEdges();
}

QString DecoratedClient::windowClass() const
{
    return d->windowClass();
}

bool DecoratedClient::hasApplicationMenu() const
{
    if (const auto *appMenuEnabledPrivate = dynamic_cast<ApplicationMenuEnabledDecoratedClientPrivate *>(d.get())) {
        return appMenuEnabledPrivate->hasApplicationMenu();
    }
    return false;
}

bool DecoratedClient::isApplicationMenuActive() const
{
    if (const auto *appMenuEnabledPrivate = dynamic_cast<ApplicationMenuEnabledDecoratedClientPrivate *>(d.get())) {
        return appMenuEnabledPrivate->isApplicationMenuActive();
    }
    return false;
}

Decoration *DecoratedClient::decoration() const
{
    return d->decoration();
}

QColor DecoratedClient::color(QPalette::ColorGroup group, QPalette::ColorRole role) const
{
    return d->palette().color(group, role);
}

QColor DecoratedClient::color(ColorGroup group, ColorRole role) const
{
    return d->color(group, role);
}

void DecoratedClient::showApplicationMenu(int actionId)
{
    if (auto *appMenuEnabledPrivate = dynamic_cast<ApplicationMenuEnabledDecoratedClientPrivate *>(d.get())) {
        appMenuEnabledPrivate->showApplicationMenu(actionId);
    }
}

} // namespace

#include "moc_decoratedclient.cpp"
