// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "diskimage.h"
#include "diskimagebackend.h"
#include "hfsplus.h"

#include <gtest/gtest.h>

#include <bzlib.h>
#include <lzfse.h>
#include <lzma.h>
#include <zlib.h>

#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <map>
#include <random>
#include <sys/stat.h>
#include <unistd.h>

// hfsplus.img.gz, made by data/make-hfsplus-image.py; the names and contents below are what it writes.
static constexpr uint64_t kBlock = 4096;
static constexpr uint64_t kSector = 512;

enum Chunk : uint32_t {
    ZeroFill = 0x00000000,
    Raw = 0x00000001,
    Ignore = 0x00000002,
    Adc = 0x80000004,
    Zlib = 0x80000005,
    Bzip2 = 0x80000006,
    Lzfse = 0x80000007,
    Lzma = 0x80000008,
};

using Bytes = std::vector<uint8_t>;

static std::string pattern(const std::string &label, size_t size)
{
    std::string out;
    for (size_t block = 0; block < size; block += kBlock) {
        char line[96];
        std::snprintf(line, sizeof(line), "%s block %06zu\n", label.c_str(), block / kBlock);
        std::string filled;
        while (filled.size() < kBlock)
            filled += line;
        out += filled.substr(0, kBlock);
    }
    out.resize(size);
    return out;
}

static Bytes fixture()
{
    Bytes volume;
    gzFile file = gzopen(FIXTURE_DIR "/hfsplus.img.gz", "rb");
    if (!file)
        return volume;
    uint8_t buffer[65536];
    int got;
    while ((got = gzread(file, buffer, sizeof(buffer))) > 0)
        volume.insert(volume.end(), buffer, buffer + got);
    gzclose(file);
    return volume;
}

static void put16(Bytes &out, size_t at, uint16_t value)
{
    out[at] = uint8_t(value >> 8);
    out[at + 1] = uint8_t(value);
}

static void put32(Bytes &out, size_t at, uint32_t value)
{
    for (int i = 0; i < 4; ++i)
        out[at + i] = uint8_t(value >> (24 - 8 * i));
}

static void put64(Bytes &out, size_t at, uint64_t value)
{
    put32(out, at, uint32_t(value >> 32));
    put32(out, at + 4, uint32_t(value));
}

static void putLe32(Bytes &out, size_t at, uint32_t value)
{
    for (int i = 0; i < 4; ++i)
        out[at + i] = uint8_t(value >> (8 * i));
}

static void putLe64(Bytes &out, size_t at, uint64_t value)
{
    putLe32(out, at, uint32_t(value));
    putLe32(out, at + 4, uint32_t(value >> 32));
}

// Literal runs and back references, the longest match of the last place each three bytes were seen.
static Bytes encodeAdc(const uint8_t *in, size_t size)
{
    Bytes out;
    std::map<uint32_t, size_t> seen;
    Bytes literal;
    const auto flush = [&] {
        for (size_t at = 0; at < literal.size(); at += 128) {
            const size_t run = std::min<size_t>(128, literal.size() - at);
            out.push_back(uint8_t(0x80 | (run - 1)));
            out.insert(out.end(), literal.begin() + at, literal.begin() + at + run);
        }
        literal.clear();
    };

    size_t i = 0;
    while (i < size) {
        size_t length = 0;
        size_t distance = 0;
        if (i + 3 <= size) {
            const uint32_t key = uint32_t(in[i]) << 16 | uint32_t(in[i + 1]) << 8 | in[i + 2];
            const auto found = seen.find(key);
            if (found != seen.end() && i - found->second <= 65536) {
                distance = i - found->second;
                while (i + length < size && length < 67 && in[found->second + length] == in[i + length])
                    ++length;
            }
            seen[key] = i;
        }

        if (length >= 4 || (length == 3 && distance <= 1024)) {
            flush();
            if (length <= 18 && distance <= 1024) {
                out.push_back(uint8_t((length - 3) << 2 | (distance - 1) >> 8));
                out.push_back(uint8_t(distance - 1));
            } else {
                out.push_back(uint8_t(0x40 | (length - 4)));
                out.push_back(uint8_t((distance - 1) >> 8));
                out.push_back(uint8_t(distance - 1));
            }
            for (size_t n = 1; n < length && i + n + 3 <= size; ++n)
                seen[uint32_t(in[i + n]) << 16 | uint32_t(in[i + n + 1]) << 8 | in[i + n + 2]] = i + n;
            i += length;
        } else {
            literal.push_back(in[i++]);
        }
    }
    flush();
    return out;
}

