// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

#include "ui/image.h"

namespace settings {

// The cursor themes installed for the user and the system.
struct CursorTheme
{
    std::string id;
    std::string name;
    std::string path;
    // The theme's example cursor, its drawing centred in a square with 2 px
    // around it, as 36 px artwork.
    Lingmo::ui::Image preview() const;

    struct List
    {
        std::vector<CursorTheme> themes;
        // The theme "default" stands for, from its link or its Inherits.
        std::string defaultTheme;
    };
    static List installed();

private:
    std::string m_sample = "left_ptr";
};

} // namespace settings
