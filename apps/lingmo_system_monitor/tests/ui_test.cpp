// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/historygraph.h"
#include "icons.h"
#include "mainwindow.h"
#include "monitor.h"
#include "processdetails.h"
#include "processlist.h"
#include "resourcepage.h"

#include "base/event_loop.h"
#include "base/timer.h"
#include "ui/application.h"
#include "ui/icon.h"
#include "ui/clipboard.h"
#include "ui/dialog_window.h"
#include "ui/header_button.h"
#include "ui/keys.h"
#include "ui/segmented_control.h"
#include "ui/test_input.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "ui/window.h"

#include <gtest/gtest.h>

#include <cmath>
#include <ctime>
#include <typeinfo>
#include <filesystem>
#include <unistd.h>
#include <cstdio>
#include <sys/wait.h>
#include <signal.h>

using namespace Lingmo;
using namespace Lingmo::ui;
using namespace std::chrono_literals;

namespace {

bool runUntil(const std::function<bool()>& condition, std::chrono::milliseconds timeout = 5000ms)
{
    if (condition())
        return true;
    base::EventLoop& loop = Application::instance()->loop();
    base::Timer poll;
    poll.onTimeout = [&] {
        if (condition())
            loop.quit();
    };
    poll.start(5ms);
    base::Timer deadline;
    deadline.setSingleShot(true);
    deadline.onTimeout = [&] { loop.quit(); };
    deadline.start(timeout);
    loop.run();
    return condition();
}

View* find(View* view, const std::string& name)
{
    if (!view)
        return nullptr;
    if (view->name() == name)
        return view;
    for (const auto& child : view->children()) {
        if (View* found = find(child.get(), name))
            return found;
    }
    return nullptr;
}

RectF windowRect(const View* view)
{
    const PointF topLeft = view->mapToWindow({});
    return {topLeft.x, topLeft.y, view->geometry().width, view->geometry().height};
}

PointF center(const View* view)
{
    const RectF rect = windowRect(view);
    return {rect.x + rect.width / 2, rect.y + rect.height / 2};
}

// A window with its first sample in, at the given size.
struct Fixture
{
    explicit Fixture(SizeF size = {1040, 700}, ApplicationLookup lookup = {})
        : monitor(std::move(lookup))
        , window(monitor)
    {
        window.window().setScale(1.5);
        window.window().resize(size);
        EXPECT_TRUE(runUntil([this] { return monitor.ready(); }));
        layout();
    }

    void layout() { window.window().grab(); }
    View* root() { return window.window().contentView(); }
    View* view(const std::string& name) { return find(root(), name); }

    Monitor monitor;
    MainWindow window;
};

std::string ownProgram()
{
    return std::filesystem::canonical("/proc/self/exe").string();
}

} // namespace

TEST(UiTest, ResourcePages)
{
    for (const bool dark : {false, true}) {
        Theme::setCurrent(dark ? Theme::dark() : Theme::light());
        Fixture fixture;
        for (const SizeF size : {SizeF{1040, 700}, SizeF{840, 600}}) {
            fixture.window.window().resize(size);
            for (int page = 0; page <= 5; ++page) {
                fixture.window.setCurrentPage(page);
                fixture.layout();
                if (page == 0)
                    continue;
                View* chart = fixture.view("resourceChart");
                ASSERT_NE(chart, nullptr);
                EXPECT_GT(chart->geometry().width, 400);
                EXPECT_EQ(std::lround(windowRect(chart).x), 252);
                auto* graph = static_cast<HistoryGraph*>(fixture.view("historyGraph"));
                View* scale = fixture.view("chartScaleLabel");
                ASSERT_TRUE(graph && scale);
                EXPECT_GE(windowRect(graph).y - windowRect(scale).bottom(), 9.5);
                if (page == 1) {
                    graph->setValues({100, 100, 100});
                    graph->setTimestamps({0, 30000, 60000});
                    EXPECT_EQ(graph->effectiveMaximum(), 100.0);
                    EXPECT_EQ(size.width - windowRect(chart).right(), 12);
                }
            }
        }
    }
    Theme::setCurrent(Theme::light());
}

