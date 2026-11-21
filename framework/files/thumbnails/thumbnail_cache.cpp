// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/files/thumbnails/thumbnail_cache.h"

#include <algorithm>
#include <array>
#include <cerrno>
#include <csetjmp>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <mutex>
#include <optional>
#include <fcntl.h>
#include <png.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#include "base/paths.h"
#include "base/strings.h"
#include "lingmo/files/mime_types.h"
#include "lingmo/files/url.h"
#include "md5_p.h"
#include "thumbnailers_p.h"
#include "../path_utils_p.h"

namespace Lingmo::Files {

using namespace Lingmo::Base;

namespace {

struct Flavor
{
    int size;
    const char* name;
};

// What a cache file holds: its pixels and its text chunks.
struct PngFile
{
    ui::Bitmap image;
    std::map<std::string, std::string> text;
};

// Where the thumbnail of one file at one size lives in the cache.
struct Entry
{
    bool valid = false;
    bool insideCache = false;
    std::string path;
    base::Size cacheSize;
    std::string uri;
    int64_t mtime = 0;
    int64_t size = 0;
    std::string root;
    std::string directory;
    std::string target;
    std::string failDirectory;
    std::string failTarget;
};

} // namespace

// Sizes and names follow the freedesktop.org Thumbnail Managing Standard.
static constexpr Flavor flavors[] = {
    {128, "normal"},
    {256, "large"},
    {512, "x-large"},
    {1024, "xx-large"},
};

static const char* const uriKey = "Thumb::URI";
static const char* const mtimeKey = "Thumb::MTime";
static const char* const sizeKey = "Thumb::Size";

// The image types thumbnailed by the toolkit's own decoder, without a thumbnailer.
static constexpr std::string_view readableTypes[] = {
    "image/png",    "image/jpeg", "image/bmp",  "image/vnd.microsoft.icon", "image/vnd.wap.wbmp",
    "image/webp",   "image/tiff", "image/heif", "image/avif",               "image/jxl",
    "image/svg+xml",
};

static std::mutex s_rootMutex;
static std::string s_cacheRoot;

void internal::setThumbnailCacheRoot(const std::string& path)
{
    const std::lock_guard lock(s_rootMutex);
    s_cacheRoot = path;
}

static std::string cacheRoot()
{
    const std::lock_guard lock(s_rootMutex);
    return s_cacheRoot.empty() ? paths::cacheHome() + "/thumbnails" : s_cacheRoot;
}

static const Flavor& flavorFor(base::Size requestedSize)
{
    const int extent = std::max(requestedSize.width(), requestedSize.height());
    for (const Flavor& flavor : flavors) {
        if (extent <= flavor.size)
            return flavor;
    }

    return flavors[std::size(flavors) - 1];
}

static bool ensurePrivateDirectory(const std::string& path)
{
    struct stat st;
    if (::stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
        return true;

    // Thumbnails leak the content of private files, so the spec requires owner-only access.
    return internal::makePath(path) && ::chmod(path.c_str(), 0700) == 0;
}

// The buffers belong to the caller, whose frame a decoding error longjmps back to.
static void readRows(png_structp png, png_infop info, std::vector<uint8_t>& rgba, std::vector<png_bytep>& rows,
                     PngFile* file)
{
    png_set_expand(png);
    png_set_strip_16(png);
    png_set_gray_to_rgb(png);
    png_set_filler(png, 0xff, PNG_FILLER_AFTER);
    png_set_interlace_handling(png);
    png_read_update_info(png, info);

    const png_uint_32 width = png_get_image_width(png, info);
    const png_uint_32 height = png_get_image_height(png, info);
    rgba.resize(size_t(width) * height * 4);
    rows.resize(height);
    for (png_uint_32 y = 0; y < height; ++y)
        rows[y] = rgba.data() + size_t(y) * width * 4;
    png_read_image(png, rows.data());

    ui::Bitmap image(int(width), int(height), ui::Bitmap::Format::Argb32Premultiplied);
    if (image.isNull())
        return;

    for (png_uint_32 y = 0; y < height; ++y) {
        const uint8_t* source = rows[y];
        uint32_t* target = reinterpret_cast<uint32_t*>(image.scanLine(int(y)));
        for (png_uint_32 x = 0; x < width; ++x) {
            const uint32_t alpha = source[3];
            const uint32_t red = (source[0] * alpha + 127) / 255;
            const uint32_t green = (source[1] * alpha + 127) / 255;
            const uint32_t blue = (source[2] * alpha + 127) / 255;
            target[x] = alpha << 24 | red << 16 | green << 8 | blue;
            source += 4;
        }
    }

    file->image = std::move(image);
}

static void collectText(png_structp png, png_infop info, std::map<std::string, std::string>* text)
{
    png_textp chunks = nullptr;
    int count = 0;
    png_get_text(png, info, &chunks, &count);
    for (int i = 0; i < count; ++i)
        text->insert_or_assign(chunks[i].key, std::string(chunks[i].text, chunks[i].text_length));
}

// A PNG decoded, with the key and text of every text chunk.
static std::optional<PngFile> readPng(const std::string& path)
{
    FILE* stream = std::fopen(path.c_str(), "rbe");
    if (!stream)
        return std::nullopt;

    std::array<png_byte, 8> signature;
    if (std::fread(signature.data(), 1, signature.size(), stream) != signature.size()
        || png_sig_cmp(signature.data(), 0, signature.size()) != 0) {
        std::fclose(stream);
        return std::nullopt;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    png_infop info = png ? png_create_info_struct(png) : nullptr;
    png_infop endInfo = info ? png_create_info_struct(png) : nullptr;
    if (!endInfo) {
        png_destroy_read_struct(&png, &info, nullptr);
        std::fclose(stream);
        return std::nullopt;
    }

    PngFile file;
    std::vector<uint8_t> rgba;
    std::vector<png_bytep> rows;
    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, &endInfo);
        std::fclose(stream);
        return std::nullopt;
    }

    png_init_io(png, stream);
    png_set_sig_bytes(png, int(signature.size()));
    png_set_user_limits(png, 0x10000, 0x10000);
    png_read_info(png, info);
    collectText(png, info, &file.text);
    readRows(png, info, rgba, rows, &file);
    png_read_end(png, endInfo);
    collectText(png, endInfo, &file.text);

    png_destroy_read_struct(&png, &info, &endInfo);
    std::fclose(stream);

    if (file.image.isNull())
        return std::nullopt;
    return file;
}

static void appendBytes(png_structp png, png_bytep data, png_size_t length)
{
    auto* output = static_cast<std::string*>(png_get_io_ptr(png));
    output->append(reinterpret_cast<const char*>(data), length);
}

static std::string encodePng(const ui::Bitmap& source, const std::map<std::string, std::string>& text)
{
    const ui::Bitmap image = source.format() == ui::Bitmap::Format::Argb32Premultiplied
        ? source
        : source.converted(ui::Bitmap::Format::Argb32Premultiplied);
    if (image.isNull())
        return {};

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    png_infop info = png ? png_create_info_struct(png) : nullptr;
    if (!info) {
        png_destroy_write_struct(&png, nullptr);
        return {};
    }

    std::string output;
    std::vector<uint8_t> row(size_t(image.width()) * 4);
    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        return {};
    }

