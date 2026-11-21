// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The launcher as the compositor draws it: its window, layout, keys, search,
// paging, menu and drags, headless on a fake renderer.

#include <chrono>
#include <cmath>

#include <linux/input-event-codes.h>

#include <gtest/gtest.h>

#include "base/settings.h"
#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "dbus_test_client.h"
#include "fake_renderer.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "loop_test_util.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "shell/common/file_thread.h"
#include "shell/common/app_launcher.h"
#include "shell/common/count_badge.h"
#include "shell/common/task_manager.h"
#include "shell/launcher/launcher.h"
#include "shell/launcher/launcher_cell.h"
#include "shell/launcher/launcher_metrics.h"
#include "shell/launcher/launcher_model.h"
#include "shell/launcher/launcher_search_field.h"
#include "shell/launcher/launcher_view.h"
#include "shell/launcher/launcher_wallpaper.h"
#include "shell/wallpaper/wallpaper.h"
#include "shell/launcher/launcher_window.h"
#include "shell/shell.h"
#include "shell/shell_window.h"
#include "shell_test_util.h"
#include "temporary_directory.h"
#include "ui/bitmap.h"
#include "ui/embedded_window.h"
#include "ui/menu.h"
#include "wm/window.h"
#include "wm/window_manager.h"

using namespace Lingmo;

namespace {

input::Timestamp at(int ms)
{
    return input::Timestamp(std::chrono::milliseconds(ms));
}

const base::Rect kOutput(0, 0, 1440, 960);
constexpr int kStatusBar = 32;

class LauncherWindowTest : public testing::Test
{
protected:
    scene::Scene scene;
    scene::Node *surfaceContainer = scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    compositor::Compositor compositor {surfaceContainer};
    testsupport::FakeRenderer renderer;
    input::Seat seat {&compositor};
    input::InputManager input {&seat};
    wm::WindowManager manager {&compositor, &seat};
    test::TemporaryDirectory directory;
    test::FakeApplications *applications = nullptr;
    std::unique_ptr<shell::Shell> shell;
    shell::Wallpaper *wallpaper = nullptr;
    shell::LauncherWindow *launcherWindow = nullptr;
    int statusBar = 0;
    int clock = 1000;

    void SetUp() override
    {
        scene.setSize(kOutput.size());
        manager.setOutputBounds(kOutput);
        manager.setExclusiveZone(&statusBar, wm::ScreenEdge::Top, kStatusBar);
        input.setBounds(kOutput.size());
        input.setEventSink(&manager);
        directory.write("services.conf", "[Appearance]\nBackgroundType=1\nBackgroundColor=#336699\n");
        startShell(nullptr);
    }

    // A shell on `bus`, which may be null.
    void startShell(base::dbus::Connection *bus)
    {
        shell.reset();
        shell::ShellEnvironment environment {&compositor, &manager, &seat, bus};
        environment.servicesSettings = directory.filePath("services.conf");
        shell = std::make_unique<shell::Shell>(environment);
        auto apps = std::make_unique<test::FakeApplications>();
        applications = apps.get();
        // More than one page at 1440x960 (6 columns, 4 rows).
        for (int i = 0; i < 30; ++i) {
            char id[16];
            snprintf(id, sizeof(id), "app%02d", i);
            applications->addSimple(id, std::string("App ") + id, "application-x-executable");
        }
        applications->addSimple("terminal", "Terminal", "utilities-terminal");
        applications->scan();
        shell->setApplications(std::move(apps));
        shell->setTaskWindows(std::make_unique<test::FakeTaskWindows>());
    }

    void TearDown() override
    {
        shell.reset();
        seat.shutdown();
    }

