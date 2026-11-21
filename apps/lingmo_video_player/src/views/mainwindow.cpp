// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "controlbar.h"
#include "dialogs.h"
#include "pages.h"
#include "sidepanel.h"
#include "urls.h"
#include "videoview.h"
#include "widgets.h"

#include "base/i18n.h"
#include "base/locale.h"
#include "base/paths.h"
#include "ui/progress_bar.h"
#include "ui/application.h"
#include "ui/drag.h"
#include "ui/header_button.h"
#include "ui/keys.h"
#include "ui/path.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"
#include "ui/window_header.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <format>
#include <numbers>

using namespace Lingmo;
using namespace Lingmo::ui;
using namespace std::chrono_literals;
using views::white;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;

static constexpr auto kDoubleClickInterval = 400ms;
static constexpr double kStartDragDistance = 10;
static const char* const kMediaPatterns =
        "*.mp4 *.m4v *.mkv *.webm *.avi *.mov *.wmv *.flv *.mpg *.mpeg *.ts *.m2ts *.mts *.ogv *.3gp *.vob *.rm "
        "*.rmvb *.asf *.divx *.f4v *.mp3 *.flac *.ogg *.oga *.opus *.m4a *.aac *.wav *.wma *.ape *.wv *.mka *.m3u "
        "*.m3u8 *.pls";

// A rounded rect: a cubic superellipse over 1.5 times the radius when
// continuous, as the theme's rounded rectangles draw them.
static void addPlate(Path& path, const RectF& rect, double radius, bool continuous)
{
    if (!continuous) {
        path.addRoundedRect(rect, radius, radius);
        return;
    }
    const double extent = std::min({radius * 1.5, rect.width / 2, rect.height / 2});
    constexpr int kSteps = 16;
    // The corner from its horizontal edge to its vertical one, about (cx, cy),
    // with sx, sy pointing outwards.
    const auto corner = [&](double cx, double cy, double sx, double sy, bool first) {
        for (int i = 0; i <= kSteps; ++i) {
            const double t = std::numbers::pi / 2 * double(i) / kSteps;
            const double x = std::cbrt(std::sin(t) * std::sin(t));
            const double y = std::cbrt(std::cos(t) * std::cos(t));
            const PointF point{cx + sx * x * extent, cy + sy * y * extent};
            if (first && i == 0)
                path.moveTo(point.x, point.y);
            else
                path.lineTo(point.x, point.y);
        }
    };
    const double left = rect.x + extent;
    const double right = rect.x + rect.width - extent;
    const double top = rect.y + extent;
    const double bottom = rect.y + rect.height - extent;
    corner(right, top, 1, -1, true);
    // The other corners, each from the edge before it to the edge after it.
    for (int i = 0; i <= kSteps; ++i) {
        const double t = std::numbers::pi / 2 * double(i) / kSteps;
        const double x = std::cbrt(std::cos(t) * std::cos(t));
        const double y = std::cbrt(std::sin(t) * std::sin(t));
        path.lineTo(right + x * extent, bottom + y * extent);
    }
    for (int i = 0; i <= kSteps; ++i) {
        const double t = std::numbers::pi / 2 * double(i) / kSteps;
        const double x = std::cbrt(std::sin(t) * std::sin(t));
        const double y = std::cbrt(std::cos(t) * std::cos(t));
        path.lineTo(left - x * extent, bottom + y * extent);
    }
    for (int i = 0; i <= kSteps; ++i) {
        const double t = std::numbers::pi / 2 * double(i) / kSteps;
        const double x = std::cbrt(std::cos(t) * std::cos(t));
        const double y = std::cbrt(std::sin(t) * std::sin(t));
        path.lineTo(left - x * extent, top - y * extent);
    }
    path.close();
}

// The title and the main menu button.
class HeaderContent : public View
{
public:
    explicit HeaderContent(MainWindow& owner)
        : title(views::addLabel(this, tr("Video Player"), 0, white()))
        , menu(addChild<HeaderButton>("menu", 32))
    {
        title->setName("windowTitle");
        title->setElide(Label::Elide::Middle);
        menu->setName("mainMenuButton");
        menu->onClicked = [&owner, this] { owner.openMainMenu(menu); };
    }

    Label* title;
    HeaderButton* menu;

protected:
    void layout() override
    {
        const double width = geometry().width;
        const double height = geometry().height;
        double right = width;
        if (menu->isVisible()) {
            menu->setGeometry({right - 32, std::floor((height - 32) / 2), 32, 32});
            right -= 32 + 12;
        }
        const SizeF size = title->sizeHint();
        title->setGeometry({20, views::layoutCenter(height, size.height),
                            std::max(0.0, right - 20), size.height});
    }
};

// Shades the top of the picture so the title and window buttons read.
class TopShade : public View
{
protected:
    void paint(Painter& painter) override
    {
        const RectF rect = localRect();
        painter.fillRoundedRect(rect, 0, LinearGradient::vertical(rect, Color{0, 0, 0, 140}, Color{0, 0, 0, 0}));
    }
};

// Over the window while files are dragged over it.
class DropOverlay : public View
{
public:
    DropOverlay()
        : m_label(views::addLabel(this, {}, 14, white(), 500))
    {
        setVisible(false);
        retranslate();
    }

    void retranslate()
    {
        m_label->setText(tr("Drop to Play"));
        requestLayout();
    }

protected:
    void layout() override
    {
        const SizeF size = m_label->sizeHint();
        m_label->setGeometry({rounding::centeredStart(geometry().width, size.width),
                              rounding::centeredStart(geometry().height, size.height), size.width, size.height});
    }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        const Corners corners = theme.continuousCorners ? Corners::Continuous : Corners::Circular;
        painter.drawRoundedRect(localRect(), {theme.hugeRadius, 0, corners},
                                theme.highlight.withAlpha(0.12), 2, theme.highlight);
    }

private:
    Label* m_label;
};

class MainWindow::Root : public View
{
public:
    explicit Root(MainWindow& owner)
        : m_owner(owner)
    {
        video = addChild<VideoView>(owner);
        shade = addChild<TopShade>();
        empty = addChild<EmptyPage>(owner);
        error = addChild<ErrorPage>(owner);
        busy = addChild<BusyIndicator>();
        flash = addChild<Flash>();
        osd = addChild<Osd>();
        side = addChild<SidePanel>(owner);
        controls = addChild<ControlBar>(owner);
        dropOverlay = addChild<DropOverlay>();
        header = addChild<WindowHeader>(kHeaderHeight);
        header->setButtonsCentered(true);
        content = header->setContent<HeaderContent>(owner);
        setTracksHover(true);
        m_singleClick.setSingleShot(true);
        m_singleClick.onTimeout = [this] { m_owner.togglePause(true); };
        m_seekFlush.setSingleShot(true);
        m_seekFlush.onTimeout = [this] {
            const double seconds = std::round(m_pendingSeek);
            if (seconds != 0) {
                m_pendingSeek -= seconds;
                m_owner.seekBy(seconds);
            }
        };
    }

    bool acceptsPointer() const override { return true; }
    CursorShape cursorShape() const override
    {
        return m_owner.controlsVisible() ? CursorShape::Default : CursorShape::Blank;
    }

    VideoView* video;
    TopShade* shade;
    EmptyPage* empty;
    ErrorPage* error;
    BusyIndicator* busy;
    Flash* flash;
    Osd* osd;
    SidePanel* side;
    ControlBar* controls;
    DropOverlay* dropOverlay;
    WindowHeader* header;
    HeaderContent* content;

    RectF controlsRect() const
    {
        const double width = geometry().width;
        const double barWidth = std::min(width - 24, ControlBar::kWidth);
        // Centred on the device pixel grid.
        const double scale = window() ? window()->scale() : 1;
        const double x = std::round((width - barWidth) / 2 * scale) / scale;
        return {x, geometry().height - ControlBar::kHeight - 16, barWidth, ControlBar::kHeight};
    }

