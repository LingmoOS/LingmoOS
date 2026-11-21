// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The notification banners and the notification centre as the compositor
// draws them: windows, places, glass, timeouts, clicks, stacking and the
// centre's groups, headless on a fake renderer and a private session bus.

#include <chrono>
#include <cmath>

#include <linux/input-event-codes.h>

#include <gtest/gtest.h>

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
#include "shell/common/task_manager.h"
#include "shell/notification/notification_card.h"
#include "shell/notification/notification_centre.h"
#include "shell/notification/notification_controls.h"
#include "shell/notification/notification_models.h"
#include "shell/notification/notification_motion.h"
#include "shell/notification/notification_popups.h"
#include "shell/notification/notification_ui.h"
#include "shell/notification/notifications.h"
#include "shell/shell.h"
#include "shell/shell_data.h"
#include "shell/shell_window.h"
#include "shell_test_util.h"
#include "temporary_directory.h"
#include "ui/scroll_view.h"
#include "ui/text_markup.h"
#include "ui/view.h"
#include "wm/window.h"
#include "wm/window_manager.h"

using namespace Lingmo;
namespace dbus = base::dbus;
using shell::NotificationCentre;
using shell::NotificationPopups;

namespace {

constexpr const char *kService = "org.freedesktop.Notifications";
constexpr const char *kPath = "/org/freedesktop/Notifications";

const base::Rect kOutput(0, 0, 1440, 960);
constexpr int kStatusBar = 32;
constexpr int kDock = 92;

class NotificationViewTest : public testing::Test
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
    std::unique_ptr<shell::Shell> shell;
    std::unique_ptr<shell::ShellWindow> statusBar;
    shell::NotificationUi *ui = nullptr;
    shell::Notifications *notifications = nullptr;
    test::DBusTestClient client;
    int statusBarZone = 0;
    int dockZone = 0;
    int ms = 1000;

    void SetUp() override
    {
        shell::setShellDataDirectoryForTesting(LINGMO_SOURCE_DIR "/data/shell");
        scene.setSize(kOutput.size());
        manager.setOutputBounds(kOutput);
        manager.setExclusiveZone(&statusBarZone, wm::ScreenEdge::Top, kStatusBar);
        manager.setExclusiveZone(&dockZone, wm::ScreenEdge::Bottom, kDock);
        input.setBounds(kOutput.size());
        input.setEventSink(&manager);
        shell::ShellEnvironment environment {&compositor, &manager, &seat,
                                             dbus::Connection::session(core::mainLoop())};
        environment.servicesSettings = directory.filePath("services.conf");
        shell = std::make_unique<shell::Shell>(environment);
        auto apps = std::make_unique<test::FakeApplications>();
        apps->addSimple("org.example.Chat", "Chat", "chat-icon");
        apps->scan();
        shell->setApplications(std::move(apps));
        shell->setTaskWindows(std::make_unique<test::FakeTaskWindows>());

        statusBar = std::make_unique<shell::ShellWindow>(shell.get(), wm::WindowRole::Panel);
        statusBar->setFocusPolicy(wm::FocusPolicy::NoFocus);
        statusBar->setContentView(std::make_unique<ui::View>());
        statusBar->setGeometry(base::Rect(0, 0, kOutput.width(), kStatusBar));
        statusBar->show();

        auto backend = std::make_unique<shell::Notifications>(
                shell->bus(), shell->applications(), &shell->tasks(),
                shell::Notifications::Files {directory.filePath("shell.conf"), directory.filePath("history"),
                                             directory.filePath("images")});
        notifications = backend.get();
        ui = shell->addComponent(std::make_unique<shell::NotificationUi>(shell.get(), std::move(backend)));
        ASSERT_TRUE(ui->registerServices());
        ASSERT_TRUE(test::runUntil([this] {
            return notifications->settings().isReady() && notifications->history().isLoaded();
        }));
        client.listen(kService);
    }

