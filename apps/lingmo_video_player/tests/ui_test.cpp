// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// The window headless: layout of the welcome page and the controls, clicks
// on the video and the side panel.
#include "application.h"
#include "urls.h"
#include "views/mainwindow.h"
#include "views/videoview.h"
#include "views/widgets.h"

#include "base/file.h"
#include "base/paths.h"
#include "base/timer.h"
#include "ui/application.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/scroll_view.h"
#include "ui/test_input.h"
#include "ui/theme.h"

#include <gtest/gtest.h>

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <functional>

namespace base = Lingmo::base;
namespace ui = Lingmo::ui;
namespace fs = std::filesystem;
using namespace std::chrono_literals;

static ui::Application* s_app = nullptr;
static base::file::TemporaryDirectory* s_dir = nullptr;
static std::string s_clip;

static void runUntil(const std::function<bool()>& done, std::chrono::milliseconds timeout)
{
    base::Timer poll;
    int ticks = 0;
    const int limit = int(timeout / 10ms);
    poll.onTimeout = [&] {
        if (done() || ++ticks > limit)
            s_app->quit();
    };
    poll.start(10ms);
    s_app->run();
}

static void wait(std::chrono::milliseconds time)
{
    runUntil([] { return false; }, time);
}

static ui::View* find(ui::View* view, const std::string& name)
{
    if (view->name() == name)
        return view;
    for (const auto& child : view->children()) {
        if (ui::View* found = find(child.get(), name))
            return found;
    }
    return nullptr;
}

static ui::RectF windowRect(const ui::View* view)
{
    const ui::PointF origin = view->mapToWindow({0, 0});
    return {origin.x, origin.y, view->geometry().width, view->geometry().height};
}

static bool visibleInTree(const ui::View* view)
{
    for (; view; view = view->parent()) {
        if (!view->isVisible())
            return false;
    }
    return true;
}

class UiTest : public testing::Test
{
protected:
    void SetUp() override
    {
        m_config = (fs::path(s_dir->path()) / ("ui" + std::to_string(s_counter++) + ".conf")).string();
        m_app = std::make_unique<Application>(m_config, (fs::path(s_dir->path()) / "covers").string());
        m_window = std::make_unique<MainWindow>(*m_app);
        m_window->window().setScale(1.5);
        m_window->window().resize({960, 600});
        m_window->show();
        wait(50ms);
    }

    void TearDown() override
    {
        m_window.reset();
        m_app.reset();
        wait(20ms);
    }

    ui::View* root() { return m_window->window().contentView(); }
    ui::View* view(const std::string& name) { return find(root(), name); }

    bool openClip()
    {
        if (s_clip.empty())
            return false;
        m_app->openUrls({urls::fromLocalFile(s_clip)});
        runUntil([&] { return m_window->player().loaded() && m_window->player().videoActive(); }, 10s);
        return m_window->player().loaded();
    }

    void click(ui::PointF point)
    {
        ui::TestInput::click(m_window->window(), point);
        wait(30ms);
    }

    std::string m_config;
    std::unique_ptr<Application> m_app;
    std::unique_ptr<MainWindow> m_window;
    static inline int s_counter = 0;
};

TEST_F(UiTest, EmptyWindow)
{
    ui::View* empty = view("emptyPage");
    ASSERT_TRUE(empty);
    EXPECT_TRUE(visibleInTree(empty));
    ui::View* bar = view("controlBar");
    ASSERT_TRUE(bar);
    EXPECT_FALSE(visibleInTree(bar));
    EXPECT_EQ(m_window->window().size().width, 960);

    // The welcome column sits centred under the header at the smallest size.
    for (const ui::SizeF size : {ui::SizeF{960, 600}, ui::SizeF{480, 320}}) {
        m_window->window().resize(size);
        wait(30ms);
        m_window->window().grab();
        const ui::RectF button = windowRect(view("openFileButton"));
        EXPECT_GE(button.y, 44);
        EXPECT_LE(button.y + button.height, size.height);
    }
}