    void open()
    {
        auto launcher = std::make_unique<shell::Launcher>(nullptr, shell->tasks(), shell->launcher(), nullptr,
                                                          directory.filePath("shell.conf"));
        wallpaper = shell->addComponent(std::make_unique<shell::Wallpaper>(shell.get(), directory.filePath("cache")));
        shell::Wallpaper::Background background;
        background.type = 1;
        background.color = ui::Color::rgb(0x336699);
        wallpaper->setBackground(background);
        launcherWindow = shell->addComponent(std::make_unique<shell::LauncherWindow>(
                shell.get(), std::move(launcher), nullptr, wallpaper));
        ASSERT_TRUE(test::runUntil([this] { return model().count() == 31; }));
        ASSERT_TRUE(test::runUntil([this] { return launcherWindow->wallpaper().isLoaded(); }));
    }

    shell::Launcher &launcher() { return launcherWindow->launcher(); }
    shell::LauncherModel &model() { return launcher().applications(); }
    shell::LauncherView *view() { return launcherWindow->view(); }

    void frame() { compositor.updateTextures(renderer); }

    void key(uint32_t code)
    {
        input.onKey(at(clock), code, true);
        input.onKey(at(clock + 1), code, false);
        clock += 10;
    }

    void move(double x, double y)
    {
        input.onPointerMotionAbsolute(at(clock), {x, y});
        clock += 10;
    }

    void click(double x, double y, uint32_t button = BTN_LEFT)
    {
        move(x, y);
        input.onPointerButton(at(clock), button, true);
        input.onPointerButton(at(clock + 1), button, false);
        clock += 10;
    }

    ui::PointF iconCentre(int row)
    {
        const ui::RectF cell = view()->cellRect(row);
        const ui::RectF icon = view()->cellAt(row)->iconRect();
        return {cell.x + icon.x + icon.width / 2, cell.y + icon.y + icon.height / 2};
    }
};

} // namespace

TEST(LauncherMetricsTest, FollowsTheQtLauncherAt1080p)
{
    shell::LauncherMetrics::Margins margins;
    margins.bottom = 86;
    const auto m = shell::LauncherMetrics::compute({1920, 1080}, margins);
    EXPECT_EQ(67, m.outerMarginH);
    EXPECT_EQ(38, m.outerMarginV);
    EXPECT_EQ(27, m.sectionSpacing);
    EXPECT_EQ(18, m.searchPixelSize);
    EXPECT_EQ(422, m.searchFieldWidth);
    EXPECT_EQ(41, m.searchFieldHeight);
    EXPECT_EQ(18, m.searchIconSize);
    EXPECT_EQ(12, m.indicatorDotSize);
    EXPECT_EQ(38, m.notFoundPixelSize);
    EXPECT_EQ(base::Rect(67, 46, 1786, 910), m.content);
    EXPECT_EQ(1786, m.gridArea.width());
    EXPECT_EQ(803, m.gridArea.height());
    EXPECT_EQ(7, m.columns);
    EXPECT_EQ(4, m.rows);
    EXPECT_EQ(219, m.cellWidth);
    EXPECT_EQ(200, m.cellHeight);
    EXPECT_EQ(16, m.labelPixelSize);
    EXPECT_EQ(21, m.labelLineHeight);
    EXPECT_EQ(105, m.iconSize);
    EXPECT_EQ(11, m.cellSpacing);
    EXPECT_EQ(31, m.cellPaddingV);
    EXPECT_EQ(151, m.selectionWidth);
    EXPECT_EQ(23, m.selectionPadding);
}

TEST(LauncherMetricsTest, ArrowsMoveWithinThePageAndTurnItAtTheEdges)
{
    using shell::LauncherDirection;
    // 6 columns, 4 rows, 30 icons: page 2 holds 6, one row.
    const auto move = [](int selected, int page, LauncherDirection direction) {
        return shell::moveLauncherSelection(selected, page, 30, 6, 24, direction);
    };
    // The first press selects the page's first icon.
    EXPECT_EQ(0, move(-1, 0, LauncherDirection::Right));
    EXPECT_EQ(24, move(-1, 1, LauncherDirection::Down));
    EXPECT_EQ(1, move(0, 0, LauncherDirection::Right));
    EXPECT_EQ(-1, move(0, 0, LauncherDirection::Left));
    EXPECT_EQ(6, move(0, 0, LauncherDirection::Down));
    EXPECT_EQ(-1, move(0, 0, LauncherDirection::Up));
    // The right edge of row 2 turns to the next page's row, clamped to its
    // last row, column 0.
    EXPECT_EQ(24, move(17, 0, LauncherDirection::Right));
    // The left edge of the next page comes back to row 0's last column.
    EXPECT_EQ(5, move(24, 1, LauncherDirection::Left));
    // Down off the page goes nowhere.
    EXPECT_EQ(-1, move(19, 0, LauncherDirection::Down));
}