    void TearDown() override
    {
        statusBar.reset();
        shell.reset();
        shell::waitForFileThread();
        seat.shutdown();
    }

    // Frames for `duration`, 17 ms apart, the loop turning between them.
    void frames(int duration = 0)
    {
        for (int elapsed = 0; elapsed <= duration; elapsed += 17) {
            test::runFor(std::chrono::milliseconds(17));
            compositor.updateTextures(renderer);
        }
    }

    uint32_t notify(const std::string &summary, const std::string &body = "Body",
                    std::vector<std::string> actions = {}, int32_t timeout = -1, dbus::VariantMap hints = {},
                    uint32_t replaces = 0)
    {
        const dbus::Reply reply = client.call(kService, kPath, kService, "Notify", std::string("Chat"), replaces,
                                              std::string(), summary, body, actions, hints, timeout);
        EXPECT_FALSE(reply.isError()) << reply.error().message;
        const uint32_t id = reply.value<uint32_t>().value_or(0);
        test::runFor(std::chrono::milliseconds(50));
        shell::waitForFileThread();
        test::runFor(std::chrono::milliseconds(20));
        frames();
        return id;
    }

    NotificationPopups &popups() { return ui->popups(); }
    NotificationCentre &centre() { return ui->centre(); }

    void move(double x, double y) { input.onPointerMotionAbsolute(input::Timestamp(std::chrono::milliseconds(++ms)), {x, y}); }

    void click(double x, double y)
    {
        move(x, y);
        input.onPointerButton(input::Timestamp(std::chrono::milliseconds(++ms)), BTN_LEFT, true);
        input.onPointerButton(input::Timestamp(std::chrono::milliseconds(++ms)), BTN_LEFT, false);
        frames();
    }

    void key(uint32_t code)
    {
        input.onKey(input::Timestamp(std::chrono::milliseconds(++ms)), code, true);
        input.onKey(input::Timestamp(std::chrono::milliseconds(++ms)), code, false);
        frames();
    }

    // The centre of a view, global.
    base::PointF centreOf(const ui::View *view, const base::Point &windowOrigin)
    {
        const ui::RectF rect = view->localRect();
        const ui::PointF point = view->mapToWindow({rect.width / 2, rect.height / 2});
        return {windowOrigin.x() + point.x, windowOrigin.y() + point.y};
    }

    std::vector<uint32_t> closedSignals(uint32_t reason)
    {
        std::vector<uint32_t> ids;
        for (const dbus::Message &message : client.signalsSeen) {
            uint32_t id = 0;
            uint32_t why = 0;
            if (message.member() == "NotificationClosed" && message.read(id, why) && why == reason)
                ids.push_back(id);
        }
        return ids;
    }

    void addHistory(const std::string &desktopEntry, const std::string &appName, const std::string &summary)
    {
        shell::Notification notification;
        notification.service = ":1.99";
        notification.appName = appName;
        notification.desktopEntry = desktopEntry;
        notification.summary = summary;
        notification.body = "Body of " + summary;
        notification.created = int64_t(1700000000000);
        notifications->history().add(notification);
    }
};

} // namespace

TEST(NotificationMotionTest, FollowsTheQtCurve)
{
    // cubic-bezier(0.25, 1, 0.5, 1): fast out, long settle.
    EXPECT_DOUBLE_EQ(0, shell::motionEasing(0));
    EXPECT_DOUBLE_EQ(1, shell::motionEasing(1));
    EXPECT_NEAR(0.5960, shell::motionEasing(0.2), 1e-3);
    EXPECT_NEAR(0.9341, shell::motionEasing(0.5), 1e-3);
    EXPECT_NEAR(0.875, shell::outCubic(0.5), 1e-9);
}

