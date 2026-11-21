// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "include/core/SkColorType.h"
#include "include/core/SkPixmap.h"
#include "ui/bitmap.h"

namespace Lingmo::ui {

SkColorType skColorType(Bitmap::Format format);
// Read-only view of the pixels.
SkPixmap skPixmap(const Bitmap& bitmap);

} // namespace Lingmo::ui
