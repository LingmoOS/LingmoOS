// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/cursortheme/cursorthemes.h"

#include <X11/Xcursor/Xcursor.h>
#include <algorithm>
#include <filesystem>

#include "base/paths.h"
#include "base/settings.h"

namespace fs = std::filesystem;
using namespace Lingmo::base;

namespace settings {

static constexpr int kSampleSize = 36;
static constexpr int kPadding = 2;

static std::vector<std::string> searchPaths()
{
    std::vector<std::string> list = {paths::home() + "/.icons", paths::dataHome() + "/icons"};
    for (const std::string& directory : paths::dataDirectories())
        list.push_back(directory + "/icons");
    list.push_back("/usr/share/pixmaps");
    std::vector<std::string> unique;
    for (std::string& path : list) {
        if (std::find(unique.begin(), unique.end(), path) == unique.end())
            unique.push_back(std::move(path));
    }
    return unique;
}

static bool hasFiles(const fs::path& directory)
{
    std::error_code error;
    for (const auto& entry : fs::directory_iterator(directory, error)) {
        if (entry.is_regular_file(error) || entry.is_symlink(error))
            return true;
    }
    return false;
}

static std::string themeFile(const fs::path& directory)
{
    return fs::exists(directory / "index.theme") ? (directory / "index.theme").string()
                                                 : (directory / "cursor.theme").string();
}

CursorTheme::List CursorTheme::installed()
{
    List list;
    bool defaultFound = false;
    for (const std::string& base : searchPaths()) {
        std::error_code error;
        std::vector<fs::path> directories;
        for (const auto& entry : fs::directory_iterator(base, error)) {
            if (entry.is_directory(error))
                directories.push_back(entry.path());
        }
        std::sort(directories.begin(), directories.end());
        for (const fs::path& directory : directories) {
            const std::string id = directory.filename().string();
            if (!defaultFound && id == "default") {
                if (fs::is_symlink(directory, error)) {
                    const fs::path target = fs::read_symlink(directory, error);
                    if (fs::exists(fs::absolute(directory.parent_path() / target), error)) {
                        list.defaultTheme = target.filename().string();
                        defaultFound = true;
                    }
                    continue;
                }
                if (!hasFiles(directory / "cursors") && fs::exists(directory / "index.theme")) {
                    const Settings config(themeFile(directory));
                    list.defaultTheme = config.stringValue("Icon Theme/Inherits");
                    defaultFound = true;
                    continue;
                }
            }
            if (!fs::exists(directory / "cursors"))
                continue;
            if (!fs::exists(directory / "index.theme") && !fs::exists(directory / "cursor.theme"))
                continue;
            if (std::any_of(list.themes.begin(), list.themes.end(),
                            [&id](const CursorTheme& theme) { return theme.id == id; }))
                continue;
            const Settings config(themeFile(directory));
            CursorTheme theme;
            theme.id = id;
            theme.path = directory.string();
            theme.name = config.stringValue("Icon Theme/Name", id);
            theme.m_sample = config.stringValue("Icon Theme/Example", "left_ptr");
            list.themes.push_back(std::move(theme));
        }
    }
    return list;
}

Lingmo::ui::Image CursorTheme::preview() const
{
    XcursorImages* images = XcursorLibraryLoadImages(m_sample.c_str(), id.c_str(), kSampleSize);
    if (!images)
        images = XcursorLibraryLoadImages("left_ptr", id.c_str(), kSampleSize);
    if (!images)
        return {};
    Lingmo::ui::Image result;
    if (images->nimage > 0) {
        const XcursorImage* frame = images->images[0];
        const int width = int(frame->width);
        const int height = int(frame->height);
        int left = width, top = height, right = -1, bottom = -1;
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                if ((frame->pixels[y * width + x] >> 24) == 0)
                    continue;
                left = std::min(left, x);
                top = std::min(top, y);
                right = std::max(right, x);
                bottom = std::max(bottom, y);
            }
        }
        if (right < left) {
            left = top = 0;
            right = width - 1;
            bottom = height - 1;
        }
        const int contentWidth = right - left + 1;
        const int contentHeight = bottom - top + 1;
        const int extent = std::max(contentWidth, contentHeight) + kPadding * 2;
        result = Lingmo::ui::Image(extent, extent);
        const int offsetX = (extent - contentWidth) / 2;
        const int offsetY = (extent - contentHeight) / 2;
        for (int y = 0; y < contentHeight; ++y) {
            for (int x = 0; x < contentWidth; ++x) {
                // Xcursor pixels are premultiplied ARGB; Image keeps them straight.
                const uint32_t pixel = frame->pixels[(top + y) * width + left + x];
                const uint32_t alpha = pixel >> 24;
                uint32_t straight = 0;
                if (alpha) {
                    const auto channel = [alpha](uint32_t value) { return std::min(255u, (value * 255 + alpha / 2) / alpha); };
                    straight = alpha << 24 | channel(pixel >> 16 & 0xff) << 16 | channel(pixel >> 8 & 0xff) << 8
                            | channel(pixel & 0xff);
                }
                result.bits()[size_t(offsetY + y) * extent + offsetX + x] = straight;
            }
        }
    }
    XcursorImagesDestroy(images);
    return result;
}

} // namespace settings