TEST(NotificationMarkupTest, ReadsStyledText)
{
    const ui::Color link {1, 2, 3, 255};
    const auto spans = ui::styledTextSpans("A <b>bold</b> &amp; <a href=\"x\">link</a><br/>next &#x263A;", link);
    std::string text;
    for (const auto &span : spans)
        text += span.text;
    EXPECT_EQ("A bold & link\nnext ☺", text);
    ASSERT_GE(spans.size(), 4u);
    EXPECT_EQ(700, spans[1].weight);
    EXPECT_EQ("bold", spans[1].text);
    EXPECT_EQ("link", spans[3].text);
    ASSERT_TRUE(spans[3].color);
    EXPECT_EQ(link, spans[3].color->fixedColor());
    EXPECT_EQ("x", spans[3].link);

    const auto styled = ui::styledTextSpans("<i>it</i><u>under</u><a href='https://e.org/a b'>go</a>", link);
    ASSERT_EQ(3u, styled.size());
    EXPECT_TRUE(styled[0].italic);
    EXPECT_TRUE(styled[1].underline);
    EXPECT_FALSE(styled[1].italic);
    EXPECT_EQ("https://e.org/a b", styled[2].link);
}

TEST(NotificationInkTest, PicksTheInkThatReads)
{
    EXPECT_FALSE(shell::inkIsDark(shell::rgba(0xF0 / 255.0, 0xF0 / 255.0, 0xF0 / 255.0, 0.82)));
    EXPECT_TRUE(shell::inkIsDark(shell::rgba(0x4D / 255.0, 0x4D / 255.0, 0x4D / 255.0, 0.65)));
    EXPECT_TRUE(shell::inkIsDark(ui::Color::rgb(0x2C2C2D)));
    EXPECT_EQ((ui::Color {0, 0, 0, 15}), shell::inkOn(ui::Color::rgb(0xFFFFFF), 0.06));
}

TEST_F(NotificationViewTest, ABannerShowsTopRightOnGlassAndTimesOut)
{
    const uint32_t id = notify("Hello", "A body line long enough to wrap onto a second line of the banner.");
    ASSERT_EQ(1u, popups().banners().size());
    const auto &banner = popups().banners().front();
    ASSERT_TRUE(banner->isShown());
    wm::Window *window = banner->window()->wmWindow();
    ASSERT_NE(nullptr, window);
    EXPECT_EQ(wm::WindowRole::Notification, window->role());
    EXPECT_EQ(wm::FocusPolicy::NoFocus, window->focusPolicy());
    // 6 from the work area's top right corner, 380 wide, as tall as the card.
    EXPECT_EQ(base::Rect(1440 - 6 - 380, kStatusBar + 6, 380, banner->card()->implicitHeight(380)),
              window->bounds());
    const compositor::Surface *surface = banner->window()->surface();
    EXPECT_TRUE(surface->glass().enabled);
    EXPECT_EQ(18, surface->backgroundBlurRadius());
    EXPECT_EQ(base::Rect(0, 0, 380, window->bounds().height()), surface->backgroundBlurRect());
    EXPECT_NEAR(0.6, surface->glass().tintAlpha, 1.0 / 256);

    // Seven seconds by default.
    EXPECT_EQ(std::chrono::milliseconds(7000), banner->timeout());
    EXPECT_TRUE(banner->isTimerRunning());

    // Hovering pauses it; the close button shows.
    move(1440 - 200, 60);
    frames();
    EXPECT_FALSE(banner->isTimerRunning());
    EXPECT_TRUE(banner->card()->closeButton()->isVisible());
    move(400, 400);
    frames();
    EXPECT_TRUE(banner->isTimerRunning());
    EXPECT_FALSE(banner->card()->closeButton()->isVisible());

    // Gone through the history when its time is up.
    EXPECT_EQ(0, notifications->history().count());
    notifications->banners().expired(id);
    frames();
    EXPECT_TRUE(popups().banners().empty());
    EXPECT_EQ(1, notifications->history().count());
}

