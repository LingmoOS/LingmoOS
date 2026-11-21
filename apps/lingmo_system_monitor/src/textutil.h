// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <string_view>

// UTF-8 text compared as Unicode case folding does.
namespace textutil {

// Without white space at either end, by Unicode's definition.
std::string trimmed(std::string_view text);
// Whether the text contains the other one, ignoring case.
bool containsIgnoringCase(std::string_view text, std::string_view part);

} // namespace textutil
