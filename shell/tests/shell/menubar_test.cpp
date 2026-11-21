// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The status bar's menu bar as the compositor draws it, in a bar of the
// test's own: entries, places, opening, switching, keys, the user's, the
// application's and the desktop's menus, on a private session bus.

#include <algorithm>
#include <chrono>
#include <cmath>
#include <map>
#include <tuple>

#include <linux/input-event-codes.h>
#include <unistd.h>

#include <gtest/gtest.h>

#include "compositor/compositor.h"
#include "dbus_test_client.h"
#include "fake_renderer.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "loop_test_util.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "shell/menubar/menu_bar_view.h"
#include "shell/shell.h"
#include "shell/shell_data.h"
#include "shell/shell_host.h"
#include "shell/shell_window.h"
#include "shell/statusbar/application_menu.h"
#include "shell/statusbar/bar_item.h"
#include "shell/statusbar/dbus_menu.h"
#include "shell/statusbar/status_bar.h"
#include "shell_test_util.h"
#include "temporary_directory.h"
#include "ui/embedded_window.h"
#include "ui/menu.h"
#include "wm/window.h"
#include "wm/window_manager.h"

using namespace Lingmo;
namespace dbus = base::dbus;

namespace {

input::Timestamp at(int ms)
{
    return input::Timestamp(std::chrono::milliseconds(ms));
}

const base::Rect kOutput(0, 0, 1440, 960);

using Layout = std::tuple<int32_t, dbus::VariantMap, std::vector<dbus::Variant>>;

dbus::Variant layout(int32_t id, dbus::VariantMap properties, std::vector<dbus::Variant> children = {})
{
    return dbus::Variant(Layout(id, std::move(properties), std::move(children)));
}

dbus::Variant label(const std::string &text)
{
    return dbus::Variant(text);
}

dbus::Variant submenu()
{
    return dbus::Variant(std::string("submenu"));
}

// A menu exporter of the test's own; every call and event is recorded.
class FakeMenuExporter
{
public:
    FakeMenuExporter(dbus::Connection *bus, const std::string &path)
        : object(bus, path, "com.canonical.dbusmenu")
    {
        object.addMethod("GetLayout", "iias", "u(ia{sv}av)", [this](dbus::MethodCall &call) {
            int32_t parent = 0;
            int32_t depth = 0;
            std::vector<std::string> names;
            call.read(parent, depth, names);
            calls.push_back("GetLayout " + std::to_string(parent));
            const auto it = layouts.find(parent);
            if (it == layouts.end()) {
                call.replyError("com.canonical.dbusmenu.Error", "No such item");
                return;
            }
            call.replyWith({dbus::Variant(revision), it->second});
        });
        object.addMethod("AboutToShow", [this](int32_t id) {
            calls.push_back("AboutToShow " + std::to_string(id));
            return false;
        });
        object.addMethod("Event", "isvu", "", [this](dbus::MethodCall &call) {
            int32_t id = 0;
            std::string event;
            call.read(id, event);
            calls.push_back("Event " + std::to_string(id) + " " + event);
            call.reply();
        });
        object.addSignal("LayoutUpdated", "ui");
        object.addSignal("ItemsPropertiesUpdated", "a(ia{sv})a(ias)");
        object.addSignal("ItemActivationRequested", "iu");
        EXPECT_TRUE(object.publish());
    }

    int count(const std::string &what) const { return int(std::count(calls.begin(), calls.end(), what)); }
    bool called(const std::string &what) const { return count(what) > 0; }