TEST_F(NotificationViewTest, TimeoutsFollowTheRequestAndTheUrgency)
{
    notify("Short", "Body", {}, 1000);
    EXPECT_EQ(std::chrono::milliseconds(3000), popups().banners().back()->timeout());
    notify("Long", "Body", {}, 9000);
    EXPECT_EQ(std::chrono::milliseconds(9000), popups().banners().back()->timeout());
    notify("Sticky", "Body", {}, 0);
    EXPECT_EQ(std::chrono::milliseconds(0), popups().banners().back()->timeout());
    EXPECT_FALSE(popups().banners().back()->isTimerRunning());
    notify("Critical", "Body", {}, -1, {{"urgency", dbus::Variant(uint8_t(2))}});
    EXPECT_EQ(std::chrono::milliseconds(0), popups().banners().back()->timeout());

    // The short one expires on its own.
    test::runFor(std::chrono::milliseconds(3200));
    frames();
    EXPECT_EQ(3u, popups().banners().size());
}

TEST_F(NotificationViewTest, BannersStackNewestFirstAndGlide)
{
    notify("First");
    const auto *first = popups().banners()[0].get();
    const int height = first->height();
    EXPECT_EQ(base::Point(1054, 38), first->window()->geometry().topLeft());
    notify("Second");
    const auto *second = popups().banners()[1].get();
    EXPECT_EQ(base::Point(1054, 38), second->window()->geometry().topLeft());
    // The older one glides down under the new one.
    EXPECT_EQ(base::Point(1054, 38 + second->height() + 9), first->stackPosition());
    frames(40);
    EXPECT_GT(first->window()->geometry().y(), 38);
    EXPECT_LT(first->window()->geometry().y(), 38 + height + 9);
    frames(450);
    EXPECT_EQ(38 + second->height() + 9, first->window()->geometry().y());

    // No more than 60 % of the work area; the oldest that do not fit hide.
    // Four lines of body each: three of these fill the 60 %.
    const std::string tall = "A body long enough to fill four lines of the banner, and more than that, so that it "
                             "is cut at the end of the fourth line with an ellipsis rather than running on.";
    notify("Third", tall);
    notify("Fourth", tall);
    notify("Fifth", tall);
    frames(450);
    const int limit = kStatusBar + int((960 - kStatusBar - kDock) * 0.6);
    int shown = 0;
    for (const auto &banner : popups().banners()) {
        if (!banner->isShown())
            continue;
        ++shown;
        EXPECT_LE(banner->stackPosition().y() + banner->height(), limit);
    }
    EXPECT_LT(shown, 5);
    EXPECT_FALSE(popups().banners().front()->isShown());
    EXPECT_EQ(nullptr, popups().banners().front()->window()->wmWindow());
    // A sixth goes past the cap of five: the oldest moves to the history.
    notify("Sixth");
    EXPECT_EQ(5u, popups().banners().size());
    EXPECT_EQ(1, notifications->history().count());
}

