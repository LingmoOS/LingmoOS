// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/icontheme/iconthemes.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <utility>

#include "base/locale.h"
#include "base/paths.h"
#include "base/settings.h"

namespace fs = std::filesystem;
using namespace Lingmo::base;

namespace settings {

// The preview draws its icons at this size, doubled for a sharp picture.
static constexpr int kPreviewCell = 22;
static constexpr int kPreviewScale = 2;
static const char* const kPreviewIcons[] = {"folder", "text-x-generic", "image-x-generic", "user-trash"};

static std::vector<std::string> basePaths()
{
    std::vector<std::string> paths = {paths::home() + "/.icons", paths::dataHome() + "/icons"};
    for (const std::string& directory : paths::dataDirectories())
        paths.push_back(directory + "/icons");
    std::vector<std::string> unique;
    for (std::string& path : paths) {
        if (std::find(unique.begin(), unique.end(), path) == unique.end())
            unique.push_back(std::move(path));
    }
    return unique;
}

static std::string indexFile(const std::vector<std::string>& bases, const std::string& id)
{
    for (const std::string& base : bases) {
        const std::string path = base + "/" + id + "/index.theme";
        if (fs::exists(path))
            return path;
    }
    return {};
}

// Scalable artwork first, then the fixed size closest to the preview's,
// then the parents and hicolor.
static std::string findIcon(const std::vector<std::string>& bases, const std::string& id, const std::string& name,
                            std::vector<std::string>& visited)
{
    if (std::find(visited.begin(), visited.end(), id) != visited.end())
        return {};
    visited.push_back(id);
    const std::string indexPath = indexFile(bases, id);
    if (indexPath.empty())
        return {};
    const Settings index(indexPath);
    std::vector<std::pair<int, std::string>> ranked;
    for (const std::string& directory : index.stringListValue("Icon Theme/Directories")) {
        const std::string type = index.stringValue(directory + "/Type", "Threshold");
        const int size = int(index.intValue(directory + "/Size", 0));
        const int rank = type == "Scalable" ? 0 : 1 + std::abs(size - kPreviewCell * kPreviewScale);
        ranked.emplace_back(rank, directory);
    }
    std::stable_sort(ranked.begin(), ranked.end());
    for (const auto& [rank, directory] : ranked) {
        for (const std::string& base : bases) {
            const std::string prefix = base + "/" + id + "/" + directory + "/" + name;
            for (const char* suffix : {".svg", ".png"}) {
                if (fs::exists(prefix + suffix))
                    return prefix + suffix;
            }
        }
    }
    std::vector<std::string> parents = index.stringListValue("Icon Theme/Inherits");
    parents.push_back("hicolor");
    for (const std::string& parent : parents) {
        std::string found = findIcon(bases, parent, name, visited);
        if (!found.empty())
            return found;
    }
    return {};
}

std::vector<IconTheme> IconTheme::installed()
{
    const std::vector<std::string> bases = basePaths();
    std::vector<IconTheme> themes;
    std::vector<std::string> seen;
    for (const std::string& base : bases) {
        std::error_code error;
        std::vector<std::string> ids;
        for (const auto& entry : fs::directory_iterator(base, error)) {
            if (entry.is_directory(error))
                ids.push_back(entry.path().filename().string());
        }
        std::sort(ids.begin(), ids.end());
        for (const std::string& id : ids) {
            const std::string indexPath = base + "/" + id + "/index.theme";
            if (std::find(seen.begin(), seen.end(), id) != seen.end() || !fs::exists(indexPath))
                continue;
            seen.push_back(id);
            const Settings index(indexPath);
            // Cursor themes and fallback themes such as hicolor carry index.theme too.
            if (index.boolValue("Icon Theme/Hidden", false) || !index.contains("Icon Theme/Directories"))
                continue;
            IconTheme theme;
            theme.id = id;
            theme.name = index.stringValue("Icon Theme/Name", id);
            for (const char* icon : kPreviewIcons) {
                std::vector<std::string> visited;
                theme.previewFiles.push_back(findIcon(bases, id, icon, visited));
            }
            themes.push_back(std::move(theme));
        }
    }
    const Locale locale = Locale::system();
    std::stable_sort(themes.begin(), themes.end(), [&locale](const IconTheme& a, const IconTheme& b) {
        return locale.compare(a.name, b.name) < 0;
    });
    return themes;
}

} // namespace settings
