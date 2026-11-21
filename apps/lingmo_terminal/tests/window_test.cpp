// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <functional>
#include <string>
#include <vector>

#include "base/event_loop.h"
#include "base/file.h"
#include "base/timer.h"
#include "ui/application.h"
#include "ui/glyph_run.h"
#include "ui/keys.h"
#include "ui/painter.h"
#include "ui/test_input.h"
#include "ui/text_field.h"
#include "ui/view.h"
#include "ui/window.h"

#include "TerminalFont.h"
#include "TerminalSession.h"
#include "TerminalView.h"
#include "Vt102Emulation.h"
#include "tools.h"

#include "terminalpage.h"
#include "terminalsettings.h"
#include "terminalwindow.h"
#include "windows.h"

using namespace Lingmo;

// Runs the loop until the condition holds, for at most five seconds.
static bool waitFor(const std::function<bool()>& condition)
{
    base::EventLoop& loop = *base::EventLoop::current();
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!condition()) {
        if (std::chrono::steady_clock::now() > deadline)
            return false;
        base::Timer tick;
        tick.setSingleShot(true);
        tick.onTimeout = [] {};
        tick.start(std::chrono::milliseconds(10));
        loop.dispatch();
    }
    return true;
}

class WindowTest : public testing::Test
{
protected:
    void SetUp() override
    {
        set_kb_layout_dir(KB_LAYOUTS_DIR);
        ASSERT_TRUE(m_directory.isValid());
    }

    TerminalWindow::Options shell() const
    {
        TerminalWindow::Options options;
        options.startDirectory = m_directory.path();
        options.command = {"/bin/sh"};
        options.restoresSize = false;
        return options;
    }

    base::file::TemporaryDirectory m_directory{"/tmp", "terminal-window-test"};
    TerminalSettings m_settings{m_directory.path() + "/terminal.conf"};
};

TEST_F(WindowTest, TabLeavingAWindowIsNoLongerHeard)
{
    Windows windows(m_settings);
    TerminalWindow* first = windows.create(shell());
    TerminalWindow* second = windows.create(shell());
    first->openTab(m_directory.path(), {"/bin/sh"});
    ASSERT_EQ(first->tabCount(), 2);

    std::unique_ptr<TerminalPage> page = first->takeTab(1);
    ASSERT_TRUE(page);
    EXPECT_TRUE(page->titleChanged.isEmpty());
    EXPECT_TRUE(page->terminalClosed.isEmpty());

    TerminalPage* moved = page.get();
    second->insertTab(std::move(page), 1);
    EXPECT_FALSE(moved->titleChanged.isEmpty());
    EXPECT_FALSE(moved->terminalClosed.isEmpty());
    EXPECT_EQ(second->tabCount(), 2);
}

TEST_F(WindowTest, ManyTabsOpenAndClose)
{
    Windows windows(m_settings);
    TerminalWindow* window = windows.create(shell());
    for (int i = 0; i < 50; ++i) {
        window->openTab(m_directory.path(), {"/bin/sh"});
        window->closeTab(1);
    }
    EXPECT_EQ(window->tabCount(), 1);
}

TEST_F(WindowTest, ShellExitClosesItsTab)
{
    Windows windows(m_settings);
    TerminalWindow* window = windows.create(shell());
    window->openTab(m_directory.path(), {"/bin/sh", "-c", "exit 0"});
    EXPECT_TRUE(waitFor([&] { return window->tabCount() == 1; }));
}

// The confirmation the window shows last, as a key press answers it.
static void confirmLastDialog(TerminalWindow* window)
{
    ui::Window* dialog = ui::Application::instance()->windows().back();
    ASSERT_NE(dialog, &window->window());
    dialog->grab();
    ui::TestInput::setKeyboardFocus(*dialog, true);
    ui::TestInput::typeKey(*dialog, ui::key::Return);
}

TEST_F(WindowTest, CloseOtherTabsKeepsTheChosenTab)
{
    Windows windows(m_settings);
    TerminalWindow* window = windows.create(shell());
    TerminalPage* busy = window->currentPage();
    window->openTab(m_directory.path(), {"/bin/sh"});
    ASSERT_EQ(window->tabCount(), 2);
    busy->session().sendText("sleep 30\n");
    ASSERT_TRUE(waitFor([&] { return busy->session().hasActiveProcess(); }));

    window->closeOtherTabs(1);
    confirmLastDialog(window);
    ASSERT_EQ(window->tabCount(), 1);
    ASSERT_NE(window->currentPage(), nullptr);
    EXPECT_TRUE(window->currentPage()->isVisible());
}

