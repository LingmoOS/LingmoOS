// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

#include "base/geometry.h"
#include "ui/bitmap.h"

namespace Lingmo::Files {

struct Thumbnail
{
    // The cache file, empty when it could not be written.
    std::string path;
    ui::Bitmap image;
};

// The freedesktop.org thumbnail cache in ~/.cache/thumbnails: entries are PNG
// files named by the MD5 of the file's URI, in the first flavour of 128, 256,
// 512 and 1024 pixels that holds the requested size, and carry the file's URI,
// modification time and size. Images the toolkit decodes are made in process;
// other types go to the thumbnailers installed in the data directories'
// thumbnailers/ folders. Every call may run on any thread.
class ThumbnailCache
{
public:
    static ThumbnailCache* self();

    // Whether the toolkit decodes mimeType or an installed thumbnailer handles it.
    bool canThumbnail(const std::string& mimeType);

    // filePath is a path or a file: URL; sizes are device pixels.
    Thumbnail requestThumbnail(const std::string& filePath, base::Size requestedSize);

    // Whether a cache entry newer than the file exists; it may still turn out stale.
    bool hasCachedThumbnail(const std::string& filePath, base::Size requestedSize);
    // The current cache entry only, never generating one; cheap enough for the UI thread.
    Thumbnail cachedThumbnail(const std::string& filePath, base::Size requestedSize);
    ui::Bitmap generateThumbnail(const std::string& source, base::Size requestedSize);
};

namespace internal {
// Tests keep the cache in a folder of their own; empty restores ~/.cache/thumbnails.
void setThumbnailCacheRoot(const std::string& path);
}

} // namespace Lingmo::Files