    RectF sideRect() const
    {
        const double width = geometry().width;
        const double height = geometry().height;
        const double panelWidth = std::min(300.0, width * 0.45);
        const double x = width - 12 - panelWidth;
        const RectF bar = controlsRect();
        // Above the controls only where the two would meet.
        const double bottom = bar.x + bar.width > x - 12 ? height - bar.y + 12 : 16;
        const double top = kHeaderHeight + 4;
        return {x, top, panelWidth, std::max(0.0, height - top - bottom)};
    }

protected:
    void layout() override
    {
        const double width = geometry().width;
        const double height = geometry().height;
        video->setGeometry(localRect());
        shade->setGeometry({0, 0, width, 96});
        empty->setGeometry({0, kHeaderHeight, width, std::max(0.0, height - kHeaderHeight)});
        error->setGeometry(localRect());
        busy->setGeometry({rounding::centeredStart(width, 40), rounding::centeredStart(height, 40), 40, 40});
        flash->setGeometry({rounding::centeredStart(width, 72), rounding::centeredStart(height, 72), 72, 72});
        const SizeF osdSize = osd->sizeHint();
        osd->setGeometry({20 - Osd::kTextInset, kHeaderHeight + 8, osdSize.width, osdSize.height});
        side->setGeometry(sideRect());
        controls->setGeometry(controlsRect());
        dropOverlay->setGeometry(localRect().adjusted(8, 8, -8, -8));
        header->setGeometry({0, 0, width, kHeaderHeight});
        m_owner.player().setViewHeight(height);
        m_owner.updateGlass();
        if (m_owner.m_launchFitPending)
            m_owner.fitToLaunch();
    }

    void paint(Painter& painter) override
    {
        // Black behind the welcome page and the error, and wherever the
        // software picture is not; a video surface shows through elsewhere.
        if (!m_owner.hasMedia() || !m_owner.m_errorMessage.empty() || !video->usesSurface())
            painter.fillRect(localRect(), Color{0, 0, 0, 255});
    }

    void hoverMove(const PointerEvent& event) override
    {
        // Surface changes resend motion at the same spot; that is not activity.
        if (event.position.x == m_lastHover.x && event.position.y == m_lastHover.y)
            return;
        m_lastHover = event.position;
        m_owner.poke();
    }

    void pointerPress(const PointerEvent& event) override
    {
        m_pressPosition = mapToWindow(event.position);
        m_pressButton = event.button;
        m_dragged = false;
        m_pressed = true;
        if (event.button == MouseButton::Left && event.clickCount == 2 && m_owner.hasMedia()) {
            m_singleClick.stop();
            m_doubleClicked = true;
            if (!m_owner.m_panelDismissed.isActive())
                m_owner.toggleFullScreen();
        } else {
            m_doubleClicked = false;
        }
    }

    void pointerMove(const PointerEvent& event) override
    {
        if (!m_pressed || m_pressButton != MouseButton::Left || m_dragged)
            return;
        const PointF position = mapToWindow(event.position);
        if (std::hypot(position.x - m_pressPosition.x, position.y - m_pressPosition.y) < kStartDragDistance)
            return;
        m_dragged = true;
        m_singleClick.stop();
        if (!m_owner.isFullScreen() || !m_owner.hasMedia())
            window()->startMove();
    }

    void pointerRelease(const PointerEvent& event) override
    {
        if (!m_pressed)
            return;
        m_pressed = false;
        const PointF position = mapToWindow(event.position);
        const bool moved = std::hypot(position.x - m_pressPosition.x, position.y - m_pressPosition.y)
                >= kStartDragDistance;
        if (m_owner.m_dismissOutside) {
            m_owner.m_dismissOutside = false;
            if (!moved)
                m_owner.togglePanel(m_owner.m_panelTab, false);
            return;
        }
        if (m_dragged || moved || !m_owner.hasMedia())
            return;
        switch (m_pressButton) {
        case MouseButton::Left:
            if (!m_doubleClicked && !m_owner.m_panelDismissed.isActive())
                m_singleClick.start(kDoubleClickInterval);
            break;
        case MouseButton::Right:
            m_owner.openContextMenu(position);
            break;
        case MouseButton::Middle:
            m_owner.toggleMute();
            break;
        case MouseButton::Back:
            m_owner.seekBy(-5);
            break;
        case MouseButton::Forward:
            m_owner.seekBy(5);
            break;
        }
    }

    bool canDrop(const DropOffer& offer) const override { return offer.hasMimeType("text/uri-list"); }

    void dragEnter(const DragEvent&) override { dropOverlay->setVisible(true); }

    uint32_t dragUpdate(const DragEvent&) override { return CopyAction; }

    void dragLeave() override { dropOverlay->setVisible(false); }

    uint32_t drop(const DragEvent& event) override
    {
        dropOverlay->setVisible(false);
        event.offer->read("text/uri-list", [this, alive = m_owner.aliveToken()](std::string data) {
            if (alive.expired())
                return;
            m_owner.app().openUrls(urls::fromUriList(data));
            m_owner.window().activate();
        });
        return CopyAction;
    }

    // Up and down change the volume; sideways, as a touchpad swipes, seeks.
    bool scroll(const ScrollEvent& event) override
    {
        if (!m_owner.hasMedia())
            return false;
        // Angle delta (positive away from the user) and pixel delta.
        const double angleX = event.value120X != 0 ? -event.value120X : -event.deltaX * 8 / 3;
        const double angleY = event.value120Y != 0 ? -event.value120Y : -event.deltaY * 8 / 3;
        const double pixelX = event.precise ? -event.deltaX : 0;
        if (std::abs(angleX) > std::abs(angleY)) {
            const double delta = pixelX != 0 ? pixelX / 8 : angleX / 24;
            m_pendingSeek -= delta;
            if (!m_seekFlush.isActive())
                m_seekFlush.start(80ms);
        } else {
            m_wheelVolume += angleY / 120 * 5;
            const int step = int(std::trunc(m_wheelVolume));
            if (step != 0) {
                m_wheelVolume -= step;
                const int volume = m_owner.player().volume();
                m_owner.setVolume(std::clamp(volume + step, 0, std::max(100, volume)));
            }
        }
        return true;
    }

private:
    MainWindow& m_owner;
    PointF m_lastHover{-1, -1};
    PointF m_pressPosition;
    MouseButton m_pressButton = MouseButton::Left;
    bool m_pressed = false;
    bool m_dragged = false;
    bool m_doubleClicked = false;
    base::Timer m_singleClick;
    base::Timer m_seekFlush;
    double m_pendingSeek = 0;
    double m_wheelVolume = 0;
};

