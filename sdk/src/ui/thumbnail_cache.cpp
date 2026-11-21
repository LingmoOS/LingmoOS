namespace Lingmo::base { class EventLoop; }
namespace base = Lingmo::base;

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/thumbnail_cache.h"

#include <algorithm>
#include <cerrno>
#include <csetjmp>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <string_view>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#include <png.h>

#include "include/core/SkBitmap.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"

#include "base/paths.h"
#include "ui/image_decoder.h"

namespace Lingmo::ui::thumbnails {

namespace {

struct Flavor
{
    int size;
    const char* name;
};

// What ThumbnailCache's locate() works out for a file and a size.
struct Entry
{
    std::string path;
    bool insideCache = false;
    int cacheSize = 0;
    std::string uri;
    int64_t mtime = 0;
    int64_t size = 0;
    std::string root;
    std::string directory;
    std::string target;
    std::string failDirectory;
    std::string failTarget;
};

struct FileCloser
{
    void operator()(FILE* file) const { std::fclose(file); }
};

struct PngFile
{
    sk_sp<SkImage> image;
    std::map<std::string, std::string> text;
};

} // namespace

// Sizes and names of the freedesktop.org Thumbnail Managing Standard.
static constexpr Flavor kFlavors[] = {
    {128, "normal"},
    {256, "large"},
    {512, "x-large"},
    {1024, "xx-large"},
};

static std::string md5Hex(std::string_view data)
{
    static constexpr uint32_t k[64] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};
    static constexpr int shifts[64] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                                       5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20,
                                       4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                                       6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};
    std::string message(data);
    message += char(0x80);
    while (message.size() % 64 != 56)
        message += '\0';
    const uint64_t bits = uint64_t(data.size()) * 8;
    for (int i = 0; i < 8; ++i)
        message += char((bits >> (8 * i)) & 0xff);

    uint32_t h[4] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};
    for (size_t offset = 0; offset < message.size(); offset += 64) {
        uint32_t w[16];
        for (int i = 0; i < 16; ++i) {
            const auto* p = reinterpret_cast<const unsigned char*>(message.data() + offset + size_t(i) * 4);
            w[i] = uint32_t(p[0]) | uint32_t(p[1]) << 8 | uint32_t(p[2]) << 16 | uint32_t(p[3]) << 24;
        }
        uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
        for (int i = 0; i < 64; ++i) {
            uint32_t f;
            int g;
            if (i < 16) {
                f = (b & c) | (~b & d);
                g = i;
            } else if (i < 32) {
                f = (d & b) | (~d & c);
                g = (5 * i + 1) % 16;
            } else if (i < 48) {
                f = b ^ c ^ d;
                g = (3 * i + 5) % 16;
            } else {
                f = c ^ (b | ~d);
                g = (7 * i) % 16;
            }
            const uint32_t next = d;
            d = c;
            c = b;
            const uint32_t x = a + f + k[i] + w[g];
            b = b + ((x << shifts[i]) | (x >> (32 - shifts[i])));
            a = next;
        }
        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
    }
    static const char digits[] = "0123456789abcdef";
    std::string hex;
    for (uint32_t word : h) {
        for (int i = 0; i < 4; ++i) {
            const unsigned byte = (word >> (8 * i)) & 0xff;
            hex += digits[byte >> 4];
            hex += digits[byte & 0xf];
        }
    }
    return hex;
}

std::string fileUri(const std::string& absolutePath)
{
    static const char hex[] = "0123456789ABCDEF";
    std::string uri = "file://";
    for (unsigned char c : absolutePath) {
        // RFC 3986's path characters, and the brackets, stay as they are.
        const bool plain = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')
                           || std::string_view("-._~!$&'()*+,;=:@/[]").find(char(c)) != std::string_view::npos;
        if (plain) {
            uri += char(c);
        } else {
            uri += '%';
            uri += hex[c >> 4];
            uri += hex[c & 0xf];
        }
    }
    return uri;
}

// Absolute, without "." and ".." and repeated slashes.
static std::string absolutePath(const std::string& path)
{
    std::string full = path;
    if (full.empty() || full[0] != '/') {
        char buffer[4096];
        if (!getcwd(buffer, sizeof(buffer)))
            return {};
        full = std::string(buffer) + "/" + full;
    }
    std::vector<std::string> parts;
    size_t start = 0;
    while (start <= full.size()) {
        const size_t slash = full.find('/', start);
        const std::string part = full.substr(start, slash == std::string::npos ? std::string::npos : slash - start);
        if (part == "..") {
            if (!parts.empty())
                parts.pop_back();
        } else if (!part.empty() && part != ".") {
            parts.push_back(part);
        }
        if (slash == std::string::npos)
            break;
        start = slash + 1;
    }
    std::string result;
    for (const std::string& part : parts)
        result += "/" + part;
    return result.empty() ? "/" : result;
}

