// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "libraryscanner.h"
#include <base/event_loop.h>
#include <base/thread_pool.h>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <array>
#include <sstream>

namespace Photos {
namespace {
std::string captureDate(const std::filesystem::file_time_type& stamp)
{
    const auto system = std::chrono::time_point_cast<std::chrono::system_clock::duration>(stamp - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    const std::time_t value = std::chrono::system_clock::to_time_t(system);
    std::tm local{};
    localtime_r(&value, &local);
    std::ostringstream result;
    result << std::put_time(&local, "%Y-%m-%d");
    return result.str();
}
bool supported(const std::filesystem::path& path) {
    static const std::vector<std::string> formats = {".jpg", ".jpeg", ".png", ".webp", ".avif", ".gif", ".bmp", ".tif", ".tiff", ".heic", ".heif", ".svg"};
    const auto ext = path.extension().string();
    auto lower = ext;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char value) { return char(std::tolower(value)); });
    return std::find(formats.begin(), formats.end(), lower) != formats.end();
}
uint32_t little32(const std::array<unsigned char, 32>& bytes, size_t offset)
{
    return uint32_t(bytes[offset]) | (uint32_t(bytes[offset + 1]) << 8) | (uint32_t(bytes[offset + 2]) << 16) | (uint32_t(bytes[offset + 3]) << 24);
}
uint32_t big32(const std::array<unsigned char, 32>& bytes, size_t offset)
{
    return (uint32_t(bytes[offset]) << 24) | (uint32_t(bytes[offset + 1]) << 16) | (uint32_t(bytes[offset + 2]) << 8) | uint32_t(bytes[offset + 3]);
}
std::pair<int, int> dimensions(const std::filesystem::path& path)
{
    std::ifstream input(path, std::ios::binary);
    std::array<unsigned char, 32> header{};
    input.read(reinterpret_cast<char*>(header.data()), header.size());
    if (!input)
        return {};
    if (header[0] == 0x89 && header[1] == 'P' && header[2] == 'N' && header[3] == 'G')
        return {int(big32(header, 16)), int(big32(header, 20))};
    if ((header[0] == 'G' && header[1] == 'I' && header[2] == 'F') || (header[0] == 'B' && header[1] == 'M'))
        return header[0] == 'G' ? std::pair<int, int>{int(header[6] | (header[7] << 8)), int(header[8] | (header[9] << 8))} : std::pair<int, int>{int(little32(header, 18)), int(little32(header, 22))};
    if (header[0] == 'R' && header[1] == 'I' && header[2] == 'F' && header[8] == 'W' && header[9] == 'E' && header[10] == 'B' && header[11] == 'P' && header[12] == 'V' && header[13] == 'P' && header[14] == '8' && header[15] == 'X')
        return {1 + int(header[24] | (header[25] << 8) | (header[26] << 16)), 1 + int(header[27] | (header[28] << 8) | (header[29] << 16))};
    if (header[0] != 0xFF || header[1] != 0xD8)
        return {};
    input.clear(); input.seekg(2);
    while (input) {
        unsigned char markerStart = 0, marker = 0; input.read(reinterpret_cast<char*>(&markerStart), 1); if (!input) break; if (markerStart != 0xFF) continue; do input.read(reinterpret_cast<char*>(&marker), 1); while (input && marker == 0xFF); if (!input || marker == 0xD9 || marker == 0xDA) break; unsigned char lengthBytes[2]{}; input.read(reinterpret_cast<char*>(lengthBytes), 2); const uint16_t length = uint16_t(lengthBytes[0] << 8 | lengthBytes[1]); if (length < 2) break; if ((marker >= 0xC0 && marker <= 0xC3) || (marker >= 0xC5 && marker <= 0xC7) || (marker >= 0xC9 && marker <= 0xCB) || (marker >= 0xCD && marker <= 0xCF)) { unsigned char sof[5]{}; input.read(reinterpret_cast<char*>(sof), 5); return {int(sof[3] << 8 | sof[4]), int(sof[1] << 8 | sof[2])}; } input.seekg(length - 2, std::ios::cur);
    }
    return {};
}
}
LibraryScanner::LibraryScanner(PhotoStore& store) : m_store(store) {}
LibraryScanner::~LibraryScanner() { cancel(); }
void LibraryScanner::cancel() { m_cancelled->store(true); }
void LibraryScanner::scan(std::string root, std::function<void()> done) {
    cancel();
    m_cancelled = std::make_shared<std::atomic_bool>(false);
    auto cancelled = m_cancelled;
    auto loop = base::EventLoop::current()->handle();
    base::ThreadPool::global().post([this, root = std::move(root), done = std::move(done), cancelled, loop] {
        std::vector<Photo> found;
        try {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(root, std::filesystem::directory_options::skip_permission_denied)) {
                if (cancelled->load()) return;
                if (!entry.is_regular_file() || !supported(entry.path())) continue;
                Photo photo;
                photo.path = entry.path().string();
                photo.name = entry.path().filename().string();
                photo.folder = entry.path().parent_path().string();
                photo.format = entry.path().extension().string();
                photo.bytes = static_cast<int64_t>(entry.file_size());
                const auto [width, height] = dimensions(entry.path());
                photo.width = width;
                photo.height = height;
                photo.captured = captureDate(entry.last_write_time());
                found.push_back(std::move(photo));
            }
        } catch (...) {
        }
        loop.post([this, done = std::move(done), found = std::move(found)] {
            for (const auto& photo : found)
                m_store.upsert(photo);
            changed();
            if (done) done();
        });
    });
}
} // namespace Photos