MainWindow::MainWindow(::Application& application)
    : m_app(application)
    , m_dialogs(nullptr)
    , m_chrome(std::make_unique<views::Tween>(1, 200ms))
{
    m_window.setTitle(tr("Video Player"));
    m_window.resize({960, 600});
    m_window.setMinimumSize({480, 320});
    // A video given at launch still fits the window to itself over this.
    m_window.setRestoresSize("Main");
    m_window.setHeaderDarkMode(true);
    m_window.setResizeAroundCenter(true);
    // The picture and the black behind the pages are the root view's.
    m_window.setBackgroundColor(Color{0, 0, 0, 0});

    auto root = std::make_unique<Root>(*this);
    m_root = root.get();
    m_video = m_root->video;
    m_controlBar = m_root->controls;
    m_sidePanel = m_root->side;
    m_window.setContentView(std::move(root));
    m_dialogs = std::make_unique<Dialogs>(*this);

    m_idleTimer.setSingleShot(true);
    m_idleTimer.onTimeout = [this] {
        m_pointerActive = false;
        updateChrome();
    };
    m_resumeTimer.onTimeout = [this] { saveResume(); };
    m_bufferingDelay.setSingleShot(true);
    m_bufferingDelay.onTimeout = [this] {
        m_bufferingElapsed = true;
        updateChrome();
    };
    m_panelDismissed.setSingleShot(true);
    m_panelDismissed.onTimeout = [] {};

    m_chrome->onUpdate = [this] {
        const double opacity = m_chrome->value();
        m_root->header->setOpacity(hasMedia() ? opacity : 1);
        m_root->header->setVisible(!hasMedia() || opacity > 0);
        m_controlBar->setOpacity(opacity);
        m_controlBar->setVisible(opacity > 0 && hasMedia());
        m_root->shade->setOpacity(opacity);
        m_root->shade->setVisible(hasMedia() && opacity > 0);
        updateGlass();
    };

    m_window.onCloseRequested = [this] { close(); };
    m_window.onStatesChanged = [this] {
        const bool fullScreen = isFullScreen();
        m_root->header->setMinimizeVisible(!fullScreen);
        m_root->header->setMaximizeVisible(!fullScreen);
        m_root->header->setCloseVisible(!fullScreen);
        m_root->content->menu->setVisible(!fullScreen);
        m_root->content->requestLayout();
        stateChanged.emit();
    };
    m_window.onInteractiveResizeChanged = [this] { m_player.setResizing(m_window.isInteractivelyResized()); };
    m_window.onActiveChanged = [this] {
        if (!m_window.isActive())
            m_pointerActive = false;
        m_root->content->title->setColor(m_window.isActive() ? white() : white(0.6));
        updateChrome();
    };
    m_window.onPointerPressed = [this](PointF position, MouseButton button) {
        m_dismissOutside = false;
        if (button != MouseButton::Left || !m_panelOpen || !m_sidePanel->isVisible())
            return;
        const bool onPanel = m_sidePanel->geometry().contains(position);
        const bool onControls = m_controlBar->isVisible() && m_controlBar->geometry().contains(position);
        if (!onPanel && !onControls) {
            m_dismissOutside = true;
            m_dismissPress = position;
            m_panelDismissed.start(kDoubleClickInterval + 300ms);
        }
    };

    m_subscriptions.push_back(stateChanged.connect([this] {
        updateChrome();
        m_controlBar->refresh();
        m_sidePanel->refresh();
    }));
    m_subscriptions.push_back(m_player.changed.connect([this](Player::Property property) { onPlayerChanged(property); }));
    m_subscriptions.push_back(m_player.renderWanted.connect([this] { m_video->attach(); }));
    m_subscriptions.push_back(m_player.fileLoaded.connect([this] {
        rememberRecent();
        m_fitPending = true;
        fitToVideo();
        for (const std::string& subtitle : m_pendingSubtitles)
            m_player.addSubtitle(subtitle);
        m_pendingSubtitles.clear();
    }));
    m_subscriptions.push_back(m_player.playbackFailed.connect([this](const std::string& message) {
        m_errorMessage = message;
        updateChrome();
    }));
    m_subscriptions.push_back(m_player.screenshotSaved.connect([this](const std::string& path, bool success) {
        if (success)
            showOsd(tr("Screenshot saved"), 4000, tr("Show"), [this, path] { m_app.showInFolder(urls::fromLocalFile(path)); });
        else
            showOsd(tr("Could not save the screenshot"));
    }));
    m_subscriptions.push_back(m_player.subtitleAdded.connect([this](bool success) {
        if (!success)
            showOsd(tr("Could not open the subtitle file"));
    }));
    m_sidePanel->onBusyChanged = [this] { updateChrome(); };

    Playlist& playlist = m_app.playlist();
    m_subscriptions.push_back(playlist.currentIndexChanged.connect([this] { loadCurrent(); }));
    for (base::Signal<>* signal : {&playlist.rowsChanged, &playlist.repeatModeChanged, &playlist.navigationChanged}) {
        m_subscriptions.push_back(signal->connect([this] {
            m_sidePanel->refreshPlaylist();
            updateMpris();
            // TRANSLATORS: a file's name, then why it could not be played.
            m_root->error->update(format(tr("{name} — {error}"), {{"name", mediaName()}, {"error", m_errorMessage}}),
                                  m_app.playlist().hasNext());
        }));
    }
    m_subscriptions.push_back(playlist.rowChanged.connect([this](int) { m_sidePanel->refreshPlaylist(); }));
    m_subscriptions.push_back(m_app.openRequested.connect(
            [this](const std::vector<std::string>& media, const std::vector<std::string>& subtitles) {
                if (!media.empty()) {
                    m_pendingSubtitles = subtitles;
                    saveResume();
                    m_app.playlist().open(media, media.size() == 1 && m_app.settings().folderPlaylist());
                } else if (hasMedia()) {
                    for (const std::string& subtitle : subtitles)
                        m_player.addSubtitle(subtitle);
                }
            }));
    m_subscriptions.push_back(
            m_app.activationRequested.connect([this](const std::string& token) { m_window.activate(token); }));
    PlayerSettings& settings = m_app.settings();
    m_subscriptions.push_back(settings.subtitleScaleChanged.connect(
            [this] { m_player.setSubtitleScale(m_app.settings().subtitleScale() / 100.0); }));
    m_subscriptions.push_back(settings.audioLanguageChanged.connect([this] { applyLanguages(); }));
    m_subscriptions.push_back(settings.subtitleLanguageChanged.connect([this] { applyLanguages(); }));
    m_subscriptions.push_back(settings.recentFilesChanged.connect([this] { m_root->empty->reload(); }));
    Thumbnailer& thumbnailer = m_app.thumbnailer();
    m_subscriptions.push_back(thumbnailer.revisionChanged.connect([this] { m_controlBar->refresh(); }));
    m_subscriptions.push_back(thumbnailer.coverChanged.connect([this] { updateMpris(); }));
    m_languageChanged = Lingmo::ui::Application::instance()->languageChanged.connect([this] { retranslate(); });

    m_player.setVolume(settings.volume());
    m_player.setMuted(settings.muted());
    m_player.setSubtitleScale(settings.subtitleScale() / 100.0);
    applyLanguages();

    buildMenus();
    bindShortcuts();
    bindMpris();
    m_launchFitPending = !m_app.launchVideoSize().isEmpty();
    updateTitle();
    updateChrome();
    m_controlBar->refresh();
    m_sidePanel->refresh();
}

MainWindow::~MainWindow()
{
    m_chrome->onUpdate = nullptr;
    m_chrome.reset();
    m_closingVideo = m_video->takeSurface();
}

// Only the text around the picture is made again: the player, its file and
// the video surface go on as they were, as do the window's state, the side
// panel's lists and the dialogs' input.
void MainWindow::retranslate()
{
    for (Menu* menu : {&m_mainMenu, &m_contextMenu, &m_playlistMenu}) {
        if (menu->isOpen())
            menu->close();
    }
    m_player.relabelTracks();
    m_root->empty->retranslate();
    m_root->error->retranslate();
    m_root->dropOverlay->retranslate();
    m_controlBar->retranslate();
    m_sidePanel->retranslate();
    m_controlBar->refresh();
    updateTitle();
    updateChrome();
    if (m_mpris)
        m_mpris->setIdentity(tr("Video Player"));
    m_app.inhibitor().retranslate();
    m_dialogs->retranslate();
}

void MainWindow::show()
{
    fitToLaunch();
    m_window.show();
}

std::string MainWindow::mediaName() const
{
    // The file's own title when it has one, else its name without the extension.
    const std::string fileName = urls::fileName(m_player.source());
    const std::string& title = m_player.mediaTitle();
    return !title.empty() && title != fileName ? title : ::Application::displayName(m_player.source());
}