TEST(LauncherWallpaperTest, BlursAsTheProviderDoes)
{
    // One white pixel in black spreads into a 19-pixel wide hump, symmetric.
    ui::Bitmap image(41, 1, ui::Bitmap::Format::Argb32Premultiplied);
    image.fill(0xff000000);
    image.setPixel(20, 0, 0xffffffff);
    shell::LauncherWallpaper::blur(image);
    EXPECT_EQ(image.pixel(17, 0), image.pixel(23, 0));
    EXPECT_GT(image.pixel(20, 0) & 0xff, image.pixel(17, 0) & 0xff);
    EXPECT_EQ(0xff000000u, image.pixel(10, 0));
    EXPECT_EQ(0xffu, image.pixel(30, 0) >> 24);
}

TEST_F(LauncherWindowTest, CoversThePrimaryOutputAndTakesTheKeyboard)
{
    open();
    EXPECT_FALSE(launcher().isShown());
    EXPECT_EQ(nullptr, launcherWindow->window()->wmWindow());

    launcher().show();
    wm::Window *window = launcherWindow->window()->wmWindow();
    ASSERT_NE(nullptr, window);
    EXPECT_EQ(wm::WindowRole::Launcher, window->role());
    // The whole output, the status bar's strip included.
    EXPECT_EQ(kOutput, window->bounds());
    EXPECT_TRUE(window->isActive());
    frame();

    // 1440x960 with no dock: 6 columns of 4 rows, two pages.
    const shell::LauncherMetrics &m = view()->metrics();
    EXPECT_EQ(6, m.columns);
    EXPECT_EQ(4, m.rows);
    EXPECT_EQ(2, view()->pageTotal());
    EXPECT_EQ(0, view()->currentPage());
    EXPECT_EQ(base::Rect(50, 42, 1340, 884), m.content);

    key(KEY_ESC);
    EXPECT_FALSE(launcher().isShown());
    EXPECT_EQ(nullptr, launcherWindow->window()->wmWindow());
    // Toggled back, it shows the first page again.
    launcher().toggle();
    EXPECT_TRUE(launcher().isShown());
    EXPECT_NE(nullptr, launcherWindow->window()->wmWindow());
}

TEST_F(LauncherWindowTest, ArrowsSelectAndTurnPages)
{
    open();
    launcher().show();
    frame();
    key(KEY_RIGHT);
    EXPECT_EQ(0, view()->selectedIndex());
    EXPECT_TRUE(view()->cellAt(0)->isSelected());
    EXPECT_GT(view()->cellAt(0)->selectionRect().width, 0);
    for (int i = 0; i < 5; ++i)
        key(KEY_RIGHT);
    EXPECT_EQ(5, view()->selectedIndex());
    // The row's right edge turns the page.
    key(KEY_RIGHT);
    EXPECT_EQ(24, view()->selectedIndex());
    EXPECT_EQ(1, view()->currentPage());
    // The slide runs on the window's frames.
    for (int i = 0; i < 40 && view()->contentX() != kOutput.width(); ++i) {
        test::runFor(std::chrono::milliseconds(16));
        frame();
    }
    EXPECT_EQ(kOutput.width(), view()->contentX());

    // Hiding resets the selection and the page.
    launcher().hide();
    EXPECT_EQ(-1, view()->selectedIndex());
    launcher().show();
    EXPECT_EQ(-1, view()->selectedIndex());
}