static Bytes encode(Chunk type, const uint8_t *in, size_t size)
{
    Bytes out;
    switch (type) {
    case Zlib: {
        uLongf length = compressBound(uLong(size));
        out.resize(length);
        EXPECT_EQ(compress2(out.data(), &length, in, uLong(size), 9), Z_OK);
        out.resize(length);
        break;
    }
    case Bzip2: {
        unsigned int length = unsigned(size + size / 100 + 600);
        out.resize(length);
        EXPECT_EQ(BZ2_bzBuffToBuffCompress(reinterpret_cast<char *>(out.data()), &length,
                                           const_cast<char *>(reinterpret_cast<const char *>(in)), unsigned(size), 9,
                                           0, 0),
                  BZ_OK);
        out.resize(length);
        break;
    }
    case Lzma: {
        out.resize(lzma_stream_buffer_bound(size));
        size_t written = 0;
        EXPECT_EQ(lzma_easy_buffer_encode(6, LZMA_CHECK_NONE, nullptr, in, size, out.data(), &written, out.size()),
                  LZMA_OK);
        out.resize(written);
        break;
    }
    case Lzfse:
        out.resize(size + 4096);
        out.resize(lzfse_encode_buffer(out.data(), out.size(), in, size, nullptr));
        EXPECT_FALSE(out.empty());
        break;
    case Adc:
        out = encodeAdc(in, size);
        break;
    default:
        out.assign(in, in + size);
        break;
    }
    return out;
}

static std::string base64(const Bytes &data)
{
    static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    for (size_t i = 0; i < data.size(); i += 3) {
        const uint32_t value = uint32_t(data[i]) << 16 | (i + 1 < data.size() ? uint32_t(data[i + 1]) << 8 : 0)
                | (i + 2 < data.size() ? data[i + 2] : 0);
        out += alphabet[value >> 18 & 63];
        out += alphabet[value >> 12 & 63];
        out += i + 1 < data.size() ? alphabet[value >> 6 & 63] : '=';
        out += i + 2 < data.size() ? alphabet[value & 63] : '=';
        if (out.size() % 77 == 76)
            out += "\n\t\t\t";
    }
    return out;
}

// A UDIF image of disk: chunks of chunkSectors, all-zero ones left as ignored or zero-filled sectors.
static Bytes udif(const Bytes &disk, Chunk type, uint64_t chunkSectors = 2048)
{
    EXPECT_EQ(disk.size() % kSector, 0u);
    const uint64_t sectors = disk.size() / kSector;

    Bytes data;
    struct Entry
    {
        uint32_t type;
        uint64_t sector;
        uint64_t count;
        uint64_t offset;
        uint64_t length;
    };
    std::vector<Entry> entries;
    entries.push_back({0x7FFFFFFE, 0, 0, 0, 0});
    for (uint64_t sector = 0; sector < sectors; sector += chunkSectors) {
        const uint64_t count = std::min(chunkSectors, sectors - sector);
        const uint8_t *in = disk.data() + sector * kSector;
        const size_t size = size_t(count * kSector);
        if (std::all_of(in, in + size, [](uint8_t b) { return b == 0; })) {
            entries.push_back({(sector / chunkSectors) % 2 ? uint32_t(ZeroFill) : uint32_t(Ignore), sector, count,
                               data.size(), 0});
            continue;
        }
        const Bytes encoded = encode(type, in, size);
        entries.push_back({type, sector, count, data.size(), encoded.size()});
        data.insert(data.end(), encoded.begin(), encoded.end());
    }
    entries.push_back({0xFFFFFFFF, sectors, 0, data.size(), 0});

    Bytes mish(204 + entries.size() * 40);
    std::memcpy(mish.data(), "mish", 4);
    put32(mish, 4, 1);
    put64(mish, 16, sectors);
    put32(mish, 36, 2048);
    put32(mish, 200, uint32_t(entries.size()));
    for (size_t i = 0; i < entries.size(); ++i) {
        const size_t at = 204 + i * 40;
        put32(mish, at, entries[i].type);
        put64(mish, at + 8, entries[i].sector);
        put64(mish, at + 16, entries[i].count);
        put64(mish, at + 24, entries[i].offset);
        put64(mish, at + 32, entries[i].length);
    }

    const std::string plist = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                              "<plist version=\"1.0\">\n<dict>\n\t<key>resource-fork</key>\n\t<dict>\n"
                              "\t\t<key>blkx</key>\n\t\t<array>\n\t\t\t<dict>\n"
                              "\t\t\t\t<key>Attributes</key>\n\t\t\t\t<string>0x0050</string>\n"
                              "\t\t\t\t<key>Data</key>\n\t\t\t\t<data>\n\t\t\t"
            + base64(mish)
            + "\n\t\t\t\t</data>\n\t\t\t\t<key>Name</key>\n\t\t\t\t<string>whole disk</string>\n"
              "\t\t\t</dict>\n\t\t</array>\n\t</dict>\n</dict>\n</plist>\n";

    Bytes image = data;
    const uint64_t xmlOffset = image.size();
    image.insert(image.end(), plist.begin(), plist.end());

    Bytes trailer(512);
    std::memcpy(trailer.data(), "koly", 4);
    put32(trailer, 4, 4);
    put32(trailer, 8, 512);
    put32(trailer, 12, 1);
    put64(trailer, 24, 0);
    put64(trailer, 32, data.size());
    put32(trailer, 56, 1);
    put32(trailer, 60, 1);
    put64(trailer, 216, xmlOffset);
    put64(trailer, 224, plist.size());
    put32(trailer, 488, 1);
    put64(trailer, 492, sectors);
    image.insert(image.end(), trailer.begin(), trailer.end());
    return image;
}

