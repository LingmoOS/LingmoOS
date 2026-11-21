// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "playlist.h"
#include "urls.h"

#include "base/collator.h"
#include "base/strings.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string_view>
#include <unistd.h>
#include <unordered_set>

using namespace Lingmo;
namespace fs = std::filesystem;

static const std::unordered_set<std::string_view>& mediaSuffixes()
{
    static const std::unordered_set<std::string_view> suffixes = {
        // Video
        "3g2", "3gp", "asf", "avi", "divx", "f4v", "flv", "m2ts", "m4v", "mkv", "mov", "mp4",
        "mpeg", "mpg", "mts", "ogv", "rm", "rmvb", "ts", "vob", "webm", "wmv",
        // Audio
        "aac", "ape", "flac", "m4a", "mka", "mp3", "oga", "ogg", "opus", "wav", "wma", "wv",
    };
    return suffixes;
}

// After the last dot of the name, lower case.
static std::string suffixOf(const std::string& fileName)
{
    const size_t slash = fileName.rfind('/');
    const std::string_view name = std::string_view(fileName).substr(slash == std::string::npos ? 0 : slash + 1);
    const size_t dot = name.rfind('.');
    if (dot == std::string_view::npos)
        return {};
    return base::strings::toAsciiLower(name.substr(dot + 1));
}

Playlist::Playlist() = default;

double Playlist::duration(int row) const
{
    const auto known = m_durations.find(url(row));
    return known == m_durations.end() ? 0 : known->second;
}

int Playlist::indexOf(const std::string& url) const
{
    const auto it = std::find(m_urls.begin(), m_urls.end(), url);
    return it == m_urls.end() ? -1 : int(it - m_urls.begin());
}

void Playlist::setCurrentIndex(int index)
{
    if (index < -1 || index >= count() || index == m_current)
        return;
    const int previous = m_current;
    m_current = index;
    if (previous >= 0)
        rowChanged.emit(previous);
    if (index >= 0)
        rowChanged.emit(index);
    currentIndexChanged.emit();
    navigationChanged.emit();
}

std::string Playlist::currentUrl() const
{
    return m_current >= 0 ? m_urls[size_t(m_current)] : std::string();
}

std::vector<std::string> Playlist::unprobed() const
{
    std::vector<std::string> urls;
    for (const std::string& url : m_urls) {
        if (!m_durations.contains(url))
            urls.push_back(url);
    }
    return urls;
}

void Playlist::setDuration(const std::string& url, double seconds)
{
    if (url.empty() || !(seconds >= 0))
        return;
    const auto known = m_durations.find(url);
    if (known != m_durations.end() && std::abs(known->second - seconds) <= 1e-12 * std::max(1.0, seconds))
        return;
    m_durations[url] = seconds;
    const int row = indexOf(url);
    if (row >= 0)
        rowChanged.emit(row);
}

void Playlist::setRepeatMode(RepeatMode mode)
{
    if (m_repeatMode == mode)
        return;
    m_repeatMode = mode;
    repeatModeChanged.emit();
    navigationChanged.emit();
}

void Playlist::open(const std::vector<std::string>& urls, bool includeFolder)
{
    std::vector<std::string> queue;
    std::string first;
    if (includeFolder && urls.size() == 1 && urls::isLocalFile(urls.front())) {
        std::error_code error;
        const fs::path file = fs::absolute(urls::toLocalFile(urls.front()), error).lexically_normal();
        first = urls::fromLocalFile(file.string());
        queue = folderMedia(file.string());
        if (std::find(queue.begin(), queue.end(), first) == queue.end())
            queue = {first};
    } else {
        for (const std::string& url : urls) {
            if (!url.empty() && std::find(queue.begin(), queue.end(), url) == queue.end())
                queue.push_back(url);
        }
        if (!queue.empty())
            first = queue.front();
    }

    m_urls = std::move(queue);
    m_current = first.empty() ? -1 : indexOf(first);
    rowsChanged.emit();
    countChanged.emit();
    currentIndexChanged.emit();
    navigationChanged.emit();
}

void Playlist::append(const std::vector<std::string>& urls)
{
    std::vector<std::string> added;
    for (const std::string& url : urls) {
        if (!url.empty() && indexOf(url) < 0 && std::find(added.begin(), added.end(), url) == added.end())
            added.push_back(url);
    }
    if (added.empty())
        return;

    m_urls.insert(m_urls.end(), added.begin(), added.end());
    rowsChanged.emit();
    countChanged.emit();
    navigationChanged.emit();
}