TEST_F(NotificationViewTest, ClicksInvokeAndClose)
{
    const uint32_t plain = notify("Plain", "Body", {"default", "Open"});
    const auto *banner = popups().banners().back().get();
    const base::Point origin = banner->window()->geometry().topLeft();
    // A click on the card invokes the default action, then dismisses.
    click(origin.x() + 100, origin.y() + 50);
    EXPECT_TRUE(test::runUntil([&] { return !closedSignals(2).empty(); }));
    EXPECT_EQ(std::vector<uint32_t> {plain}, closedSignals(2));
    bool invoked = false;
    for (const dbus::Message &message : client.signalsSeen) {
        uint32_t id = 0;
        std::string key;
        if (message.member() == "ActionInvoked" && message.read(id, key))
            invoked = invoked || (id == plain && key == "default");
    }
    EXPECT_TRUE(invoked);
    frames();
    EXPECT_TRUE(popups().banners().empty());
    // Dismissed, not archived.
    EXPECT_EQ(0, notifications->history().count());

    const uint32_t withActions = notify("Actions", "Body", {"ok", "OK", "later", "Later"});
    banner = popups().banners().back().get();
    const auto rects = banner->card()->actionRects();
    ASSERT_EQ(2u, rects.size());
    // Two equal pills 6 apart across the card, 30 high, 12 from the bottom.
    EXPECT_DOUBLE_EQ(rects[0].width, rects[1].width);
    EXPECT_DOUBLE_EQ(12, rects[0].x);
    EXPECT_DOUBLE_EQ(rects[0].x + rects[0].width + 6, rects[1].x);
    EXPECT_DOUBLE_EQ(30, rects[0].height);
    EXPECT_DOUBLE_EQ(banner->height() - 12 - 30, rects[0].y);
    const base::Point bannerOrigin = banner->window()->geometry().topLeft();
    client.signalsSeen.clear();
    click(bannerOrigin.x() + rects[1].x + 10, bannerOrigin.y() + rects[1].y + 10);
    EXPECT_TRUE(test::runUntil([&] { return !closedSignals(2).empty(); }));
    bool later = false;
    for (const dbus::Message &message : client.signalsSeen) {
        uint32_t id = 0;
        std::string key;
        if (message.member() == "ActionInvoked" && message.read(id, key))
            later = later || (id == withActions && key == "later");
    }
    EXPECT_TRUE(later);

    // The close button, there under the pointer.
    const uint32_t closable = notify("Close me");
    banner = popups().banners().back().get();
    const base::Point at = banner->window()->geometry().topLeft();
    move(at.x() + 200, at.y() + 40);
    frames();
    ASSERT_TRUE(banner->card()->closeButton()->isVisible());
    // 7 down and 7 in from the trailing edge.
    const ui::RectF closeRect = banner->card()->closeButton()->geometry();
    EXPECT_DOUBLE_EQ(380 - 7 - 24, closeRect.x);
    EXPECT_DOUBLE_EQ(7, closeRect.y);
    EXPECT_DOUBLE_EQ(24, closeRect.width);
    client.signalsSeen.clear();
    click(at.x() + 380 - 7 - 12, at.y() + 7 + 12);
    EXPECT_TRUE(test::runUntil([&] { return !closedSignals(2).empty(); }));
    EXPECT_EQ(std::vector<uint32_t> {closable}, closedSignals(2));
}

TEST_F(NotificationViewTest, AReplacementRestartsTheCountdownAndGlidesTheProgress)
{
    const uint32_t id = notify("Copying", "1 of 4", {}, -1, {{"value", dbus::Variant(int32_t(25))}});
    const auto *banner = popups().banners().back().get();
    const int height = banner->height();
    notify("Copying", "2 of 4", {}, -1, {{"value", dbus::Variant(int32_t(50))}}, id);
    ASSERT_EQ(1u, popups().banners().size());
    EXPECT_EQ(banner, popups().banners().back().get());
    EXPECT_EQ("2 of 4", banner->notification().body);
    EXPECT_EQ(height, banner->height());
    EXPECT_TRUE(banner->isTimerRunning());
}

TEST_F(NotificationViewTest, TheCentreShowsProgressFromTheHistory)
{
    notify("Updating", "Downloading 40%", {}, 0, {{"value", dbus::Variant(int32_t(40))}});
    centre().open(statusBar.get(), base::Point(1440 - 120, kStatusBar));
    frames();
    ASSERT_EQ(1u, centre().rows().size());
    EXPECT_EQ(40, centre().rows()[0]->card()->notification().value);
}

