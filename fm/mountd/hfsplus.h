// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "diskimage.h"

#include "base/lru_cache.h"

#include <cstdint>
#include <memory>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

// An HFS+ or HFSX volume, read only. Names come as UTF-8 with '/' shown as ':',
// hard links resolve to the file they share, and compressed files read as their
// contents.
class HfsPlus
{
public:
    struct Extent
    {
        uint32_t start = 0;
        uint32_t count = 0;
    };

    struct Fork
    {
        uint64_t logicalSize = 0;
        uint32_t totalBlocks = 0;
        // The first eight; the extents overflow file has the rest.
        std::vector<Extent> extents;
    };

    struct Item
    {
        uint32_t id = 0;
        bool folder = false;
        mode_t mode = 0;
        nlink_t links = 1;
        // As the reader sees it, uncompressed.
        uint64_t size = 0;
        time_t created = 0;
        time_t modified = 0;
        time_t changed = 0;
        time_t accessed = 0;
        bool compressed = false;
        Fork data;
        Fork resource;
        // As stored: a hard link names the file it shares through these.
        uint32_t fileType = 0;
        uint32_t creator = 0;
        uint32_t special = 0;
    };

    // Reads a file's contents.
    class File
    {
    public:
        virtual ~File() = default;
        // A byte count, or a negative errno.
        virtual int read(char *buffer, size_t size, uint64_t offset) = 0;
    };

    HfsPlus(DiskImage *disk, uint64_t offset, uint64_t size);
    ~HfsPlus();

    HfsPlus(const HfsPlus &) = delete;
    HfsPlus &operator=(const HfsPlus &) = delete;

    // False with a message the user can read.
    bool open(std::string *message);

    const std::string &name() const
    {
        return m_name;
    }

    uint32_t blockSize() const
    {
        return m_blockSize;
    }
    uint64_t totalBlocks() const
    {
        return m_totalBlocks;
    }
    uint64_t freeBlocks() const
    {
        return m_freeBlocks;
    }
    uint64_t fileCount() const
    {
        return m_fileCount;
    }

    // 0, or a negative errno. path is absolute within the volume.
    int lookup(const std::string &path, Item *item);
    int list(const Item &folder, std::vector<std::pair<std::string, Item>> *entries);
    int openFile(const Item &item, std::unique_ptr<File> *file);
    int readLink(const Item &item, std::string *target);

    // A fork's bytes; false past its end or on a damaged volume.
    bool readFork(uint32_t fileId, uint8_t forkType, const Fork &fork, uint64_t offset, void *buffer,
                  size_t size);

private:
    struct Tree
    {
        uint32_t fileId = 0;
        Fork fork;
        uint32_t nodeSize = 0;
        uint32_t root = 0;
        uint16_t maxKeyLength = 0;
        bool variableIndexKeys = false;
    };

    struct Position
    {
        uint32_t node = 0;
        uint16_t record = 0;
    };

    struct Directory
    {
        std::vector<std::pair<std::string, Item>> entries;
        std::unordered_map<std::string, size_t> byName;
    };

    // compare: a record key against the one searched for, as <0, 0 or >0.
    using Compare = int (*)(const uint8_t *key, size_t length, const void *target);

    bool openTree(Tree *tree, uint32_t fileId, const Fork &fork);
    const std::vector<uint8_t> *node(const Tree &tree, uint32_t index);
    // The first record not below target, or false at the end of the tree.
    bool seek(const Tree &tree, Compare compare, const void *target, Position *position);
    bool next(const Tree &tree, Position *position);
    // The record's key and data.
    bool record(const Tree &tree, const Position &position, const uint8_t **key, size_t *keyLength,
                const uint8_t **data, size_t *dataLength);

    bool resolve(uint32_t fileId, uint8_t forkType, const Fork &fork, std::vector<Extent> *extents);
    // Children of a folder id as stored: hard links unresolved, private folders kept.
    int listRaw(uint32_t folderId, std::vector<std::pair<std::string, Item>> *entries);
    Directory *directory(uint32_t folderId, int *error);
    int child(uint32_t folderId, const std::string &name, Item *item);
    int finish(Item *item, const uint8_t *record, size_t length);
    bool attribute(uint32_t fileId, const std::u16string &name, std::vector<uint8_t> *value);

    DiskImage *m_disk;
    uint64_t m_offset;
    uint64_t m_size;
    uint32_t m_blockSize = 0;
    uint64_t m_totalBlocks = 0;
    uint64_t m_freeBlocks = 0;
    uint64_t m_fileCount = 0;
    std::string m_name;
    Item m_root;
    Tree m_catalog;
    Tree m_extents;
    Tree m_attributes;
    bool m_hasAttributes = false;
    uint32_t m_fileLinks = 0;
    uint32_t m_folderLinks = 0;
    Lingmo::base::LruCache<uint64_t, std::vector<uint8_t>> m_nodes {8 * 1024 * 1024};
    Lingmo::base::LruCache<uint32_t, Directory> m_directories {256 * 1024};
};