    png_set_write_fn(png, &output, appendBytes, nullptr);
    png_set_IHDR(png, info, png_uint_32(image.width()), png_uint_32(image.height()), 8, PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    std::vector<std::string> keys;
    std::vector<png_text> chunks;
    for (const auto& [key, value] : text) {
        png_text chunk{};
        chunk.compression = PNG_TEXT_COMPRESSION_NONE;
        chunk.key = const_cast<char*>(key.c_str());
        chunk.text = const_cast<char*>(value.c_str());
        chunk.text_length = value.size();
        chunks.push_back(chunk);
    }
    if (!chunks.empty())
        png_set_text(png, info, chunks.data(), int(chunks.size()));

    png_write_info(png, info);

    for (int y = 0; y < image.height(); ++y) {
        const uint32_t* pixels = reinterpret_cast<const uint32_t*>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            const uint32_t pixel = pixels[x];
            const uint32_t alpha = pixel >> 24;
            const auto unpremultiply = [alpha](uint32_t channel) {
                return alpha ? uint8_t(std::min<uint32_t>(255, (channel * 255 + alpha / 2) / alpha)) : uint8_t(0);
            };
            row[size_t(x) * 4] = unpremultiply((pixel >> 16) & 0xff);
            row[size_t(x) * 4 + 1] = unpremultiply((pixel >> 8) & 0xff);
            row[size_t(x) * 4 + 2] = unpremultiply(pixel & 0xff);
            row[size_t(x) * 4 + 3] = uint8_t(alpha);
        }
        png_write_row(png, row.data());
    }

