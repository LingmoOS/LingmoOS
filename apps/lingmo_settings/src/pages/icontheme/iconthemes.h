// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

namespace settings {

// The icon themes installed for the user and the system.
struct IconTheme
{
    std::string id;
    std::string name;
    // The preview's icons: folder, text, image and trash; empty where the
    // theme and its parents have none.
    std::vector<std::string> previewFiles;

    // Sorted by name; hidden and fallback-only themes left out.
    static std::vector<IconTheme> installed();
};

} // namespace settings