bool MainWindow::menuOpen() const
{
    return m_mainMenu.isOpen() || m_contextMenu.isOpen() || m_playlistMenu.isOpen();
}

bool MainWindow::controlsVisible() const
{
    return !hasMedia() || !m_errorMessage.empty() || m_player.paused() || m_player.endReached() || m_pointerActive
            || menuOpen() || m_controlBar->isBusy() || m_controlBar->isHovered() || m_root->header->isHovered()
            || (m_panelOpen && m_sidePanel->isBusy());
}

double MainWindow::chromeOpacity() const
{
    return m_chrome->value();
}

void MainWindow::poke()
{
    m_pointerActive = true;
    m_idleTimer.start(2000ms);
    updateChrome();
}

void MainWindow::updateChrome()
{
    const bool visible = controlsVisible();
    m_chrome->setTarget(m_root, hasMedia() && !visible ? 0 : 1);
    if (m_chrome->onUpdate)
        m_chrome->onUpdate();
    m_root->setCursorShape(visible ? CursorShape::Default : CursorShape::Blank);
    m_root->video->setCursorShape(visible ? CursorShape::Default : CursorShape::Blank);

    const bool media = hasMedia();
    m_root->empty->setVisible(!media);
    m_root->error->setVisible(media && !m_errorMessage.empty());
    if (media && !m_errorMessage.empty())
        m_root->error->update(format(tr("{name} — {error}"), {{"name", ::Application::displayName(m_player.source())},
                                                              {"error", m_errorMessage}}),
                              m_app.playlist().hasNext());
    m_root->video->setVisible(media && m_errorMessage.empty());
    m_sidePanel->setVisible(m_panelOpen && media);
    const bool buffering = m_player.buffering() && !m_player.paused();
    if (buffering && !m_bufferingDelay.isActive() && !m_bufferingElapsed)
        m_bufferingDelay.start(1000ms);
    if (!buffering) {
        m_bufferingDelay.stop();
        m_bufferingElapsed = false;
    }
    m_root->busy->setVisible(media && m_bufferingElapsed);
    m_root->busy->setRunning(media && m_bufferingElapsed);

    // Subtitles clear the bottom controls while those show.
    const double inset = media && visible ? m_root->geometry().height - m_root->controlsRect().y + 8 : 0;
    if (inset != m_appliedInset) {
        m_appliedInset = inset;
        m_player.setSubtitleBottomInset(inset);
    }
    m_root->requestPaint();
    updateGlass();
}

void MainWindow::updateGlass()
{
    if (!views::compositorGlass() || !m_root)
        return;
    const double opacity = m_chrome->value();
    const bool controls = hasMedia() && m_controlBar->isVisible() && opacity > 0;
    const bool side = m_sidePanel->isVisible();
    if (!controls && !side) {
        m_window.setBackgroundGlass(std::nullopt);
        m_window.setBackgroundBlurShape(Path());
        m_window.setBackgroundBlur({}, 0);
        return;
    }
    const bool continuous = Theme::current().continuousCorners;
    Path shape;
    RectF bounds;
    bool first = true;
    const auto add = [&](const RectF& rect) {
        addPlate(shape, rect, 20, continuous);
        if (first) {
            bounds = rect;
            first = false;
            return;
        }
        const double left = std::min(bounds.x, rect.x);
        const double top = std::min(bounds.y, rect.y);
        const double right = std::max(bounds.x + bounds.width, rect.x + rect.width);
        const double bottom = std::max(bounds.y + bounds.height, rect.y + rect.height);
        bounds = {left, top, right - left, bottom - top};
    };
    if (controls)
        add(m_controlBar->geometry());
    if (side)
        add(m_sidePanel->geometry());
    // Frosted video: a refraction-free glass, blurred, desaturated and
    // tinted; it fades with the controls.
    const double fade = side ? 1 : opacity;
    GlassMaterial glass;
    glass.blurSigma = 26 * fade;
    glass.bevel = 0;
    glass.displacement = 0;
    glass.chroma = 0;
    glass.specularIntensity = 0;
    glass.sheen = 0;
    glass.ambientOcclusion = 0;
    glass.saturation = 1 - 0.35 * fade;
    glass.tintRed = 0.07;
    glass.tintGreen = 0.07;
    glass.tintBlue = 0.08;
    glass.tintAlpha = 0.62 * fade;
    glass.rim = 0;
    m_window.setBackgroundBlur(bounds, 20);
    m_window.setBackgroundBlurShape(shape);
    m_window.setBackgroundGlass(glass);
}

void MainWindow::showOsd(const std::string& message, int timeoutMs, std::string actionText,
                         std::function<void()> action)
{
    m_root->osd->show(message, timeoutMs, std::move(actionText), std::move(action));
}

void MainWindow::togglePanel(int tab, bool focusList)
{
    if (tab < 0 || (m_panelOpen && m_panelTab != tab)) {
        if (tab >= 0)
            m_panelTab = tab;
        m_panelOpen = tab >= 0 ? true : !m_panelOpen;
    } else {
        m_panelOpen = !m_panelOpen;
        m_panelTab = tab;
    }
    m_sidePanel->setVisible(m_panelOpen && hasMedia());
    m_sidePanel->refresh();
    if (m_panelOpen && focusList && m_panelTab == 0)
        m_sidePanel->focusList();
    else if (!m_panelOpen && m_sidePanel->hasFocus())
        m_root->setFocus();
    stateChanged.emit();
}

void MainWindow::setPanelTab(int tab)
{
    m_panelTab = tab;
    stateChanged.emit();
}

void MainWindow::openFiles(bool append)
{
    m_fileChooser = std::make_unique<FileChooser>(&m_window);
    m_fileChooser->setMode(FileChooser::Mode::OpenFiles);
    m_fileChooser->setTitle(append ? tr("Add to Playlist") : tr("Open Video"));
    const std::string last = urls::toLocalFile(m_app.settings().lastFolder());
    m_fileChooser->setCurrentFolder(!last.empty() ? last : base::paths::userDirectory(base::paths::UserDirectory::Videos));
    m_fileChooser->setFilters({FileChooser::Filter::fromNameFilter(format(tr("Videos and audio ({patterns})"),
                                                                          {{"patterns", kMediaPatterns}})),
                               FileChooser::Filter::fromNameFilter(tr("All files (*)"))});
    m_fileChooser->onAccepted = [this, append](const std::vector<std::string>& paths, const std::string&) {
        if (paths.empty())
            return;
        m_app.settings().setLastFolder(urls::fromLocalFile(std::filesystem::path(paths.front()).parent_path().string()));
        std::vector<std::string> urls;
        for (const std::string& path : paths)
            urls.push_back(urls::fromLocalFile(path));
        if (append)
            m_app.playlist().append(urls);
        else
            m_app.openUrls(urls);
    };
    m_fileChooser->open();
}

void MainWindow::openSubtitleFile()
{
    m_fileChooser = std::make_unique<FileChooser>(&m_window);
    m_fileChooser->setMode(FileChooser::Mode::OpenFile);
    m_fileChooser->setTitle(tr("Add Subtitle File"));
    if (urls::isLocalFile(m_player.source()))
        m_fileChooser->setCurrentFolder(std::filesystem::path(urls::toLocalFile(m_player.source())).parent_path().string());
    m_fileChooser->setFilters({FileChooser::Filter::fromNameFilter(tr("Subtitles (*.srt *.ass *.ssa *.vtt *.sub *.idx *.sup *.smi)")),
                               FileChooser::Filter::fromNameFilter(tr("All files (*)"))});
    m_fileChooser->onAccepted = [this](const std::vector<std::string>& paths, const std::string&) {
        if (!paths.empty())
            m_player.addSubtitle(urls::fromLocalFile(paths.front()));
    };
    m_fileChooser->open();
}

