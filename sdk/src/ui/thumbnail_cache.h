// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"

namespace Lingmo::ui::thumbnails {

// The thumbnail of a file for a box of device pixels, from the freedesktop
// cache in ~/.cache/thumbnails or made and stored there, as lingmo-framework's
// ThumbnailCache does, so both share entries: the flavour is
// the first of 128, 256, 512 and 1024 that holds the box's longer side, and a
// file that cannot be read is marked in fail/lingmo. Null for such a file.
// On any thread; it decodes and writes files.
sk_sp<SkImage> request(const std::string& path, int width, int height);

// The cache file's URI key: file:// and the path percent-encoded but for RFC
// 3986's path characters and brackets.
std::string fileUri(const std::string& absolutePath);

} // namespace Lingmo::ui::thumbnails
