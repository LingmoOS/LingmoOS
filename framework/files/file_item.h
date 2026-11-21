// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "lingmo/files/url.h"

namespace Lingmo::base {
class Locale;
}

namespace Lingmo::Files {

// A file or folder with what was read about it. Copies share their data until
// one of them changes, so lists of thousands of items copy cheaply; one item
// must not be changed and read on two threads at once.
//
// Times are milliseconds since the epoch: nothing here converts to local time,
// which stats the time zone file on every conversion.
class FileItem
{
public:
    enum FileTime {
        CreationTime,
        ModificationTime,
        AccessTime
    };

    FileItem() = default;
    // Reads the file's metadata, and for an item at the top of a trash its info file.
    explicit FileItem(const Url& url);

    const Url& url() const;
    Url mostLocalUrl() const { return Url::fromLocalFile(localPath()); }
    const std::string& localPath() const;
    // The URL's file name, as the listing decoded it.
    const std::string& name() const;
    std::string text() const;

    void setDisplayName(const std::string& name);
    void setIconName(const std::string& icon);

    Url targetUrl() const;
    void setTargetUrl(const Url& url);

    bool isNull() const { return !d; }
    bool isLocalFile() const { return d && d->url.isLocalFile(); }

    // Links followed, so a broken link is neither file nor folder. Read on
    // first use for an item a listing has not prepared.
    bool exists() const;
    bool isFile() const;
    bool isDir() const;
    bool isLink() const;
    bool isHidden() const;
    bool isWritable() const;
    bool isDesktopFile() const;
    uint64_t size() const;

    // For an item at the top of a trash, where it was trashed from and when, as its
    // info file says; empty otherwise.
    std::string originalPath() const;
    std::optional<int64_t> deletionTime() const;

    // Empty when the file system keeps no such time.
    std::optional<int64_t> time(FileTime type) const;

    // Descriptions and icon names by MIME type, shared by the items of one listing.
    using TypeDetails = std::unordered_map<std::string, std::pair<std::string, std::string>>;

    // Reads the metadata the accessors use and detects the type.
    void prepareMetadata();
    // The same, but when previous is this file, unchanged, its type, icon and names
    // are kept instead; returns whether they were.
    bool prepareMetadata(const FileItem& previous, TypeDetails* details);

    // The canonical MIME type; detected now for an item not prepared.
    std::string mimetype() const;
    std::string typeDescription() const;
    std::string iconName() const;

    // Whether both were prepared from a file or directory of the same kind, size and
    // times, which answers for size(), isDir(), isLink() and the modification time.
    bool sameStat(const FileItem& other) const;

    bool operator==(const FileItem& other) const { return d == other.d; }

private:
    friend class FileEnumerator;

    // For a child the enumerator found in a directory it already resolved; name is
    // the URL's file name.
    FileItem(const Url& url, std::string localPath, std::string name);

    // What prepareMetadata() saw, so a later listing can compare without reading
    // the item, which other threads may share.
    struct Stamp
    {
        int64_t size = -1;
        int64_t modified = 0;
        int64_t changed = 0;
        // Set only for a directory or regular file; links are followed.
        unsigned kind = 0;

        bool operator==(const Stamp&) const = default;
    };

    struct Stat
    {
        bool read = false;
        bool exists = false;
        bool link = false;
        bool file = false;
        bool dir = false;
        bool writable = false;
        // The kind of special file, as st_mode has it; 0 for none.
        unsigned special = 0;
        uint64_t size = 0;
        std::optional<int64_t> birth;
        int64_t access = 0;
        int64_t modified = 0;
        int64_t changed = 0;
    };

    struct Data
    {
        Url url;
        std::string name;
        std::string localPath;
        Stat stat;
        Stamp stamp;
        std::string mimeType;
        std::string displayName;
        std::string iconName;
        std::string typeDescription;
        // The Locale::system() the description was read for.
        const base::Locale* descriptionLocale = nullptr;
        Url targetUrl;
        std::string originalPath;
        std::optional<int64_t> deletionTime;
    };

    Data& detach();
    const Stat& stat() const;
    void readStat(Stat* stat) const;
    void detectType(TypeDetails* details);

    std::shared_ptr<Data> d;
};

using FileItemList = std::vector<FileItem>;

class FileItemListProperties
{
public:
    explicit FileItemListProperties(const FileItemList& items)
        : m_items(items)
    {
    }

    bool supportsMoving() const;
    bool supportsDeleting() const { return supportsMoving(); }

private:
    FileItemList m_items;
};

// In powers of 1024 with two decimals in the session's locale, e.g. "1.50 KiB".
std::string convertSize(uint64_t size);

// Empty unless path is the home directory or an XDG user directory, as they were
// on the first call.
std::string standardFolderIconName(const std::string& path);
bool isStandardFolder(const Url& url);
bool canMoveStandardFolderTo(const Url& source, const Url& destination);

} // namespace Lingmo::Files
