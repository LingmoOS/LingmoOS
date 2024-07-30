/*
    SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kwindowshadow.h"
#include "kwindowshadow_dummy_p.h"
#include "kwindowshadow_p.h"
#include "kwindowsystem_debug.h"
#include "pluginwrapper_p.h"

#include <array>

KWindowShadowTile::KWindowShadowTile()
    : d(KWindowSystemPluginWrapper::self().createWindowShadowTile())
{
}

KWindowShadowTile::~KWindowShadowTile()
{
    if (d->isCreated) {
        d->destroy();
    }
}

QImage KWindowShadowTile::image() const
{
    return d->image;
}

void KWindowShadowTile::setImage(const QImage &image)
{
    if (d->isCreated) {
        qCWarning(LOG_KWINDOWSYSTEM,
                  "Cannot change the image on a tile that already has native "
                  "platform resources allocated.");
        return;
    }
    d->image = image;
}

bool KWindowShadowTile::isCreated() const
{
    return d->isCreated;
}

bool KWindowShadowTile::create()
{
    if (d->isCreated) {
        return true;
    }
    d->isCreated = d->create();
    return d->isCreated;
}

KWindowShadow::KWindowShadow(QObject *parent)
    : QObject(parent)
    , d(KWindowSystemPluginWrapper::self().createWindowShadow())
{
}

KWindowShadow::~KWindowShadow()
{
    destroy();
}

KWindowShadowTile::Ptr KWindowShadow::leftTile() const
{
    return d->leftTile;
}

void KWindowShadow::setLeftTile(KWindowShadowTile::Ptr tile)
{
    if (d->isCreated) {
        qCWarning(LOG_KWINDOWSYSTEM,
                  "Cannot attach a left tile to a shadow that already has "
                  "native platform resources allocated. To do so, destroy() the shadow and then "
                  "setLeftTile() and create()");
        return;
    }
    d->leftTile = tile;
}

KWindowShadowTile::Ptr KWindowShadow::topLeftTile() const
{
    return d->topLeftTile;
}

void KWindowShadow::setTopLeftTile(KWindowShadowTile::Ptr tile)
{
    if (d->isCreated) {
        qCWarning(LOG_KWINDOWSYSTEM,
                  "Cannot attach a top-left tile to a shadow that already has "
                  "native platform resources allocated. To do so, destroy() the shadow and then "
                  "setTopLeftTile() and create()");
        return;
    }
    d->topLeftTile = tile;
}

KWindowShadowTile::Ptr KWindowShadow::topTile() const
{
    return d->topTile;
}

void KWindowShadow::setTopTile(KWindowShadowTile::Ptr tile)
{
    if (d->isCreated) {
        qCWarning(LOG_KWINDOWSYSTEM,
                  "Cannot attach a top tile to a shadow that already has "
                  "native platform resources allocated. To do so, destroy() the shadow and then "
                  "setTopTile() and create()");
        return;
    }
    d->topTile = tile;
}

KWindowShadowTile::Ptr KWindowShadow::topRightTile() const
{
    return d->topRightTile;
}

void KWindowShadow::setTopRightTile(KWindowShadowTile::Ptr tile)
{
    if (d->isCreated) {
        qCWarning(LOG_KWINDOWSYSTEM,
                  "Cannot attach a top-right tile to a shadow that already "
                  "has native platform resources allocated. To do so, destroy() the shadow and "
                  "then setTopRightTile() and create()");
        return;
    }
    d->topRightTile = tile;
}

KWindowShadowTile::Ptr KWindowShadow::rightTile() const
{
    return d->rightTile;
}

void KWindowShadow::setRightTile(KWindowShadowTile::Ptr tile)
{
    if (d->isCreated) {
        qCWarning(LOG_KWINDOWSYSTEM,
                  "Cannot attach a right tile to a shadow that already has "
                  "native platform resources allocated. To do so, destroy() the shadow and then "
                  "setRightTile() and create()");
        return;
    }
    d->rightTile = tile;
}

KWindowShadowTile::Ptr KWindowShadow::bottomRightTile() const
{
    return d->bottomRightTile;
}

void KWindowShadow::setBottomRightTile(KWindowShadowTile::Ptr tile)
{
    if (d->isCreated) {
        qCWarning(LOG_KWINDOWSYSTEM,
                  "Cannot attach a bottom-right tile to a shadow that already "
                  "has native platform resources allocated. To do so, destroy() the shadow and "
                  "then setBottomRightTile() and create()");
        return;
    }
    d->bottomRightTile = tile;
}

KWindowShadowTile::Ptr KWindowShadow::bottomTile() const
{
    return d->bottomTile;
}

void KWindowShadow::setBottomTile(KWindowShadowTile::Ptr tile)
{
    if (d->isCreated) {
        qCWarning(LOG_KWINDOWSYSTEM,
                  "Cannot attach a bottom tile to a shadow that already has "
                  "native platform resources allocated. To do so, destroy() the shadow and then "
                  "setBottomTile() and create()");
        return;
    }
    d->bottomTile = tile;
}

KWindowShadowTile::Ptr KWindowShadow::bottomLeftTile() const
{
    return d->bottomLeftTile;
}

void KWindowShadow::setBottomLeftTile(KWindowShadowTile::Ptr tile)
{
    if (d->isCreated) {
        qCWarning(LOG_KWINDOWSYSTEM,
                  "Cannot attach a bottom-left tile to a shadow that already "
                  "has native platform resources allocated. To do so, destroy() the shadow and "
                  "then setBottomLeftTile() and create()");
        return;
    }
    d->bottomLeftTile = tile;
}

QMargins KWindowShadow::padding() const
{
    return d->padding;
}

void KWindowShadow::setPadding(const QMargins &padding)
{
    if (d->isCreated) {
        qCWarning(LOG_KWINDOWSYSTEM,
                  "Cannot set the padding on a shadow that already has "
                  "native platform resources allocated. To do so, destroy() the shadow and "
                  "then setPadding() and create()");
        return;
    }
    d->padding = padding;
}

QWindow *KWindowShadow::window() const
{
    return d->window;
}

void KWindowShadow::setWindow(QWindow *window)
{
    if (d->isCreated) {
        qCWarning(LOG_KWINDOWSYSTEM,
                  "Cannot set the target window on a shadow that already has "
                  "native platform resources allocated. To do so, destroy() the shadow and then "
                  "setWindow() and create()");
        return;
    }
    d->window = window;
}

bool KWindowShadow::isCreated() const
{
    return d->isCreated;
}

bool KWindowShadow::create()
{
    if (d->isCreated) {
        return true;
    }
    if (!d->window) {
        qCWarning(LOG_KWINDOWSYSTEM,
                  "Cannot allocate the native platform resources for the shadow "
                  "because the target window is not specified.");
        return false;
    }
    if (!d->prepareTiles()) {
        return false;
    }
    d->isCreated = d->create();
    return d->isCreated;
}

void KWindowShadow::destroy()
{
    if (!d->isCreated) {
        return;
    }
    d->destroy();
    d->isCreated = false;
}

KWindowShadowTilePrivate::~KWindowShadowTilePrivate()
{
}

KWindowShadowTilePrivate *KWindowShadowTilePrivate::get(const KWindowShadowTile *tile)
{
    return tile->d.data();
}

KWindowShadowPrivate::~KWindowShadowPrivate()
{
}

bool KWindowShadowPrivate::create()
{
    return false;
}

void KWindowShadowPrivate::destroy()
{
}

bool KWindowShadowPrivate::prepareTiles()
{
    const std::array<KWindowShadowTile *, 8> tiles{
        leftTile.data(),
        topLeftTile.data(),
        topTile.data(),
        topRightTile.data(),
        rightTile.data(),
        bottomRightTile.data(),
        bottomTile.data(),
        bottomLeftTile.data(),
    };

    for (KWindowShadowTile *tile : tiles) {
        if (!tile) {
            continue;
        }
        if (tile->isCreated()) {
            continue;
        }
        if (!tile->create()) {
            return false;
        }
    }

    return true;
}

bool KWindowShadowTilePrivateDummy::create()
{
    return false;
}

void KWindowShadowTilePrivateDummy::destroy()
{
}

bool KWindowShadowPrivateDummy::create()
{
    return false;
}

void KWindowShadowPrivateDummy::destroy()
{
}

#include "moc_kwindowshadow.cpp"