void Playlist::remove(int index)
{
    if (index < 0 || index >= count())
        return;

    m_urls.erase(m_urls.begin() + index);
    const bool currentChanged = index <= m_current;
    if (index < m_current)
        --m_current;
    else if (index == m_current)
        m_current = -1;

    rowsChanged.emit();
    countChanged.emit();
    if (currentChanged)
        currentIndexChanged.emit();
    navigationChanged.emit();
}

void Playlist::move(int from, int to)
{
    if (from < 0 || from >= count() || to < 0 || to >= count() || from == to)
        return;

    std::string url = std::move(m_urls[size_t(from)]);
    m_urls.erase(m_urls.begin() + from);
    m_urls.insert(m_urls.begin() + to, std::move(url));
    int current = m_current;
    if (current == from)
        current = to;
    else if (from < current && current <= to)
        --current;
    else if (to <= current && current < from)
        ++current;
    const bool currentChanged = current != m_current;
    m_current = current;

    rowsChanged.emit();
    if (currentChanged)
        currentIndexChanged.emit();
    navigationChanged.emit();
}

void Playlist::clear()
{
    if (m_urls.empty())
        return;
    m_urls.clear();
    m_current = -1;
    rowsChanged.emit();
    countChanged.emit();
    currentIndexChanged.emit();
    navigationChanged.emit();
}

int Playlist::nextIndex(bool automatic) const
{
    if (m_urls.empty())
        return -1;
    if (automatic && m_repeatMode == RepeatMode::One && m_current >= 0)
        return m_current;
    if (m_current + 1 < count())
        return m_current + 1;
    return m_repeatMode == RepeatMode::All ? 0 : -1;
}

int Playlist::previousIndex() const
{
    if (m_current < 0)
        return -1;
    if (m_current > 0)
        return m_current - 1;
    return m_repeatMode == RepeatMode::All && count() > 1 ? count() - 1 : -1;
}

bool Playlist::next(bool automatic)
{
    const int index = nextIndex(automatic);
    if (index < 0)
        return false;
    if (index == m_current)
        currentIndexChanged.emit();
    else
        setCurrentIndex(index);
    return true;
}

bool Playlist::previous()
{
    const int index = previousIndex();
    if (index < 0)
        return false;
    if (index == m_current)
        currentIndexChanged.emit();
    else
        setCurrentIndex(index);
    return true;
}

bool Playlist::isMediaFile(const std::string& fileName)
{
    const std::string suffix = suffixOf(fileName);
    if (!mediaSuffixes().contains(suffix))
        return false;
    // .ts is also a translation source, which is XML; a transport stream is binary.
    if (suffix == "ts") {
        std::ifstream file(fileName, std::ios::binary);
        if (file) {
            char head[64];
            file.read(head, sizeof(head));
            const std::string_view start(head, size_t(file.gcount()));
            return !base::strings::trimmed(start).starts_with('<');
        }
    }
    return true;
}

bool Playlist::isSubtitleFile(const std::string& fileName)
{
    static const std::unordered_set<std::string_view> suffixes = {
        "ass", "idx", "smi", "srt", "ssa", "sub", "sup", "vtt",
    };
    return suffixes.contains(suffixOf(fileName));
}

std::vector<std::string> Playlist::folderMedia(const std::string& file)
{
    std::error_code error;
    const fs::path path = fs::absolute(file, error).lexically_normal();
    const fs::path folder = path.parent_path();
    const std::string ownName = path.filename().string();

    std::vector<std::string> names;
    for (const fs::directory_entry& entry : fs::directory_iterator(folder, error)) {
        std::error_code entryError;
        if (!entry.is_regular_file(entryError))
            continue;
        const std::string name = entry.path().filename().string();
        // Readable files only, and no hidden ones.
        if (name.starts_with('.') || access(entry.path().c_str(), R_OK) != 0)
            continue;
        if (isMediaFile(entry.path().string()) || name == ownName)
            names.push_back(name);
    }

    base::Collator collator;
    collator.setNumericMode(true);
    collator.setCaseSensitive(false);
    std::sort(names.begin(), names.end(), collator);

    std::vector<std::string> urls;
    urls.reserve(names.size());
    for (const std::string& name : names)
        urls.push_back(urls::fromLocalFile((folder / name).string()));
    return urls;
}

std::string Playlist::titleFor(const std::string& url)
{
    if (urls::isLocalFile(url))
        return fs::path(urls::toLocalFile(url)).filename().string();
    const std::string name = urls::fileName(url);
    return name.empty() ? urls::displayString(url) : name;
}