TEST_F(NotificationViewTest, TheCentreOpensUnderTheBarAndTakesTheBanners)
{
    notify("On screen");
    ASSERT_EQ(1u, popups().banners().size());
    centre().open(statusBar.get(), base::Point(1440 - 120, kStatusBar));
    frames();
    ASSERT_TRUE(centre().isOpen());
    // 6 from the right edge, the work area's height less 6 at either end.
    EXPECT_EQ(base::Rect(1054, kStatusBar + 6, 380, 960 - kStatusBar - kDock - 12), centre().geometry());
    shell::ShellWindow *window = centre().shellWindow();
    ASSERT_NE(nullptr, window);
    ASSERT_NE(nullptr, window->wmWindow());
    EXPECT_TRUE(window->isPopup());
    EXPECT_EQ(centre().geometry(), window->wmWindow()->bounds());
    EXPECT_TRUE(window->surface()->glass().enabled);
    EXPECT_FLOAT_EQ(shell::glassFixed(20.0), window->surface()->glass().blurSigma);
    EXPECT_FLOAT_EQ(shell::glassFixed(2.4), window->surface()->glass().saturation);
    EXPECT_EQ(18, window->surface()->backgroundBlurRadius());

    // The banner went into the history, where the centre shows it.
    frames();
    EXPECT_TRUE(popups().banners().empty());
    ASSERT_EQ(1u, centre().rows().size());
    EXPECT_EQ(1, centre().liveCount());
    // While open, new notifications go straight to the history.
    notify("While open");
    EXPECT_TRUE(popups().banners().empty());
    EXPECT_EQ(2, notifications->history().count());
    frames();
    EXPECT_EQ(2u, centre().rows().size());

    // A left edge further in is kept.
    centre().close();
    frames();
    EXPECT_FALSE(centre().isOpen());
    EXPECT_FALSE(notifications->banners().bannersSuppressed());
    centre().open(statusBar.get(), base::Point(900, kStatusBar));
    frames();
    EXPECT_EQ(900, centre().geometry().x());

    // Escape closes it, as does a press outside.
    key(KEY_ESC);
    EXPECT_FALSE(centre().isOpen());
    centre().toggle(statusBar.get(), base::Point(900, kStatusBar));
    frames();
    EXPECT_TRUE(centre().isOpen());
    click(200, 400);
    EXPECT_FALSE(centre().isOpen());
    centre().toggle(statusBar.get(), base::Point(900, kStatusBar));
    centre().toggle(statusBar.get(), base::Point(900, kStatusBar));
    EXPECT_FALSE(centre().isOpen());
}

