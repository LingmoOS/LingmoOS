// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string_view>
#include <utility>
#include <vector>

// The application's own glyphs, by name, as SVG.
const std::vector<std::pair<std::string_view, std::string_view>>& embeddedIcons();