void MainWindow::openUrlDialog()
{
    m_dialogs->openUrl();
}

void MainWindow::openPreferences()
{
    m_dialogs->preferences();
}

void MainWindow::openShortcuts()
{
    m_dialogs->shortcuts();
}

void MainWindow::openAbout()
{
    m_dialogs->about();
}

void MainWindow::showMediaInfo()
{
    if (m_player.loaded())
        m_dialogs->mediaInfo(m_player.mediaInfo(), mediaName());
}

void MainWindow::buildMenus()
{
    m_mainMenu.onAboutToShow = [this] {
        m_mainMenu.clear();
        m_mainMenu.addItem(tr("Open File…"), [this] { openFiles(false); }, "Ctrl+O");
        m_mainMenu.addItem(tr("Open URL…"), [this] { openUrlDialog(); }, "Ctrl+L");
        const std::vector<PlayerSettings::RecentFile> recent = m_app.settings().recentFiles();
        Menu* recentMenu = m_mainMenu.addSubmenu(tr("Recent"));
        for (const auto& file : recent) {
            const std::string url = file.url;
            recentMenu->addItem(file.name, [this, url] { m_app.openUrls({url}); });
        }
        recentMenu->addSeparator();
        recentMenu->addItem(tr("Clear Recent"), [this] { m_app.settings().clearRecentFiles(); });
        m_mainMenu.setItemEnabled(2, !recent.empty());
        m_mainMenu.addSeparator();
        const size_t show = m_mainMenu.addItem(tr("Show in Files"), [this] { m_app.showInFolder(m_player.source()); });
        m_mainMenu.setItemEnabled(show, hasMedia() && urls::isLocalFile(m_player.source()));
        const size_t screenshot = m_mainMenu.addItem(tr("Take Screenshot"), [this] { takeScreenshot(); }, "Ctrl+Alt+S");
        m_mainMenu.setItemEnabled(screenshot, m_player.loaded() && m_player.hasVideo());
        const size_t info = m_mainMenu.addItem(tr("Media Info"), [this] { showMediaInfo(); }, "Ctrl+I");
        m_mainMenu.setItemEnabled(info, m_player.loaded());
        m_mainMenu.addSeparator();
        m_mainMenu.addItem(tr("Settings"), [this] { openPreferences(); }, "Ctrl+,");
        m_mainMenu.addItem(tr("Keyboard Shortcuts"), [this] { openShortcuts(); }, "Ctrl+?");
        m_mainMenu.addItem(tr("About Video Player"), [this] { openAbout(); });
        m_root->content->menu->setChecked(true);
        updateChrome();
    };
    m_mainMenu.onClosed = [this] {
        m_root->content->menu->setChecked(false);
        updateChrome();
    };

    m_contextMenu.onAboutToShow = [this] {
        Menu& menu = m_contextMenu;
        menu.clear();
        menu.addItem(m_player.paused() || m_player.endReached() ? tr("Play") : tr("Pause"), [this] { togglePause(false); },
                     "Space");
        const bool playlist = m_app.playlist().count() > 1;
        const size_t previous = menu.addItem(tr("Previous"), [this] { playPrevious(); }, "B");
        menu.setItemVisible(previous, playlist);
        menu.setItemEnabled(previous, m_app.playlist().hasPrevious() || m_player.position() > 5);
        const size_t next = menu.addItem(tr("Next"), [this] { playNext(false); }, "N");
        menu.setItemVisible(next, playlist);
        menu.setItemEnabled(next, m_app.playlist().hasNext());
        menu.addSeparator();

        Menu* subtitles = menu.addSubmenu(tr("Subtitles"));
        // TRANSLATORS: no subtitles shown.
        const size_t off = subtitles->addItem(tr("Off"), [this] { selectSubtitle(0); });
        subtitles->setItemChecked(off, m_player.subtitleTrack() == 0);
        for (const auto& track : m_player.subtitleTracks()) {
            const int id = track.id;
            const size_t item = subtitles->addItem(track.label, [this, id] { selectSubtitle(id); });
            subtitles->setItemChecked(item, m_player.subtitleTrack() == id);
        }
        subtitles->addSeparator();
        subtitles->addItem(tr("Add Subtitle File…"), [this] { openSubtitleFile(); });
        Menu* timing = subtitles->addSubmenu(tr("Subtitle Timing"));
        timing->addItem(tr("Show Earlier"), [this] { shiftSubtitles(-0.1); }, "Z");
        timing->addItem(tr("Show Later"), [this] { shiftSubtitles(0.1); }, "Shift+Z");
        const size_t reset = timing->addItem(format(tr("Reset ({delay})"), {{"delay", delayText(m_player.subtitleDelay())}}),
                                             [this] { shiftSubtitles(-m_player.subtitleDelay()); });
        timing->setItemEnabled(reset, m_player.subtitleDelay() != 0);
        // Off, the tracks, the separator, Add Subtitle File, then the timing.
        subtitles->setItemEnabled(m_player.subtitleTracks().size() + 3, m_player.subtitleTrack() > 0);

        Menu* audio = menu.addSubmenu(tr("Audio"));
        if (m_player.audioTracks().empty()) {
            const size_t none = audio->addItem(tr("No Audio Tracks"), nullptr);
            audio->setItemEnabled(none, false);
        }
        for (const auto& track : m_player.audioTracks()) {
            const int id = track.id;
            const size_t item = audio->addItem(track.label, [this, id] { selectAudio(id); });
            audio->setItemChecked(item, m_player.audioTrack() == id);
        }

        Menu* speed = menu.addSubmenu(tr("Speed"));
        for (const double value : kSpeeds) {
            // TRANSLATORS: the normal playback speed.
            const size_t item = speed->addItem(value == 1 ? tr("Normal") : speedText(value), [this, value] { setSpeed(value); });
            speed->setItemChecked(item, std::abs(m_player.speed() - value) < 0.001);
        }

        if (!m_player.chapters().empty()) {
            Menu* chapters = menu.addSubmenu(tr("Chapters"));
            int index = 0;
            for (const auto& chapter : m_player.chapters()) {
                const std::string title = chapter.title.empty()
                        ? format(tr("Chapter {number}"), {{"number", std::to_string(index + 1)}})
                        : chapter.title;
                const size_t item = chapters->addItem(title + "    " + ::Application::formatTime(chapter.time, m_player.duration()),
                                                      [this, index] { m_player.setChapter(index); });
                chapters->setItemChecked(item, m_player.chapter() == index);
                ++index;
            }
        }
        const size_t repeat = menu.addItem(tr("Repeat"), [this] { m_player.setLoopFile(!m_player.loopFile()); });
        menu.setItemChecked(repeat, m_player.loopFile());
        menu.addSeparator();
        menu.addItem(isFullScreen() ? tr("Exit Full Screen") : tr("Full Screen"), [this] { toggleFullScreen(); }, "F");
        const size_t playlistItem = menu.addItem(tr("Playlist"), [this] { togglePanel(0, false); }, "P");
        menu.setItemChecked(playlistItem, m_panelOpen && m_panelTab == 0);
        menu.addSeparator();
        const size_t screenshot = menu.addItem(tr("Take Screenshot"), [this] { takeScreenshot(); }, "Ctrl+Alt+S");
        menu.setItemEnabled(screenshot, m_player.loaded() && m_player.hasVideo());
        const size_t info = menu.addItem(tr("Media Info"), [this] { showMediaInfo(); }, "Ctrl+I");
        menu.setItemEnabled(info, m_player.loaded());
        const size_t show = menu.addItem(tr("Show in Files"), [this] { m_app.showInFolder(m_player.source()); });
        menu.setItemEnabled(show, urls::isLocalFile(m_player.source()));
        updateChrome();
    };
    m_contextMenu.onClosed = [this] { updateChrome(); };

    m_playlistMenu.onAboutToShow = [this] {
        m_playlistMenu.clear();
        m_playlistMenu.addItem(tr("Play"), [this] { playIndex(m_playlistMenuIndex); });
        m_playlistMenu.addItem(tr("Show in Files"), [this] {
            if (m_playlistMenuIndex >= 0 && m_playlistMenuIndex < m_app.playlist().count())
                m_app.showInFolder(m_app.playlist().url(m_playlistMenuIndex));
        });
        m_playlistMenu.addSeparator();
        m_playlistMenu.addItem(tr("Remove from Playlist"), [this] { m_app.playlist().remove(m_playlistMenuIndex); });
        updateChrome();
    };
    m_playlistMenu.onClosed = [this] { updateChrome(); };
}