    dbus::Object object;
    std::map<int32_t, dbus::Variant> layouts;
    std::vector<std::string> calls;
    uint32_t revision = 1;
};

// The desktop's File and Edit menus.
void exportDesktopMenus(FakeMenuExporter &exporter)
{
    exporter.layouts[0] = layout(0, {},
                                 {layout(1, {{"label", label("File")}, {"children-display", submenu()}}),
                                  layout(2, {{"label", label("Edit")}, {"children-display", submenu()}})});
    exporter.layouts[1] = layout(1, {}, {layout(10, {{"label", label("New Window")}}),
                                         layout(11, {{"label", label("New Folder")}}),
                                         layout(12, {{"type", dbus::Variant(std::string("separator"))}}),
                                         layout(13, {{"label", label("Properties")}})});
    exporter.layouts[2] = layout(2, {}, {layout(20, {{"label", label("Undo")}, {"enabled", dbus::Variant(false)}}),
                                         layout(21, {{"label", label("Select All")}})});
}

// The status bar's part the menu bar needs: a 32 px panel along the top, its
// slot from the left edge clipped to 1000 px, and a press elsewhere on the
// bar closing the menus.
class TestBar final : public shell::Component, public shell::BarContext
{
public:
    explicit TestBar(shell::Shell *shell)
    {
        window = std::make_unique<shell::ShellWindow>(shell, wm::WindowRole::Panel);
        window->setFocusPolicy(wm::FocusPolicy::NoFocus);
        window->window()->setBackgroundColor(ui::Color {0, 0, 0, 0});
        auto root = std::make_unique<Root>(this);
        this->root = root.get();
        slot = root->addChild<ui::View>();
        slot->setClipsChildren(true);
        window->setContentView(std::move(root));
        window->setGeometry({0, 0, kOutput.width(), 32});
        window->setExclusiveZone(wm::ScreenEdge::Top, 32);
        window->show();
    }

    ~TestBar() override
    {
        if (menuBar)
            std::unique_ptr<ui::View> gone = slot->takeChild(menuBar);
        window.reset();
    }

    void setMenuBar(std::unique_ptr<shell::StatusBarMenuBar> bar)
    {
        menuBar = bar.get();
        slot->addChild(std::move(bar));
        root->requestLayout();
    }

    shell::StatusBarMenuBarHost host(shell::StatusBar *statusBar)
    {
        shell::StatusBarMenuBarHost host;
        host.bar = this;
        host.statusBar = statusBar;
        host.closePanels = [this] { ++panelCloses; };
        host.closeTrayMenu = [this] { ++trayCloses; };
        host.availableGeometry = [] { return ui::RectF {0, 32, double(kOutput.width()), double(kOutput.height() - 32)}; };
        return host;
    }

    bool lightContents() const override { return light; }

    std::unique_ptr<shell::ShellWindow> window;
    ui::View *slot = nullptr;
    shell::StatusBarMenuBar *menuBar = nullptr;
    bool light = false;
    int panelCloses = 0;
    int trayCloses = 0;

private:
    class Root final : public ui::View
    {
    public:
        explicit Root(TestBar *bar)
            : m_bar(bar)
        {
        }

        bool acceptsPointer() const override { return true; }

    protected:
        void layout() override
        {
            m_bar->slot->setGeometry({0, 0, 1000, geometry().height});
            if (m_bar->menuBar)
                m_bar->menuBar->setGeometry({0, 0, 1000, geometry().height});
        }

        void pointerPress(const ui::PointerEvent &) override
        {
            if (m_bar->menuBar)
                m_bar->menuBar->closeMenu();
        }

    private:
        TestBar *m_bar;
    };

    Root *root = nullptr;
};

class MenuBarTest : public testing::Test
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
    test::FakeTaskWindows *windows = nullptr;
    std::unique_ptr<shell::Shell> shell;
    std::unique_ptr<shell::StatusBar> statusBar;
    TestBar *bar = nullptr;
    shell::MenuBarView *view = nullptr;
    int time = 0;
    base::PointF lastClick {-100, -100};
    uint32_t lastButton = 0;

    void SetUp() override
    {
        scene.setSize(kOutput.size());
        manager.setOutputBounds(kOutput);
        input.setBounds(kOutput.size());
        input.setEventSink(&manager);
        shell::ShellEnvironment environment {&compositor, &manager, &seat};
        environment.bus = dbus::Connection::session(core::mainLoop());
        environment.servicesSettings = directory.filePath("services.conf");
        shell = std::make_unique<shell::Shell>(environment);
        auto apps = std::make_unique<test::FakeApplications>();
        applications = apps.get();
        applications->addSimple("terminal", "Terminal", "utilities-terminal");
        applications->scan();
        shell->setApplications(std::move(apps));
        auto tasks = std::make_unique<test::FakeTaskWindows>();
        windows = tasks.get();
        shell->setTaskWindows(std::move(tasks));
        statusBar = std::make_unique<shell::StatusBar>(environment.bus, nullptr, nullptr,
                                                       directory.filePath("shell.conf"));
        ASSERT_TRUE(statusBar->registerServices());
        bar = shell->addComponent(std::make_unique<TestBar>(shell.get()));
        auto menuBar = std::make_unique<shell::MenuBarView>(shell.get(), bar->host(statusBar.get()));
        view = menuBar.get();
        bar->setMenuBar(std::move(menuBar));
        frame();
    }

