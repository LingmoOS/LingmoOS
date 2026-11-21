// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "diskimage.h"

#include "base/i18n.h"

#include <algorithm>
#include <bzlib.h>
#include <cstring>
#include <fcntl.h>
#include <lzfse.h>
#include <lzma.h>
#include <string_view>
#include <sys/stat.h>
#include <unistd.h>
#include <zlib.h>

using Lingmo::i18n::tr;

static constexpr uint32_t kChunkZeroFill = 0x00000000;
static constexpr uint32_t kChunkRaw = 0x00000001;
static constexpr uint32_t kChunkIgnore = 0x00000002;
static constexpr uint32_t kChunkAdc = 0x80000004;
static constexpr uint32_t kChunkZlib = 0x80000005;
static constexpr uint32_t kChunkBzip2 = 0x80000006;
static constexpr uint32_t kChunkLzfse = 0x80000007;
static constexpr uint32_t kChunkLzma = 0x80000008;
static constexpr uint32_t kChunkComment = 0x7FFFFFFE;
static constexpr uint32_t kChunkEnd = 0xFFFFFFFF;

static constexpr uint64_t kSector = 512;
static constexpr size_t kTrailerSize = 512;
// Images made by the platform's tools use chunks of 1 MiB at most; a larger one is damage.
static constexpr uint64_t kMaxChunk = 64 * 1024 * 1024;
static constexpr size_t kMaxTable = 64 * 1024 * 1024;

static uint16_t be16(const uint8_t *p)
{
    return uint16_t(p[0] << 8 | p[1]);
}

static uint32_t be32(const uint8_t *p)
{
    return uint32_t(p[0]) << 24 | uint32_t(p[1]) << 16 | uint32_t(p[2]) << 8 | p[3];
}

static uint64_t be64(const uint8_t *p)
{
    return uint64_t(be32(p)) << 32 | be32(p + 4);
}

static uint32_t le32(const uint8_t *p)
{
    return uint32_t(p[3]) << 24 | uint32_t(p[2]) << 16 | uint32_t(p[1]) << 8 | p[0];
}

static uint64_t le64(const uint8_t *p)
{
    return uint64_t(le32(p + 4)) << 32 | le32(p);
}

static bool decodeBase64(std::string_view text, std::vector<uint8_t> *out)
{
    uint32_t bits = 0;
    int count = 0;
    for (const char c : text) {
        int value;
        if (c >= 'A' && c <= 'Z')
            value = c - 'A';
        else if (c >= 'a' && c <= 'z')
            value = c - 'a' + 26;
        else if (c >= '0' && c <= '9')
            value = c - '0' + 52;
        else if (c == '+')
            value = 62;
        else if (c == '/')
            value = 63;
        else if (c == '=' || c == ' ' || c == '\t' || c == '\n' || c == '\r')
            continue;
        else
            return false;

        bits = bits << 6 | uint32_t(value);
        count += 6;
        if (count >= 8) {
            count -= 8;
            out->push_back(uint8_t(bits >> count));
        }
    }
    return true;
}

// The <data> values of the blkx array in the image's property list.
static bool blockTables(std::string_view plist, std::vector<std::vector<uint8_t>> *tables)
{
    const size_t key = plist.find("<key>blkx</key>");
    if (key == std::string_view::npos)
        return false;
    const size_t begin = plist.find("<array>", key);
    const size_t end = plist.find("</array>", key);
    if (begin == std::string_view::npos || end == std::string_view::npos || end < begin)
        return false;

    const std::string_view array = plist.substr(begin, end - begin);
    for (size_t at = array.find("<data>"); at != std::string_view::npos; at = array.find("<data>", at)) {
        at += 6;
        const size_t close = array.find("</data>", at);
        if (close == std::string_view::npos)
            return false;
        std::vector<uint8_t> table;
        if (!decodeBase64(array.substr(at, close - at), &table))
            return false;
        tables->push_back(std::move(table));
        at = close;
    }
    return !tables->empty();
}

// Apple Data Compression: literal runs and back references of up to 64 KiB.
static bool decodeAdc(const uint8_t *in, size_t inSize, uint8_t *out, size_t outSize)
{
    size_t i = 0;
    size_t o = 0;
    while (i < inSize && o < outSize) {
        const uint8_t byte = in[i++];
        if (byte & 0x80) {
            const size_t length = (byte & 0x7F) + 1u;
            if (i + length > inSize || o + length > outSize)
                return false;
            std::memcpy(out + o, in + i, length);
            i += length;
            o += length;
            continue;
        }

        size_t length;
        size_t distance;
        if (byte & 0x40) {
            if (i + 2 > inSize)
                return false;
            length = (byte & 0x3F) + 4u;
            distance = size_t(be16(in + i)) + 1;
            i += 2;
        } else {
            if (i + 1 > inSize)
                return false;
            length = ((byte >> 2) & 0x0F) + 3u;
            distance = (size_t(byte & 0x03) << 8 | in[i]) + 1;
            i += 1;
        }
        if (distance > o || o + length > outSize)
            return false;
        // The reference may overlap what it writes.
        for (size_t n = 0; n < length; ++n, ++o)
            out[o] = out[o - distance];
    }
    return o == outSize;
}

