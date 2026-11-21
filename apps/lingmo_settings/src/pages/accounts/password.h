// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

namespace settings {

// The password as AccountsService takes it: SHA-512 crypt(3) with a fresh salt.
std::string cryptPassword(const std::string& password);

} // namespace settings
