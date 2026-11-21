// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "durationprobe.h"
#include "player.h"
#include "playersettings.h"
#include "playlist.h"
#include "resumestore.h"
#include "sleepinhibitor.h"
#include "thumbnailer.h"

#include "base/dbus.h"
#include "base/signal.h"

#include <memory>
#include <string>
#include <vector>

// What the window works with, and the D-Bus face of the running player
// (org.lingmo.VideoPlayer at /VideoPlayer): a second launch hands its files
// over through open().
class Application
{
public:
    static constexpr const char* kService = "org.lingmo.VideoPlayer";
    static constexpr const char* kPath = "/VideoPlayer";

    // settingsFile is video-player.conf; covers go in coverFolder (the
    // cache's by default).
    explicit Application(const std::string& settingsFile, std::string coverFolder = {});

    Playlist& playlist() { return m_playlist; }
    PlayerSettings& settings() { return m_settings; }
    ResumeStore& resume() { return m_resume; }
    SleepInhibitor& inhibitor() { return *m_inhibitor; }
    Thumbnailer& thumbnailer() { return m_thumbnailer; }

    // The picture size of the file on the command line, known before the
    // window first shows so that it opens already fitted to it.
    Player::Size launchVideoSize() const { return m_launchVideoSize; }
    void setLaunchVideoSize(Player::Size size) { m_launchVideoSize = size; }

    // Publishes the D-Bus object; done before the name is taken, so a second
    // launch that finds the name always finds someone to hand its files to.
    bool publish(Lingmo::base::dbus::Connection* bus);

    // 1:05 or 1:02:05; hours are shown whenever reference reaches an hour,
    // so elapsed and total times keep the same shape.
    static std::string formatTime(double seconds, double reference = 0);
    static std::string formatBytes(int64_t bytes);
    // The name to show for a file: its title without the extension.
    static std::string displayName(const std::string& url);
    // A URL typed or pasted by the user, or empty if it is not one.
    static std::string urlFromInput(const std::string& text);
    // The clipboard's text when it is a URL to play, else empty.
    static std::string clipboardUrl(const std::string& clipboardText);
    // Where a screenshot of title taken now is saved; the folder is created.
    std::string screenshotPath(const std::string& title) const;
    void showInFolder(const std::string& url) const;
    static bool isSubtitle(const std::string& url);

    // Splits urls into media to play and subtitles to add, and asks the
    // window to open them.
    void openUrls(const std::vector<std::string>& urls);
    // D-Bus: files and URLs from another launch, as given on its command
    // line, with the activation token that launch was started with.
    void open(const std::vector<std::string>& arguments, const std::string& activationToken = {});
    void activateWindow();

    Lingmo::base::Signal<const std::vector<std::string>&, const std::vector<std::string>&> openRequested;
    // The window comes forward, with an activation token from another
    // launch or an empty one.
    Lingmo::base::Signal<const std::string&> activationRequested;

private:
    Playlist m_playlist;
    PlayerSettings m_settings;
    ResumeStore m_resume;
    std::unique_ptr<SleepInhibitor> m_inhibitor;
    Thumbnailer m_thumbnailer;
    DurationProbe m_durationProbe;
    Player::Size m_launchVideoSize;
    std::unique_ptr<Lingmo::base::dbus::Object> m_object;
    Lingmo::base::dbus::Connection* m_bus = nullptr;
    std::vector<Lingmo::base::Subscription> m_subscriptions;
};