TEST(UiTest, KeyboardAndSelection)
{
    Fixture fixture;
    MainWindow& window = fixture.window;
    TestInput::setKeyboardFocus(window.window(), true);
    ProcessesPage* page = window.processesPage();
    auto* scope = static_cast<SegmentedControl*>(fixture.view("processScope"));
    auto* search = static_cast<TextField*>(fixture.view("processSearch"));
    ASSERT_TRUE(scope && search);
    const auto segment = [scope](int index) {
        return windowRect(scope).topLeft() + PointF{scope->geometry().width * (index + 0.5) / 3, 18};
    };
    TestInput::click(window.window(), segment(2));
    EXPECT_EQ(fixture.monitor.scope(), 2);
    TestInput::click(window.window(), segment(1));
    EXPECT_EQ(fixture.monitor.scope(), 1);

    window.setCurrentPage(2);
    TestInput::typeKey(window.window(), 'f', ControlModifier);
    EXPECT_EQ(window.currentPage(), 0);
    EXPECT_TRUE(search->hasFocus());
    TestInput::typeText(window.window(), std::to_string(getpid()));
    ASSERT_TRUE(runUntil([&] { return fixture.monitor.processes().count() > 0; }));
    fixture.layout();
    TestInput::click(window.window(), windowRect(page->listView()).topLeft() + PointF{80, 18});
    ASSERT_NE(window.selectedProcess(), nullptr);
    EXPECT_EQ(window.selectedProcess()->pid, getpid());
    fixture.monitor.sortBy("memory");
    EXPECT_EQ(window.selectedProcess()->pid, getpid());

    // Escape while searching clears the search and returns to the list.
    search->setFocus();
    TestInput::typeKey(window.window(), key::Escape);
    EXPECT_TRUE(search->text().empty());
    EXPECT_TRUE(page->listView()->hasFocus());

    search->setText(std::to_string(getpid()));
    fixture.layout();
    // The clear button sits 12 from the field's end.
    const RectF field = windowRect(search);
    TestInput::click(window.window(), {field.right() - 12 - 7, field.y + field.height / 2});
    EXPECT_TRUE(search->text().empty());

    // Up and Down move the selection; Return opens the details.
    fixture.layout();
    TestInput::click(window.window(), windowRect(page->listView()).topLeft() + PointF{80, 18});
    const int first = window.selectedProcess() ? window.selectedProcess()->pid : 0;
    TestInput::typeKey(window.window(), key::Down);
    ASSERT_NE(window.selectedProcess(), nullptr);
    EXPECT_NE(window.selectedProcess()->pid, first);
    TestInput::typeKey(window.window(), key::Up);
    EXPECT_EQ(window.selectedProcess()->pid, first);
    TestInput::typeKey(window.window(), key::Return);
    ASSERT_NE(window.details(), nullptr);
    ASSERT_NE(window.details()->window(), nullptr);
}

