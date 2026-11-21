// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cursor/theme.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <limits>
#include <map>
#include <set>
#include <sys/stat.h>

#include "base/byte_stream.h"
#include "base/file.h"
#include "base/paths.h"
#include "base/settings.h"
#include "base/strings.h"
#include "ui/bitmap_painter.h"

namespace Lingmo::Cursor {

std::string resourceName(input::CursorShape shape)
{
    static constexpr std::array names = {"",
                                         "default",
                                         "context-menu",
                                         "help",
                                         "pointer",
                                         "progress",
                                         "wait",
                                         "cell",
                                         "crosshair",
                                         "text",
                                         "vertical-text",
                                         "alias",
                                         "copy",
                                         "move",
                                         "no-drop",
                                         "not-allowed",
                                         "grab",
                                         "grabbing",
                                         "e-resize",
                                         "n-resize",
                                         "ne-resize",
                                         "nw-resize",
                                         "s-resize",
                                         "se-resize",
                                         "sw-resize",
                                         "w-resize",
                                         "ew-resize",
                                         "ns-resize",
                                         "nesw-resize",
                                         "nwse-resize",
                                         "col-resize",
                                         "row-resize",
                                         "all-scroll",
                                         "zoom-in",
                                         "zoom-out"};
    const auto index = static_cast<size_t>(shape);
    return names[index < names.size() ? index : 1];
}

std::vector<Frame> readXCursor(std::string_view bytes, int size)
{
    base::ByteReader in(bytes, std::endian::little);
    const uint32_t magic = in.read<uint32_t>();
    const uint32_t header = in.read<uint32_t>();
    in.read<uint32_t>(); // version
    const uint32_t count = in.read<uint32_t>();
    if (!in.ok() || magic != 0x72756358 || header < 16 || count > 4096
        || uint64_t(header) + uint64_t(count) * 12 > uint64_t(bytes.size()))
        return {};
    in.seek(header);

    struct Entry {
        uint32_t size;
        uint32_t offset;
    };

    std::vector<Entry> entries;
    uint32_t bestSize = 0;
    int64_t bestDistance = std::numeric_limits<int64_t>::max();
    for (uint32_t i = 0; i < count; ++i) {
        const uint32_t type = in.read<uint32_t>();
        const uint32_t nominal = in.read<uint32_t>();
        const uint32_t offset = in.read<uint32_t>();
        if (type != 0xfffd0002)
            continue;
        entries.push_back({nominal, offset});
        const auto distance = std::abs(int64_t(nominal) - size);
        if (distance < bestDistance || (distance == bestDistance && nominal > bestSize)) {
            bestDistance = distance;
            bestSize = nominal;
        }
    }
    std::vector<Frame> result;
    uint64_t totalPixels = 0;
    for (const auto &entry : entries) {
        if (entry.size != bestSize)
            continue;
        if (uint64_t(entry.offset) + 36 > uint64_t(bytes.size()))
            return {};
        in.seek(entry.offset);
        const uint32_t chunkHeader = in.read<uint32_t>();
        const uint32_t type = in.read<uint32_t>();
        const uint32_t nominal = in.read<uint32_t>();
        in.read<uint32_t>(); // version
        const uint32_t width = in.read<uint32_t>();
        const uint32_t height = in.read<uint32_t>();
        const uint32_t xhot = in.read<uint32_t>();
        const uint32_t yhot = in.read<uint32_t>();
        const uint32_t delay = in.read<uint32_t>();
        totalPixels += uint64_t(width) * height;
        if (chunkHeader < 36 || type != 0xfffd0002 || nominal != bestSize || !width || !height
            || width > 256 || height > 256 || xhot >= width || yhot >= height
            || totalPixels > 16 * 1024 * 1024
            || uint64_t(entry.offset) + chunkHeader + uint64_t(width) * height * 4
                    > uint64_t(bytes.size()))
            return {};
        in.seek(uint64_t(entry.offset) + chunkHeader);
        ui::Bitmap image(int(width), int(height), ui::Bitmap::Format::Argb32Premultiplied);
        if (image.isNull())
            return {};
        for (uint32_t y = 0; y < height; ++y) {
            auto *row = reinterpret_cast<uint32_t *>(image.scanLine(int(y)));
            for (uint32_t x = 0; x < width; ++x)
                row[x] = in.read<uint32_t>();
        }
        if (!in.ok())
            return {};
        result.push_back({std::move(image), base::Point(int(xhot), int(yhot)),
                          int(std::clamp(delay, 1u, 60000u))});
    }
    return result;
}

bool validThemeName(std::string_view theme)
{
    return !theme.empty() && base::strings::utf16Length(theme) <= 255 && theme != "."
            && theme != ".." && theme.find_first_of(std::string_view("/\\\0", 3))
            == std::string_view::npos;
}

static std::vector<std::string> &testThemePaths()
{
    static std::vector<std::string> paths;
    return paths;
}

void setThemePathsForTesting(std::vector<std::string> paths)
{
    testThemePaths() = std::move(paths);
}

std::vector<std::string> themePaths()
{
    std::vector<std::string> paths = testThemePaths();
    if (paths.empty()) {
        paths.push_back(base::paths::home() + "/.icons");
        paths.push_back(base::paths::dataHome() + "/icons");
        for (const auto &path : base::paths::dataDirectories())
            paths.push_back(path + "/icons");
        paths.push_back("/usr/share/pixmaps");
    }
    for (auto &path : paths) {
        if (path.starts_with("~/"))
            path.replace(0, 1, base::paths::home());
    }

    std::vector<std::string> unique;
    for (auto &path : paths) {
        if (std::find(unique.begin(), unique.end(), path) == unique.end())
            unique.push_back(std::move(path));
    }
    return unique;
}

// Up to 64 MiB of the file; nothing when it cannot be read.
static std::optional<std::string> readCursorFile(const std::string &path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0 || !S_ISREG(info.st_mode) || info.st_size > 64 * 1024 * 1024)
        return std::nullopt;
    return base::file::read(path);
}

