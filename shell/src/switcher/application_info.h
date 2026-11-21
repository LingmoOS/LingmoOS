// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "base/lru_cache.h"

namespace Lingmo::Switcher {

// What the dock shows for an application it cannot find either.
inline const std::string kFallbackIcon = "application-x-desktop";

// What the application switcher shows for an application.
struct ApplicationInfo {
    std::string name;
    // An icon theme name, or an absolute path.
    std::string icon;
};

// Finds an application's desktop entry from its window's application id, as
// the dock does: the entry named after the id, or one whose StartupWMClass is
// the id. Answers, found or not, are kept until an application directory
// changes.
class ApplicationInfoCache
{
public:
    // The applications directories, the user's first; the toolkit's by default.
    ApplicationInfoCache();
    explicit ApplicationInfoCache(std::vector<std::string> directories);

    // Forgets every answer once an application directory changed. Checked once
    // per list rather than once per entry: it reads each directory's time.
    void refresh();

    // `fallbackName` names an application with no desktop entry.
    ApplicationInfo lookup(const std::string &applicationId, const std::string &fallbackName);

private:
    // Lower-cased StartupWMClass to desktop file, read once per stamp.
    const std::unordered_map<std::string, std::string> &wmClasses();
    int64_t applicationsStamp() const;

    std::vector<std::string> m_directories;
    int64_t m_stamp = -1;
    // Bounded, least recently used out first: a lookup that found nothing is
    // cached like one that did, and application ids come from clients.
    base::LruCache<std::string, ApplicationInfo> m_cache {256};
    std::optional<std::unordered_map<std::string, std::string>> m_wmClasses;
};

} // namespace Lingmo::Switcher