TEST_F(UiTest, EmptyWindowScrollsRecentFiles)
{
    // Out of the clip's folder, which the later tests open as a playlist.
    const fs::path folder = fs::path(s_dir->path()) / "recent";
    fs::create_directories(folder);
    for (int i = 0; i < 5; ++i) {
        const fs::path file = folder / ("recent" + std::to_string(i) + ".mkv");
        base::file::writeAtomically(file.string(), "");
        m_app->settings().addRecentFile(urls::fromLocalFile(file.string()));
    }
    auto* page = dynamic_cast<ui::ScrollView*>(view("emptyPage"));
    ASSERT_TRUE(page);
    ui::View* list = view("recentList");
    ASSERT_TRUE(list);

    // Room for all of it: centred, nothing to scroll.
    m_window->window().grab();
    EXPECT_EQ(page->contentSize().height, page->geometry().height);

    // Too short: 24 clear above the icon at the top and below the list at
    // the bottom.
    m_window->window().resize({480, 320});
    wait(30ms);
    m_window->window().grab();
    EXPECT_GT(page->contentSize().height, page->geometry().height);
    const ui::View* icon = page->contentView()->children().front().get();
    EXPECT_EQ(windowRect(icon).y, 44 + 24);
    page->setScrollPosition({0, page->contentSize().height});
    wait(30ms);
    m_window->window().grab();
    const ui::RectF card = windowRect(list);
    EXPECT_EQ(card.y + card.height, 320 - 24);
}

TEST_F(UiTest, PlaybackLayout)
{
    if (!openClip())
        GTEST_SKIP() << "ffmpeg is needed to make the test clip";
    EXPECT_EQ(m_app->settings().recentFiles().size(), 1u);
    ui::View* bar = view("controlBar");
    ASSERT_TRUE(bar);

    // The window fits the 640x360 picture: one device pixel per video pixel.
    runUntil([&] { return m_window->window().size().width != 960; }, 3s);
    EXPECT_NEAR(m_window->window().size().width, std::max(480.0, std::round(640 / 1.5)), 1);

    for (const ui::SizeF size : {ui::SizeF{1280, 800}, ui::SizeF{801, 501}, ui::SizeF{480, 320}}) {
        m_window->window().resize(size);
        wait(30ms);
        m_window->window().grab();
        EXPECT_TRUE(visibleInTree(bar));
        const ui::RectF plate = windowRect(bar);
        EXPECT_GE(plate.x, 12 - 0.01);
        EXPECT_LE(plate.x + plate.width, size.width - 12 + 0.01);
        EXPECT_EQ(plate.width, std::min(460.0, size.width - 24));
        // Centred to the device pixel.
        EXPECT_LE(std::abs(plate.x + plate.width / 2 - size.width / 2) * 1.5, 1.0);

        // Glyphs in the middle of their buttons, buttons apart and on the plate.
        std::vector<ui::RectF> rects;
        for (const char* name : {"muteButton", "skipBackButton", "playButton", "skipForwardButton", "sidePanelButton",
                                 "fullscreenButton"}) {
            ui::View* button = view(name);
            ASSERT_TRUE(button) << name;
            const ui::RectF rect = windowRect(button);
            ASSERT_FALSE(button->children().empty());
            const ui::RectF glyph = windowRect(button->children().front().get());
            EXPECT_LE(std::abs(glyph.x + glyph.width / 2 - rect.x - rect.width / 2) * 1.5, 1) << name;
            EXPECT_LE(std::abs(glyph.y + glyph.height / 2 - rect.y - rect.height / 2) * 1.5, 1) << name;
            EXPECT_TRUE(rect.x >= plate.x && rect.x + rect.width <= plate.x + plate.width) << name;
            for (const ui::RectF& other : rects) {
                const bool apart = rect.x >= other.x + other.width || other.x >= rect.x + rect.width;
                EXPECT_TRUE(apart) << name << " overlaps at " << size.width;
            }
            rects.push_back(rect);
        }
        // The play button in the middle of the plate.
        const ui::RectF play = windowRect(view("playButton"));
        EXPECT_LT(std::abs(play.x + play.width / 2 - plate.x - plate.width / 2), 1) << size.width;

        // The times line up with the outer glyphs of the row above.
        const ui::RectF mute = windowRect(view("muteButton")->children().front().get());
        const ui::RectF full = windowRect(view("fullscreenButton")->children().front().get());
        const ui::RectF elapsed = windowRect(view("elapsedLabel"));
        const ui::RectF total = windowRect(view("totalLabel"));
        EXPECT_LE(std::abs(elapsed.x - mute.x) * 1.5, 1);
        EXPECT_LE(std::abs(total.x + total.width - full.x - full.width) * 1.5, 1);
    }

    // Playing with the pointer still hides the controls and the pointer.
    m_window->window().resize({960, 600});
    m_window->player().setPaused(false);
    runUntil([&] { return !m_window->player().paused(); }, 3s);
    ui::TestInput::movePointer(m_window->window(), {480, 250});
    EXPECT_TRUE(m_window->controlsVisible());
    runUntil([&] { return !m_window->controlsVisible(); }, 4s);
    EXPECT_FALSE(m_window->controlsVisible());
    EXPECT_EQ(root()->cursorShape(), ui::CursorShape::Blank);

    // The playlist opens over the right side, clear of the control bar.
    m_window->poke();
    m_window->togglePanel(0, false);
    wait(30ms);
    m_window->window().grab();
    ui::View* panel = view("sidePanel");
    ASSERT_TRUE(panel);
    EXPECT_TRUE(visibleInTree(panel));
    const ui::RectF side = windowRect(panel);
    EXPECT_EQ(std::lround(side.x + side.width), 960 - 12);
    const ui::RectF plate = windowRect(bar);
    EXPECT_TRUE(side.y + side.height <= plate.y || side.x >= plate.x + plate.width);
    for (int tab = 0; tab < 3; ++tab) {
        m_window->setPanelTab(tab);
        wait(10ms);
        m_window->window().grab();
    }
}