static bool locate(const std::string& filePath, int width, int height, Entry* entry)
{
    if (width <= 0 || height <= 0)
        return false;
    entry->path = absolutePath(filePath);
    struct stat info;
    if (entry->path.empty() || stat(entry->path.c_str(), &info) != 0 || !S_ISREG(info.st_mode))
        return false;

    const int extent = std::max(width, height);
    const Flavor* flavor = &kFlavors[std::size(kFlavors) - 1];
    for (const Flavor& candidate : kFlavors) {
        if (extent <= candidate.size) {
            flavor = &candidate;
            break;
        }
    }
    entry->cacheSize = flavor->size;
    entry->root = base::paths::cacheHome() + "/thumbnails";
    entry->insideCache = entry->path.starts_with(entry->root + "/");
    entry->uri = fileUri(entry->path);
    const std::string name = md5Hex(entry->uri) + ".png";
    entry->mtime = int64_t(info.st_mtim.tv_sec);
    entry->size = int64_t(info.st_size);
    entry->directory = entry->root + "/" + flavor->name;
    entry->target = entry->directory + "/" + name;
    entry->failDirectory = entry->root + "/fail/lingmo";
    entry->failTarget = entry->failDirectory + "/" + name;
    return true;
}

static void collectText(png_structp png, png_infop info, std::map<std::string, std::string>* text)
{
    png_textp chunks = nullptr;
    int count = 0;
    if (png_get_text(png, info, &chunks, &count) > 0) {
        for (int i = 0; i < count; ++i)
            (*text)[chunks[i].key] = std::string(chunks[i].text, chunks[i].itxt_length > 0 ? chunks[i].itxt_length
                                                                                           : chunks[i].text_length);
    }
}

// A PNG with its text chunks.
static std::optional<PngFile> readPng(const std::string& path)
{
    std::unique_ptr<FILE, FileCloser> file(std::fopen(path.c_str(), "rb"));
    if (!file)
        return std::nullopt;
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png)
        return std::nullopt;
    png_infop info = png_create_info_struct(png);
    png_infop endInfo = png_create_info_struct(png);
    PngFile result;
    std::vector<uint8_t> pixels;
    std::vector<png_bytep> rows;
    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, &endInfo);
        return std::nullopt;
    }
    png_init_io(png, file.get());
    png_read_info(png, info);
    const png_uint_32 width = png_get_image_width(png, info);
    const png_uint_32 height = png_get_image_height(png, info);
    if (width == 0 || height == 0 || uint64_t(width) * height * 4 > uint64_t(256) * 1024 * 1024) {
        png_destroy_read_struct(&png, &info, &endInfo);
        return std::nullopt;
    }
    png_set_expand(png);
    png_set_strip_16(png);
    png_set_gray_to_rgb(png);
    png_set_add_alpha(png, 0xff, PNG_FILLER_AFTER);
    png_set_interlace_handling(png);
    png_read_update_info(png, info);
    pixels.resize(size_t(width) * size_t(height) * 4);
    rows.resize(height);
    for (png_uint_32 y = 0; y < height; ++y)
        rows[y] = pixels.data() + size_t(y) * size_t(width) * 4;
    png_read_image(png, rows.data());
    png_read_end(png, endInfo);
    collectText(png, info, &result.text);
    collectText(png, endInfo, &result.text);
    png_destroy_read_struct(&png, &info, &endInfo);

    SkBitmap bitmap;
    if (!bitmap.tryAllocPixels(SkImageInfo::MakeN32Premul(int(width), int(height))))
        return std::nullopt;
    const SkImageInfo source = SkImageInfo::Make(int(width), int(height), kRGBA_8888_SkColorType, kUnpremul_SkAlphaType);
    if (!bitmap.writePixels(SkPixmap(source, pixels.data(), size_t(width) * 4)))
        return std::nullopt;
    bitmap.setImmutable();
    result.image = bitmap.asImage();
    return result;
}

static void appendBytes(png_structp png, png_bytep data, png_size_t length)
{
    auto* out = static_cast<std::string*>(png_get_io_ptr(png));
    out->append(reinterpret_cast<const char*>(data), length);
}

static std::string encodePng(const SkImage* image, const std::map<std::string, std::string>& text)
{
    const int width = image->width();
    const int height = image->height();
    std::vector<uint8_t> pixels(size_t(width) * size_t(height) * 4);
    if (!image->readPixels(SkImageInfo::Make(width, height, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType),
                           pixels.data(), size_t(width) * 4, 0, 0))
        return {};
    std::string out;
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png)
        return {};
    png_infop info = png_create_info_struct(png);
    std::vector<png_text> chunks;
    std::vector<png_bytep> rows(static_cast<size_t>(height));
    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        return {};
    }
    png_set_write_fn(png, &out, appendBytes, nullptr);
    const bool opaque = image->isOpaque();
    png_set_IHDR(png, info, png_uint_32(width), png_uint_32(height), 8,
                 opaque ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    for (const auto& [key, value] : text) {
        png_text chunk{};
        chunk.compression = PNG_TEXT_COMPRESSION_NONE;
        chunk.key = const_cast<char*>(key.c_str());
        chunk.text = const_cast<char*>(value.c_str());
        chunk.text_length = value.size();
        chunks.push_back(chunk);
    }
    png_set_text(png, info, chunks.data(), int(chunks.size()));
    png_write_info(png, info);
    if (opaque)
        png_set_filler(png, 0, PNG_FILLER_AFTER);
    for (int y = 0; y < height; ++y)
        rows[size_t(y)] = pixels.data() + size_t(y) * size_t(width) * 4;
    png_write_image(png, rows.data());
    png_write_end(png, info);
    png_destroy_write_struct(&png, &info);
    return out;
}

