// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "application.h"
#include "mprisserver.h"
#include "player.h"

#include "base/signal.h"
#include "base/timer.h"
#include "ui/file_chooser.h"
#include "ui/menu.h"
#include "ui/window.h"

#include <memory>
#include <string>
#include <vector>

class ControlBar;
class SidePanel;
class VideoView;
class VideoSurface;
class Dialogs;
namespace views {
class Tween;
}

// The player's window and what it does.
class MainWindow
{
public:
    static constexpr double kHeaderHeight = 44;
    static constexpr double kSpeeds[] = {0.5, 0.75, 1, 1.25, 1.5, 2};

    explicit MainWindow(Application& application);
    ~MainWindow();

    Lingmo::ui::Window& window() { return m_window; }
    // Expires with the window, for answers that arrive later.
    std::weak_ptr<bool> aliveToken() const { return m_alive; }
    Application& app() { return m_app; }
    Player& player() { return m_player; }
    VideoView* videoView() const { return m_video; }

    // Before show(): fits the window to the file the command line opens.
    void fitToLaunch();
    void show();

    bool hasMedia() const { return !m_player.source().empty(); }
    const std::string& errorMessage() const { return m_errorMessage; }
    std::string mediaName() const;
    bool isFullScreen() const { return m_window.states().fullScreen; }
    bool panelOpen() const { return m_panelOpen; }
    int panelTab() const { return m_panelTab; }
    bool controlsVisible() const;
    double chromeOpacity() const;
    bool menuOpen() const;

    // What the window does.
    void poke();
    void togglePanel(int tab, bool focusList);
    void setPanelTab(int tab);
    void openFiles(bool append);
    void openSubtitleFile();
    void openUrlDialog();
    void openPreferences();
    void openShortcuts();
    void openAbout();
    void showMediaInfo();
    void openMainMenu(Lingmo::ui::View* button);
    void openContextMenu(Lingmo::ui::PointF windowPosition);
    void openPlaylistMenu(int index, Lingmo::ui::PointF windowPosition);
    void playIndex(int index);
    void playNext(bool automatic);
    void playPrevious();
    void togglePause(bool showFlash);
    void seekTo(double seconds);
    void seekBy(double seconds);
    void setVolume(int value);
    void toggleMute();
    void setSpeed(double speed);
    void stepSpeed(int direction);
    void stepChapter(int direction);
    void selectSubtitle(int id);
    void selectAudio(int id);
    void cycleSubtitle(int direction);
    void toggleSubtitles();
    void cycleAudio();
    void shiftSubtitles(double seconds);
    void takeScreenshot();
    void toggleFullScreen();
    void close();
    void showOsd(const std::string& message, int timeoutMs = 0, std::string actionText = {},
                 std::function<void()> action = {});

    static std::string speedText(double speed);
    static std::string delayText(double seconds);

    // Something the views show changed: the window's state, the player's.
    Lingmo::base::Signal<> stateChanged;

private:
    class Root;
    friend class Root;

    void retranslate();
    void buildMenus();
    void bindShortcuts();
    void bindMpris();
    void applyLanguages();
    void loadCurrent();
    void rememberRecent();
    void saveResume();
    void fileEnded();
    void fitToVideo();
    Lingmo::ui::SizeF fittedSize(Player::Size video) const;
    void onPlayerChanged(Player::Property property);
    void updateChrome();
    void updateInhibitor();
    void updateMpris();
    void updateTitle();
    void updateGlass();

    Application& m_app;
    Player m_player;
    // Goes after the window, which then closes with the picture still in it.
    std::unique_ptr<VideoSurface> m_closingVideo;
    Lingmo::ui::Window m_window;
    Root* m_root = nullptr;
    VideoView* m_video = nullptr;
    ControlBar* m_controlBar = nullptr;
    SidePanel* m_sidePanel = nullptr;
    std::unique_ptr<Dialogs> m_dialogs;
    std::unique_ptr<MprisServer> m_mpris;
    std::unique_ptr<views::Tween> m_chrome;

    std::string m_errorMessage;
    bool m_panelOpen = false;
    int m_panelTab = 0;
    bool m_pointerActive = true;
    bool m_fitPending = false;
    bool m_launchFitPending = false;
    bool m_recentPending = false;
    bool m_wasMaximized = false;
    int m_lastSubtitle = 0;
    std::vector<std::string> m_pendingSubtitles;
    double m_appliedInset = -1;

    Lingmo::ui::Menu m_mainMenu;
    Lingmo::ui::Menu m_contextMenu;
    Lingmo::ui::Menu m_playlistMenu;
    int m_playlistMenuIndex = -1;
    std::unique_ptr<Lingmo::ui::FileChooser> m_fileChooser;

    Lingmo::base::Timer m_idleTimer;
    Lingmo::base::Timer m_resumeTimer;
    Lingmo::base::Timer m_bufferingDelay;
    Lingmo::base::Timer m_panelDismissed;
    bool m_bufferingElapsed = false;
    bool m_dismissOutside = false;
    Lingmo::ui::PointF m_dismissPress;
    std::vector<Lingmo::base::Subscription> m_subscriptions;
    Lingmo::base::Subscription m_languageChanged;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};