TEST_F(LauncherWindowTest, TypingSearchesAndShowsNotFound)
{
    open();
    launcher().show();
    frame();
    key(KEY_T);
    shell::LauncherSearchField *search = view()->searchField();
    EXPECT_TRUE(search->hasFieldFocus());
    EXPECT_EQ("t", search->text());
    // Debounced by 150 ms.
    EXPECT_EQ(31, model().count());
    ASSERT_TRUE(test::runUntil([this] { return model().count() < 31; }));
    EXPECT_EQ(1, model().count());
    EXPECT_EQ("terminal", model().rows()[0].id);
    EXPECT_EQ(0, view()->selectedIndex());
    EXPECT_EQ(1, view()->pageTotal());

    key(KEY_Q);
    ASSERT_TRUE(test::runUntil([this] { return model().count() == 0; }));
    EXPECT_TRUE(view()->isNotFoundVisible());
    EXPECT_EQ(-1, view()->selectedIndex());

    // Clearing the field shows everything at once.
    search->setText({});
    EXPECT_EQ(31, model().count());
    EXPECT_FALSE(view()->isNotFoundVisible());

    // Escape in the field hides; the field is empty on the next show.
    key(KEY_A);
    EXPECT_EQ("a", search->text());
    key(KEY_ESC);
    EXPECT_FALSE(launcher().isShown());
    EXPECT_EQ("", search->text());
}

TEST_F(LauncherWindowTest, ClicksOutsideTheIconsHideAndRightClicksOpenTheMenu)
{
    open();
    launcher().show();
    frame();
    // The icon's right button opens Open, Send to dock, Send to desktop.
    const ui::PointF icon = iconCentre(0);
    click(icon.x, icon.y, BTN_RIGHT);
    frame();
    ASSERT_TRUE(view()->isMenuOpen());
    EXPECT_TRUE(view()->cellAt(0)->isPressDimmed());
    EXPECT_TRUE(launcher().isShown());
    view()->menu()->close();
    EXPECT_FALSE(view()->cellAt(0)->isPressDimmed());

    // A label is not the icon: it dismisses.
    const ui::RectF cell = view()->cellRect(0);
    click(cell.x + cell.width / 2, cell.y + cell.height - 4);
    EXPECT_FALSE(launcher().isShown());
}

TEST_F(LauncherWindowTest, TheWheelTurnsAPagePerNotchAndTheDotsPage)
{
    open();
    launcher().show();
    frame();
    move(700, 500);
    input.onPointerAxis(at(clock), {0, 15}, true);
    clock += 10;
    EXPECT_EQ(1, view()->currentPage());
    // The slide keeps the next notch from paging again.
    input.onPointerAxis(at(clock), {0, -15}, true);
    clock += 10;
    EXPECT_EQ(1, view()->currentPage());

    const std::vector<ui::PointF> dots = view()->indicatorDots();
    ASSERT_EQ(2u, dots.size());
    test::runFor(std::chrono::milliseconds(320));
    click(dots[0].x, dots[0].y);
    EXPECT_EQ(0, view()->currentPage());
    EXPECT_TRUE(launcher().isShown());
}

TEST_F(LauncherWindowTest, LosingTheActivationHides)
{
    open();
    launcher().show();
    frame();
    ASSERT_TRUE(launcher().isShown());
    manager.activation()->deactivate(launcherWindow->window()->wmWindow());
    ASSERT_TRUE(test::runUntil([this] { return !launcher().isShown(); }));

    // Not while a drag holds it.
    launcher().show();
    launcher().setDragging(true);
    manager.activation()->deactivate(launcherWindow->window()->wmWindow());
    test::runFor(std::chrono::milliseconds(20));
    EXPECT_TRUE(launcher().isShown());
    launcher().setDragging(false);
}