void MainWindow::openMainMenu(View* button)
{
    m_mainMenu.popupFor(button, 4);
}

void MainWindow::openContextMenu(PointF windowPosition)
{
    m_contextMenu.popup(&m_window, windowPosition);
}

void MainWindow::openPlaylistMenu(int index, PointF windowPosition)
{
    m_playlistMenuIndex = index;
    m_playlistMenu.popup(&m_window, windowPosition);
}

void MainWindow::bindShortcuts()
{
    const auto bind = [this](uint32_t modifiers, uint32_t key, std::function<void()> action) {
        m_window.addShortcut(modifiers, key, action);
    };
    const auto letter = [](char c) { return key::letter(c); };
    // While the playlist has the keyboard, its arrows and keys go to it.
    const auto listWanted = [this] { return m_panelOpen && m_sidePanel->isBusy(); };

    for (const uint32_t key : {uint32_t(key::Space), letter('k'), uint32_t(0x1008ff14)}) {
        bind(NoModifier, key, [this, listWanted] {
            if (hasMedia() && !listWanted())
                togglePause(true);
        });
    }
    bind(NoModifier, 0x1008ff31, [this] { m_player.setPaused(true); });
    bind(NoModifier, 0x1008ff15, [this] { m_player.setPaused(true); });
    for (const auto& [modifiers, key] : std::vector<std::pair<uint32_t, uint32_t>>{
             {NoModifier, letter('n')}, {NoModifier, 0x1008ff17}, {ShiftModifier, '>'}, {NoModifier, '>'}})
        bind(modifiers, key, [this] { playNext(false); });
    for (const auto& [modifiers, key] : std::vector<std::pair<uint32_t, uint32_t>>{
             {NoModifier, letter('b')}, {NoModifier, 0x1008ff16}, {ShiftModifier, '<'}, {NoModifier, '<'}})
        bind(modifiers, key, [this] { playPrevious(); });
    bind(NoModifier, key::Left, [this] { seekBy(-5); });
    bind(NoModifier, key::Right, [this] { seekBy(5); });
    bind(ShiftModifier, key::Left, [this] { seekBy(-30); });
    bind(ShiftModifier, key::Right, [this] { seekBy(30); });
    bind(NoModifier, key::Home, [this] { seekTo(0); });
    bind(NoModifier, key::End, [this] { seekTo(std::max(0.0, m_player.duration() - 1)); });
    bind(NoModifier, key::Up, [this, listWanted] {
        if (!listWanted())
            setVolume(std::min(std::max(100, m_player.volume()), m_player.volume() + 5));
    });
    bind(NoModifier, key::Down, [this, listWanted] {
        if (!listWanted())
            setVolume(m_player.volume() - 5);
    });
    bind(NoModifier, letter('m'), [this] { toggleMute(); });
    for (const uint32_t key : {letter('f'), uint32_t(0xffc8)}) {
        bind(NoModifier, key, [this] {
            if (hasMedia())
                toggleFullScreen();
        });
    }
    bind(NoModifier, key::Escape, [this] {
        if (isFullScreen() && !m_sidePanel->isBusy())
            toggleFullScreen();
    });
    bind(NoModifier, '[', [this] { stepSpeed(-1); });
    bind(NoModifier, ']', [this] { stepSpeed(1); });
    bind(NoModifier, key::BackSpace, [this] { setSpeed(1); });
    bind(NoModifier, ',', [this] { m_player.frameStep(true); });
    bind(NoModifier, '.', [this] { m_player.frameStep(false); });
    bind(NoModifier, key::PageUp, [this] { stepChapter(-1); });
    bind(NoModifier, key::PageDown, [this] { stepChapter(1); });
    bind(NoModifier, letter('j'), [this] { cycleSubtitle(1); });
    bind(ShiftModifier, letter('j'), [this] { cycleSubtitle(-1); });
    bind(NoModifier, letter('v'), [this] { toggleSubtitles(); });
    bind(NoModifier, letter('a'), [this] { cycleAudio(); });
    bind(NoModifier, letter('z'), [this] { shiftSubtitles(-0.1); });
    bind(ShiftModifier, letter('z'), [this] { shiftSubtitles(0.1); });
    bind(ControlModifier | AltModifier, letter('s'), [this] { takeScreenshot(); });
    bind(NoModifier, letter('p'), [this] {
        if (hasMedia())
            togglePanel(0, true);
    });
    bind(ControlModifier, letter('o'), [this] { openFiles(false); });
    bind(ControlModifier, letter('l'), [this] { openUrlDialog(); });
    bind(ControlModifier, letter('i'), [this] { showMediaInfo(); });
    bind(ControlModifier | ShiftModifier, '?', [this] { openShortcuts(); });
    bind(ControlModifier, '?', [this] { openShortcuts(); });
    bind(ControlModifier | ShiftModifier, '/', [this] { openShortcuts(); });
    bind(ControlModifier, letter('w'), [this] { close(); });
    bind(ControlModifier, letter('q'), [this] { close(); });
    bind(ControlModifier, ',', [this] { openPreferences(); });
}

void MainWindow::bindMpris()
{
    MprisServer::Description description;
    description.identity = tr("Video Player");
    description.desktopEntry = "lingmo-video-player";
    description.supportedUriSchemes = {"file", "http", "https"};
    description.canQuit = true;
    description.canRaise = true;
    description.canSetFullscreen = true;
    m_mpris = std::make_unique<MprisServer>(base::dbus::Connection::session(), "lingmo-video-player", description);
    m_mpris->setCanControl(true);
    m_mpris->setMinimumRate(0.5);
    m_mpris->setMaximumRate(2);
    m_subscriptions.push_back(m_mpris->playRequested.connect([this] {
        if (m_player.paused())
            togglePause(false);
    }));
    m_subscriptions.push_back(m_mpris->pauseRequested.connect([this] { m_player.setPaused(true); }));
    m_subscriptions.push_back(m_mpris->stopRequested.connect([this] { m_player.setPaused(true); }));
    m_subscriptions.push_back(m_mpris->nextRequested.connect([this] { playNext(false); }));
    m_subscriptions.push_back(m_mpris->previousRequested.connect([this] { playPrevious(); }));
    m_subscriptions.push_back(m_mpris->seekRequested.connect([this](int64_t offset) {
        m_player.seekRelative(double(offset) / 1e6);
        m_mpris->seeked(int64_t(std::lround(std::max(0.0, m_player.position() + double(offset) / 1e6) * 1e6)));
    }));
    m_subscriptions.push_back(m_mpris->setPositionRequested.connect([this](const std::string&, int64_t position) {
        m_player.seek(double(position) / 1e6, true);
        m_mpris->seeked(position);
    }));
    m_subscriptions.push_back(m_mpris->volumeRequested.connect(
            [this](double volume) { setVolume(int(std::lround(volume * 100))); }));
    m_subscriptions.push_back(m_mpris->rateRequested.connect([this](double rate) { setSpeed(rate); }));
    m_subscriptions.push_back(m_mpris->fullscreenRequested.connect([this](bool fullScreen) {
        if (fullScreen != isFullScreen())
            toggleFullScreen();
    }));
    m_subscriptions.push_back(m_mpris->raiseRequested.connect([this] { m_app.activateWindow(); }));
    m_subscriptions.push_back(m_mpris->quitRequested.connect([this] { close(); }));
    updateMpris();
}