    void TearDown() override
    {
        shell.reset();
        statusBar.reset();
        seat.shutdown();
    }

    void frame()
    {
        test::processPendingEvents();
        compositor.updateTextures(renderer);
    }

    void move(double x, double y) { input.onPointerMotionAbsolute(at(time += 5), {x, y}); }

    void click(double x, double y, uint32_t button = BTN_LEFT)
    {
        // Two clicks on one spot within the toolkit's 400 ms are a double click.
        if (button == lastButton && std::abs(x - lastClick.x()) < 8 && std::abs(y - lastClick.y()) < 8)
            test::runFor(std::chrono::milliseconds(420));
        lastClick = {x, y};
        lastButton = button;
        move(x, y);
        input.onPointerButton(at(time += 5), button, true);
        input.onPointerButton(at(time += 5), button, false);
        frame();
    }

    void key(uint32_t code)
    {
        input.onKey(at(time += 5), code, true);
        input.onKey(at(time += 5), code, false);
        frame();
    }

    std::vector<std::string> titles() const
    {
        std::vector<std::string> result;
        for (const shell::MenuBarModel::Row &row : view->model().rows())
            result.push_back(row.title);
        return result;
    }

    // An entry's rect in the bar.
    ui::RectF entryRect(size_t index) const { return view->entry(index)->geometry(); }

    base::PointF entryCentre(size_t index) const
    {
        const ui::RectF rect = entryRect(index);
        return {rect.x + rect.width / 2, rect.y + rect.height / 2};
    }

    // The open menu's window as the window manager placed it.
    std::optional<base::Rect> menuBounds() const
    {
        ui::Menu *menu = view->openMenu();
        if (!menu || !menu->window())
            return std::nullopt;
        shell::ShellWindow *window =
                shell->host()->windowFor(static_cast<ui::EmbeddedWindow *>(menu->window()));
        if (!window || !window->wmWindow())
            return std::nullopt;
        return window->wmWindow()->bounds();
    }

    // The desktop as the file manager has it: a Desktop window of its
    // process, whose menus the process exports and registers as window 0.
    std::unique_ptr<shell::ShellWindow> showDesktop(test::DBusTestClient &client, FakeMenuExporter &exporter,
                                                    int64_t pid = getpid(), const std::string &path = "/MenuBar")
    {
        exportDesktopMenus(exporter);
        auto desktop = std::make_unique<shell::ShellWindow>(shell.get(), wm::WindowRole::Desktop);
        desktop->setGeometry(kOutput);
        desktop->show();
        desktop->wmWindow()->setProcessId(pid);
        client.call("com.canonical.AppMenu.Registrar", "/com/canonical/AppMenu/Registrar",
                    "com.canonical.AppMenu.Registrar", "RegisterWindow", uint32_t(0),
                    dbus::ObjectPath {path});
        EXPECT_TRUE(test::runUntil([this] { return view->model().count() == 4; }));
        frame();
        return desktop;
    }
};

} // namespace

TEST_F(MenuBarTest, TheUserEntryAloneWithNothingFocusedAndNoDesktopMenu)
{
    // Nothing focused is the desktop: its name, with no menus of its own yet.
    EXPECT_TRUE(view->model().isDesktop());
    ASSERT_EQ(titles(), (std::vector<std::string> {"", "Desktop"}));
    EXPECT_EQ(shell::MenuBarModel::Kind::User, view->model().rows()[0].kind);
    EXPECT_EQ(shell::MenuBarModel::Kind::Application, view->model().rows()[1].kind);

    // The user's entry: 46 wide, its plate 9 from the screen edge.
    EXPECT_EQ(ui::RectF(0, 0, 46, 32), entryRect(0));
    EXPECT_EQ(9, view->entry(0)->leftInset());
    EXPECT_EQ(3, view->entry(0)->rightInset());
    // The next abuts it, 24 wider than its title.
    EXPECT_EQ(46, entryRect(1).x);
    EXPECT_EQ(3, view->entry(1)->leftInset());
}