TEST_F(LauncherWindowTest, ACancelledDragPutsTheOrderBackUnsaved)
{
    std::string stored;
    for (int i = 0; i < 30; ++i)
        stored += (i ? ", app" : "app") + std::string(i < 10 ? "0" : "") + std::to_string(i);
    stored += ", terminal";
    directory.write("shell.conf", "[Launcher]\norder=" + stored + "\n");
    open();
    launcher().show();
    frame();
    const auto order = [this] {
        std::vector<std::string> ids;
        for (const shell::LauncherItem &item : model().allItems())
            ids.push_back(item.id);
        return ids;
    };
    const auto saved = [this] {
        shell::waitForFileThread();
        return base::Settings(directory.filePath("shell.conf")).stringListValue("Launcher/order");
    };
    const std::vector<std::string> before = order();
    ASSERT_EQ("app00", before.front());
    const ui::PointF from = iconCentre(0);
    const ui::PointF to = iconCentre(3);
    const auto dragToFourth = [&] {
        move(from.x, from.y);
        input.onPointerButton(at(clock), BTN_LEFT, true);
        move(from.x + 20, from.y);
        test::processPendingEvents();
        move(to.x, to.y);
        // The dwell moves it into the fourth slot while the drag runs.
        ASSERT_TRUE(test::runUntil([&] { return model().findById("app00") == 3; }));
    };

    // Escape: the order as it was, and the moved one never written.
    dragToFourth();
    test::runFor(shell::LauncherModel::kSaveDelay + std::chrono::milliseconds(200));
    EXPECT_EQ(before, saved());
    key(KEY_ESC);
    input.onPointerButton(at(clock), BTN_LEFT, false);
    clock += 10;
    ASSERT_TRUE(test::runUntil([this] { return view()->draggedId().empty(); }));
    EXPECT_EQ(before, order());
    test::runFor(shell::LauncherModel::kSaveDelay + std::chrono::milliseconds(200));
    EXPECT_EQ(before, saved());

    // Dropped on the launcher, the move stays and is written.
    frame();
    dragToFourth();
    input.onPointerButton(at(clock), BTN_LEFT, false);
    clock += 10;
    ASSERT_TRUE(test::runUntil([this] { return view()->draggedId().empty(); }));
    test::runFor(shell::LauncherModel::kSaveDelay + std::chrono::milliseconds(200));
    EXPECT_EQ(3, model().findById("app00"));
    EXPECT_EQ(order(), saved());
}

TEST_F(LauncherWindowTest, ThePictureIsTheDesktopsDecodeShrunkAndBlurred)
{
    open();
    ui::Bitmap grey(64, 64, ui::Bitmap::Format::Argb32Premultiplied);
    grey.fill(0xff808080u);
    ASSERT_TRUE(grey.savePng(directory.filePath("grey.png")));
    shell::Wallpaper::Background background;
    background.path = directory.filePath("grey.png");
    wallpaper->setBackground(background);
    ASSERT_TRUE(test::runUntil([this] {
        return launcherWindow->wallpaper().isPicture() && !launcherWindow->wallpaper().image().isNull();
    }));
    // An eighth of the surface, from the decode the desktop shows.
    EXPECT_EQ(180, launcherWindow->wallpaper().image().width());
    EXPECT_EQ(120, launcherWindow->wallpaper().image().height());
    EXPECT_FALSE(wallpaper->cache().cached(directory.filePath("grey.png"), kOutput.size()).isNull());
    // The icons sit on the average under the dimmer.
    EXPECT_EQ((ui::Color {77, 77, 77, 255}), launcherWindow->wallpaper().backdropColor());
}