void MainWindow::updateMpris()
{
    if (!m_mpris)
        return;
    const Playlist& playlist = m_app.playlist();
    m_mpris->setFullscreen(isFullScreen());
    m_mpris->setCanPlay(hasMedia());
    m_mpris->setCanPause(hasMedia());
    m_mpris->setCanSeek(m_player.seekable());
    m_mpris->setCanGoNext(playlist.hasNext());
    m_mpris->setCanGoPrevious(playlist.hasPrevious());
    m_mpris->setPlaybackStatus(!m_player.loaded() ? MprisServer::PlaybackStatus::Stopped
                               : m_player.paused() ? MprisServer::PlaybackStatus::Paused
                                                   : MprisServer::PlaybackStatus::Playing);
    m_mpris->setLoopStatus(m_player.loopFile() ? MprisServer::LoopStatus::Track
                           : playlist.repeatMode() == Playlist::RepeatMode::All ? MprisServer::LoopStatus::Playlist
                           : playlist.repeatMode() == Playlist::RepeatMode::One ? MprisServer::LoopStatus::Track
                                                                                : MprisServer::LoopStatus::None);
    m_mpris->setRate(m_player.speed());
    m_mpris->setVolume(m_player.muted() ? 0 : m_player.volume() / 100.0);
    m_mpris->setPosition(int64_t(std::lround(m_player.position() * 1e6)));
    MprisServer::Metadata metadata;
    metadata.trackId = "/org/lingmo/VideoPlayer/Track/" + std::to_string(std::max(0, playlist.currentIndex()));
    metadata.title = hasMedia() ? mediaName() : std::string();
    metadata.length = int64_t(std::lround(m_player.duration() * 1e6));
    metadata.url = m_player.source();
    metadata.artUrl = m_app.thumbnailer().cover();
    m_mpris->setMetadata(metadata);
}

void MainWindow::updateInhibitor()
{
    m_app.inhibitor().setMode(m_player.loaded() && !m_player.paused()
                                      ? (m_player.hasVideo() ? SleepInhibitor::Mode::Screen
                                                             : SleepInhibitor::Mode::Suspend)
                                      : SleepInhibitor::Mode::Off);
}

void MainWindow::updateTitle()
{
    const std::string title = hasMedia() ? mediaName() : tr("Video Player");
    m_window.setTitle(title);
    if (m_root->content->title->text() != title) {
        m_root->content->title->setText(title);
        m_root->content->requestLayout();
    }
}

void MainWindow::onPlayerChanged(Player::Property property)
{
    using P = Player::Property;
    switch (property) {
    case P::Position:
        m_controlBar->refresh();
        if (m_mpris)
            m_mpris->setPosition(int64_t(std::lround(m_player.position() * 1e6)));
        return;
    case P::Duration:
        // Streams are not probed; the playlist learns their length from playback.
        if (m_player.loaded() && m_player.duration() > 0)
            m_app.playlist().setDuration(m_player.source(), m_player.duration());
        m_app.thumbnailer().setDuration(m_player.duration());
        break;
    case P::VideoSize:
        fitToVideo();
        break;
    case P::Tracks:
        rememberRecent();
        break;
    case P::EndReached:
        if (m_player.endReached())
            fileEnded();
        break;
    case P::SubtitleTrack:
        if (m_player.subtitleTrack() > 0)
            m_lastSubtitle = m_player.subtitleTrack();
        break;
    case P::Loaded:
        m_app.thumbnailer().setSource(m_player.loaded() ? m_player.source() : std::string());
        break;
    default:
        break;
    }
    const bool loadedOrPaused = property == P::Loaded || property == P::Paused;
    if (loadedOrPaused || property == P::Tracks) {
        updateInhibitor();
        m_resumeTimer.stop();
        if (m_player.loaded() && !m_player.paused())
            m_resumeTimer.start(30000ms);
    }
    if (property == P::Source || property == P::MediaTitle)
        updateTitle();
    updateMpris();
    stateChanged.emit();
}

void MainWindow::applyLanguages()
{
    const PlayerSettings& settings = m_app.settings();
    m_player.setPreferredLanguages(settings.languagePreference(settings.audioLanguage()),
                                   settings.languagePreference(settings.subtitleLanguage()));
}

void MainWindow::loadCurrent()
{
    const std::string url = m_app.playlist().currentUrl();
    if (url == m_player.source() && m_player.loaded() && !m_player.endReached()) {
        // The same file again: from the start, as Repeat One does.
        m_player.seek(0, true);
        m_player.setPaused(false);
        return;
    }
    saveResume();
    m_errorMessage.clear();
    if (url.empty()) {
        m_player.stop();
        stateChanged.emit();
        return;
    }

    Player::FileOptions options;
    double resumed = 0;
    if (m_app.settings().resumePlayback()) {
        if (const auto entry = m_app.resume().entry(url); entry && entry->position > 0) {
            resumed = entry->position;
            options.emplace_back("start", entry->position);
            if (entry->audioTrack >= 0)
                options.emplace_back("aid", entry->audioTrack > 0 ? MpvNode(entry->audioTrack) : MpvNode("no"));
            if (entry->subtitleTrack >= 0)
                options.emplace_back("sid", entry->subtitleTrack > 0 ? MpvNode(entry->subtitleTrack) : MpvNode("no"));
            if (entry->subtitleDelay != 0)
                options.emplace_back("sub-delay", entry->subtitleDelay);
        }
    }
    m_player.load(url, options, false);
    m_recentPending = true;
    if (resumed > 0)
        showOsd(format(tr("Resumed from {time}"), {{"time", ::Application::formatTime(resumed)}}), 5000);
    poke();
}

void MainWindow::rememberRecent()
{
    if (!m_recentPending || !m_player.loaded() || (!m_player.hasVideo() && m_player.audioTracks().empty()))
        return;
    m_recentPending = false;
    m_app.settings().addRecentFile(m_player.source());
}

void MainWindow::saveResume()
{
    if (!hasMedia() || !m_player.loaded() || m_player.duration() <= 0)
        return;
    if (!m_app.settings().resumePlayback()) {
        m_app.resume().forget(m_player.source());
        return;
    }
    ResumeStore::Entry choices;
    choices.audioTrack = m_player.audioTrack();
    choices.subtitleTrack = m_player.subtitleTrack();
    choices.subtitleDelay = m_player.subtitleDelay();
    m_app.resume().save(m_player.source(), m_player.endReached() ? m_player.duration() : m_player.position(),
                        m_player.duration(), choices);
}

void MainWindow::fileEnded()
{
    if (m_player.loopFile())
        return;
    saveResume();
    if (!m_app.playlist().next(true))
        poke();
}

void MainWindow::playIndex(int index)
{
    if (index == m_app.playlist().currentIndex()) {
        m_player.seek(0, true);
        m_player.setPaused(false);
    } else {
        m_app.playlist().setCurrentIndex(index);
    }
}

void MainWindow::playNext(bool automatic)
{
    if (!m_app.playlist().next(automatic))
        showOsd(tr("End of playlist"));
}

void MainWindow::playPrevious()
{
    // Like a CD player: back to the start first, unless it just began.
    if (m_player.position() > 5 || !m_app.playlist().hasPrevious()) {
        seekTo(0);
        return;
    }
    m_app.playlist().previous();
}

