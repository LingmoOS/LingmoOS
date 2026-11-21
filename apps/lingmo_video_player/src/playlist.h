// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/signal.h"

#include <string>
#include <unordered_map>
#include <vector>

// The files queued for playback, as URLs. Opening one local file queues the
// other media files of its folder around it, in the order a file manager
// lists them.
class Playlist
{
public:
    enum class RepeatMode {
        Off,
        One,
        All,
    };

    Playlist();

    int count() const { return int(m_urls.size()); }
    const std::vector<std::string>& urls() const { return m_urls; }
    const std::string& url(int row) const { return m_urls.at(size_t(row)); }
    std::string title(int row) const { return titleFor(url(row)); }
    // Seconds, 0 while unknown.
    double duration(int row) const;

    int currentIndex() const { return m_current; }
    void setCurrentIndex(int index);
    // Empty when nothing is current.
    std::string currentUrl() const;
    RepeatMode repeatMode() const { return m_repeatMode; }
    void setRepeatMode(RepeatMode mode);
    bool hasNext() const { return nextIndex(false) >= 0; }
    bool hasPrevious() const { return previousIndex() >= 0; }

    // The queued files whose length has not been read yet.
    std::vector<std::string> unprobed() const;
    // Kept per URL across queues, so a file is read once.
    void setDuration(const std::string& url, double seconds);

    // Replaces the queue and makes the first of urls current. With
    // includeFolder, a single local file brings its folder's media files along.
    void open(const std::vector<std::string>& urls, bool includeFolder = true);
    // Adds urls that are not queued yet to the end.
    void append(const std::vector<std::string>& urls);
    void remove(int index);
    void move(int from, int to);
    void clear();

    // What follows the current file: automatic says the current one ended by
    // itself, so Repeat One plays it again and Repeat Off stops at the end.
    int nextIndex(bool automatic = false) const;
    int previousIndex() const;
    bool next(bool automatic = false);
    bool previous();

    static bool isMediaFile(const std::string& fileName);
    static bool isSubtitleFile(const std::string& fileName);
    // The media files in the folder of file, file included, in natural order,
    // as URLs.
    static std::vector<std::string> folderMedia(const std::string& file);
    static std::string titleFor(const std::string& url);

    // Rows added, removed, moved or replaced.
    Lingmo::base::Signal<> rowsChanged;
    // One row's data (current mark, length) changed.
    Lingmo::base::Signal<int> rowChanged;
    Lingmo::base::Signal<> countChanged;
    // Also when the same file is made current again (Repeat One).
    Lingmo::base::Signal<> currentIndexChanged;
    Lingmo::base::Signal<> repeatModeChanged;
    Lingmo::base::Signal<> navigationChanged;

private:
    int indexOf(const std::string& url) const;

    std::vector<std::string> m_urls;
    std::unordered_map<std::string, double> m_durations;
    int m_current = -1;
    RepeatMode m_repeatMode = RepeatMode::Off;
};