static bool isCurrent(const PngFile& file, const Entry& entry)
{
    const auto uri = file.text.find("Thumb::URI");
    if (!file.image || uri == file.text.end() || uri->second != entry.uri)
        return false;
    // Other implementations may write a fractional MTime.
    const auto mtime = file.text.find("Thumb::MTime");
    if (mtime == file.text.end())
        return false;
    char* end = nullptr;
    const double cached = std::strtod(mtime->second.c_str(), &end);
    while (end && (*end == ' ' || *end == '\t' || *end == '\n'))
        ++end;
    if (end == mtime->second.c_str() || *end != '\0' || int64_t(cached) != entry.mtime)
        return false;
    const auto size = file.text.find("Thumb::Size");
    return size == file.text.end() || size->second.empty() || std::strtoll(size->second.c_str(), nullptr, 10) == entry.size;
}

// Thumbnails leak the content of private files, so the spec wants
// owner-only directories.
static bool ensurePrivateDirectory(const std::string& path)
{
    struct stat info;
    if (stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode))
        return true;
    for (size_t slash = path.find('/', 1); slash != std::string::npos; slash = path.find('/', slash + 1))
        mkdir(path.substr(0, slash).c_str(), 0777);
    if (mkdir(path.c_str(), 0700) != 0 && errno != EEXIST)
        return false;
    return chmod(path.c_str(), 0700) == 0;
}

// Owner-only, and never seen half written by another process.
static bool writeCacheFile(const std::string& path, const std::string& contents)
{
    if (contents.empty())
        return false;
    std::string temporary = path + ".XXXXXX";
    const int fd = mkstemp(temporary.data());
    if (fd < 0)
        return false;
    fchmod(fd, 0600);
    size_t written = 0;
    while (written < contents.size()) {
        const ssize_t n = ::write(fd, contents.data() + written, contents.size() - written);
        if (n < 0 && errno == EINTR)
            continue;
        if (n <= 0)
            break;
        written += size_t(n);
    }
    const bool complete = written == contents.size();
    ::close(fd);
    if (!complete || rename(temporary.c_str(), path.c_str()) != 0) {
        unlink(temporary.c_str());
        return false;
    }
    return true;
}

static std::map<std::string, std::string> metadata(const Entry& entry)
{
    return {{"Thumb::URI", entry.uri},
            {"Thumb::MTime", std::to_string(entry.mtime)},
            {"Thumb::Size", std::to_string(entry.size)}};
}

static sk_sp<SkImage> generate(const Entry& entry)
{
    ImageRequest request;
    request.path = entry.path;
    request.width = entry.cacheSize;
    request.height = entry.cacheSize;
    request.scaling = ImageScaling::Fit;
    request.enlarge = true;
    request.autoTransform = true;
    return decodeImage(request).image;
}

sk_sp<SkImage> request(const std::string& path, int width, int height)
{
    Entry entry;
    if (!locate(path, width, height, &entry))
        return nullptr;

    // The spec forbids thumbnailing the cache itself, which would feed on its output.
    if (entry.insideCache)
        return generate(entry);

    if (std::optional<PngFile> cached = readPng(entry.target); cached && isCurrent(*cached, entry))
        return cached->image;
    if (std::optional<PngFile> failed = readPng(entry.failTarget); failed && isCurrent(*failed, entry))
        return nullptr;

    sk_sp<SkImage> image = generate(entry);
    if (!image) {
        // Remembered, so a broken file is not decoded again on every scroll.
        SkBitmap marker;
        marker.allocPixels(SkImageInfo::MakeN32Premul(1, 1));
        marker.eraseColor(SK_ColorTRANSPARENT);
        if (ensurePrivateDirectory(entry.root) && ensurePrivateDirectory(entry.failDirectory))
            writeCacheFile(entry.failTarget, encodePng(marker.asImage().get(), metadata(entry)));
        return nullptr;
    }
    // An unwritable cache must not hide the thumbnail.
    if (ensurePrivateDirectory(entry.root) && ensurePrivateDirectory(entry.directory))
        writeCacheFile(entry.target, encodePng(image.get(), metadata(entry)));
    return image;
}

} // namespace Lingmo::ui::thumbnails