TEST(UiTest, ContextMenu)
{
    Fixture fixture;
    MainWindow& window = fixture.window;
    auto* search = static_cast<TextField*>(fixture.view("processSearch"));
    search->setText(std::to_string(getpid()));
    search->onTextChanged();
    ASSERT_TRUE(runUntil([&] { return window.processesPage()->list().count() > 0; }));
    fixture.layout();
    TestInput::click(window.window(), windowRect(window.processesPage()->listView()).topLeft() + PointF{80, 20},
                     MouseButton::Right);
    EXPECT_TRUE(window.contextMenu().isOpen());
    ASSERT_NE(window.selectedProcess(), nullptr);
    EXPECT_EQ(window.selectedProcess()->pid, getpid());
    // The monitor's own process is never offered for quitting.
    EXPECT_FALSE(Monitor::canManage(*window.selectedProcess()));
    EXPECT_FALSE(fixture.view("endProcessButton")->isEnabled());
    window.contextMenu().close();
    Clipboard::setText({});
    TestInput::click(window.window(), windowRect(window.processesPage()->listView()).topLeft() + PointF{80, 20},
                     MouseButton::Right);
    ASSERT_TRUE(window.contextMenu().isOpen());
    Window* menu = window.contextMenu().window();
    // Down to the second row, Copy Process ID.
    TestInput::typeKey(window.window(), key::Down);
    TestInput::typeKey(window.window(), key::Down);
    TestInput::typeKey(window.window(), key::Return);
    (void)menu;
    std::string copied;
    Clipboard::requestText([&copied](std::string text) { copied = std::move(text); });
    EXPECT_EQ(copied, std::to_string(getpid()));
    EXPECT_FALSE(window.contextMenu().isOpen());
}

TEST(UiTest, ApplicationView)
{
    const std::string program = ownProgram();
    ApplicationLookup lookup;
    lookup.resolve = [program](const std::string& name, const std::string& path) -> std::optional<DesktopApplication> {
        if (path == program || name == std::filesystem::path(program).filename().string())
            return DesktopApplication{std::filesystem::path(program).filename().string(), "Fixture Application", {}};
        return std::nullopt;
    };
    lookup.byId = [program](const std::string& id) -> std::optional<DesktopApplication> {
        if (id == std::filesystem::path(program).filename().string())
            return DesktopApplication{id, "Fixture Application", {}};
        return std::nullopt;
    };
    Fixture fixture({1040, 700}, lookup);
    MainWindow& window = fixture.window;
    auto* scope = static_cast<SegmentedControl*>(fixture.view("processScope"));
    TestInput::click(window.window(), windowRect(scope).topLeft() + PointF{scope->geometry().width / 6, 18});
    EXPECT_EQ(fixture.monitor.scope(), 0);
    auto* search = static_cast<TextField*>(fixture.view("processSearch"));
    search->setText("Fixture Application");
    search->onTextChanged();
    ASSERT_TRUE(runUntil([&] { return fixture.monitor.applications().count() == 1; }));
    fixture.layout();
    TestInput::click(window.window(), windowRect(window.processesPage()->listView()).topLeft() + PointF{80, 20},
                     MouseButton::Right);
    ASSERT_TRUE(window.contextMenu().isOpen());
    ASSERT_NE(window.selectedProcess(), nullptr);
    EXPECT_EQ(window.selectedProcess()->pid, getpid());
    window.contextMenu().close();
    EXPECT_FALSE(fixture.view("endProcessButton")->isEnabled());
    window.openDetails();
    ASSERT_NE(window.details(), nullptr);
    EXPECT_EQ(window.selectedProcess()->members.size(), 1u);
}

TEST(UiTest, LanguageChangeKeepsWhereTheUserWas)
{
    Fixture fixture;
    MainWindow& window = fixture.window;
    auto* search = static_cast<TextField*>(fixture.view("processSearch"));
    search->setText("lingmo");
    search->onTextChanged();
    window.setCurrentPage(3);
    fixture.layout();
    const View* before = fixture.view("processSearch");

    Application::instance()->languageChanged.emit();
    fixture.layout();
    EXPECT_NE(fixture.view("processSearch"), before);
    EXPECT_EQ(window.currentPage(), 3);
    ASSERT_NE(fixture.view("resourceChart"), nullptr);
    EXPECT_EQ(static_cast<TextField*>(fixture.view("processSearch"))->text(), "lingmo");
    EXPECT_EQ(fixture.monitor.processes().search(), "lingmo");
}

