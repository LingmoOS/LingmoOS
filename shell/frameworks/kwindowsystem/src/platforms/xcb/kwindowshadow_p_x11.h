/*
    SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KWINDOWSHADOW_P_X11_H
#define KWINDOWSHADOW_P_X11_H

#include "kwindowshadow_p.h"

#include <xcb/xcb.h>

class KWindowShadowTilePrivateX11 final : public KWindowShadowTilePrivate
{
public:
    bool create() override;
    void destroy() override;

    static KWindowShadowTilePrivateX11 *get(const KWindowShadowTile *tile);

    xcb_pixmap_t pixmap = XCB_PIXMAP_NONE;
    xcb_gcontext_t gc = XCB_NONE;
};

class KWindowShadowPrivateX11 final : public KWindowShadowPrivate
{
public:
    bool create() override;
    void destroy() override;

    KWindowShadowTile::Ptr getOrCreateEmptyTile();

    KWindowShadowTile::Ptr emptyTile;
};

#endif // KWINDOWSHADOW_P_X11_H
