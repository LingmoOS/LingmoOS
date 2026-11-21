// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <map>
#include <string>
#include <vector>

namespace Lingmo::Core {

// A process environment by variable name.
using Environment = std::map<std::string, std::string>;

// This process's environment.
Environment processEnvironment();
// `NAME=value` entries, for base::Process::setEnvironment() and execve().
std::vector<std::string> environmentEntries(const Environment& environment);

} // namespace Lingmo::Core