void MainWindow::togglePause(bool showFlash)
{
    if (!hasMedia()) {
        openFiles(false);
        return;
    }
    const bool play = m_player.paused() || m_player.endReached();
    m_player.setPaused(!play);
    if (showFlash)
        m_root->flash->show(play ? "play" : "pause");
    poke();
}

void MainWindow::seekTo(double seconds)
{
    if (!m_player.seekable())
        return;
    m_player.seek(seconds, true);
    showOsd(::Application::formatTime(seconds, m_player.duration()) + " / "
            + ::Application::formatTime(m_player.duration(), m_player.duration()));
}

void MainWindow::seekBy(double seconds)
{
    if (!m_player.seekable())
        return;
    const double target = std::clamp(m_player.position() + seconds, 0.0, m_player.duration());
    m_player.seekRelative(seconds);
    // TRANSLATORS: a jump such as "+10 s", then the time it lands on.
    showOsd(format(tr("{sign}{seconds} s   {time}"), {{"sign", seconds > 0 ? "+" : "−"},
                                                      {"seconds", std::format("{}", std::abs(seconds))},
                                                      {"time", ::Application::formatTime(target, m_player.duration())}}));
}

void MainWindow::setVolume(int value)
{
    value = std::clamp(value, 0, 150);
    m_player.setVolume(value);
    m_app.settings().setVolume(value);
    if (m_player.muted() && value > 0)
        toggleMute();
    showOsd(format(tr("Volume {percent}%"), {{"percent", std::to_string(value)}}));
}

void MainWindow::toggleMute()
{
    // player.muted follows mpv a moment later.
    const bool muted = !m_player.muted();
    m_player.setMuted(muted);
    m_app.settings().setMuted(muted);
    showOsd(muted ? std::string(tr("Muted"))
                  : format(tr("Volume {percent}%"), {{"percent", std::to_string(m_player.volume())}}));
}

std::string MainWindow::speedText(double speed)
{
    const bool tenths = std::fmod(std::round(speed * 100), 10) == 0;
    return base::Locale::system().toString(speed, tenths ? 1 : 2) + "×";
}

void MainWindow::setSpeed(double speed)
{
    m_player.setSpeed(speed);
    showOsd(format(tr("Speed {speed}"), {{"speed", speedText(speed)}}));
}

void MainWindow::stepSpeed(int direction)
{
    const double current = m_player.speed();
    size_t index = 0;
    for (size_t i = 0; i < std::size(kSpeeds); ++i) {
        if (std::abs(kSpeeds[i] - current) < 0.001 || kSpeeds[i] < current)
            index = i;
    }
    const bool exact = std::abs(kSpeeds[index] - current) < 0.001;
    if (direction > 0)
        index = std::min(std::size(kSpeeds) - 1, index + 1);
    else if (exact && index > 0)
        --index;
    setSpeed(kSpeeds[index]);
}

void MainWindow::stepChapter(int direction)
{
    const auto& chapters = m_player.chapters();
    if (chapters.empty())
        return;
    m_player.setChapter(std::max(0, m_player.chapter() + direction));
    const int index = std::clamp(m_player.chapter() + direction, 0, int(chapters.size()) - 1);
    const auto& chapter = chapters[size_t(index)];
    showOsd(!chapter.title.empty() ? chapter.title : ::Application::formatTime(chapter.time, m_player.duration()));
}

void MainWindow::selectSubtitle(int id)
{
    m_player.setSubtitleTrack(id);
    std::string label = tr("Subtitles off");
    for (const auto& track : m_player.subtitleTracks()) {
        if (track.id == id)
            label = format(tr("Subtitles: {track}"), {{"track", track.label}});
    }
    showOsd(label);
}

void MainWindow::selectAudio(int id)
{
    m_player.setAudioTrack(id);
    for (const auto& track : m_player.audioTracks()) {
        if (track.id == id)
            showOsd(format(tr("Audio: {track}"), {{"track", track.label}}));
    }
}

void MainWindow::cycleSubtitle(int direction)
{
    std::vector<int> ids = {0};
    for (const auto& track : m_player.subtitleTracks())
        ids.push_back(track.id);
    const auto current = std::find(ids.begin(), ids.end(), m_player.subtitleTrack());
    const int position = current == ids.end() ? -1 : int(current - ids.begin());
    const int count = int(ids.size());
    selectSubtitle(ids[size_t(((position + direction) % count + count) % count)]);
}

void MainWindow::toggleSubtitles()
{
    if (m_player.subtitleTrack() > 0)
        selectSubtitle(0);
    else if (m_lastSubtitle > 0)
        selectSubtitle(m_lastSubtitle);
    else if (!m_player.subtitleTracks().empty())
        selectSubtitle(m_player.subtitleTracks().front().id);
}

void MainWindow::cycleAudio()
{
    const auto& tracks = m_player.audioTracks();
    if (tracks.size() < 2)
        return;
    for (size_t i = 0; i < tracks.size(); ++i) {
        if (tracks[i].id == m_player.audioTrack()) {
            selectAudio(tracks[(i + 1) % tracks.size()].id);
            return;
        }
    }
    selectAudio(tracks.front().id);
}

std::string MainWindow::delayText(double seconds)
{
    const long ms = std::lround(seconds * 1000);
    // TRANSLATORS: a subtitle delay such as "+100 ms".
    return format(tr("{sign}{milliseconds} ms"), {{"sign", ms > 0 ? "+" : ms < 0 ? "−" : ""},
                                                  {"milliseconds", std::to_string(std::abs(ms))}});
}

void MainWindow::shiftSubtitles(double seconds)
{
    const double delay = std::round((m_player.subtitleDelay() + seconds) * 10) / 10;
    m_player.setSubtitleDelay(delay);
    showOsd(format(tr("Subtitle delay {delay}"), {{"delay", delayText(delay)}}));
}

void MainWindow::takeScreenshot()
{
    if (!m_player.loaded() || !m_player.hasVideo())
        return;
    m_player.screenshot(m_app.screenshotPath(mediaName()));
}

void MainWindow::toggleFullScreen()
{
    if (isFullScreen()) {
        m_window.setFullScreen(false);
        if (m_wasMaximized)
            m_window.setMaximized(true);
    } else {
        m_wasMaximized = m_window.states().maximized;
        m_window.setFullScreen(true);
    }
}

void MainWindow::close()
{
    saveResume();
    Lingmo::ui::Application::instance()->quit();
}

void MainWindow::fitToLaunch()
{
    if (!m_launchFitPending)
        return;
    if (m_window.frameStatistics().presented > 0) {
        m_launchFitPending = false;
        return;
    }
    const SizeF size = fittedSize(m_app.launchVideoSize());
    if (size.width != m_window.size().width || size.height != m_window.size().height)
        m_window.resize(size);
}

void MainWindow::fitToVideo()
{
    const Player::Size video = m_player.videoSize();
    if (!m_fitPending || video.isEmpty())
        return;
    m_fitPending = false;
    if (isFullScreen() || m_window.states().maximized || !m_player.hasVideo())
        return;
    const SizeF size = fittedSize(video);
    if (size.width != m_window.size().width || size.height != m_window.size().height)
        m_window.resize(size);
}

// The window size for a picture: device pixels one to one, within three
// quarters of the work area.
SizeF MainWindow::fittedSize(Player::Size video) const
{
    const double ratio = m_window.scale() > 0 ? m_window.scale() : 1;
    const double w = video.width / ratio;
    const double h = video.height / ratio;
    const SizeF screen = m_window.availableSize();
    double scale = 1;
    if (screen.width > 0 && screen.height > 0)
        scale = std::min({1.0, screen.width * 0.75 / w, screen.height * 0.75 / h});
    return {std::max(480.0, std::round(w * scale)), std::max(320.0, std::round(h * scale))};
}
