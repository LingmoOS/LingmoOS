// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"

class SkCanvas;

namespace Lingmo::ui {

// An image clipped to a rounded rectangle through two layers, as a GPU scene
// graph renders it: the image drawn into a layer texture of ceil(size) * dpr
// pixels, the mask's rounded rectangle snapped and drawn into one of its
// own, and the effect sampling both linearly where the item's quad covers
// pixel centres, at its exact position.
struct MaskedLayerItem
{
    // The Image item in logical window coordinates, unsnapped.
    double x = 0;
    double y = 0;
    double width = 0;
    double height = 0;
    double scale = 1;
    enum class FillMode {
        Stretch,
        PreserveAspectFit,
        PreserveAspectCrop,
    } fillMode = FillMode::Stretch;
    bool smooth = true;
    bool mipmap = false;
    // The mask's rounded rectangle.
    double radius = 0;
    bool continuousCorners = false;
};

// The layer and the mask as textures; they do not depend on where the item is.
struct MaskedLayerTextures
{
    sk_sp<SkImage> layer;
    sk_sp<SkImage> mask;
    double width = 0;
    double height = 0;
    double scale = 1;
};

MaskedLayerTextures renderMaskedLayerTextures(const SkImage& picture, const MaskedLayerItem& item);
// The effect: both textures filtered where the quad of the item at x, y
// (logical) covers pixel centres, on a canvas in device pixels.
void drawMaskedLayer(SkCanvas* canvas, const MaskedLayerTextures& textures, double x, double y, float opacity);

} // namespace Lingmo::ui
