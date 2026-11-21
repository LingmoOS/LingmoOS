// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/lru_cache.h"

#include <cstdint>
#include <string>
#include <vector>

// The disk a .dmg holds: a UDIF image (raw, zero, zlib, bzip2, LZMA, LZFSE or ADC
// chunks) or a plain disk copy, read at any offset.
class DiskImage
{
public:
    enum class Format {
        Hfs,
        Apfs,
    };

    // A file system found on the disk.
    struct Volume
    {
        Format format = Format::Hfs;
        uint64_t offset = 0;
        uint64_t size = 0;
    };

    DiskImage() = default;
    ~DiskImage();

    DiskImage(const DiskImage &) = delete;
    DiskImage &operator=(const DiskImage &) = delete;

    // False with a message the user can read when the file is no image this can read.
    bool open(const std::string &path, std::string *message);

    uint64_t size() const
    {
        return m_size;
    }

    // False when the image is damaged or the range lies past the end.
    bool read(uint64_t offset, void *buffer, size_t size);

    // The HFS+ and APFS volumes in the partition table, or the whole disk.
    std::vector<Volume> volumes();

private:
    struct Chunk
    {
        uint32_t type = 0;
        // Within the disk.
        uint64_t start = 0;
        uint64_t length = 0;
        // Within the image file.
        uint64_t dataOffset = 0;
        uint64_t dataLength = 0;
    };

    bool parseTable(const std::vector<uint8_t> &table, uint64_t dataForkOffset, std::string *message);
    const std::vector<uint8_t> *chunkData(size_t index);
    bool readFile(uint64_t offset, void *buffer, size_t size);
    bool probe(uint64_t offset, uint64_t size, std::vector<Volume> *volumes);

    int m_fd = -1;
    uint64_t m_fileSize = 0;
    uint64_t m_size = 0;
    // Sorted by start, covering the disk without gaps.
    std::vector<Chunk> m_chunks;
    Lingmo::base::LruCache<size_t, std::vector<uint8_t>> m_cache {16 * 1024 * 1024};
    std::vector<uint8_t> m_lzfseScratch;
};
