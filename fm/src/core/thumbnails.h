// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"

#include "ui/bitmap.h"

#include <cstdint>
#include <functional>
#include <string>

namespace fm::thumbnails {

// Whether files of the type get a thumbnail instead of their type's icon.
bool canThumbnail(const std::string& mimeType);
// Under a mount of lingmo-mountd: a phone's or a server's file, which a
// thumbnail would read whole.
bool isOnMount(const std::string& path);

// A file's thumbnail for a box of device pixels: from memory, else the disk
// cache's entry read at once, so a first frame has it, else null and loaded
// on a worker, `done` called here once it is there. A changed modification
// time makes a new entry.
ui::Bitmap cached(const std::string& path, int64_t modified, int width, int height);
void load(const std::string& path, int64_t modified, int width, int height,
          std::function<void(const ui::Bitmap& image)> done);

} // namespace fm::thumbnails