TEST_F(UiTest, ReplayAtEnd)
{
    if (!openClip())
        GTEST_SKIP() << "ffmpeg is needed to make the test clip";
    Player& player = m_window->player();
    player.seek(19, true);
    player.setPaused(false);
    runUntil([&] { return player.endReached() && player.paused(); }, 10s);
    ASSERT_TRUE(player.endReached());
    m_window->window().grab();
    const ui::RectF play = windowRect(view("playButton"));
    click({play.x + play.width / 2, play.y + play.height / 2});
    runUntil([&] { return !player.endReached() && !player.paused() && player.position() > 0.5; }, 5s);
    EXPECT_FALSE(player.paused());
    EXPECT_LT(player.position(), 5);
}

TEST_F(UiTest, ClickAndDoubleClick)
{
    if (!openClip())
        GTEST_SKIP() << "ffmpeg is needed to make the test clip";
    Player& player = m_window->player();
    runUntil([&] { return !player.paused(); }, 3s);
    m_window->window().grab();
    const ui::PointF video{40, m_window->window().size().height / 2};

    // A click pauses once the double-click interval has passed.
    click(video);
    EXPECT_FALSE(player.paused());
    runUntil([&] { return player.paused(); }, 1500ms);
    EXPECT_TRUE(player.paused());
}

TEST_F(UiTest, MuteIsRemembered)
{
    if (!openClip())
        GTEST_SKIP() << "ffmpeg is needed to make the test clip";
    // Each toggle is saved as it is made, before mpv reports it back.
    m_window->toggleMute();
    EXPECT_TRUE(m_app->settings().muted());
    runUntil([&] { return m_window->player().muted(); }, 2s);
    EXPECT_TRUE(m_window->player().muted());
    m_window->toggleMute();
    EXPECT_FALSE(m_app->settings().muted());
    runUntil([&] { return !m_window->player().muted(); }, 2s);
    EXPECT_FALSE(m_window->player().muted());
}

TEST_F(UiTest, SidePanelKeepsInput)
{
    if (!openClip())
        GTEST_SKIP() << "ffmpeg is needed to make the test clip";
    Player& player = m_window->player();
    runUntil([&] { return !player.paused(); }, 3s);
    m_window->togglePanel(0, false);
    wait(30ms);
    m_window->window().grab();
    ui::View* list = view("playlistView");
    ASSERT_TRUE(list);
    runUntil([&] {
        m_window->window().grab();
        auto* duration = dynamic_cast<ui::Label*>(find(list, "playlistDuration"));
        return duration && duration->isVisible() && duration->text() == "0:20";
    }, 5s);
    auto* duration = dynamic_cast<ui::Label*>(find(list, "playlistDuration"));
    ASSERT_TRUE(duration);
    EXPECT_EQ(duration->text(), "0:20");
    const ui::RectF rect = windowRect(list);
    const int volume = player.volume();

    // A row, and the empty list below it: nothing reaches the video behind.
    for (const ui::PointF point : {ui::PointF{rect.x + rect.width / 2, rect.y + 18},
                                   ui::PointF{rect.x + rect.width / 2, rect.y + rect.height - 12}}) {
        click(point);
        wait(700ms);
        EXPECT_FALSE(player.paused()) << "a click paused the video";
        ui::ScrollEvent wheel;
        wheel.value120Y = -120;
        wheel.deltaY = -15;
        ui::TestInput::scroll(m_window->window(), point, wheel);
        wait(100ms);
        EXPECT_EQ(player.volume(), volume);
    }
}