TEST(UiTest, TimestampedGraph)
{
    Window window;
    window.resize({202, 104});
    auto graph = std::make_unique<HistoryGraph>();
    HistoryGraph* view = graph.get();
    window.setContentView(std::move(graph));
    window.setBackgroundColor(Color::rgb(0xFFFFFF));
    view->setColor(Color::rgb(0xFF0000));
    view->setDuration(10000);
    view->setValues({0, 0, 100, 0});
    view->setTimestamps({0, 1000, 3000, 10000});
    const Image image = window.grab();
    int left = image.width();
    int right = -1;
    for (int y = 0; y < 6; ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const Color pixel = image.pixel(x, y);
            if (pixel.r > 200 && pixel.g < 120) {
                left = std::min(left, x);
                right = std::max(right, x);
            }
        }
    }
    ASSERT_GE(right, left);
    // The peak occurred seven seconds ago in a ten-second window.
    EXPECT_LT(std::abs((left + right) / 2.0 - 61.0), 5);
}

TEST(UiTest, MeasuredAlignment)
{
    Fixture fixture;
    View* applicationTitle = fixture.view("applicationTitle");
    View* pageTitle = fixture.view("pageTitle");
    ASSERT_TRUE(applicationTitle && pageTitle);
    EXPECT_EQ(windowRect(applicationTitle).y, windowRect(pageTitle).y);
    EXPECT_EQ(std::lround(windowRect(pageTitle).x), 252);

    View* search = fixture.view("processSearch");
    const double searchCenter = center(search).y;

    View* interval = fixture.view("refreshInterval");
    View* endButton = fixture.view("endProcessButton");
    View* table = fixture.view("processTable");
    View* list = fixture.view("processList");
    ASSERT_TRUE(interval && endButton && table && list);
    EXPECT_EQ(interval->geometry().size(), (SizeF{32, 32}));
    const RectF action = windowRect(endButton);
    EXPECT_EQ(action.height, search->geometry().height);
    EXPECT_NEAR(action.y + action.height / 2, searchCenter, 0.01);
    EXPECT_EQ(action.right(), windowRect(table).right());
    EXPECT_EQ(action.width, action.height);
    View* actionIcon = fixture.view("endProcessIcon");
    ASSERT_NE(actionIcon, nullptr);
    EXPECT_EQ(actionIcon->geometry().size(), (SizeF{16, 16}));
    EXPECT_NEAR(center(actionIcon).x, action.x + action.width / 2, 0.01);
    EXPECT_NEAR(center(actionIcon).y, searchCenter, 0.01);
    // The list's scroll bar is at the window's edge, beside the table.
    EXPECT_EQ(windowRect(list).right(), fixture.window.window().size().width);
    for (const char* name : {"cpuSummary", "memorySummary", "networkSummary", "diskSummary"}) {
        View* summary = fixture.view(name);
        ASSERT_NE(summary, nullptr);
        EXPECT_EQ(std::lround(windowRect(summary).x), 12);
        EXPECT_EQ(summary->geometry().width, 216);
        EXPECT_EQ(summary->geometry().height, 96);
    }
}

TEST(UiTest, PagesFromTheKeyboard)
{
    Fixture fixture;
    MainWindow& window = fixture.window;
    TestInput::setKeyboardFocus(window.window(), true);
    // Space on a focused card opens its page; Escape returns to the processes.
    fixture.view("memorySummary")->setFocus(FocusReason::Keyboard);
    TestInput::typeKey(window.window(), key::Space);
    EXPECT_EQ(window.currentPage(), 2);
    EXPECT_NE(window.resourcePage(), nullptr);
    TestInput::typeKey(window.window(), key::Escape);
    EXPECT_EQ(window.currentPage(), 0);
    EXPECT_EQ(window.resourcePage(), nullptr);
    // Escape on the process list with nothing searched does nothing.
    window.processesPage()->listView()->setFocus();
    TestInput::typeKey(window.window(), key::Escape);
    EXPECT_EQ(window.currentPage(), 0);
    // Ctrl+F from a resource page shows the processes and their search.
    window.setCurrentPage(4);
    TestInput::typeKey(window.window(), 'f', ControlModifier);
    EXPECT_EQ(window.currentPage(), 0);
    EXPECT_TRUE(fixture.view("processSearch")->hasFocus());
}