static Bytes withGpt(const Bytes &volume, uint64_t firstSector = 40)
{
    Bytes disk((firstSector + 34) * kSector + volume.size());
    std::copy(volume.begin(), volume.end(), disk.begin() + firstSector * kSector);
    disk[446 + 4] = 0xEE;
    disk[510] = 0x55;
    disk[511] = 0xAA;
    std::memcpy(disk.data() + 512, "EFI PART", 8);
    putLe64(disk, 512 + 72, 2);
    putLe32(disk, 512 + 80, 128);
    putLe32(disk, 512 + 84, 128);
    static const uint8_t hfsType[16] = {0x00, 0x53, 0x46, 0x48, 0x00, 0x00, 0xAA, 0x11,
                                        0xAA, 0x11, 0x00, 0x30, 0x65, 0x43, 0xEC, 0xAC};
    std::memcpy(disk.data() + 1024, hfsType, 16);
    disk[1024 + 16] = 1;
    putLe64(disk, 1024 + 32, firstSector);
    putLe64(disk, 1024 + 40, firstSector + volume.size() / kSector - 1);
    return disk;
}

static Bytes withApm(const Bytes &volume)
{
    static constexpr uint64_t first = 64;
    Bytes disk(first * kSector + volume.size());
    std::copy(volume.begin(), volume.end(), disk.begin() + first * kSector);
    put16(disk, 0, 0x4552);
    put16(disk, 2, 512);
    const auto entry = [&](uint64_t block, uint32_t start, uint32_t count, const char *type) {
        const size_t at = block * kSector;
        put16(disk, at, 0x504D);
        put32(disk, at + 4, 2);
        put32(disk, at + 8, start);
        put32(disk, at + 12, count);
        std::memcpy(disk.data() + at + 48, type, std::strlen(type));
    };
    entry(1, 1, 63, "Apple_partition_map");
    entry(2, first, uint32_t(volume.size() / kSector), "Apple_HFS");
    return disk;
}

static Bytes withMbr(const Bytes &volume)
{
    static constexpr uint64_t first = 63;
    Bytes disk(first * kSector + volume.size());
    std::copy(volume.begin(), volume.end(), disk.begin() + first * kSector);
    disk[446 + 4] = 0xAF;
    putLe32(disk, 446 + 8, first);
    putLe32(disk, 446 + 12, uint32_t(volume.size() / kSector));
    disk[510] = 0x55;
    disk[511] = 0xAA;
    return disk;
}