TEST_F(NotificationViewTest, TheCentreStacksEachApplicationAndFansItOut)
{
    addHistory("org.example.Mail", "Mail", "Mail 1");
    addHistory("org.example.Chat", "Chat", "Chat 1");
    addHistory("org.example.Chat", "Chat", "Chat 2");
    addHistory("org.example.Chat", "Chat", "Chat 3");
    centre().open(statusBar.get(), base::Point(1440 - 120, kStatusBar));
    frames(100);
    const auto &rows = centre().rows();
    ASSERT_EQ(4u, rows.size());
    // Chat's newest leads a stack of the two others; Mail stands alone.
    EXPECT_EQ("Chat 3", rows[0]->notification().summary);
    EXPECT_TRUE(rows[0]->isStacked());
    EXPECT_EQ(2, rows[0]->stackCount);
    EXPECT_EQ(2, rows[0]->peekCount());
    EXPECT_FALSE(rows[1]->isShown());
    EXPECT_EQ(0, rows[1]->targetHeight());
    EXPECT_EQ("Mail 1", rows[3]->notification().summary);
    EXPECT_FALSE(rows[3]->isStacked());
    // The stack: the card, two 8 px strips and the 9 px gap.
    const double card = rows[0]->card()->implicitHeight(356);
    EXPECT_DOUBLE_EQ(card + 16 + 9, rows[0]->targetHeight());
    EXPECT_DOUBLE_EQ(card + 16 + 9, rows[3]->targetY());
    EXPECT_EQ(4, centre().liveCount());
    EXPECT_TRUE(centre().clearButton()->isVisible());

    // A click on the stack fans it out under a header.
    const base::Point origin = centre().geometry().topLeft();
    click(centreOf(rows[0]->card(), origin).x(), centreOf(rows[0]->card(), origin).y());
    EXPECT_TRUE(centre().isExpanded("org.example.Chat"));
    EXPECT_TRUE(centre().isInMotion());
    ASSERT_EQ(1u, centre().groupHeaders().size());
    const auto *header = centre().groupHeaders().front();
    EXPECT_TRUE(header->isShown());
    EXPECT_FALSE(rows[0]->isStacked());
    EXPECT_TRUE(rows[1]->isShown());
    EXPECT_DOUBLE_EQ(35, rows[0]->targetY());
    frames(450);
    EXPECT_DOUBLE_EQ(35, rows[0]->geometry().y);
    EXPECT_DOUBLE_EQ(1, rows[1]->contentOpacity());

    // "Show Less" folds it back.
    click(centreOf(header->showLessButton(), origin).x(), centreOf(header->showLessButton(), origin).y());
    EXPECT_FALSE(centre().isExpanded("org.example.Chat"));
    EXPECT_TRUE(rows[0]->isStacked());
    frames(450);

    // A click on a single card shows its whole body.
    click(centreOf(rows[3]->card(), origin).x(), centreOf(rows[3]->card(), origin).y());
    EXPECT_TRUE(rows[3]->isDetailed());
    click(centreOf(rows[3]->card(), origin).x(), centreOf(rows[3]->card(), origin).y());
    EXPECT_FALSE(rows[3]->isDetailed());
}

TEST_F(NotificationViewTest, ALongGroupGlidesOnlyTheCardsInView)
{
    for (int i = 1; i <= 30; ++i)
        addHistory("org.example.Chat", "Chat", "Chat " + std::to_string(i));
    centre().open(statusBar.get(), base::Point(1440 - 120, kStatusBar));
    frames(100);
    const auto &rows = centre().rows();
    ASSERT_EQ(30u, rows.size());
    const base::Point origin = centre().geometry().topLeft();
    click(centreOf(rows[0]->card(), origin).x(), centreOf(rows[0]->card(), origin).y());
    ASSERT_TRUE(centre().isExpanded("org.example.Chat"));

    // In view: on its way.
    EXPECT_LT(rows[1]->geometry().y, rows[1]->targetY());
    // Past the bottom of the list: there already, whole.
    const shell::NotificationCentre::Row *last = rows.back();
    EXPECT_GT(last->targetY(), centre().geometry().height());
    EXPECT_DOUBLE_EQ(last->targetY(), last->geometry().y);
    EXPECT_DOUBLE_EQ(1, last->contentOpacity());
    frames(450);
    EXPECT_DOUBLE_EQ(rows[1]->targetY(), rows[1]->geometry().y);

    // The list scrolls down to them.
    const base::Point at = centreOf(rows[1]->card(), origin).toPoint();
    move(at.x(), at.y());
    for (int i = 0; i < 10; ++i)
        input.onPointerAxis(input::Timestamp(std::chrono::milliseconds(++ms)), {0, 10}, true);
    frames(450);
    EXPECT_LT(rows[1]->mapToWindow({0, 0}).y, 0);
}