TEST_F(MenuBarTest, TheUserMenuOpensUnderItsPlateAndClosesOnASecondClick)
{
    click(entryCentre(0).x(), 16);
    ASSERT_EQ(0, view->currentIndex());
    EXPECT_TRUE(view->entry(0)->isChecked());
    EXPECT_EQ(1, bar->panelCloses);
    EXPECT_EQ(1, bar->trayCloses);
    ui::Menu *menu = view->openMenu();
    ASSERT_NE(nullptr, menu);
    EXPECT_TRUE(menu->isOpen());
    // Plate's left edge, the popup margin under the bar.
    const auto bounds = menuBounds();
    ASSERT_TRUE(bounds);
    EXPECT_EQ(9, bounds->x());
    EXPECT_EQ(38, bounds->y());
    EXPECT_EQ(1u, manager.popupGrabs().size());

    // Again on the open entry: closed.
    click(entryCentre(0).x(), 16);
    EXPECT_EQ(-1, view->currentIndex());
    EXPECT_FALSE(menu->isOpen());
    EXPECT_FALSE(view->entry(0)->isChecked());
    EXPECT_TRUE(manager.popupGrabs().empty());

    // A second click soon enough to make a double click closes it too.
    click(entryCentre(0).x(), 16);
    ASSERT_EQ(0, view->currentIndex());
    input.onPointerButton(at(time += 5), BTN_LEFT, true);
    input.onPointerButton(at(time += 5), BTN_LEFT, false);
    frame();
    EXPECT_EQ(-1, view->currentIndex());
    EXPECT_FALSE(menu->isOpen());
    EXPECT_TRUE(manager.popupGrabs().empty());

    // The right button opens nothing.
    click(entryCentre(0).x(), 16, BTN_RIGHT);
    EXPECT_EQ(-1, view->currentIndex());

    // Escape closes it; so does a press on the bar between the entries.
    click(entryCentre(0).x(), 16);
    ASSERT_EQ(0, view->currentIndex());
    key(KEY_ESC);
    EXPECT_EQ(-1, view->currentIndex());
    EXPECT_FALSE(view->entry(0)->isChecked());
    click(entryCentre(0).x(), 16);
    ASSERT_EQ(0, view->currentIndex());
    click(1200, 16);
    EXPECT_EQ(-1, view->currentIndex());
    // A press outside the bar and the menus ends the grab.
    click(entryCentre(0).x(), 16);
    ASSERT_EQ(0, view->currentIndex());
    click(700, 500);
    EXPECT_EQ(-1, view->currentIndex());
    EXPECT_TRUE(manager.popupGrabs().empty());
}

TEST_F(MenuBarTest, TheDesktopShowsItsExportedMenus)
{
    test::DBusTestClient desktopProcess;
    FakeMenuExporter exporter(desktopProcess.connection.get(), "/MenuBar");
    const auto desktop = showDesktop(desktopProcess, exporter);
    ASSERT_EQ(titles(), (std::vector<std::string> {"", "Desktop", "File", "Edit"}));
    // The application's name is bold, its menus regular; File's plate starts
    // where its entry does, plus the inset.
    const double file = entryRect(2).x;
    EXPECT_EQ(entryRect(1).x + entryRect(1).width, file);
    // The submenus were fetched ahead.
    ASSERT_TRUE(test::runUntil([&] { return exporter.called("AboutToShow 1") && exporter.called("AboutToShow 2"); }));

    test::runFor(std::chrono::milliseconds(50));
    const int openedBefore = exporter.count("Event 1 opened");
    click(entryCentre(2).x(), 16);
    ASSERT_EQ(2, view->currentIndex());
    // One AboutToShow and one "opened" per opening.
    test::runFor(std::chrono::milliseconds(50));
    EXPECT_EQ(openedBefore + 1, exporter.count("Event 1 opened"));
    const auto bounds = menuBounds();
    ASSERT_TRUE(bounds);
    EXPECT_EQ(int(std::lround(file + 3)), bounds->x());
    EXPECT_EQ(38, bounds->y());
    ASSERT_TRUE(test::runUntil([&] { return exporter.called("Event 1 opened"); }));

    // Moving onto Edit while File is open switches to Edit's menu; the
    // grab stays with the bar.
    move(entryCentre(3).x(), 16);
    frame();
    ASSERT_EQ(3, view->currentIndex());
    EXPECT_FALSE(view->entry(2)->isChecked());
    EXPECT_TRUE(view->entry(3)->isChecked());
    EXPECT_EQ(int(std::lround(entryRect(3).x + 3)), menuBounds()->x());
    EXPECT_EQ(1u, manager.popupGrabs().size());
    ASSERT_TRUE(test::runUntil([&] { return exporter.called("Event 1 closed") && exporter.called("Event 2 opened"); }));

    // Right wraps around to the user's entry, Left comes back.
    key(KEY_RIGHT);
    EXPECT_EQ(0, view->currentIndex());
    key(KEY_LEFT);
    EXPECT_EQ(3, view->currentIndex());
    key(KEY_LEFT);
    EXPECT_EQ(2, view->currentIndex());
    EXPECT_EQ(1u, manager.popupGrabs().size());

    // Down lights New Window, Return chooses it: the exporter hears the
    // click and the menu goes.
    key(KEY_DOWN);
    key(KEY_ENTER);
    EXPECT_EQ(-1, view->currentIndex());
    EXPECT_TRUE(manager.popupGrabs().empty());
    EXPECT_TRUE(test::runUntil([&] { return exporter.called("Event 10 clicked"); }));

    // Without a menu open, moving along the bar opens nothing.
    move(entryCentre(2).x(), 16);
    move(entryCentre(3).x(), 16);
    EXPECT_EQ(-1, view->currentIndex());
}

