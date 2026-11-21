// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/wallpaper/wallpaper_cache.h"

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>

#include "base/hash.h"
#include "base/paths.h"
#include "ui/geometry.h"

namespace Lingmo::Shell {

static constexpr char kMagic[4] = {'C', 'F', 'W', 'P'};
// Wallpapers larger than this are not decoded.
static constexpr int kMaxDecodeSide = 16384;

// A cache file: the magic, the key's length and the key, width, height and
// row stride, then the rows of premultiplied 0xAARRGGBB pixels.
struct FileHeader {
    char magic[4];
    uint32_t keyLength;
};

struct ImageHeader {
    int32_t width;
    int32_t height;
    uint32_t stride;
};

static bool readAll(int fd, void *data, size_t size)
{
    auto *bytes = static_cast<uint8_t *>(data);
    while (size > 0) {
        const ssize_t got = ::read(fd, bytes, size);
        if (got < 0 && errno == EINTR)
            continue;
        if (got <= 0)
            return false;
        bytes += got;
        size -= size_t(got);
    }
    return true;
}

static bool writeAll(int fd, const void *data, size_t size)
{
    const auto *bytes = static_cast<const uint8_t *>(data);
    while (size > 0) {
        const ssize_t put = ::write(fd, bytes, size);
        if (put < 0 && errno == EINTR)
            continue;
        if (put <= 0)
            return false;
        bytes += put;
        size -= size_t(put);
    }
    return true;
}

static std::string cacheFilePath(const std::string &directory, const std::string &key)
{
    return directory + '/' + base::CryptographicHash::hashHex(key, base::CryptographicHash::Algorithm::Sha1);
}

static ui::Bitmap readCacheFile(const std::string &fileName, const std::string &key)
{
    const int fd = ::open(fileName.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0)
        return {};
    struct stat info;
    FileHeader header;
    std::string storedKey;
    ImageHeader image {};
    bool ok = ::fstat(fd, &info) == 0 && readAll(fd, &header, sizeof(header))
            && std::memcmp(header.magic, kMagic, sizeof(kMagic)) == 0 && header.keyLength < 65536;
    if (ok) {
        storedKey.resize(header.keyLength);
        ok = readAll(fd, storedKey.data(), storedKey.size());
    }
    // Another key with the same hash is not this picture, but not broken either.
    if (ok && storedKey != key) {
        ::close(fd);
        return {};
    }
    ui::Bitmap bitmap;
    if (ok && readAll(fd, &image, sizeof(image)) && image.width > 0 && image.height > 0
        && image.width <= kMaxDecodeSide && image.height <= kMaxDecodeSide
        && image.stride == uint32_t(image.width) * 4
        && off_t(sizeof(header) + header.keyLength + sizeof(image)) + off_t(image.stride) * image.height
                == info.st_size) {
        bitmap = ui::Bitmap(image.width, image.height, ui::Bitmap::Format::Argb32Premultiplied);
        for (int y = 0; !bitmap.isNull() && y < image.height; ++y) {
            if (!readAll(fd, bitmap.scanLine(y), image.stride))
                bitmap = {};
        }
    }
    ::close(fd);
    if (bitmap.isNull()) {
        ::unlink(fileName.c_str());
        return {};
    }
    // Recently used files survive the trim.
    ::utimensat(AT_FDCWD, fileName.c_str(), nullptr, 0);
    return bitmap;
}

static bool makeDirectories(const std::string &path)
{
    for (size_t slash = path.find('/', 1);; slash = path.find('/', slash + 1)) {
        const std::string part = path.substr(0, slash);
        if (::mkdir(part.c_str(), 0700) != 0 && errno != EEXIST)
            return false;
        if (slash == std::string::npos)
            return true;
    }
}

// Keeps the most recently used files.
static void trimDirectory(const std::string &directory)
{
    DIR *dir = ::opendir(directory.c_str());
    if (!dir)
        return;
    std::vector<std::pair<int64_t, std::string>> files;
    while (const dirent *entry = ::readdir(dir)) {
        const std::string path = directory + '/' + entry->d_name;
        struct stat info;
        if (entry->d_name[0] == '.' || ::stat(path.c_str(), &info) != 0 || !S_ISREG(info.st_mode))
            continue;
        files.emplace_back(int64_t(info.st_mtim.tv_sec) * 1000000000 + info.st_mtim.tv_nsec, path);
    }
    ::closedir(dir);
    std::sort(files.begin(), files.end(), std::greater<>());
    for (size_t i = WallpaperCache::kMaxCachedFiles; i < files.size(); ++i)
        ::unlink(files[i].second.c_str());
}

static void writeCacheFile(const std::string &directory, const std::string &fileName, const std::string &key,
                           const ui::Bitmap &bitmap)
{
    if (bitmap.format() != ui::Bitmap::Format::Argb32Premultiplied || !makeDirectories(directory))
        return;
    // Written whole or not at all: a crash leaves no half file to read.
    std::string temporary = fileName + ".XXXXXX";
    const int fd = ::mkostemp(temporary.data(), O_CLOEXEC);
    if (fd < 0)
        return;
    FileHeader header;
    std::memcpy(header.magic, kMagic, sizeof(kMagic));
    header.keyLength = uint32_t(key.size());
    const ImageHeader image {bitmap.width(), bitmap.height(), uint32_t(bitmap.width()) * 4};
    bool ok = writeAll(fd, &header, sizeof(header)) && writeAll(fd, key.data(), key.size())
            && writeAll(fd, &image, sizeof(image));
    for (int y = 0; ok && y < bitmap.height(); ++y)
        ok = writeAll(fd, bitmap.constScanLine(y), image.stride);
    ok = ::close(fd) == 0 && ok;
    if (!ok || ::rename(temporary.c_str(), fileName.c_str()) != 0) {
        ::unlink(temporary.c_str());
        return;
    }
    trimDirectory(directory);
}

WallpaperCache::WallpaperCache(std::string directory)
    : m_directory(directory.empty() ? defaultDirectory() : std::move(directory))
{
}

WallpaperCache::~WallpaperCache() = default;

std::string WallpaperCache::defaultDirectory()
{
    return base::paths::cacheHome() + "/lingmo/wallpapers";
}

std::string WallpaperCache::cacheKey(const std::string &path, const base::Size &deviceSize)
{
    char resolved[PATH_MAX];
    struct stat info;
    if (!::realpath(path.c_str(), resolved) || ::stat(resolved, &info) != 0 || !S_ISREG(info.st_mode))
        return {};
    const int64_t modified = int64_t(info.st_mtim.tv_sec) * 1000 + info.st_mtim.tv_nsec / 1000000;
    return std::string(resolved) + '\n' + std::to_string(int64_t(info.st_size)) + '\n' + std::to_string(modified)
            + '\n' + std::to_string(deviceSize.width()) + 'x' + std::to_string(deviceSize.height()) + '\n'
            + std::to_string(kCacheVersion);
}

ui::Bitmap WallpaperCache::decode(const std::string &path, const base::Size &deviceSize)
{
    if (path.empty() || deviceSize.isEmpty() || deviceSize.width() > kMaxDecodeSide
        || deviceSize.height() > kMaxDecodeSide)
        return {};
    ui::Bitmap image = ui::Bitmap::decodeFile(path, deviceSize, ui::Bitmap::Scaling::Cover, true);
    if (image.isNull())
        return {};
    // Integer rounding can leave the cover a pixel short of the target.
    const base::Size cover(std::max(image.width(), deviceSize.width()),
                           std::max(image.height(), deviceSize.height()));
    if (image.size() != cover)
        image = image.scaled(cover);
    if (image.size() != deviceSize) {
        image = image.copy(base::Rect((cover.width() - deviceSize.width()) / 2,
                                      (cover.height() - deviceSize.height()) / 2, deviceSize.width(),
                                      deviceSize.height()));
    }
    if (image.format() != ui::Bitmap::Format::Argb32Premultiplied)
        image = image.converted(ui::Bitmap::Format::Argb32Premultiplied);
    return image;
}

ui::Bitmap WallpaperCache::cachedOrDecode(const std::string &path, const base::Size &deviceSize,
                                          const std::string &directory)
{
    const std::string key = cacheKey(path, deviceSize);
    if (key.empty() || deviceSize.isEmpty() || directory.empty())
        return decode(path, deviceSize);

    const std::string fileName = cacheFilePath(directory, key);
    ui::Bitmap image = readCacheFile(fileName, key);
    if (!image.isNull())
        return image;

    image = decode(path, deviceSize);
    // Off the decode's path: everyone waiting gets the picture first.
    if (!image.isNull()) {
        base::ThreadPool::global().post([directory, fileName, key, image] {
            writeCacheFile(directory, fileName, key, image);
        });
    }
    return image;
}

ui::Bitmap WallpaperCache::cached(const std::string &path, const base::Size &deviceSize) const
{
    const Key key {path, deviceSize.width(), deviceSize.height()};
    for (const auto &[entry, image] : m_images) {
        if (entry == key)
            return image;
    }
    return {};
}

std::shared_ptr<void> WallpaperCache::load(const std::string &path, const base::Size &deviceSize, Done done)
{
    const Key key {path, deviceSize.width(), deviceSize.height()};
    for (auto it = m_images.begin(); it != m_images.end(); ++it) {
        if (it->first == key) {
            m_images.splice(m_images.begin(), m_images, it);
            const ui::Bitmap image = it->second;
            done(image);
            return nullptr;
        }
    }

    auto request = std::make_shared<Request>(Request {std::move(done)});
    auto inFlight = m_inFlight.find(key);
    if (inFlight == m_inFlight.end()) {
        inFlight = m_inFlight.emplace(key, InFlight()).first;
        inFlight->second.job = base::ThreadPool::global().run(
                [path, deviceSize, directory = m_directory] {
                    return cachedOrDecode(path, deviceSize, directory);
                },
                [this, key](ui::Bitmap image) { finish(key, image); });
    }
    inFlight->second.requests.push_back(request);
    return request;
}

void WallpaperCache::finish(const Key &key, const ui::Bitmap &image)
{
    const auto it = m_inFlight.find(key);
    if (it == m_inFlight.end())
        return;
    std::vector<std::weak_ptr<Request>> requests = std::move(it->second.requests);
    it->second.job.detach();
    m_inFlight.erase(it);

    if (!image.isNull()) {
        m_images.emplace_front(key, image);
        while (int(m_images.size()) > kMaxCachedImages)
            m_images.pop_back();
    }
    for (const std::weak_ptr<Request> &weak : requests) {
        if (const std::shared_ptr<Request> request = weak.lock())
            request->done(image);
    }
}

} // namespace Lingmo::Shell