TEST_F(WindowTest, CloseOtherTabsAfterTheKeptTabEnded)
{
    Windows windows(m_settings);
    TerminalWindow* window = windows.create(shell());
    TerminalPage* busy = window->currentPage();
    window->openTab(m_directory.path(), {"/bin/sh", "-c", "sleep 1"});
    ASSERT_EQ(window->tabCount(), 2);
    busy->session().sendText("sleep 30\n");
    ASSERT_TRUE(waitFor([&] { return busy->session().hasActiveProcess(); }));

    window->closeOtherTabs(1);
    ASSERT_TRUE(waitFor([&] { return window->tabCount() == 1; }));
    confirmLastDialog(window);
    ASSERT_EQ(window->tabCount(), 1);
    EXPECT_EQ(window->currentPage(), busy);
    EXPECT_TRUE(busy->isVisible());
}

static ui::View* findView(ui::View* view, const std::string& name)
{
    if (!view || view->name() == name)
        return view;
    for (const auto& child : view->children()) {
        if (ui::View* found = findView(child.get(), name))
            return found;
    }
    return nullptr;
}

TEST_F(WindowTest, LanguageChangeKeepsTheShellsAndTheSearch)
{
    Windows windows(m_settings);
    TerminalWindow* window = windows.create(shell());
    TerminalPage* busy = window->currentPage();
    window->openTab(m_directory.path(), {"/bin/sh"});
    ASSERT_EQ(window->tabCount(), 2);
    TerminalPage* page = window->currentPage();
    Konsole::TerminalSession* session = &page->session();
    Konsole::TerminalView* terminal = page->terminal();
    page->setCustomTitle("Build");
    page->openFind();
    auto* field = static_cast<ui::TextField*>(findView(page, "findField"));
    ASSERT_NE(field, nullptr);
    field->setText("needle");

    window->showSettings();
    ui::Window* settings = ui::Application::instance()->windows().back();
    ASSERT_NE(settings, &window->window());
    const ui::View* settingsContent = findView(settings->contentView(), "settingsContent");
    ASSERT_NE(settingsContent, nullptr);

    busy->session().sendText("sleep 30\n");
    ASSERT_TRUE(waitFor([&] { return busy->session().hasActiveProcess(); }));
    window->closeOtherTabs(1);

    ui::Application::instance()->languageChanged.emit();
    ASSERT_EQ(window->tabCount(), 2);
    EXPECT_EQ(window->currentPage(), page);
    EXPECT_EQ(&page->session(), session);
    EXPECT_EQ(page->terminal(), terminal);
    EXPECT_EQ(page->title(), "Build");
    EXPECT_TRUE(busy->session().hasActiveProcess());
    EXPECT_EQ(static_cast<ui::TextField*>(findView(page, "findField"))->text(), "needle");
    const ui::View* rebuilt = findView(settings->contentView(), "settingsContent");
    ASSERT_NE(rebuilt, nullptr);
    EXPECT_NE(rebuilt, settingsContent);

    // The close question is asked again and still closes the other tab.
    confirmLastDialog(window);
    ASSERT_EQ(window->tabCount(), 1);
    EXPECT_EQ(window->currentPage(), page);
}

TEST_F(WindowTest, CursorBlinksOnlyWithTheKeyboard)
{
    Windows windows(m_settings);
    TerminalWindow* window = windows.create(shell());
    ui::Window& surface = window->window();
    Konsole::TerminalView* terminal = window->currentPage()->terminal();
    ASSERT_TRUE(terminal->hasFocus());

    ui::TestInput::setKeyboardFocus(surface, true);
    surface.grab();
    EXPECT_TRUE(terminal->isCursorBlinking());

    ui::TestInput::setKeyboardFocus(surface, false);
    surface.grab();
    EXPECT_FALSE(terminal->isCursorBlinking());

    ui::TestInput::setKeyboardFocus(surface, true);
    surface.grab();
    EXPECT_TRUE(terminal->isCursorBlinking());

    // Without a frame drawn, the next blink stops the timer.
    ui::TestInput::setKeyboardFocus(surface, false);
    EXPECT_TRUE(waitFor([&] { return !terminal->isCursorBlinking(); }));
}

TEST_F(WindowTest, CursorWithoutBlinkingNeverStartsTheTimer)
{
    m_settings.setBlinkingCursor(false);
    Windows windows(m_settings);
    TerminalWindow* window = windows.create(shell());
    ui::Window& surface = window->window();
    Konsole::TerminalView* terminal = window->currentPage()->terminal();

    ui::TestInput::setKeyboardFocus(surface, true);
    surface.grab();
    EXPECT_FALSE(terminal->isCursorBlinking());

    ui::TestInput::setKeyboardFocus(surface, false);
    surface.grab();
    EXPECT_FALSE(terminal->isCursorBlinking());
}

// A tab whose program takes the keyboard raw, asks for bracketed paste and
// writes what it receives to `path`.
static TerminalWindow::Options recorder(const std::string& directory, const std::string& path,
                                        const std::string& output = "\\033[?2004h")
{
    TerminalWindow::Options options;
    options.startDirectory = directory;
    options.command = {"/bin/sh", "-c", "stty raw -echo; printf '" + output + "'; exec cat > '" + path + "'"};
    options.restoresSize = false;
    return options;
}

static std::string contents(const std::string& path)
{
    return base::file::read(path).value_or(std::string());
}