TEST_F(MenuBarTest, TheDesktopsProcessOtherWindowsDoNotShowItsMenus)
{
    // The file manager's daemon draws the desktop and owns the browser
    // windows: a focused browser window shows no File/Edit of the desktop's.
    test::DBusTestClient desktopProcess;
    FakeMenuExporter exporter(desktopProcess.connection.get(), "/MenuBar");
    const auto desktop = showDesktop(desktopProcess, exporter);
    ASSERT_EQ(titles(), (std::vector<std::string> {"", "Desktop", "File", "Edit"}));

    shell::TaskWindow browser;
    browser.appId = "lingmo-filemanager";
    browser.title = "Home";
    browser.pid = getpid();
    const uint64_t id = windows->add(browser);
    windows->setActive(id);
    ASSERT_TRUE(test::runUntil([&] { return !view->model().isDesktop(); }));
    test::runFor(std::chrono::milliseconds(100));
    frame();
    const std::vector<std::string> shown = titles();
    EXPECT_EQ(2u, shown.size());
    EXPECT_EQ(shown.end(), std::find(shown.begin(), shown.end(), "File"));

    // Back on the desktop, its menus return.
    windows->setActive(0);
    ASSERT_TRUE(test::runUntil([&] { return view->model().isDesktop() && view->model().count() == 4; }));
    EXPECT_EQ(titles(), (std::vector<std::string> {"", "Desktop", "File", "Edit"}));
}

TEST_F(MenuBarTest, TheDesktopIsFoundByItsExportPath)
{
    // The session hands the file manager's desktop its Wayland connection as a
    // socket pair: its Desktop window carries another process's id.
    test::DBusTestClient desktopProcess;
    FakeMenuExporter exporter(desktopProcess.connection.get(), shell::MenuBarModel::kDesktopMenuPath);
    const auto desktop = showDesktop(desktopProcess, exporter, 1, shell::MenuBarModel::kDesktopMenuPath);
    ASSERT_EQ(titles(), (std::vector<std::string> {"", "Desktop", "File", "Edit"}));
    ASSERT_NE(nullptr, view->model().desktopMenu());
    EXPECT_EQ(desktopProcess.connection->uniqueName(), view->model().desktopMenu()->service());
}

