// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "base/geometry.h"
#include "input/cursor_shape.h"
#include "ui/bitmap.h"

namespace Lingmo::Cursor {

struct Frame {
    // Argb32Premultiplied.
    ui::Bitmap image;
    base::Point hotspot;
    int delayMs = 0;
};

std::string resourceName(input::CursorShape shape);
std::vector<Frame> readXCursor(std::string_view bytes, int size);
std::vector<Frame> load(input::CursorShape shape, int size, const std::string &theme = "default");
std::vector<std::string> themePaths();
// Replaces the search path; an empty list restores the standard one.
void setThemePathsForTesting(std::vector<std::string> paths);
bool validThemeName(std::string_view theme);
int defaultSize();

} // namespace Lingmo::Cursor