    png_write_end(png, info);
    png_destroy_write_struct(&png, &info);
    return output;
}

// Concurrent requests and separate processes must never see a partial cache entry.
static std::string writeCacheFile(const std::string& path, const std::string& contents)
{
    if (contents.empty())
        return {};

    std::string temporary = path + ".XXXXXX";
    const int fd = ::mkostemp(temporary.data(), O_CLOEXEC);
    if (fd < 0)
        return {};

    bool written = ::fchmod(fd, 0600) == 0;
    size_t offset = 0;
    while (written && offset < contents.size()) {
        const ssize_t count = ::write(fd, contents.data() + offset, contents.size() - offset);
        if (count < 0 && errno == EINTR)
            continue;
        written = count > 0;
        if (written)
            offset += size_t(count);
    }

    if (::close(fd) != 0)
        written = false;

    if (!written || ::rename(temporary.c_str(), path.c_str()) != 0) {
        ::unlink(temporary.c_str());
        return {};
    }

    return path;
}

// Other implementations may write a fractional MTime.
static bool isCurrent(const PngFile& file, const Entry& entry)
{
    const auto value = [&](const char* key) {
        const auto found = file.text.find(key);
        return found == file.text.cend() ? std::string() : found->second;
    };

    if (file.image.isNull() || value(uriKey) != entry.uri)
        return false;

    const std::optional<double> cachedMtime = strings::toDouble(value(mtimeKey));
    if (!cachedMtime || int64_t(*cachedMtime) != entry.mtime)
        return false;

    const std::string cachedSize = value(sizeKey);
    return cachedSize.empty() || strings::toNumber<int64_t>(cachedSize) == entry.size;
}

static std::map<std::string, std::string> metadata(const Entry& entry)
{
    return {{uriKey, entry.uri}, {mtimeKey, strings::number((long long)entry.mtime)},
            {sizeKey, strings::number((long long)entry.size)}};
}

static bool locate(const std::string& filePath, base::Size requestedSize, Entry* entry)
{
    if (requestedSize.width() <= 0 || requestedSize.height() <= 0)
        return false;

    const Url url(filePath);
    entry->path = url.isLocalFile() ? url.toLocalFile() : filePath;
    const std::string absolute = internal::absoluteFilePath(entry->path);

    struct stat st;
    if (::stat(entry->path.c_str(), &st) != 0 || !S_ISREG(st.st_mode))
        return false;

    const Flavor& flavor = flavorFor(requestedSize);
    entry->valid = true;
    entry->cacheSize = base::Size(flavor.size, flavor.size);
    entry->root = cacheRoot();
    entry->insideCache = absolute.starts_with(entry->root + '/');

    entry->uri = Url::fromLocalFile(absolute).toString();
    const std::string name = internal::md5Hex(entry->uri) + ".png";
    entry->mtime = int64_t(st.st_mtim.tv_sec);
    entry->size = int64_t(st.st_size);
    entry->directory = entry->root + '/' + flavor.name;
    entry->target = entry->directory + '/' + name;
    entry->failDirectory = entry->root + "/fail/lingmo";
    entry->failTarget = entry->failDirectory + '/' + name;
    return true;
}