static bool decodeLzma(const uint8_t *in, size_t inSize, uint8_t *out, size_t outSize)
{
    lzma_stream stream = LZMA_STREAM_INIT;
    if (lzma_auto_decoder(&stream, UINT64_MAX, 0) != LZMA_OK)
        return false;
    stream.next_in = in;
    stream.avail_in = inSize;
    stream.next_out = out;
    stream.avail_out = outSize;
    const lzma_ret result = lzma_code(&stream, LZMA_FINISH);
    const bool done = (result == LZMA_STREAM_END || result == LZMA_OK) && stream.avail_out == 0;
    lzma_end(&stream);
    return done;
}

DiskImage::~DiskImage()
{
    if (m_fd >= 0)
        ::close(m_fd);
}

bool DiskImage::readFile(uint64_t offset, void *buffer, size_t size)
{
    if (offset > m_fileSize || size > m_fileSize - offset)
        return false;

    auto *out = static_cast<uint8_t *>(buffer);
    while (size > 0) {
        const ssize_t got = ::pread(m_fd, out, size, off_t(offset));
        if (got < 0 && errno == EINTR)
            continue;
        if (got <= 0)
            return false;
        out += got;
        offset += uint64_t(got);
        size -= size_t(got);
    }
    return true;
}

bool DiskImage::open(const std::string &path, std::string *message)
{
    const std::string notImage = tr("The file is not a disk image that can be opened.");

    m_fd = ::open(path.c_str(), O_RDONLY | O_CLOEXEC);
    struct stat st;
    if (m_fd < 0 || ::fstat(m_fd, &st) != 0) {
        *message = std::strerror(errno);
        return false;
    }
    if (!S_ISREG(st.st_mode)) {
        *message = notImage;
        return false;
    }
    m_fileSize = uint64_t(st.st_size);

    uint8_t head[8] = {};
    uint8_t trailer[kTrailerSize] = {};
    const bool hasHead = readFile(0, head, sizeof(head));
    const bool hasTrailer = m_fileSize >= kTrailerSize && readFile(m_fileSize - kTrailerSize, trailer, kTrailerSize);

    if ((hasHead && std::memcmp(head, "encrcdsa", 8) == 0)
        || (hasTrailer && std::memcmp(trailer + kTrailerSize - 8, "cdsaencr", 8) == 0)) {
        *message = tr("Encrypted disk images cannot be opened.");
        return false;
    }

    // Without the UDIF trailer the file is the disk itself, as a read-write image or a copy.
    if (!hasTrailer || std::memcmp(trailer, "koly", 4) != 0) {
        if (hasHead && std::memcmp(head, "sprs", 4) == 0) {
            *message = tr("Sparse disk images cannot be opened.");
            return false;
        }
        m_size = m_fileSize;
        m_chunks.push_back({kChunkRaw, 0, m_size, 0, m_size});
        return true;
    }

    const uint64_t dataForkOffset = be64(trailer + 24);
    const uint32_t segmentCount = be32(trailer + 60);
    const uint64_t xmlOffset = be64(trailer + 216);
    const uint64_t xmlLength = be64(trailer + 224);
    const uint64_t sectorCount = be64(trailer + 492);

    if (segmentCount > 1) {
        *message = tr("Disk images split into several files cannot be opened.");
        return false;
    }
    if (xmlLength == 0 || xmlLength > kMaxTable || sectorCount > UINT64_MAX / kSector) {
        *message = notImage;
        return false;
    }

    std::string plist(size_t(xmlLength), '\0');
    if (!readFile(xmlOffset, plist.data(), plist.size())) {
        *message = notImage;
        return false;
    }

    std::vector<std::vector<uint8_t>> tables;
    if (!blockTables(plist, &tables)) {
        *message = notImage;
        return false;
    }

    m_size = sectorCount * kSector;
    for (const std::vector<uint8_t> &table : tables) {
        if (!parseTable(table, dataForkOffset, message))
            return false;
    }

    std::sort(m_chunks.begin(), m_chunks.end(), [](const Chunk &a, const Chunk &b) {
        return a.start < b.start;
    });

    // Sectors no table covers read as zeros.
    std::vector<Chunk> covered;
    uint64_t next = 0;
    for (const Chunk &chunk : m_chunks) {
        if (chunk.start < next) {
            *message = tr("The disk image is damaged.");
            return false;
        }
        if (chunk.start > next)
            covered.push_back({kChunkIgnore, next, chunk.start - next, 0, 0});
        covered.push_back(chunk);
        next = chunk.start + chunk.length;
    }
    if (next < m_size)
        covered.push_back({kChunkIgnore, next, m_size - next, 0, 0});
    m_chunks = std::move(covered);
    return true;
}