// An HFS volume whose only content is the HFS+ volume embedded in it.
static Bytes withHfsWrapper(const Bytes &volume)
{
    static constexpr uint64_t blockSize = 4096;
    static constexpr uint16_t firstBlockSector = 16;
    static constexpr uint16_t embedStart = 2;
    const uint64_t offset = firstBlockSector * kSector + embedStart * blockSize;
    Bytes disk(offset + volume.size() + blockSize);
    std::copy(volume.begin(), volume.end(), disk.begin() + offset);
    put16(disk, 1024, 0x4244);
    put32(disk, 1024 + 0x14, blockSize);
    put16(disk, 1024 + 0x1C, firstBlockSector);
    put16(disk, 1024 + 0x7C, 0x482B);
    put16(disk, 1024 + 0x7E, embedStart);
    put16(disk, 1024 + 0x80, uint16_t(volume.size() / blockSize));
    return disk;
}

class DiskImageTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_volume = fixture();
        ASSERT_EQ(m_volume.size(), 1024 * kBlock) << "data/hfsplus.img.gz is missing";
        char path[] = "/tmp/diskimage-test-XXXXXX";
        ASSERT_NE(::mkdtemp(path), nullptr);
        m_directory = path;
    }

    void TearDown() override
    {
        std::error_code error;
        std::filesystem::remove_all(m_directory, error);
    }

    std::string write(const std::string &name, const Bytes &bytes)
    {
        const std::string path = m_directory + '/' + name;
        std::ofstream(path, std::ios::binary).write(reinterpret_cast<const char *>(bytes.data()),
                                                    std::streamsize(bytes.size()));
        return path;
    }

    Bytes m_volume;
    std::string m_directory;
};

TEST_F(DiskImageTest, ReadsEveryChunkFormat)
{
    const Bytes disk = withGpt(m_volume);
    std::mt19937_64 random(7);

    for (const Chunk type : {Raw, Zlib, Bzip2, Lzma, Lzfse, Adc}) {
        SCOPED_TRACE(testing::Message() << std::hex << type);
        // Odd chunk sizes put chunk edges inside blocks.
        const std::string path = write("disk.dmg", udif(disk, type, type == Adc ? 129 : 1000));

        DiskImage image;
        std::string message;
        ASSERT_TRUE(image.open(path, &message)) << message;
        EXPECT_EQ(image.size(), disk.size());

        Bytes whole(disk.size());
        ASSERT_TRUE(image.read(0, whole.data(), whole.size()));
        EXPECT_TRUE(whole == disk);

        for (int i = 0; i < 200; ++i) {
            const uint64_t offset = random() % disk.size();
            const size_t size = size_t(random() % std::min<uint64_t>(300000, disk.size() - offset));
            Bytes part(size);
            ASSERT_TRUE(image.read(offset, part.data(), size));
            EXPECT_TRUE(std::equal(part.begin(), part.end(), disk.begin() + std::ptrdiff_t(offset)));
        }

        uint8_t byte;
        EXPECT_FALSE(image.read(disk.size(), &byte, 1));
        EXPECT_FALSE(image.read(disk.size() - 1, &byte, 2));
    }
}

TEST_F(DiskImageTest, FindsTheVolumeInEveryLayout)
{
    const struct
    {
        const char *name;
        Bytes disk;
        uint64_t offset;
    } layouts[] = {
        {"gpt", withGpt(m_volume), 40 * kSector},
        {"apm", withApm(m_volume), 64 * kSector},
        {"mbr", withMbr(m_volume), 63 * kSector},
        {"wrapper", withHfsWrapper(m_volume), 16 * kSector + 2 * 4096},
        {"bare", m_volume, 0},
    };

    for (const auto &layout : layouts) {
        SCOPED_TRACE(layout.name);
        for (const bool compressed : {false, true}) {
            const std::string path = write("layout.dmg", compressed ? udif(layout.disk, Zlib) : layout.disk);
            DiskImage image;
            std::string message;
            ASSERT_TRUE(image.open(path, &message)) << message;
            const std::vector<DiskImage::Volume> volumes = image.volumes();
            ASSERT_EQ(volumes.size(), 1u);
            EXPECT_EQ(volumes[0].format, DiskImage::Format::Hfs);
            EXPECT_EQ(volumes[0].offset, layout.offset);

            HfsPlus volume(&image, volumes[0].offset, volumes[0].size);
            ASSERT_TRUE(volume.open(&message)) << message;
            EXPECT_EQ(volume.name(), "Fixture Disk");
        }
    }
}

