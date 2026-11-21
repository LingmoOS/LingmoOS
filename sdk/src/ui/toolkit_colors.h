// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/color_mixer.h"

namespace Lingmo::ui {

// The toolkit's recipes, which every provider starts from.
void addToolkitColors(ColorMixer& mixer, const Theme& theme);

} // namespace Lingmo::ui
