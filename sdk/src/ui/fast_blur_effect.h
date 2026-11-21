// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>

#include "ui/bitmap.h"

namespace Lingmo::ui {

// The FastBlur effect's weights of the source and its five halved levels for
// a radius, normalised together.
std::array<double, 6> fastBlurWeights(double radius);

// FastBlur over a premultiplied source texture that covers an item of the
// logical size at the scale: the levels are the source, then the item rounded
// up to multiples of 32 and halved five times, each a four-tap pass over the
// one before, kept in eight bits as the effect's textures are. The result has
// the source's size, Argb32Premultiplied.
Bitmap fastBlur(const Bitmap& source, double logicalWidth, double logicalHeight, double radius, double scale);

} // namespace Lingmo::ui
