// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string_view>

namespace settings::network {

// Empty, or a number from 1 to 65535, as the proxy's port fields take it.
bool isPortText(std::string_view text);
// Empty, or digits making a number from 0 to `maximum`.
bool isNumberText(std::string_view text, long long maximum);

} // namespace settings::network