TEST_F(DiskImageTest, ReadsTheVolume)
{
    const std::string path = write("fixture.dmg", udif(withGpt(m_volume), Zlib));
    DiskImage image;
    std::string message;
    ASSERT_TRUE(image.open(path, &message)) << message;
    const DiskImage::Volume found = image.volumes().at(0);
    HfsPlus volume(&image, found.offset, found.size);
    ASSERT_TRUE(volume.open(&message)) << message;
    EXPECT_EQ(volume.blockSize(), kBlock);
    EXPECT_EQ(volume.totalBlocks(), 1024u);

    HfsPlus::Item root;
    ASSERT_EQ(volume.lookup("/", &root), 0);
    std::vector<std::pair<std::string, HfsPlus::Item>> entries;
    ASSERT_EQ(volume.list(root, &entries), 0);
    std::map<std::string, HfsPlus::Item> byName(entries.begin(), entries.end());

    // Composed names, ':' for the catalog's '/', and no private folders of hard links.
    for (const char *name : {"hello.txt", "empty", "Folder", "Caf\u00e9 \u00fcn\u00efcode.txt",
                             "\u2126 \u00c5ngstr\u00f6m.txt", "a:b.txt", "link", "Applications", "original.bin",
                             "filler", "fragmented.bin", "resources", "zlib-inline.txt"})
        EXPECT_TRUE(byName.contains(name)) << name;
    for (const auto &[name, item] : entries)
        EXPECT_FALSE(name.find("HFS+ Private") != std::string::npos) << name;

    const auto contents = [&](const std::string &path) {
        HfsPlus::Item item;
        EXPECT_EQ(volume.lookup(path, &item), 0) << path;
        std::unique_ptr<HfsPlus::File> file;
        EXPECT_EQ(volume.openFile(item, &file), 0) << path;
        std::string data(size_t(item.size), '\0');
        if (file)
            EXPECT_EQ(file->read(data.data(), data.size(), 0), int(data.size())) << path;
        return data;
    };

    EXPECT_EQ(contents("/hello.txt"), "Hello, disk image.\n");
    EXPECT_EQ(byName["hello.txt"].mode, mode_t(S_IFREG | 0640));
    EXPECT_EQ(contents("/empty"), "");
    EXPECT_EQ(contents("/Folder/nested/deep.txt"), pattern("deep", 5000));
    EXPECT_EQ(contents("/a:b.txt"), "colon\n");
    EXPECT_EQ(contents("/Caf\u00e9 \u00fcn\u00efcode.txt"), "unicode\n");
    EXPECT_EQ(contents("/\u2126 \u00c5ngstr\u00f6m.txt"), "ohm\n");
    EXPECT_EQ(contents("/filler/0001"), pattern("filler 1", kBlock));

    std::string target;
    EXPECT_EQ(volume.readLink(byName["link"], &target), 0);
    EXPECT_EQ(target, "hello.txt");
    EXPECT_EQ(volume.readLink(byName["Applications"], &target), 0);
    EXPECT_EQ(target, "/Applications");
    EXPECT_TRUE(S_ISLNK(byName["link"].mode));

    HfsPlus::Item linked;
    ASSERT_EQ(volume.lookup("/Folder/hardlink.bin", &linked), 0);
    EXPECT_EQ(linked.id, byName["original.bin"].id);
    EXPECT_EQ(linked.links, 2u);
    EXPECT_EQ(contents("/Folder/hardlink.bin"), pattern("linked", 9000));

    // More extents than the catalog record holds.
    const HfsPlus::Item fragmented = byName["fragmented.bin"];
    uint64_t recorded = 0;
    for (const HfsPlus::Extent &extent : fragmented.data.extents)
        recorded += extent.count;
    EXPECT_EQ(fragmented.data.extents.size(), 8u);
    EXPECT_GT(fragmented.data.totalBlocks, recorded);
    const std::string expected = pattern("fragmented", 40 * kBlock);
    EXPECT_EQ(contents("/fragmented.bin"), expected);
    std::unique_ptr<HfsPlus::File> file;
    ASSERT_EQ(volume.openFile(fragmented, &file), 0);
    std::string middle(10000, '\0');
    EXPECT_EQ(file->read(middle.data(), middle.size(), 30000), 10000);
    EXPECT_EQ(middle, expected.substr(30000, 10000));
    EXPECT_EQ(file->read(middle.data(), middle.size(), expected.size() - 10), 10);
    EXPECT_EQ(file->read(middle.data(), middle.size(), expected.size()), 0);

    HfsPlus::Item missing;
    EXPECT_EQ(volume.lookup("/nothing", &missing), -ENOENT);
    EXPECT_EQ(volume.lookup("/hello.txt/inside", &missing), -ENOTDIR);
    std::vector<std::pair<std::string, HfsPlus::Item>> filler;
    ASSERT_EQ(volume.list(byName["filler"], &filler), 0);
    EXPECT_GT(filler.size(), 300u);
}

