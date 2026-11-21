// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <string_view>

namespace Lingmo::Files::internal {

// RFC 1321 MD5 in lower-case hex, which names thumbnail cache entries.
std::string md5Hex(std::string_view data);

} // namespace Lingmo::Files::internal
