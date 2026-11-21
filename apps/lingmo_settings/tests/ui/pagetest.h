// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <string>

#include "ui/geometry.h"
#include "ui/window.h"
#include "window/settingswindow.h"

// Opens a page in a headless settings window at 150%, as the references show
// it, and reads its layout back.
class PageFixture
{
public:
    explicit PageFixture(const std::string& name, const std::string& subPage = {})
        : window(std::make_unique<settings::SettingsWindow>())
    {
        window->window().setScale(1.5);
        window->window().resize({900, 820});
        window->switchToPage(name);
        if (!subPage.empty())
            window->pushPage(subPage);
        window->reveal();
        window->window().grab();
    }

    std::string dump() const { return window->window().layoutDump(); }

    // How many views of the type the page shows, hidden ones and those
    // under a hidden view left out.
    int count(const std::string& type) const
    {
        const std::string text = dump();
        int found = 0;
        // The indentation of the shallowest hidden view above the line; -1 for none.
        int hiddenDepth = -1;
        size_t start = 0;
        while (start < text.size()) {
            size_t end = text.find('\n', start);
            if (end == std::string::npos)
                end = text.size();
            const std::string line = text.substr(start, end - start);
            start = end + 1;
            const int depth = int(line.find_first_not_of(' '));
            if (hiddenDepth >= 0 && depth <= hiddenDepth)
                hiddenDepth = -1;
            if (hiddenDepth >= 0)
                continue;
            if (line.ends_with(" hidden")) {
                hiddenDepth = depth;
                continue;
            }
            if (line.compare(size_t(depth), type.size() + 1, type + " ") == 0)
                ++found;
        }
        return found;
    }

    std::unique_ptr<settings::SettingsWindow> window;
};