TEST(UiTest, ColumnHeadersSort)
{
    Fixture fixture;
    MainWindow& window = fixture.window;
    ProcessList& processes = fixture.monitor.processes();
    View* table = fixture.view("processTable");
    ASSERT_NE(table, nullptr);
    // PID is the last column; a click sorts it up, another down.
    const RectF rect = windowRect(table);
    TestInput::click(window.window(), {rect.right() - 20, rect.y + 15});
    EXPECT_EQ(processes.sortColumn(), "pid");
    EXPECT_TRUE(processes.ascending());
    TestInput::click(window.window(), {rect.right() - 20, rect.y + 15});
    EXPECT_FALSE(processes.ascending());
    // Name sorts up first.
    TestInput::click(window.window(), {rect.x + 30, rect.y + 15});
    EXPECT_EQ(processes.sortColumn(), "name");
    EXPECT_TRUE(processes.ascending());
    EXPECT_EQ(fixture.monitor.applications().sortColumn(), "name");
}

TEST(UiTest, IntervalMenu)
{
    Fixture fixture;
    MainWindow& window = fixture.window;
    auto* button = static_cast<HeaderButton*>(fixture.view("refreshInterval"));
    ASSERT_NE(button, nullptr);
    TestInput::click(window.window(), center(button));
    ASSERT_TRUE(window.intervalMenu().isOpen());
    TestInput::typeKey(window.window(), key::Down);
    TestInput::typeKey(window.window(), key::Down);
    TestInput::typeKey(window.window(), key::Down);
    TestInput::typeKey(window.window(), key::Return);
    EXPECT_FALSE(window.intervalMenu().isOpen());
    EXPECT_EQ(fixture.monitor.interval(), 5000);
}

TEST(UiTest, DetailsSize)
{
    Fixture fixture;
    MainWindow& window = fixture.window;
    auto* search = static_cast<TextField*>(fixture.view("processSearch"));
    search->setText(std::to_string(getpid()));
    search->onTextChanged();
    ASSERT_TRUE(runUntil([&] { return window.processesPage()->list().count() > 0; }));
    fixture.layout();
    TestInput::click(window.window(), windowRect(window.processesPage()->listView()).topLeft() + PointF{80, 18},
                     MouseButton::Left);
    window.openDetails();
    ASSERT_NE(window.details(), nullptr);
    Window* dialog = window.details()->window();
    ASSERT_NE(dialog, nullptr);
    dialog->setScale(1.5);
    dialog->grab();
    // The dialog is 612 high, as the Qt version's.
    EXPECT_EQ(dialog->size().height, 612);
    EXPECT_EQ(dialog->size().width, 480);
}

// Whether the strip holds anything but one colour, as a shown scroll bar does.
static bool stripInked(const Image& image, int x0, int y0, int x1, int y1)
{
    const Color first = image.pixel(x0, y0);
    for (int y = y0; y < y1; ++y) {
        for (int x = x0; x < x1; ++x) {
            if (!(image.pixel(x, y) == first))
                return true;
        }
    }
    return false;
}

TEST(UiTest, ResourcePageScrollBarHidden)
{
    Fixture fixture({840, 500});
    MainWindow& window = fixture.window;
    const double scale = window.window().scale();
    const int right = int(std::lround(840 * scale));
    // Opening a page lays it out without scrolling it, so its bar stays hidden.
    for (int page = 1; page <= 5; ++page) {
        window.setCurrentPage(page);
        const Image image = window.window().grab();
        EXPECT_FALSE(stripInked(image, right - int(8 * scale), int(60 * scale), right, int(480 * scale))) << page;
    }
    window.setCurrentPage(1);
    fixture.layout();
    ScrollEvent step;
    step.deltaY = 40;
    step.value120Y = 120;
    TestInput::scroll(window.window(), {500, 300}, step);
    // Once the bar has faded in.
    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    const uint64_t later = uint64_t(now.tv_sec) * 1000000000 + uint64_t(now.tv_nsec) + 400000000;
    window.window().grab();
    const Image scrolled = window.window().grab(later);
    EXPECT_TRUE(stripInked(scrolled, right - int(8 * scale), int(60 * scale), right, int(480 * scale)));
}

