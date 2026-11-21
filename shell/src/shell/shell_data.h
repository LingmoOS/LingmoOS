// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <string>
#include <string_view>

#include "render/glass.h"

namespace Lingmo::Shell {

// The shell's own artwork, installed with the compositor.
std::string shellDataFile(std::string_view name);
// Tests read the artwork from the source tree.
void setShellDataDirectoryForTesting(std::string directory);

// FishUI's glass as a client's lingmo_surface_blur_v1 requests carry it:
// every value through wl_fixed, so the material is the one the separate
// shell's surfaces got.
float glassFixed(double value);

// FishUI's GlassEffect defaults (glassmaterial.h), enabled, before a
// component sets its own values.
render::GlassParams fishuiGlass();
// FishUI's PopupGlass: tooltips, menus, the OSD; `tint` premultiplied, from
// rgba components rounded to 16 bits each.
render::GlassParams popupGlass(double red, double green, double blue, double alpha);

} // namespace Lingmo::Shell

