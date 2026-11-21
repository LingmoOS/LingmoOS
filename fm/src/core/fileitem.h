// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace fm {

// A file as a folder listing found it: what the views show and sort by, read
// on the worker that listed it. Times are milliseconds since the epoch, UTC.
struct FileItem
{
    // file:///… for local files; trash:/… for the top of the trash.
    std::string url;
    // The file on disk, also for an item in the trash.
    std::string path;
    std::string name;
    // A .desktop file's own name; empty for the file name.
    std::string displayName;
    std::string mimeType;
    std::string iconName;
    std::string typeDescription;
    // A link's or .desktop file's target, when it has one.
    std::string targetUrl;
    // For an item at the top of a trash: where it was trashed from and when.
    std::string originalPath;
    int64_t deletionTime = 0;
    // The collation key of the name the folder sorts it by (a standard
    // folder's shown name), made with the snapshot; empty until then.
    std::string sortKey;
    uint64_t size = 0;
    // Present for a Computer view disk entry; ordinary files leave these empty.
    std::optional<uint64_t> storageUsed;
    std::optional<uint64_t> storageTotal;
    std::string filesystemType;
    std::string mountPoint;
    int64_t modified = 0;
    int64_t accessed = 0;
    int64_t created = 0;
    bool isDir = false;
    bool isFile = false;
    bool isLink = false;
    bool isHidden = false;
    bool isWritable = false;

    const std::string& text() const { return displayName.empty() ? name : displayName; }
    bool isDesktopFile() const { return name.ends_with(".desktop"); }
    bool isLocalFile() const { return url.starts_with("file://"); }
    // Kind, size and times: what tells a file changed.
    bool sameStat(const FileItem& other) const
    {
        return isDir == other.isDir && isLink == other.isLink && size == other.size
                && modified == other.modified;
    }
    bool sameMetadata(const FileItem& other) const;
};

using FileItemPtr = std::shared_ptr<const FileItem>;
using FileItemList = std::vector<FileItemPtr>;

// "1.5 MB": powers of 1000 with one decimal, as the file manager writes sizes.
std::string formatSize(uint64_t bytes);

// The type description of a MIME type in the session language; the type
// itself when there is none.
std::string mimeComment(const std::string& mimeType);

} // namespace fm