TEST_F(NotificationViewTest, TheCentreDismissesAfterTheCardsFade)
{
    addHistory("org.example.Mail", "Mail", "Mail 1");
    addHistory("org.example.Chat", "Chat", "Chat 1");
    addHistory("org.example.Chat", "Chat", "Chat 2");
    centre().open(statusBar.get(), base::Point(1440 - 120, kStatusBar));
    frames(100);
    const base::Point origin = centre().geometry().topLeft();
    auto rows = centre().rows();
    ASSERT_EQ(3u, rows.size());

    // The lone card's close button: one click, a fade, then the history.
    shell::NotificationCentre::Row *mail = rows[2];
    move(centreOf(mail->card(), origin).x(), centreOf(mail->card(), origin).y());
    frames();
    ASSERT_TRUE(mail->card()->closeButton()->isVisible());
    const base::PointF close = centreOf(mail->card()->closeButton(), origin);
    click(close.x(), close.y());
    EXPECT_TRUE(mail->dismissing);
    frames(100);
    EXPECT_LT(mail->contentOpacity(), 1);
    EXPECT_EQ(3, notifications->history().count());
    test::runFor(std::chrono::milliseconds(700));
    frames();
    EXPECT_EQ(2, notifications->history().count());
    ASSERT_EQ(2u, centre().rows().size());

    // The stack's close asks first: the first click arms it as "Clear".
    rows = centre().rows();
    move(centreOf(rows[0]->card(), origin).x(), centreOf(rows[0]->card(), origin).y());
    frames();
    shell::ClearButton *stackClose = rows[0]->card()->closeButton();
    const base::PointF stackAt = centreOf(stackClose, origin);
    click(stackAt.x(), stackAt.y());
    EXPECT_TRUE(stackClose->isArmed());
    frames(300);
    EXPECT_DOUBLE_EQ(1, stackClose->progress());
    // A press elsewhere backs it out.
    click(origin.x() + 190, origin.y() + 20);
    EXPECT_FALSE(stackClose->isArmed());

    // The header's clear button: arm, then clear everything.
    const base::PointF clear = centreOf(centre().clearButton(), origin);
    click(clear.x(), clear.y());
    EXPECT_TRUE(centre().clearButton()->isArmed());
    frames(300);
    const base::PointF armed = centreOf(centre().clearButton(), origin);
    click(armed.x(), armed.y());
    test::runFor(std::chrono::milliseconds(700));
    frames();
    EXPECT_EQ(0, notifications->history().count());
    EXPECT_TRUE(centre().rows().empty());
    EXPECT_EQ(0, centre().liveCount());
    EXPECT_FALSE(centre().clearButton()->isVisible());
}

TEST_F(NotificationViewTest, ClosingCommitsPendingRemovalsAndFoldsGroups)
{
    addHistory("org.example.Chat", "Chat", "Chat 1");
    addHistory("org.example.Chat", "Chat", "Chat 2");
    addHistory("org.example.Mail", "Mail", "Mail 1");
    centre().open(statusBar.get(), base::Point(1440 - 120, kStatusBar));
    frames(100);
    const base::Point origin = centre().geometry().topLeft();
    auto rows = centre().rows();
    // Mail is newest: first. Expand Chat, show Mail's whole body.
    click(centreOf(rows[1]->card(), origin).x(), centreOf(rows[1]->card(), origin).y());
    EXPECT_TRUE(centre().isExpanded("org.example.Chat"));
    click(centreOf(rows[0]->card(), origin).x(), centreOf(rows[0]->card(), origin).y());
    EXPECT_TRUE(rows[0]->isDetailed());
    // Dismiss Mail and close before the removal lands.
    move(centreOf(rows[0]->card(), origin).x(), centreOf(rows[0]->card(), origin).y());
    frames();
    const base::PointF close = centreOf(rows[0]->card()->closeButton(), origin);
    click(close.x(), close.y());
    centre().close();
    frames();
    EXPECT_EQ(2, notifications->history().count());
    EXPECT_FALSE(centre().isExpanded("org.example.Chat"));
    centre().open(statusBar.get(), base::Point(1440 - 120, kStatusBar));
    frames(100);
    ASSERT_EQ(2u, centre().rows().size());
    EXPECT_TRUE(centre().rows()[0]->isStacked());
    EXPECT_FALSE(centre().rows()[0]->isDetailed());
    EXPECT_TRUE(centre().groupHeaders().empty());
}