bool DiskImage::parseTable(const std::vector<uint8_t> &table, uint64_t dataForkOffset, std::string *message)
{
    static constexpr size_t kHeaderSize = 204;
    static constexpr size_t kEntrySize = 40;

    if (table.size() < kHeaderSize || std::memcmp(table.data(), "mish", 4) != 0) {
        *message = tr("The disk image is damaged.");
        return false;
    }

    const uint64_t firstSector = be64(table.data() + 8);
    const uint64_t dataStart = be64(table.data() + 24);
    const uint32_t count = be32(table.data() + 200);
    if (count > (table.size() - kHeaderSize) / kEntrySize) {
        *message = tr("The disk image is damaged.");
        return false;
    }

    for (uint32_t i = 0; i < count; ++i) {
        const uint8_t *entry = table.data() + kHeaderSize + size_t(i) * kEntrySize;
        Chunk chunk;
        chunk.type = be32(entry);
        if (chunk.type == kChunkComment)
            continue;
        if (chunk.type == kChunkEnd)
            break;

        const uint64_t sector = be64(entry + 8);
        const uint64_t sectors = be64(entry + 16);
        chunk.dataOffset = dataForkOffset + dataStart + be64(entry + 24);
        chunk.dataLength = be64(entry + 32);
        if (sectors == 0)
            continue;

        const bool known = chunk.type == kChunkZeroFill || chunk.type == kChunkRaw || chunk.type == kChunkIgnore
                || chunk.type == kChunkAdc || chunk.type == kChunkZlib || chunk.type == kChunkBzip2
                || chunk.type == kChunkLzfse || chunk.type == kChunkLzma;
        if (!known) {
            *message = tr("The disk image uses a compression that cannot be read.");
            return false;
        }

        const uint64_t limit = m_size / kSector;
        if (firstSector > limit || sector > limit - firstSector || sectors > limit - firstSector - sector
            || (chunk.type != kChunkRaw && sectors * kSector > kMaxChunk)) {
            *message = tr("The disk image is damaged.");
            return false;
        }
        chunk.start = (firstSector + sector) * kSector;
        chunk.length = sectors * kSector;

        const bool stored = chunk.type != kChunkZeroFill && chunk.type != kChunkIgnore;
        if (stored && (chunk.dataOffset > m_fileSize || chunk.dataLength > m_fileSize - chunk.dataOffset
                       || (chunk.type == kChunkRaw && chunk.dataLength < chunk.length)
                       || (chunk.type != kChunkRaw && chunk.dataLength > 2 * kMaxChunk))) {
            *message = tr("The disk image is damaged.");
            return false;
        }
        m_chunks.push_back(chunk);
    }
    return true;
}

const std::vector<uint8_t> *DiskImage::chunkData(size_t index)
{
    if (const std::vector<uint8_t> *cached = m_cache.find(index))
        return cached;

    const Chunk &chunk = m_chunks[index];
    std::vector<uint8_t> input(size_t(chunk.dataLength));
    if (!readFile(chunk.dataOffset, input.data(), input.size()))
        return nullptr;

    auto output = std::make_unique<std::vector<uint8_t>>(size_t(chunk.length));
    uint8_t *out = output->data();
    const size_t outSize = output->size();
    bool decoded = false;

    switch (chunk.type) {
    case kChunkZlib: {
        uLongf length = outSize;
        decoded = ::uncompress(out, &length, input.data(), uLong(input.size())) == Z_OK && length == outSize;
        break;
    }
    case kChunkBzip2: {
        unsigned int length = unsigned(outSize);
        decoded = BZ2_bzBuffToBuffDecompress(reinterpret_cast<char *>(out), &length,
                                             reinterpret_cast<char *>(input.data()), unsigned(input.size()), 0, 0)
                        == BZ_OK
                && length == outSize;
        break;
    }
    case kChunkLzfse:
        if (m_lzfseScratch.empty())
            m_lzfseScratch.resize(lzfse_decode_scratch_size());
        decoded = lzfse_decode_buffer(out, outSize, input.data(), input.size(), m_lzfseScratch.data()) == outSize;
        break;
    case kChunkLzma:
        decoded = decodeLzma(input.data(), input.size(), out, outSize);
        break;
    case kChunkAdc:
        decoded = decodeAdc(input.data(), input.size(), out, outSize);
        break;
    }

    if (!decoded)
        return nullptr;

    std::vector<uint8_t> *result = output.get();
    m_cache.insert(index, std::move(output), outSize);
    return result;
}

