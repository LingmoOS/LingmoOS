// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/signal.h"

#include <string>
#include <vector>

// The player's preferences and recent files in video-player.conf.
class PlayerSettings
{
public:
    static constexpr int RecentCapacity = 10;

    struct RecentFile
    {
        std::string url;
        std::string name;
        // The folder of a local file, the host of a stream.
        std::string folder;
    };

    explicit PlayerSettings(std::string file);

    int volume() const { return m_volume; }
    void setVolume(int volume);
    bool muted() const { return m_muted; }
    void setMuted(bool muted);
    bool resumePlayback() const { return m_resumePlayback; }
    void setResumePlayback(bool resume);
    // A Playlist::RepeatMode.
    int repeatMode() const { return m_repeatMode; }
    void setRepeatMode(int mode);
    // Subtitle size in percent of the default.
    int subtitleScale() const { return m_subtitleScale; }
    void setSubtitleScale(int scale);
    // Opening one file queues the other media files of its folder.
    bool folderPlaylist() const { return m_folderPlaylist; }
    void setFolderPlaylist(bool enabled);
    // ISO 639 codes, or empty for the display language.
    const std::string& audioLanguage() const { return m_audioLanguage; }
    void setAudioLanguage(const std::string& language);
    const std::string& subtitleLanguage() const { return m_subtitleLanguage; }
    void setSubtitleLanguage(const std::string& language);
    // The track languages to ask mpv for, the chosen one first, then the
    // display language and English.
    std::vector<std::string> languagePreference(const std::string& language) const;
    const std::string& screenshotFolder() const { return m_screenshotFolder; }
    void setScreenshotFolder(const std::string& folder);
    // A URL.
    const std::string& lastFolder() const { return m_lastFolder; }
    void setLastFolder(const std::string& folder);
    // Most recent first; entries that no longer exist on disk are left out.
    std::vector<RecentFile> recentFiles() const;

    void addRecentFile(const std::string& url);
    void removeRecentFile(const std::string& url);
    void clearRecentFiles();

    Lingmo::base::Signal<> volumeChanged;
    Lingmo::base::Signal<> mutedChanged;
    Lingmo::base::Signal<> resumePlaybackChanged;
    Lingmo::base::Signal<> repeatModeChanged;
    Lingmo::base::Signal<> subtitleScaleChanged;
    Lingmo::base::Signal<> folderPlaylistChanged;
    Lingmo::base::Signal<> audioLanguageChanged;
    Lingmo::base::Signal<> subtitleLanguageChanged;
    Lingmo::base::Signal<> screenshotFolderChanged;
    Lingmo::base::Signal<> lastFolderChanged;
    Lingmo::base::Signal<> recentFilesChanged;

private:
    template<typename T>
    void write(const std::string& key, const T& value);

    std::string m_file;
    int m_volume = 100;
    bool m_muted = false;
    bool m_resumePlayback = true;
    int m_repeatMode = 0;
    int m_subtitleScale = 100;
    bool m_folderPlaylist = true;
    std::string m_audioLanguage;
    std::string m_subtitleLanguage;
    std::string m_screenshotFolder;
    std::string m_lastFolder;
    std::vector<std::string> m_recent;
};
