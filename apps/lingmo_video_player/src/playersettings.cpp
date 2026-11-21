// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "playersettings.h"
#include "languages.h"
#include "urls.h"

#include "base/paths.h"
#include "base/settings.h"

#include <algorithm>
#include <filesystem>

using namespace Lingmo;
namespace fs = std::filesystem;

static const std::string kRecentKey = "Recent/Files";

static std::string playerKey(const char* name)
{
    return std::string("Player/") + name;
}

PlayerSettings::PlayerSettings(std::string file)
    : m_file(std::move(file))
{
    const base::Settings settings(m_file);
    m_volume = int(std::clamp<int64_t>(settings.intValue(playerKey("Volume"), 100), 0, 150));
    m_muted = settings.boolValue(playerKey("Muted"), false);
    m_resumePlayback = settings.boolValue(playerKey("ResumePlayback"), true);
    m_repeatMode = int(std::clamp<int64_t>(settings.intValue(playerKey("RepeatMode"), 0), 0, 2));
    m_subtitleScale = int(std::clamp<int64_t>(settings.intValue(playerKey("SubtitleScale"), 100), 50, 200));
    m_folderPlaylist = settings.boolValue(playerKey("FolderPlaylist"), true);
    m_audioLanguage = settings.stringValue(playerKey("AudioLanguage"));
    m_subtitleLanguage = settings.stringValue(playerKey("SubtitleLanguage"));
    m_screenshotFolder = settings.stringValue(
            playerKey("ScreenshotFolder"),
            (fs::path(base::paths::userDirectory(base::paths::UserDirectory::Pictures)) / "Screenshots").string());
    m_lastFolder = settings.stringValue(playerKey("LastFolder"));
    m_recent = settings.stringListValue(kRecentKey);
}

template<typename T>
void PlayerSettings::write(const std::string& key, const T& value)
{
    base::Settings settings(m_file);
    settings.setValue(key, value);
}

void PlayerSettings::setVolume(int volume)
{
    volume = std::clamp(volume, 0, 150);
    if (m_volume == volume)
        return;
    m_volume = volume;
    write(playerKey("Volume"), volume);
    volumeChanged.emit();
}

void PlayerSettings::setMuted(bool muted)
{
    if (m_muted == muted)
        return;
    m_muted = muted;
    write(playerKey("Muted"), muted);
    mutedChanged.emit();
}

void PlayerSettings::setResumePlayback(bool resume)
{
    if (m_resumePlayback == resume)
        return;
    m_resumePlayback = resume;
    write(playerKey("ResumePlayback"), resume);
    resumePlaybackChanged.emit();
}

void PlayerSettings::setRepeatMode(int mode)
{
    mode = std::clamp(mode, 0, 2);
    if (m_repeatMode == mode)
        return;
    m_repeatMode = mode;
    write(playerKey("RepeatMode"), mode);
    repeatModeChanged.emit();
}

void PlayerSettings::setSubtitleScale(int scale)
{
    scale = std::clamp(scale, 50, 200);
    if (m_subtitleScale == scale)
        return;
    m_subtitleScale = scale;
    write(playerKey("SubtitleScale"), scale);
    subtitleScaleChanged.emit();
}

void PlayerSettings::setFolderPlaylist(bool enabled)
{
    if (m_folderPlaylist == enabled)
        return;
    m_folderPlaylist = enabled;
    write(playerKey("FolderPlaylist"), enabled);
    folderPlaylistChanged.emit();
}

void PlayerSettings::setAudioLanguage(const std::string& language)
{
    if (m_audioLanguage == language)
        return;
    m_audioLanguage = language;
    write(playerKey("AudioLanguage"), std::string_view(language));
    audioLanguageChanged.emit();
}

void PlayerSettings::setSubtitleLanguage(const std::string& language)
{
    if (m_subtitleLanguage == language)
        return;
    m_subtitleLanguage = language;
    write(playerKey("SubtitleLanguage"), std::string_view(language));
    subtitleLanguageChanged.emit();
}

std::vector<std::string> PlayerSettings::languagePreference(const std::string& language) const
{
    std::vector<std::string> codes;
    auto add = [&codes](const std::string& code) {
        // mpv matches both the two- and three-letter forms a file may carry.
        for (std::string& form : languages::codes(code)) {
            if (std::find(codes.begin(), codes.end(), form) == codes.end())
                codes.push_back(std::move(form));
        }
    };
    if (!language.empty())
        add(language);
    add(languages::systemLanguage());
    add("en");
    return codes;
}

void PlayerSettings::setScreenshotFolder(const std::string& folder)
{
    if (m_screenshotFolder == folder || folder.empty())
        return;
    m_screenshotFolder = folder;
    write(playerKey("ScreenshotFolder"), std::string_view(folder));
    screenshotFolderChanged.emit();
}

void PlayerSettings::setLastFolder(const std::string& folder)
{
    if (m_lastFolder == folder)
        return;
    m_lastFolder = folder;
    write(playerKey("LastFolder"), std::string_view(folder));
    lastFolderChanged.emit();
}

std::vector<PlayerSettings::RecentFile> PlayerSettings::recentFiles() const
{
    std::vector<RecentFile> files;
    for (const std::string& url : m_recent) {
        RecentFile file;
        file.url = url;
        if (urls::isLocalFile(url)) {
            const fs::path path(urls::toLocalFile(url));
            std::error_code error;
            if (!fs::exists(path, error))
                continue;
            file.name = path.filename().string();
            file.folder = path.parent_path().string();
        } else {
            file.name = urls::displayString(url);
            file.folder = urls::host(url);
        }
        files.push_back(std::move(file));
    }
    return files;
}

void PlayerSettings::addRecentFile(const std::string& url)
{
    if (url.empty())
        return;
    if (!m_recent.empty() && m_recent.front() == url)
        return;
    std::erase(m_recent, url);
    m_recent.insert(m_recent.begin(), url);
    if (m_recent.size() > size_t(RecentCapacity))
        m_recent.resize(size_t(RecentCapacity));
    write(kRecentKey, m_recent);
    recentFilesChanged.emit();
}

void PlayerSettings::removeRecentFile(const std::string& url)
{
    if (std::erase(m_recent, url) == 0)
        return;
    write(kRecentKey, m_recent);
    recentFilesChanged.emit();
}

void PlayerSettings::clearRecentFiles()
{
    if (m_recent.empty())
        return;
    m_recent.clear();
    write(kRecentKey, m_recent);
    recentFilesChanged.emit();
}