TEST_F(WindowTest, PasteCannotEndTheBracketEarly)
{
    const std::string path = m_directory.path() + "/received";
    Windows windows(m_settings);
    TerminalWindow* window = windows.create(recorder(m_directory.path(), path));
    TerminalPage* page = window->currentPage();
    ASSERT_TRUE(waitFor([&] { return page->session().emulation()->programBracketedPasteMode(); }));

    page->terminal()->pasteText("echo \033[201~touch pwned");
    const std::string expected = "\033[200~echo touch pwned\033[201~";
    EXPECT_TRUE(waitFor([&] { return contents(path) == expected; })) << contents(path);
}

TEST_F(WindowTest, DroppedFilesArriveAsAPaste)
{
    const std::string path = m_directory.path() + "/received";
    Windows windows(m_settings);
    TerminalWindow* window = windows.create(recorder(m_directory.path(), path));
    TerminalPage* page = window->currentPage();
    ASSERT_TRUE(waitFor([&] { return page->session().emulation()->programBracketedPasteMode(); }));

    // Ctrl+U in a name would erase the opening quote at a prompt.
    page->dropUrls({"file:///tmp/a%15b%20c"});
    const std::string expected = "\033[200~'/tmp/a\x15" "b c' \033[201~";
    EXPECT_TRUE(waitFor([&] { return contents(path) == expected; })) << contents(path);
}

TEST_F(WindowTest, DroppedFileUrlsWithAHostBecomePaths)
{
    const std::string path = m_directory.path() + "/received";
    Windows windows(m_settings);
    TerminalWindow* window = windows.create(recorder(m_directory.path(), path));
    TerminalPage* page = window->currentPage();
    ASSERT_TRUE(waitFor([&] { return page->session().emulation()->programBracketedPasteMode(); }));

    page->dropUrls({"file://localhost/tmp/a%23b", "file://elsewhere/tmp/c", "https://example.com/d"});
    const std::string expected = "\033[200~'/tmp/a#b' file://elsewhere/tmp/c https://example.com/d \033[201~";
    EXPECT_TRUE(waitFor([&] { return contents(path) == expected; })) << contents(path);
}

// A wheel turn up, three lines, over the middle of the window.
static void scrollUp(TerminalWindow* window)
{
    ui::Window& surface = window->window();
    surface.grab();
    ui::ScrollEvent event;
    event.value120Y = -120;
    ui::TestInput::scroll(surface, {surface.size().width / 2, surface.size().height / 2}, event);
}

TEST_F(WindowTest, WheelWithoutScrollbackSendsArrowsOnlyOnTheAlternateScreen)
{
    const std::string primary = m_directory.path() + "/primary";
    Windows windows(m_settings);
    TerminalWindow* window = windows.create(recorder(m_directory.path(), primary));
    TerminalPage* page = window->currentPage();
    ASSERT_TRUE(waitFor([&] { return page->session().emulation()->programBracketedPasteMode(); }));
    scrollUp(window);
    page->session().sendText("x");
    EXPECT_TRUE(waitFor([&] { return contents(primary) == "x"; })) << contents(primary);

    const std::string alternate = m_directory.path() + "/alternate";
    window->openTab(m_directory.path(), recorder(m_directory.path(), alternate, "\\033[?1049h").command);
    page = window->currentPage();
    ASSERT_TRUE(waitFor([&] { return page->session().emulation()->usesAlternateScreen(); }));
    scrollUp(window);
    page->session().sendText("x");
    EXPECT_TRUE(waitFor([&] { return contents(alternate) == "\033[A\033[A\033[Ax"; })) << contents(alternate);
}

namespace {

class Canvas : public ui::View
{
public:
    explicit Canvas(std::function<void(ui::Painter&)> draw)
        : m_draw(std::move(draw))
    {
    }

protected:
    void paint(ui::Painter& painter) override { m_draw(painter); }

private:
    std::function<void(ui::Painter&)> m_draw;
};

}

TEST(TerminalFontTest, ShapedClustersStayBounded)
{
    Konsole::TerminalFont font;
    font.setFont("Monospace", 10);

    // Distinct ideographs, a hundred a call, as output of every code point would draw them.
    auto draw = [&font](ui::Painter& painter) {
        char32_t character = 0x4E00;
        std::vector<std::u32string> texts;
        for (int call = 0; call < 200; ++call) {
            texts.clear();
            for (int i = 0; i < 100; ++i)
                texts.emplace_back(1, character++);
            std::vector<ui::GlyphCluster> clusters;
            for (const std::u32string& text : texts)
                clusters.push_back({text, 0});
            font.draw(painter, Konsole::TerminalFont::Regular, ui::Color::rgb(0), 1, 12, clusters, {1, 1}, {});
        }
    };

    ui::Window window;
    window.resize({100, 60});
    window.setContentView(std::make_unique<Canvas>(draw));
    window.show();
    window.grab();

    EXPECT_LE(font.cachedClusters(), 8192u + 100u);
}