ThumbnailCache* ThumbnailCache::self()
{
    static ThumbnailCache* cache = new ThumbnailCache;
    return cache;
}

bool ThumbnailCache::canThumbnail(const std::string& mimeType)
{
    const std::string name = mime::canonicalName(mimeType).empty() ? mimeType : mime::canonicalName(mimeType);
    for (const std::string_view type : readableTypes) {
        if (type == name)
            return true;
    }

    return internal::Thumbnailers::self()->contains(name);
}

Thumbnail ThumbnailCache::requestThumbnail(const std::string& filePath, base::Size requestedSize)
{
    Entry entry;
    if (!locate(filePath, requestedSize, &entry))
        return {};

    // The spec forbids thumbnailing the cache itself, which would otherwise feed on its output.
    if (entry.insideCache)
        return {{}, generateThumbnail(entry.path, entry.cacheSize)};

    if (std::optional<PngFile> cached = readPng(entry.target); cached && isCurrent(*cached, entry))
        return {entry.target, std::move(cached->image)};

    if (std::optional<PngFile> failed = readPng(entry.failTarget); failed && isCurrent(*failed, entry))
        return {};

    ui::Bitmap image = generateThumbnail(entry.path, entry.cacheSize);
    if (image.isNull()) {
        // Remember the failure so broken images are not decoded again on every scroll.
        ui::Bitmap marker(1, 1, ui::Bitmap::Format::Argb32Premultiplied);
        marker.fill(0);

        if (ensurePrivateDirectory(entry.root) && ensurePrivateDirectory(entry.failDirectory))
            writeCacheFile(entry.failTarget, encodePng(marker, metadata(entry)));

        return {};
    }

    // An unwritable cache must not hide the thumbnail; only the path goes missing.
    if (!ensurePrivateDirectory(entry.root) || !ensurePrivateDirectory(entry.directory))
        return {{}, image};

    return {writeCacheFile(entry.target, encodePng(image, metadata(entry))), image};
}

bool ThumbnailCache::hasCachedThumbnail(const std::string& filePath, base::Size requestedSize)
{
    Entry entry;
    if (!locate(filePath, requestedSize, &entry) || entry.insideCache)
        return false;

    // Without decoding it, an entry older than the file is the likely stale one.
    struct stat st;
    return ::stat(entry.target.c_str(), &st) == 0 && S_ISREG(st.st_mode) && int64_t(st.st_mtim.tv_sec) >= entry.mtime;
}

Thumbnail ThumbnailCache::cachedThumbnail(const std::string& filePath, base::Size requestedSize)
{
    Entry entry;
    if (!locate(filePath, requestedSize, &entry) || entry.insideCache)
        return {};

    std::optional<PngFile> cached = readPng(entry.target);
    if (!cached || !isCurrent(*cached, entry))
        return {};

    return {entry.target, std::move(cached->image)};
}

ui::Bitmap ThumbnailCache::generateThumbnail(const std::string& source, base::Size requestedSize)
{
    if (requestedSize.width() <= 0 || requestedSize.height() <= 0)
        return {};

    ui::Bitmap image = ui::Bitmap::decodeFile(source, requestedSize, ui::Bitmap::Scaling::Fit, true);
    if (image.isNull())
        return internal::Thumbnailers::self()->generate(source, mime::typeForFile(source), requestedSize);

    if (image.width() > requestedSize.width() || image.height() > requestedSize.height())
        image = image.scaled(ui::scaledSize(image.size(), requestedSize, ui::Bitmap::Scaling::Fit));

    return image;
}

} // namespace Lingmo::Files