TEST_F(DiskImageTest, ReadsCompressedFiles)
{
    const std::string path = write("fixture.dmg", udif(m_volume, Lzfse));
    DiskImage image;
    std::string message;
    ASSERT_TRUE(image.open(path, &message)) << message;
    HfsPlus volume(&image, 0, image.size());
    ASSERT_TRUE(volume.open(&message)) << message;

    const std::pair<const char *, size_t> files[] = {
        {"zlib-inline.txt", 3000},   {"stored-inline.txt", 700},  {"zlib-resource.txt", 200000},
        {"lzvn-inline.txt", 2000},   {"lzvn-resource.txt", 3000}, {"lzfse-resource.txt", 150000},
    };
    for (const auto &[name, size] : files) {
        SCOPED_TRACE(name);
        HfsPlus::Item item;
        ASSERT_EQ(volume.lookup(std::string("/") + name, &item), 0);
        EXPECT_TRUE(item.compressed);
        EXPECT_EQ(item.data.logicalSize, 0u);
        EXPECT_EQ(item.size, size);

        std::unique_ptr<HfsPlus::File> file;
        ASSERT_EQ(volume.openFile(item, &file), 0);
        const std::string expected = pattern(name, size);
        std::string data(size, '\0');
        // Across the 64 KiB blocks of the resource fork, in pieces.
        size_t done = 0;
        while (done < size) {
            const int got = file->read(data.data() + done, std::min<size_t>(40000, size - done), done);
            ASSERT_GT(got, 0);
            done += size_t(got);
        }
        EXPECT_EQ(data, expected);
    }
}

TEST_F(DiskImageTest, ServesTheVolumeReadOnly)
{
    DiskImageBackend backend(write("fixture.dmg", udif(withGpt(m_volume), Bzip2)));
    const ConnectResult result = backend.connect({});
    ASSERT_EQ(result.status, ConnectResult::Connected) << result.message;
    EXPECT_EQ(backend.volumeName(), "Fixture Disk");
    EXPECT_TRUE(backend.readOnly());

    struct stat st;
    ASSERT_EQ(backend.stat("/", &st), 0);
    EXPECT_TRUE(S_ISDIR(st.st_mode));
    EXPECT_EQ(st.st_uid, ::getuid());
    ASSERT_EQ(backend.stat("/Folder/nested/deep.txt", &st), 0);
    EXPECT_EQ(st.st_size, 5000);
    ASSERT_EQ(backend.stat("/zlib-resource.txt", &st), 0);
    EXPECT_EQ(st.st_size, 200000);

    std::vector<DirectoryEntry> entries;
    ASSERT_EQ(backend.list("/Folder", &entries), 0);
    ASSERT_EQ(entries.size(), 2u);

    uint64_t handle = 0;
    ASSERT_EQ(backend.open("/hello.txt", O_RDONLY, 0, &handle), 0);
    char buffer[64] = {};
    EXPECT_EQ(backend.read(handle, buffer, sizeof(buffer), 7), 12);
    EXPECT_STREQ(buffer, "disk image.\n");
    EXPECT_EQ(backend.release(handle), 0);
    EXPECT_EQ(backend.read(handle, buffer, sizeof(buffer), 0), -EBADF);

    std::string target;
    EXPECT_EQ(backend.readLink("/link", &target), 0);
    EXPECT_EQ(target, "hello.txt");

    EXPECT_EQ(backend.open("/hello.txt", O_RDWR, 0, &handle), -EROFS);
    EXPECT_EQ(backend.open("/new", O_WRONLY | O_CREAT, 0644, &handle), -EROFS);
    EXPECT_EQ(backend.remove("/hello.txt"), -EROFS);
    EXPECT_EQ(backend.makeDirectory("/new", 0755), -EROFS);
    EXPECT_EQ(backend.rename("/hello.txt", "/other", 0), -EROFS);
    EXPECT_EQ(backend.open("/nothing", O_RDONLY, 0, &handle), -ENOENT);

    struct statvfs vfs;
    ASSERT_EQ(backend.statfs("/", &vfs), 0);
    EXPECT_EQ(vfs.f_bsize, kBlock);
    EXPECT_TRUE(vfs.f_flag & ST_RDONLY);
}

