// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "ui/geometry.h"

namespace settings {

class Page;
class SettingsWindow;

// Keeps telling whether a page's hardware or service is there, from the
// first answer on; dropping it stops.
class AvailabilityWatch
{
public:
    virtual ~AvailabilityWatch() = default;
};

using AvailabilityCallback = std::function<void(bool available)>;
using PageFactory = std::function<std::unique_ptr<Page>(SettingsWindow& window)>;
using AvailabilityFactory =
        std::function<std::unique_ptr<AvailabilityWatch>(SettingsWindow& window, AvailabilityCallback callback)>;

struct PageInfo
{
    // Unique among the pages.
    std::string id;
    // What -m and switchToPage() take; empty for pages they cannot open.
    std::string name;
    std::string title;
    // A file under images/sidebar/dark, or a toolkit glyph name.
    std::string icon;
    Lingmo::ui::Color iconColor;
    // Sidebar pages only.
    std::string category;
    // Comma-separated search terms.
    std::string keywords;
    // Sub pages: the id of the sidebar page they are opened from.
    std::string parentId;
    // Whether the entry is there before its watch first answers: entries that
    // come with hardware join the sidebar later, others may leave it.
    bool initiallyAvailable = true;
    // From the page's PageRegistration; null while the page is not built.
    PageFactory create;
    // Null for a page that is always there.
    AvailabilityFactory watchAvailability;
};

// Plugs a page into its entry from the page's own source file, so adding a
// page touches no shared file:
//   static const PageRegistration registration("about", [](SettingsWindow& window) {
//       return std::make_unique<AboutPage>(window);
//   });
class PageRegistration
{
public:
    PageRegistration(const char* id, PageFactory create, AvailabilityFactory watchAvailability = {});
};

// Sidebar pages in sidebar order, then sub pages.
const std::vector<PageInfo>& pages();
const PageInfo* findPage(const std::string& id);

} // namespace settings