TEST_F(MenuBarTest, AnApplicationShowsItsNameItsMenusAndTheirChanges)
{
    test::DBusTestClient application;
    FakeMenuExporter exporter(application.connection.get(), "/MenuBar");
    exporter.layouts[0] = layout(0, {},
                                 {layout(1, {{"label", label("_File")}, {"children-display", submenu()}}),
                                  layout(2, {{"label", label("_View")}, {"children-display", submenu()}}),
                                  layout(3, {{"label", label("Hidden")}, {"visible", dbus::Variant(false)}}),
                                  layout(4, {{"label", label("_Run")}})});
    exporter.layouts[1] = layout(1, {}, {layout(10, {{"label", label("_Open")}})});
    exporter.layouts[2] = layout(2, {}, {layout(20, {{"label", label("Zoom")}})});

    shell::TaskWindow terminal;
    terminal.appId = "terminal";
    terminal.title = "Terminal window";
    terminal.pid = 4242;
    terminal.menuService = application.connection->uniqueName();
    terminal.menuPath = "/MenuBar";
    const uint64_t id = windows->add(terminal);
    windows->setActive(id);
    ASSERT_TRUE(test::runUntil([&] { return view->model().count() == 5; }));
    frame();
    EXPECT_FALSE(view->model().isDesktop());
    // The entry's name; mnemonic markers gone; hidden menus have no entry.
    EXPECT_EQ(titles(), (std::vector<std::string> {"", "Terminal", "File", "View", "Run"}));

    // The application entry: Hide Terminal, Hide Others, Quit.
    click(entryCentre(1).x(), 16);
    ASSERT_EQ(1, view->currentIndex());
    key(KEY_DOWN);
    key(KEY_ENTER);
    ASSERT_FALSE(windows->requests.empty());
    EXPECT_EQ("hideApplication " + std::to_string(id), windows->requests.back());
    click(entryCentre(1).x(), 16);
    key(KEY_END);
    key(KEY_ENTER);
    EXPECT_EQ("quit " + std::to_string(id), windows->requests.back());

    // A plain action entry is triggered, not opened.
    click(entryCentre(4).x(), 16);
    EXPECT_EQ(-1, view->currentIndex());
    EXPECT_TRUE(test::runUntil([&] { return exporter.called("Event 4 clicked"); }));

    // The application asks for View (its Alt+V): it opens from the keyboard.
    exporter.object.emitSignal("ItemActivationRequested", int32_t(2), uint32_t(0));
    ASSERT_TRUE(test::runUntil([&] { return view->currentIndex() == 3; }));
    frame();
    ASSERT_TRUE(menuBounds());

    // A renamed menu renames its entry; the open one stays open.
    exporter.object.emitSignal("ItemsPropertiesUpdated",
                               std::vector<std::tuple<int32_t, dbus::VariantMap>> {
                                       {4, {{"label", dbus::Variant(std::string("_Go"))}}}},
                               std::vector<std::tuple<int32_t, std::vector<std::string>>> {});
    ASSERT_TRUE(test::runUntil([&] { return titles().back() == "Go"; }));
    EXPECT_EQ(3, view->currentIndex());
    EXPECT_TRUE(view->openMenu() && view->openMenu()->isOpen());

    // Another application takes the bar: the open menu goes.
    windows->setActive(0);
    ASSERT_TRUE(test::runUntil([&] { return view->model().isDesktop(); }));
    EXPECT_EQ(-1, view->currentIndex());
    EXPECT_TRUE(manager.popupGrabs().empty());
    EXPECT_EQ(titles(), (std::vector<std::string> {"", "Desktop"}));

    // The user's menu stays open across a switch.
    click(entryCentre(0).x(), 16);
    ASSERT_EQ(0, view->currentIndex());
    windows->setActive(id);
    ASSERT_TRUE(test::runUntil([&] { return view->model().count() == 5; }));
    EXPECT_EQ(0, view->currentIndex());
    EXPECT_TRUE(view->openMenu()->isOpen());
}

TEST_F(MenuBarTest, AnApplicationThatLeavesTakesItsOpenMenu)
{
    auto application = std::make_unique<test::DBusTestClient>();
    auto exporter = std::make_unique<FakeMenuExporter>(application->connection.get(), "/MenuBar");
    exporter->layouts[0] = layout(0, {}, {layout(1, {{"label", label("_File")}, {"children-display", submenu()}})});
    exporter->layouts[1] = layout(1, {}, {layout(10, {{"label", label("_Open")}})});

    shell::TaskWindow terminal;
    terminal.appId = "terminal";
    terminal.title = "Terminal window";
    terminal.pid = 4242;
    terminal.menuService = application->connection->uniqueName();
    terminal.menuPath = "/MenuBar";
    windows->setActive(windows->add(terminal));
    ASSERT_TRUE(test::runUntil([&] { return view->model().count() == 3; }));
    frame();
    click(entryCentre(2).x(), 16);
    ASSERT_EQ(2, view->currentIndex());
    ASSERT_TRUE(view->openMenu() && view->openMenu()->isOpen());

    // Its exporter leaves the bus with the menu up: the menu closes, and
    // nothing reaches the menu client that went with the exporter.
    exporter.reset();
    application.reset();
    ASSERT_TRUE(test::runUntil([&] { return view->currentIndex() == -1; }));
    frame();
    EXPECT_TRUE(manager.popupGrabs().empty());
}
