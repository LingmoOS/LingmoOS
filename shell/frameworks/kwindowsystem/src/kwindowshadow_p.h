/*
    SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KWINDOWSHADOW_P_H
#define KWINDOWSHADOW_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KF API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "kwindowshadow.h"

#include <QPointer>

class KWINDOWSYSTEM_EXPORT KWindowShadowTilePrivate
{
public:
    virtual ~KWindowShadowTilePrivate();

    virtual bool create() = 0;
    virtual void destroy() = 0;

    static KWindowShadowTilePrivate *get(const KWindowShadowTile *tile);

    QImage image;
    bool isCreated = false;
};

class KWINDOWSYSTEM_EXPORT KWindowShadowPrivate
{
public:
    virtual ~KWindowShadowPrivate();

    virtual bool create() = 0;
    virtual void destroy() = 0;

    bool prepareTiles();

    QPointer<QWindow> window;
    KWindowShadowTile::Ptr leftTile;
    KWindowShadowTile::Ptr topLeftTile;
    KWindowShadowTile::Ptr topTile;
    KWindowShadowTile::Ptr topRightTile;
    KWindowShadowTile::Ptr rightTile;
    KWindowShadowTile::Ptr bottomRightTile;
    KWindowShadowTile::Ptr bottomTile;
    KWindowShadowTile::Ptr bottomLeftTile;
    QMargins padding;
    bool isCreated = false;
};

#endif // KWINDOWSHADOW_P_H
