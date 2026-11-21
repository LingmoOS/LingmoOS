// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "application.h"
#include "urls.h"

#include "base/locale.h"
#include "base/strings.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <ctime>
#include <filesystem>
#include <format>
#include <unistd.h>

using namespace Lingmo;
namespace fs = std::filesystem;

static std::string currentDirectory()
{
    std::error_code error;
    const fs::path path = fs::current_path(error);
    return error ? std::string() : path.string();
}

Application::Application(const std::string& settingsFile, std::string coverFolder)
    : m_settings(settingsFile)
    , m_resume(settingsFile)
    , m_inhibitor(std::make_unique<SleepInhibitor>())
    , m_thumbnailer(std::move(coverFolder))
{
    m_playlist.setRepeatMode(Playlist::RepeatMode(m_settings.repeatMode()));
    m_subscriptions.push_back(m_playlist.repeatModeChanged.connect([this] {
        m_settings.setRepeatMode(int(m_playlist.repeatMode()));
    }));
    m_subscriptions.push_back(m_playlist.countChanged.connect([this] {
        m_durationProbe.probe(m_playlist.unprobed());
    }));
    m_subscriptions.push_back(m_durationProbe.found.connect([this](const std::string& url, double seconds) {
        m_playlist.setDuration(url, seconds);
    }));
}

bool Application::publish(base::dbus::Connection* bus)
{
    if (!bus)
        return false;
    m_bus = bus;
    m_object = std::make_unique<base::dbus::Object>(bus, kPath, kService);
    m_object->addMethod(
            "open",
            [this](const std::vector<std::string>& arguments, const std::string& activationToken) {
                open(arguments, activationToken);
            },
            base::dbus::ArgumentNames{{"arguments", "activationToken"}, {}});
    m_object->addMethod("activateWindow", [this] { activateWindow(); });
    return m_object->publish();
}

std::string Application::formatTime(double seconds, double reference)
{
    if (!std::isfinite(seconds) || seconds < 0)
        seconds = 0;
    const auto total = int64_t(std::floor(seconds));
    const int64_t hours = total / 3600;
    const int64_t minutes = total / 60 % 60;
    const int64_t rest = total % 60;
    if (hours > 0 || reference >= 3600)
        return std::format("{}:{:02}:{:02}", hours, minutes, rest);
    return std::format("{}:{:02}", minutes, rest);
}

std::string Application::formatBytes(int64_t bytes)
{
    return base::Locale::system().formattedDataSize(bytes, 1, base::Locale::DataSizeFormat::Traditional);
}

std::string Application::displayName(const std::string& url)
{
    // The name up to its last dot.
    const auto completeBaseName = [](const std::string& path) {
        const std::string name = fs::path(path).filename().string();
        const size_t dot = name.rfind('.');
        return dot == std::string::npos ? name : name.substr(0, dot);
    };
    if (urls::isLocalFile(url))
        return completeBaseName(urls::toLocalFile(url));
    const std::string name = completeBaseName(urls::path(url));
    return name.empty() ? urls::displayString(url) : name;
}

std::string Application::urlFromInput(const std::string& text)
{
    const std::string_view trimmed = base::strings::trimmed(text);
    if (trimmed.empty() || trimmed.find('\n') != std::string_view::npos)
        return {};
    const std::string url = urls::fromUserInput(trimmed, currentDirectory());
    if (url.empty())
        return {};
    if (urls::isLocalFile(url)) {
        std::error_code error;
        return fs::exists(urls::toLocalFile(url), error) ? url : std::string();
    }
    // Schemes mpv streams from; a bare word would otherwise become http://word.
    static constexpr std::array<std::string_view, 8> kSchemes = {
        "http", "https", "rtsp", "rtmp", "mms", "ftp", "smb", "sftp",
    };
    const auto parts = urls::parse(url);
    if (!parts || std::find(kSchemes.begin(), kSchemes.end(), parts->scheme) == kSchemes.end() || parts->host.empty())
        return {};
    if (trimmed.find("://") == std::string_view::npos && parts->host.find('.') == std::string::npos)
        return {};
    return url;
}

std::string Application::clipboardUrl(const std::string& clipboardText)
{
    const std::string text(base::strings::trimmed(clipboardText));
    const std::string url = urlFromInput(text);
    return !url.empty() && !urls::isLocalFile(url) && text.find("://") != std::string::npos ? text : std::string();
}

std::string Application::screenshotPath(const std::string& title) const
{
    const fs::path folder(m_settings.screenshotFolder());
    std::error_code error;
    fs::create_directories(folder, error);
    const std::string name = base::strings::replaceAll(title, "/", "-");
    const std::time_t now = std::time(nullptr);
    std::tm local {};
    localtime_r(&now, &local);
    char stamp[32];
    std::strftime(stamp, sizeof(stamp), "%Y-%m-%d %H.%M.%S", &local);
    fs::path path = folder / std::format("{} {}.png", name, stamp);
    for (int i = 2; fs::exists(path, error); ++i)
        path = folder / std::format("{} {} ({}).png", name, stamp, i);
    return path.string();
}

void Application::showInFolder(const std::string& url) const
{
    base::dbus::Connection* bus = m_bus ? m_bus : base::dbus::Connection::session();
    base::dbus::Proxy fileManager(bus, "org.freedesktop.FileManager1", "/org/freedesktop/FileManager1",
                                  "org.freedesktop.FileManager1");
    fileManager.send("ShowItems", std::vector<std::string>{url}, std::string());
}

bool Application::isSubtitle(const std::string& url)
{
    return Playlist::isSubtitleFile(urls::path(url));
}

void Application::openUrls(const std::vector<std::string>& urlList)
{
    std::vector<std::string> media;
    std::vector<std::string> subtitles;
    for (const std::string& url : urlList) {
        if (url.empty())
            continue;
        if (urls::isLocalFile(url)) {
            std::error_code error;
            const fs::path folder(urls::toLocalFile(url));
            if (fs::is_directory(folder, error)) {
                std::vector<std::string> names;
                for (const fs::directory_entry& entry : fs::directory_iterator(folder, error)) {
                    std::error_code entryError;
                    const std::string name = entry.path().filename().string();
                    if (entry.is_regular_file(entryError) && !name.starts_with('.')
                            && Playlist::isMediaFile(entry.path().string()))
                        names.push_back(name);
                }
                if (!names.empty()) {
                    // Any file of the folder brings all of its media along.
                    const std::vector<std::string> files =
                            Playlist::folderMedia((folder / *std::min_element(names.begin(), names.end())).string());
                    media.insert(media.end(), files.begin(), files.end());
                }
                continue;
            }
        }
        (isSubtitle(url) ? subtitles : media).push_back(url);
    }
    if (!media.empty() || !subtitles.empty())
        openRequested.emit(media, subtitles);
}

void Application::open(const std::vector<std::string>& arguments, const std::string& activationToken)
{
    std::vector<std::string> urlList;
    const std::string directory = currentDirectory();
    for (const std::string& argument : arguments)
        urlList.push_back(urls::fromUserInput(argument, directory, true));
    openUrls(urlList);
    activationRequested.emit(activationToken);
}

void Application::activateWindow()
{
    activationRequested.emit(std::string());
}
