// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common/environment.h"

#include <cstring>

extern char** environ;

namespace Lingmo::Core {

Environment processEnvironment()
{
    Environment environment;
    for (char** entry = environ; entry && *entry; ++entry) {
        const char* equals = std::strchr(*entry, '=');
        if (!equals || equals == *entry)
            continue;
        environment.emplace(std::string(*entry, size_t(equals - *entry)), std::string(equals + 1));
    }
    return environment;
}

std::vector<std::string> environmentEntries(const Environment& environment)
{
    std::vector<std::string> entries;
    entries.reserve(environment.size());
    for (const auto& [name, value] : environment)
        entries.push_back(name + '=' + value);
    return entries;
}

} // namespace Lingmo::Core