TEST_F(UiTest, OutsideClickClosesSidePanel)
{
    if (!openClip())
        GTEST_SKIP() << "ffmpeg is needed to make the test clip";
    Player& player = m_window->player();
    runUntil([&] { return !player.paused(); }, 3s);
    const ui::PointF video{40, m_window->window().size().height / 2};

    // A click on the video only closes the panel.
    m_window->togglePanel(0, false);
    wait(30ms);
    m_window->window().grab();
    click(video);
    EXPECT_FALSE(m_window->panelOpen());
    wait(800ms);
    EXPECT_FALSE(player.paused());

    // A tab click reaches no further.
    m_window->togglePanel(0, false);
    wait(30ms);
    m_window->window().grab();
    const ui::RectF panel = windowRect(view("sidePanel"));
    click({panel.x + panel.width / 2, panel.y + 23});
    EXPECT_EQ(m_window->panelTab(), 1);
    wait(800ms);
    EXPECT_FALSE(player.paused());

    // A press that drags leaves it open.
    ui::TestInput::press(m_window->window(), video);
    for (int step = 1; step <= 4; ++step)
        ui::TestInput::movePointer(m_window->window(), {video.x + step * 10, video.y});
    ui::TestInput::release(m_window->window(), {video.x + 40, video.y});
    wait(800ms);
    EXPECT_TRUE(m_window->panelOpen());
    EXPECT_FALSE(player.paused());
}

TEST_F(UiTest, LanguageChangeKeepsPlaying)
{
    if (!openClip())
        GTEST_SKIP() << "ffmpeg is needed to make the test clip";
    Player& player = m_window->player();
    runUntil([&] { return !player.paused() && player.position() > 0.5; }, 3s);
    m_window->setVolume(40);
    m_window->togglePanel(1, false);
    wait(30ms);
    m_window->window().grab();
    const std::string source = player.source();
    const VideoView* video = m_window->videoView();
    const bool surface = video->usesSurface();
    const ui::View* panel = view("sidePanel");
    ui::View* tracks = find(view("sidePanel"), "firstSection");
    ASSERT_TRUE(tracks);
    // Marks the rows made before; a new row could take an old one's address.
    tracks->parent()->children()[1]->setName("oldTrackRow");
    int loads = 0;
    const base::Subscription loaded = player.fileLoaded.connect([&] { ++loads; });
    const double position = player.position();

    ui::Application::instance()->languageChanged.emit();
    wait(300ms);
    m_window->window().grab();
    EXPECT_EQ(loads, 0);
    EXPECT_EQ(player.source(), source);
    EXPECT_TRUE(player.loaded());
    EXPECT_FALSE(player.paused());
    EXPECT_GT(player.position(), position);
    EXPECT_EQ(player.volume(), 40);
    EXPECT_EQ(m_window->videoView(), video);
    EXPECT_EQ(video->usesSurface(), surface);
    EXPECT_TRUE(m_window->panelOpen());
    EXPECT_EQ(m_window->panelTab(), 1);
    EXPECT_EQ(view("sidePanel"), panel);
    EXPECT_TRUE(visibleInTree(panel));
    // The track rows are made again, in the language of the moment.
    EXPECT_TRUE(find(view("sidePanel"), "firstSection"));
    EXPECT_FALSE(find(view("sidePanel"), "oldTrackRow"));
    auto* tab = dynamic_cast<ui::Label*>(view("panelTab1")->children().front().get());
    ASSERT_TRUE(tab);
    EXPECT_FALSE(tab->text().empty());
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    ui::Application app(argc, argv, ui::Application::Platform::Headless);
    s_app = &app;
    views::registerGlyphs();
    ui::Theme::setCurrent(ui::Theme::light());
    base::file::TemporaryDirectory dir(fs::temp_directory_path().string(), "video-player-ui");
    s_dir = &dir;
    if (!GTEST_FLAG_GET(list_tests) && !base::paths::findExecutable("ffmpeg").empty()) {
        const std::string clip = (fs::path(dir.path()) / "Sample Clip.mkv").string();
        const std::string command = "ffmpeg -v error -y -f lavfi -i testsrc=size=640x360:rate=25:duration=20 "
                                    "-pix_fmt yuv420p '" + clip + "'";
        if (std::system(command.c_str()) == 0)
            s_clip = clip;
    }
    return RUN_ALL_TESTS();
}