bool DiskImage::read(uint64_t offset, void *buffer, size_t size)
{
    if (offset > m_size || size > m_size - offset)
        return false;

    auto *out = static_cast<uint8_t *>(buffer);
    auto it = std::upper_bound(m_chunks.begin(), m_chunks.end(), offset, [](uint64_t value, const Chunk &chunk) {
        return value < chunk.start;
    });
    if (it == m_chunks.begin())
        return size == 0;
    --it;

    while (size > 0 && it != m_chunks.end()) {
        const Chunk &chunk = *it;
        const uint64_t within = offset - chunk.start;
        const size_t part = size_t(std::min<uint64_t>(size, chunk.length - within));

        if (chunk.type == kChunkZeroFill || chunk.type == kChunkIgnore) {
            std::memset(out, 0, part);
        } else if (chunk.type == kChunkRaw) {
            if (!readFile(chunk.dataOffset + within, out, part))
                return false;
        } else {
            const std::vector<uint8_t> *data = chunkData(size_t(it - m_chunks.begin()));
            if (!data)
                return false;
            std::memcpy(out, data->data() + within, part);
        }

        out += part;
        offset += part;
        size -= part;
        ++it;
    }
    return size == 0;
}

bool DiskImage::probe(uint64_t offset, uint64_t size, std::vector<Volume> *volumes)
{
    uint8_t block[1536];
    if (size < sizeof(block) || !read(offset, block, sizeof(block)))
        return false;

    if (std::memcmp(block + 32, "NXSB", 4) == 0) {
        volumes->push_back({Format::Apfs, offset, size});
        return true;
    }

    const uint8_t *header = block + 1024;
    const uint16_t signature = be16(header);
    if (signature == 0x482B || signature == 0x4858) {
        volumes->push_back({Format::Hfs, offset, size});
        return true;
    }

    // An HFS volume wrapping an HFS+ one, as older images were made.
    if (signature == 0x4244 && be16(header + 0x7C) == 0x482B) {
        const uint64_t blockSize = be32(header + 0x14);
        const uint64_t firstBlock = uint64_t(be16(header + 0x1C)) * kSector;
        const uint64_t start = firstBlock + be16(header + 0x7E) * blockSize;
        const uint64_t length = be16(header + 0x80) * blockSize;
        if (blockSize == 0 || start >= size || length > size - start)
            return false;
        volumes->push_back({Format::Hfs, offset + start, length});
        return true;
    }
    return false;
}

std::vector<DiskImage::Volume> DiskImage::volumes()
{
    std::vector<Volume> found;
    uint8_t head[1024];
    if (!read(0, head, sizeof(head)))
        return found;

    const auto probeSectors = [this, &found](uint64_t first, uint64_t count) {
        if (first < m_size / kSector && count <= m_size / kSector - first)
            probe(first * kSector, count * kSector, &found);
    };

    if (std::memcmp(head + 512, "EFI PART", 8) == 0) {
        const uint64_t entries = le64(head + 512 + 72);
        const uint32_t count = std::min<uint32_t>(le32(head + 512 + 80), 256);
        const uint32_t entrySize = le32(head + 512 + 84);
        std::vector<uint8_t> entry(entrySize);
        if (entrySize >= 128 && entrySize <= 4096) {
            for (uint32_t i = 0; i < count; ++i) {
                const uint64_t at = entries * kSector + uint64_t(i) * entrySize;
                if (entries > m_size / kSector || !read(at, entry.data(), entrySize))
                    break;
                if (std::all_of(entry.begin(), entry.begin() + 16, [](uint8_t b) { return b == 0; }))
                    continue;
                const uint64_t first = le64(entry.data() + 32);
                const uint64_t last = le64(entry.data() + 40);
                if (last >= first)
                    probeSectors(first, last - first + 1);
            }
        }
    } else if (be16(head + 512) == 0x504D) {
        // An Apple partition map, one entry per block after the driver descriptor.
        const uint64_t blockSize = be16(head) == 0x4552 && be16(head + 2) ? be16(head + 2) : kSector;
        const uint32_t count = std::min<uint32_t>(be32(head + 512 + 4), 256);
        uint8_t entry[512];
        for (uint32_t i = 1; i <= count; ++i) {
            if (!read(i * blockSize, entry, sizeof(entry)) || be16(entry) != 0x504D)
                break;
            const uint64_t scale = blockSize / kSector;
            probeSectors(be32(entry + 8) * scale, be32(entry + 12) * scale);
        }
    } else if (head[510] == 0x55 && head[511] == 0xAA) {
        for (int i = 0; i < 4; ++i) {
            const uint8_t *entry = head + 446 + i * 16;
            if (entry[4] != 0 && entry[4] != 0xEE)
                probeSectors(le32(entry + 8), le32(entry + 12));
        }
    }

    if (found.empty())
        probe(0, m_size, &found);
    return found;
}