TEST(UiTest, QuitErrorShowsToast)
{
    // A process of the user's own, found by its unique command line.
    pid_t child = fork();
    if (child == 0) {
        execl("/usr/bin/sleep", "sleep", "31337", nullptr);
        _exit(127);
    }
    Fixture fixture;
    MainWindow& window = fixture.window;
    auto* search = static_cast<TextField*>(fixture.view("processSearch"));
    search->setText("sleep 31337");
    ASSERT_TRUE(runUntil([&] { return window.processesPage()->list().count() > 0; }));
    fixture.layout();
    TestInput::click(window.window(), windowRect(window.processesPage()->listView()).topLeft() + PointF{80, 18});
    ASSERT_NE(window.selectedProcess(), nullptr);
    window.confirmEnd(false);
    ASSERT_NE(window.confirmDialog(), nullptr);
    kill(child, SIGKILL);
    waitpid(child, nullptr, 0);
    const Image before = window.window().grab();
    window.confirmDialog()->onAccepted();
    // The process is gone: the error goes to a toast over the bottom centre.
    ASSERT_FALSE(window.window().overlay()->children().empty());
    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    const uint64_t later = uint64_t(now.tv_sec) * 1000000000 + uint64_t(now.tv_nsec) + 600000000;
    window.window().grab();
    const Image after = window.window().grab(later);
    const int x0 = after.width() / 2 - 60;
    const int y0 = after.height() - int(60 * window.window().scale());
    int changed = 0;
    for (int y = y0; y < after.height(); ++y) {
        for (int x = x0; x < x0 + 120; ++x)
            changed += !(after.pixel(x, y) == before.pixel(x, y));
    }
    EXPECT_GT(changed, 500);
}

// The view itself when named, else the nearest named ancestor and the class of the view.
static std::string focusName(const View* view)
{
    if (!view)
        return "(none)";
    if (!view->name().empty())
        return view->name();
    for (const View* parent = view->parent(); parent; parent = parent->parent()) {
        if (!parent->name().empty())
            return parent->name() + "/" + typeid(*view).name();
    }
    return typeid(*view).name();
}

TEST(UiTest, TabOrder)
{
    Fixture fixture;
    MainWindow& window = fixture.window;
    TestInput::setKeyboardFocus(window.window(), true);
    // As Qt's chain: the cards, the scope's three buttons, the search field,
    // then the six column titles.
    std::vector<std::string> stops;
    for (int i = 0; i < 15; ++i) {
        TestInput::typeKey(window.window(), key::Tab);
        stops.push_back(focusName(window.window().focusView()));
    }
    const std::vector<std::string> cards{"cpuSummary", "memorySummary", "networkSummary", "diskSummary", "gpuSummary"};
    const size_t first = fixture.monitor.metrics().gpuAvailable ? 5 : 4;
    for (size_t i = 0; i < first; ++i)
        EXPECT_EQ(stops[i], cards[i]) << i;
    for (size_t i = first; i < first + 3; ++i)
        EXPECT_EQ(stops[i].substr(0, 13), "processScope/") << i;
    EXPECT_EQ(stops[first + 3], "processSearch");
    for (size_t i = first + 4; i < first + 10 && i < stops.size(); ++i)
        EXPECT_EQ(stops[i].substr(0, 13), "processTable/") << i;
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    Application application(argc, argv, Application::Platform::Headless);
    for (const auto& [name, svg] : embeddedIcons())
        Icon::registerGlyph(std::string(name), std::string(svg));
    return RUN_ALL_TESTS();
}