TEST_F(LauncherWindowTest, AnApplicationsCountSitsOnItsIconsCorner)
{
    namespace dbus = base::dbus;
    dbus::Connection *bus = dbus::Connection::session(core::mainLoop());
    ASSERT_NE(nullptr, bus);
    startShell(bus);
    open();
    test::roundTrip(bus);
    launcher().show();
    frame();

    test::DBusTestClient client;
    client.sendLauncherEntry("application://terminal.desktop",
                             dbus::VariantMap {{"count", dbus::Variant(int64_t(5))},
                                               {"count-visible", dbus::Variant(true)}});
    shell::LauncherCell *cell = view()->cell("terminal");
    ASSERT_NE(nullptr, cell);
    ASSERT_TRUE(test::runUntil([cell] { return cell->countBadge() && cell->countBadge()->isVisible(); }));
    shell::CountBadge *badge = cell->countBadge();
    EXPECT_EQ("5", badge->text());
    const ui::RectF icon = cell->iconRect();
    const double height = shell::CountBadge::heightFor(icon.width, shell::CountBadge::kLauncherRatio);
    const double outset = std::round(height * 0.2);
    EXPECT_EQ(ui::RectF(icon.x + icon.width + outset - height, icon.y - outset, height, height), badge->geometry());
    EXPECT_EQ(nullptr, view()->cell("app00")->countBadge());
    frame();

    // Hidden with its cell while dragged.
    cell->setDragging(true);
    EXPECT_FALSE(badge->isVisible());
    cell->setDragging(false);
    EXPECT_TRUE(badge->isVisible());

    client.sendLauncherEntry("application://terminal.desktop", dbus::VariantMap {{"count", dbus::Variant(int64_t(0))}});
    ASSERT_TRUE(test::runUntil([badge] { return !badge->isVisible(); }));
}

TEST_F(LauncherWindowTest, PinchTracksBothDirectionsAndSettlesOrCancels)
{
    open();
    ASSERT_TRUE(launcherWindow->beginGesture());
    launcherWindow->updateGesture(0.3);
    frame();
    EXPECT_NEAR(view()->opacity(), 0.3, 0.001);
    EXPECT_GT(view()->transform().scale, 1);
    EXPECT_FALSE(view()->isEnabled());
    launcherWindow->updateGesture(0.8);
    EXPECT_NEAR(view()->opacity(), 0.8, 0.001);
    launcherWindow->updateGesture(0.2);
    EXPECT_NEAR(view()->opacity(), 0.2, 0.001);
    launcherWindow->endGesture(false);
    ASSERT_TRUE(test::runUntil([&] { frame(); return !launcher().isShown(); }));
    ASSERT_TRUE(launcherWindow->beginGesture());
    launcherWindow->updateGesture(0.9);
    launcherWindow->endGesture(true);
    ASSERT_TRUE(test::runUntil([&] { frame(); return !launcher().isShown(); }));
    ASSERT_TRUE(launcherWindow->beginGesture());
    launcherWindow->updateGesture(0.7);
    launcherWindow->endGesture(false);
    ASSERT_TRUE(test::runUntil([&] { frame(); return view()->isEnabled(); }));
    EXPECT_TRUE(launcher().isShown());
    EXPECT_DOUBLE_EQ(view()->opacity(), 1);
    EXPECT_TRUE(view()->transform().isIdentity());
    launcher().hide();
    launcher().show();
    EXPECT_TRUE(view()->isEnabled());
}

TEST_F(LauncherWindowTest, SpreadingClosesAnOpenLauncherContinuouslyAndCanCancel)
{
    open(); launcher().show(); frame();
    ASSERT_TRUE(launcherWindow->beginGesture(false));
    launcherWindow->updateGesture(0.4);
    EXPECT_NEAR(view()->opacity(), 0.6, 0.001);
    launcherWindow->updateGesture(0.1);
    EXPECT_NEAR(view()->opacity(), 0.9, 0.001);
    launcherWindow->endGesture(true);
    ASSERT_TRUE(test::runUntil([&] { frame(); return view()->isEnabled(); }));
    EXPECT_TRUE(launcher().isShown());
    ASSERT_TRUE(launcherWindow->beginGesture(false));
    launcherWindow->updateGesture(0.8);
    launcherWindow->endGesture(false);
    ASSERT_TRUE(test::runUntil([&] { frame(); return !launcher().isShown(); }));
}