std::vector<Frame> load(input::CursorShape shape, int size, const std::string &theme)
{
    if (shape == input::CursorShape::Hidden)
        return {};
    const auto paths = themePaths();
    std::vector<std::string> themes;
    std::set<std::string> visited;
    const auto collect = [&](auto &&self, const std::string &name) -> void {
        if (!validThemeName(name) || visited.contains(name) || visited.size() >= 64)
            return;
        visited.insert(name);
        themes.push_back(name);
        for (const auto &path : paths) {
            base::Settings index(path + "/" + name + "/index.theme");
            const auto parents = index.stringListValue("Icon Theme/Inherits");
            for (const auto &parent : parents)
                self(self, std::string(base::strings::trimmed(parent)));
        }
    };
    collect(collect, theme);
    collect(collect, "default");
    collect(collect, "Adwaita");
    std::vector<std::string> names {resourceName(shape)};
    static const std::map<std::string, std::vector<std::string>> aliases {
            {"default", {"left_ptr", "arrow"}},
            {"pointer", {"hand2", "hand1"}},
            {"text", {"xterm"}},
            {"wait", {"watch"}},
            {"progress", {"left_ptr_watch"}},
            {"move", {"fleur"}},
            {"ew-resize", {"sb_h_double_arrow", "size_hor"}},
            {"ns-resize", {"sb_v_double_arrow", "size_ver"}},
            {"nesw-resize", {"fd_double_arrow", "size_bdiag"}},
            {"nwse-resize", {"bd_double_arrow", "size_fdiag"}},
            {"not-allowed", {"crossed_circle"}},
            {"help", {"question_arrow"}},
            {"n-resize", {"top_side"}},
            {"s-resize", {"bottom_side"}},
            {"e-resize", {"right_side"}},
            {"w-resize", {"left_side"}},
            {"ne-resize", {"top_right_corner"}},
            {"nw-resize", {"top_left_corner"}},
            {"se-resize", {"bottom_right_corner"}},
            {"sw-resize", {"bottom_left_corner"}},
            {"grab", {"openhand"}},
            {"grabbing", {"closedhand"}}};
    if (const auto alias = aliases.find(names.front()); alias != aliases.end())
        names.insert(names.end(), alias->second.begin(), alias->second.end());
    const auto find = [&](const std::vector<std::string> &candidates) -> std::vector<Frame> {
        for (const auto &themeName : themes) {
            for (const auto &name : candidates) {
                for (const auto &path : paths) {
                    const auto bytes = readCursorFile(path + "/" + themeName + "/cursors/" + name);
                    if (!bytes)
                        continue;
                    auto frames = readXCursor(*bytes, size);
                    if (!frames.empty())
                        return frames;
                }
            }
        }
        return {};
    };
    auto frames = find(names);
    if (frames.empty())
        frames = find({"default", "left_ptr"});
    if (!frames.empty())
        return frames;
    // Keep the pointer visible even when no usable cursor theme is installed.
    const int pixels = std::clamp(size, 16, 256);
    ui::Bitmap image(pixels, pixels, ui::Bitmap::Format::Argb32Premultiplied);
    {
        ui::BitmapPainter painter(image);
        painter.setAntialiasing(false);
        painter.scale(pixels / 24.0, pixels / 24.0);
        painter.drawPolygon({{1, 1}, {1, 20}, {6, 15}, {10, 23}, {13, 21}, {9, 14}, {17, 14}},
                            ui::Color::rgb(0x000000), ui::Color::rgb(0xffffff), 1,
                            ui::LineJoin::Bevel);
    }
    return {{image, base::Point(pixels / 24, pixels / 24), 0}};
}

int defaultSize()
{
    return 24;
}

} // namespace Lingmo::Cursor