TEST_F(DiskImageTest, RefusesWhatItCannotRead)
{
    std::string message;

    Bytes encrypted(4096);
    std::memcpy(encrypted.data(), "encrcdsa", 8);
    DiskImage locked;
    EXPECT_FALSE(locked.open(write("encrypted.dmg", encrypted), &message));
    EXPECT_FALSE(message.empty());

    Bytes sparse(4096);
    std::memcpy(sparse.data(), "sprs", 4);
    DiskImage sparseImage;
    EXPECT_FALSE(sparseImage.open(write("sparse.dmg", sparse), &message));

    // An APFS container in the partition.
    Bytes container(1024 * kBlock);
    std::memcpy(container.data() + 32, "NXSB", 4);
    DiskImageBackend apfs(write("apfs.dmg", udif(withGpt(container), Zlib)));
    ConnectResult result = apfs.connect({});
    EXPECT_EQ(result.status, ConnectResult::Failed);
    EXPECT_NE(result.message.find("APFS"), std::string::npos);

    Bytes noise(1024 * kBlock);
    std::mt19937 random(3);
    std::generate(noise.begin(), noise.end(), [&] { return uint8_t(random()); });
    DiskImageBackend unknown(write("noise.dmg", noise));
    result = unknown.connect({});
    EXPECT_EQ(result.status, ConnectResult::Failed);
    EXPECT_FALSE(result.message.empty());

    // A damaged chunk fails the reads that need it, nothing else.
    Bytes damaged = udif(withGpt(m_volume), Zlib, 64);
    for (size_t i = damaged.size() / 2; i < damaged.size() / 2 + 64; ++i)
        damaged[i] ^= 0x5A;
    DiskImage image;
    ASSERT_TRUE(image.open(write("damaged.dmg", damaged), &message)) << message;
    Bytes whole(image.size());
    EXPECT_FALSE(image.read(0, whole.data(), whole.size()));
    uint8_t first[512];
    EXPECT_TRUE(image.read(0, first, sizeof(first)));

    // Chunks that run past the disk are refused when the image opens.
    Bytes pastEnd = udif(withGpt(m_volume), Raw);
    put64(pastEnd, pastEnd.size() - 512 + 492, 4096);
    DiskImage shortImage;
    EXPECT_FALSE(shortImage.open(write("short.dmg", pastEnd), &message));

    // Fuzzed volumes may fail but never crash or hang. Runs with --gtest_shuffle try other seeds.
    std::mt19937 flips(unsigned(::testing::UnitTest::GetInstance()->random_seed()));
    for (int round = 0; round < 40; ++round) {
        Bytes volume = m_volume;
        for (int i = 0; i < 64; ++i) {
            // Mostly the metadata at the start: the header, the trees.
            const size_t at = flips() % (round % 2 ? volume.size() : 160 * kBlock);
            volume[at] = uint8_t(flips());
        }
        DiskImageBackend fuzzed(write("fuzzed.dmg", volume));
        if (fuzzed.connect({}).status != ConnectResult::Connected)
            continue;
        std::vector<DirectoryEntry> entries;
        if (fuzzed.list("/", &entries) != 0)
            continue;
        for (const DirectoryEntry &entry : entries) {
            struct stat st;
            fuzzed.stat("/" + entry.name, &st);
            uint64_t handle;
            if (fuzzed.open("/" + entry.name, O_RDONLY, 0, &handle) == 0) {
                char buffer[8192];
                fuzzed.read(handle, buffer, sizeof(buffer), 0);
                fuzzed.release(handle);
            }
            std::vector<DirectoryEntry> inner;
            fuzzed.list("/" + entry.name, &inner);
        }
    }
}
