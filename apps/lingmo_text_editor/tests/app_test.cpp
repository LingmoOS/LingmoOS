// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "application.h"
#include "editorpage.h"
#include "editorsettings.h"
#include "editorview.h"
#include "editorwindow.h"
#include "pagebars.h"
#include "session.h"

#include "base/elapsed_timer.h"
#include "base/file.h"
#include "test_support.h"
#include "ui/application.h"
#include "ui/drag.h"
#include "ui/keys.h"
#include "ui/test_input.h"
#include "ui/theme.h"
#include "ui/window.h"
#include "ui/window_tab_bar.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>

using namespace Lingmo::ui;
namespace fs = std::filesystem;

static std::string writeFile(const std::string &name, std::string_view data)
{
    const std::string path = TestSupport::directory() + "/" + name;
    Lingmo::base::file::writeAtomically(path, data);
    return path;
}

// One editor process: its settings in the scratch folder, a fresh session.
class AppTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        std::error_code error;
        fs::remove_all(Session::dataDirectory(), error);
        const std::string conf = TestSupport::directory() + "/text-editor-app.conf";
        fs::remove(conf, error);
        m_settings = std::make_unique<EditorSettings>(conf);
        m_app = std::make_unique<EditorApplication>(*m_settings);
    }

    void TearDown() override
    {
        m_app.reset();
        TestSupport::wait(20);
    }

    EditorWindow *start(const std::vector<std::string> &files, SizeF size = {900, 640})
    {
        m_app->start(files, TestSupport::directory(), std::nullopt);
        if (m_app->windows().empty())
            return nullptr;
        EditorWindow *window = m_app->windows().back();
        TestInput::configure(window->window(), int(size.width), int(size.height), {true});
        TestInput::setKeyboardFocus(window->window(), true);
        for (EditorPage *page : window->pages())
            page->document().waitForLoaded();
        frame(window);
        return window;
    }

    static void frame(EditorWindow *window)
    {
        TestSupport::wait(5);
        window->window().grab();
    }

    static EditorView *editor(EditorWindow *window) { return &window->currentPage()->editor(); }

    // Window coordinates of a point of a view.
    static PointF at(View *view, PointF point) { return view->mapToWindow(point); }

    std::unique_ptr<EditorSettings> m_settings;
    std::unique_ptr<EditorApplication> m_app;
};

// The shown window that is none of the editor's: the dialog asking about unsaved changes.
static Window *openDialog(EditorApplication &application)
{
    for (Window *window : Lingmo::ui::Application::instance()->windows()) {
        if (window->isHidden())
            continue;
        const std::vector<EditorWindow *> windows = application.windows();
        if (std::none_of(windows.begin(), windows.end(), [window](EditorWindow *w) { return &w->window() == window; }))
            return window;
    }
    return nullptr;
}

static std::string readFile(const std::string &path)
{
    return Lingmo::base::file::read(path).value_or(std::string());
}

// Files dragged in from another client, which answers only when told to.
class SlowFileOffer : public DropOffer
{
public:
    const std::vector<std::string> &mimeTypes() const override { return m_types; }
    uint32_t sourceActions() const override { return CopyAction; }
    void read(const std::string &, std::function<void(std::string data)> done) override { m_done = std::move(done); }

    bool asked() const { return bool(m_done); }
    void answer(const std::string &data)
    {
        if (m_done)
            std::exchange(m_done, nullptr)(data);
    }

private:
    std::vector<std::string> m_types{"text/uri-list"};
    std::function<void(std::string data)> m_done;
};

TEST_F(AppTest, OpensFileAndTypes)
{
    const std::string path = writeFile("hello.c", "int main(void)\n{\n    return 0; /* done */\n}\n");
    EditorWindow *window = start({path});
    ASSERT_TRUE(window);
    EditorView *view = editor(window);
    Document &document = *view->document();
    EXPECT_EQ(document.language(), "c");
    EXPECT_EQ(window->currentPage()->title(), "hello.c");
    EXPECT_GT(view->contentHeight(), 4 * view->lineHeight());
    EXPECT_TRUE(view->hasFocus());

    document.setCursor({2, 4});
    TestInput::typeText(window->window(), "x = 1;");
    TestInput::typeKey(window->window(), key::Return);
    EXPECT_EQ(document.buffer().line(2), "    x = 1;");
    // Enter keeps the indentation.
    EXPECT_EQ(document.buffer().line(3), "    return 0; /* done */");
    EXPECT_EQ(document.cursor(), (TextPosition{3, 4}));
    EXPECT_TRUE(document.isModified());
    for (int i = 0; i < 4; ++i)
        TestInput::typeKey(window->window(), key::letter('z'), ControlModifier);
    EXPECT_FALSE(document.isModified());

    // A click lands on the text under the pointer.
    frame(window);
    TestInput::click(window->window(), at(view, {view->gutterWidth() + 2, 6 + view->lineHeight() * 1.5}));
    EXPECT_EQ(document.cursor().line, 1);
}

TEST_F(AppTest, ClickSelection)
{
    EditorWindow *window = start({writeFile("words.txt", "alpha beta gamma\nsecond line\n")});
    ASSERT_TRUE(window);
    EditorView *view = editor(window);
    Document &document = *view->document();
    const double advance = view->gutterWidth();
    const PointF beta = at(view, {advance + 7.8 * 8, 6 + view->lineHeight() * 0.5});
    // A double click selects the word, a third click the line.
    TestInput::click(window->window(), beta);
    TestInput::click(window->window(), beta);
    EXPECT_EQ(document.selectedText(), "beta");
    TestInput::click(window->window(), beta);
    EXPECT_EQ(document.selectedText(), "alpha beta gamma\n");
}

TEST_F(AppTest, SelectionHasNoSeams)
{
    std::string data;
    for (int i = 0; i < 200; ++i)
        data += "line " + std::to_string(i) + "\n";
    EditorWindow *window = start({writeFile("seams.txt", data)});
    ASSERT_TRUE(window);
    window->window().setScale(1.5);
    frame(window);
    EditorView *view = editor(window);
    view->document()->selectAll();
    // Scrolled to half a device pixel, where rounding each line on its own split them.
    const double target = 40 * view->lineHeight() + 0.5 / 1.5;
    view->setContentPosition(0, target);
    const Image image = window->window().grab();
    const PointF origin = view->mapToWindow({0, 0});
    const int x = int(std::lround((origin.x + view->geometry().width / 2) * 1.5));
    const int middle = int(std::lround((origin.y + view->geometry().height / 2) * 1.5));
    const Color selection = image.pixel(x, middle);
    const int top = int(std::ceil(origin.y * 1.5)) + 1;
    const int bottom = int(std::floor((origin.y + view->geometry().height) * 1.5)) - 1;
    std::string seams;
    for (int y = top; y < bottom; ++y) {
        if (image.pixel(x, y) != selection)
            seams += std::to_string(y) + " ";
    }
    EXPECT_TRUE(seams.empty()) << seams;
}

TEST_F(AppTest, ScrollBarDrag)
{
    std::string data;
    for (int i = 0; i < 500; ++i)
        data += "line\n";
    EditorWindow *window = start({writeFile("long.txt", data)});
    ASSERT_TRUE(window);
    EditorView *view = editor(window);
    EXPECT_EQ(view->contentY(), 0.0);
    // The handle sits at the top of the right edge; dragging it scrolls the text.
    const double width = view->geometry().width;
    const PointF handle = at(view, {width - 4, 10});
    const PointF lower = at(view, {width - 4, view->geometry().height / 2});
    TestInput::movePointer(window->window(), handle);
    TestInput::press(window->window(), handle);
    TestInput::movePointer(window->window(), {handle.x, handle.y + 10});
    TestInput::movePointer(window->window(), lower);
    TestInput::release(window->window(), lower);
    EXPECT_GT(view->contentY(), 100 * view->lineHeight());
}

TEST_F(AppTest, TabsAndSession)
{
    const std::string a = writeFile("a.txt", "first file\n");
    const std::string b = writeFile("b.md", "# Title\n\nSome *text*.\n");
    EditorWindow *window = start({a, b});
    ASSERT_TRUE(window);
    EXPECT_EQ(window->tabCount(), 2);
    // Opening an open file again focuses its tab.
    m_app->openPaths({a});
    EXPECT_EQ(window->tabCount(), 2);
    EXPECT_EQ(window->currentPage()->document().filePath(), a);

    TestInput::typeKey(window->window(), key::letter('t'), ControlModifier);
    EXPECT_EQ(window->tabCount(), 3);
    TestInput::typeText(window->window(), "draft text");
    EXPECT_EQ(window->currentPage()->title(), "draft text");

    m_app->saveSession();
    window->currentPage()->document().writeDraftNow();
    const std::vector<WindowState> session = Session::load();
    ASSERT_EQ(session.size(), 1u);
    ASSERT_EQ(session.front().tabs.size(), 3u);
    EXPECT_TRUE(session.front().tabs.at(2).unsaved);
    EXPECT_EQ(session.front().current, 2);

    // A new process restores it, the untitled text included.
    m_app = std::make_unique<EditorApplication>(*m_settings);
    EditorWindow *restored = start({});
    ASSERT_TRUE(restored);
    EXPECT_EQ(restored->tabCount(), 3);
    EXPECT_EQ(restored->currentPage()->document().text(), "draft text");
    EXPECT_TRUE(restored->currentPage()->document().isModified());

    // Closing a tab can be undone.
    restored->closeTab(0, false);
    EXPECT_EQ(restored->tabCount(), 2);
    EXPECT_TRUE(m_app->canReopenClosedTab());
    TestInput::typeKey(restored->window(), key::letter('t'), ControlModifier | ShiftModifier);
    EXPECT_EQ(restored->tabCount(), 3);
}

TEST_F(AppTest, CrowdedTabs)
{
    std::vector<std::string> files;
    for (int i = 0; i < 12; ++i)
        files.push_back(writeFile("crowded-" + std::to_string(i) + ".txt", "text\n"));
    EditorWindow *window = start(files, {700, 480});
    ASSERT_TRUE(window);
    ASSERT_EQ(window->tabCount(), 12);
    WindowTabBar *bar = window->tabBar();
    // The strip's scroll, from where the first tab is drawn.
    const auto contentX = [bar] { return -bar->tabRect(0).x; };
    // Titles keep their room; the bar scrolls instead.
    EXPECT_EQ(bar->tabRect(0).width, 140);
    const double limit = 12 * 140 - bar->geometry().width;
    EXPECT_GT(limit, 0);

    window->selectTab(0);
    frame(window);
    EXPECT_EQ(contentX(), 0);
    window->selectTab(11);
    frame(window);
    EXPECT_EQ(contentX(), limit);
    window->selectTab(0);
    frame(window);
    EXPECT_EQ(contentX(), 0);

    // A dragged tab becomes the current one as soon as it moves.
    const PointF grab = bar->mapToWindow({140 * 2.5, bar->geometry().height / 2});
    TestInput::movePointer(window->window(), grab);
    TestInput::press(window->window(), grab);
    for (int dx = 4; dx <= 24; dx += 4)
        TestInput::movePointer(window->window(), {grab.x + dx, grab.y});
    EXPECT_EQ(window->currentIndex(), 2);
    TestInput::release(window->window(), {grab.x + 24, grab.y});
    EXPECT_EQ(window->currentIndex(), 2);

    // A wheel scrolls the strip, which stays where the wheel left it.
    window->selectTab(0);
    frame(window);
    ScrollEvent wheel;
    wheel.deltaY = 30;
    wheel.value120Y = 120;
    TestInput::scroll(window->window(), bar->mapToWindow({bar->geometry().width / 2, 10}), wheel);
    EXPECT_GT(contentX(), 0);
    const double scrolled = contentX();
    TestSupport::wait(300);
    EXPECT_EQ(contentX(), scrolled);

    // A tab out of view scrolls just into it.
    window->selectTab(11);
    frame(window);
    EXPECT_EQ(contentX(), limit);
    window->selectTab(1);
    frame(window);
    EXPECT_EQ(contentX(), 140);
}

TEST_F(AppTest, FindBarAndStatus)
{
    EditorWindow *window = start({writeFile("find.txt", "apple banana apple\ncherry apple\n")});
    ASSERT_TRUE(window);
    Document &document = window->currentPage()->document();
    TestInput::typeKey(window->window(), key::letter('f'), ControlModifier);
    FindBar *findBar = window->currentPage()->findBarIfMade();
    ASSERT_TRUE(findBar);
    EXPECT_TRUE(findBar->isVisible());
    TestInput::typeText(window->window(), "apple");
    TestSupport::wait(80);
    EXPECT_EQ(document.matchCount(), 3);
    EXPECT_EQ(document.selection(), (TextRange{{0, 0}, {0, 5}}));
    TestInput::typeKey(window->window(), key::Return);
    EXPECT_EQ(document.selection(), (TextRange{{0, 13}, {0, 18}}));
    EXPECT_EQ(document.matchIndex(), 2);
    TestInput::typeKey(window->window(), key::Escape);
    EXPECT_FALSE(findBar->isVisible());
    EXPECT_TRUE(window->currentPage()->editor().hasFocus());

    TestInput::typeKey(window->window(), key::letter('h'), ControlModifier);
    frame(window);
    const auto rect = [findBar](const char *name) {
        for (const auto &child : findBar->children()) {
            if (child->name() == name) {
                const PointF origin = child->mapToWindow({0, 0});
                return RectF{origin.x, origin.y, child->geometry().width, child->geometry().height};
            }
        }
        return RectF{};
    };
    // Both fields start on one edge, and the rows end on one edge.
    EXPECT_NEAR(rect("replaceField").x, rect("findField").x, 0.01);
    const PointF cardOrigin = findBar->mapToWindow({0, 0});
    const RectF card{cardOrigin.x, cardOrigin.y, findBar->geometry().width, findBar->geometry().height};
    // The close plate (inset round(28 * 0.1)) ends where Replace All does.
    EXPECT_NEAR(rect("replaceAllButton").right(), rect("closeFindButton").right() - 3, 0.7);
    const double leftPadding = rect("replaceToggle").x + 3 - card.x;
    const double rightPadding = card.right() - (rect("closeFindButton").right() - 3);
    EXPECT_NEAR(leftPadding, rightPadding, 0.7);
    // The card is inset from the editor's top and right edges alike.
    View *view = &window->currentPage()->editor();
    const PointF area = view->mapToWindow({0, 0});
    EXPECT_NEAR(card.y - area.y, 12, 0.01);
    EXPECT_NEAR(area.x + view->geometry().width - card.right(), 12, 0.01);
    EXPECT_EQ(window->currentPage()->editor().contentY(), 0.0);
    TestInput::typeKey(window->window(), key::Escape);

    TestInput::typeKey(window->window(), key::letter('i'), ControlModifier);
    TestInput::typeText(window->window(), "2:3");
    TestInput::typeKey(window->window(), key::Return);
    EXPECT_EQ(document.cursor(), (TextPosition{1, 2}));
}

TEST_F(AppTest, FindCardKeepsMatchVisible)
{
    // A match at the end of the first line lies where the card floats.
    EditorWindow *window = start({writeFile("under.txt", std::string(70, 'x') + " needle\nsecond line\nthird\n")}, {700, 400});
    ASSERT_TRUE(window);
    EditorView *view = editor(window);
    const double before = view->contentY();
    window->currentPage()->showFind(false);
    TestInput::typeText(window->window(), "needle");
    TestSupport::wait(80);
    frame(window);
    EXPECT_EQ(view->document()->matchCount(), 1);
    View *card = window->currentPage()->card();
    ASSERT_TRUE(card);
    const double cardBottom = card->mapToWindow({0, card->geometry().height}).y;
    const double matchTop = view->mapToWindow({0, view->cursorRectangle().y}).y;
    EXPECT_GE(matchTop, cardBottom);
    EXPECT_LT(view->contentY(), before);

    // Closing puts the first line back at the top.
    TestInput::typeKey(window->window(), key::Escape);
    frame(window);
    EXPECT_EQ(view->contentY(), 0.0);
    EXPECT_TRUE(view->hasFocus());
}

TEST_F(AppTest, Gutter)
{
    for (const int lines : {5, 15, 10050}) {
        SCOPED_TRACE(lines);
        std::string data;
        for (int i = 0; i < lines; ++i)
            data += "#line\n";
        EditorWindow *window = start({writeFile("gutter-" + std::to_string(lines) + ".c", data)});
        ASSERT_TRUE(window);
        EditorView *view = editor(window);
        // Two digits at least, plus the side padding on both sides.
        const int digits = std::max(2, int(std::to_string(lines + 1).size()));
        EXPECT_NEAR(view->gutterWidth(), 14 + digits * 7.8 + 14, 1);
        m_app->releaseWindow(window);
        TestSupport::wait(10);
    }
}

TEST_F(AppTest, WrappingAndDialogs)
{
    EditorWindow *window = start({writeFile("wrap.md", "# Notes\n\n- A list item that is long enough to wrap around the edge of the window at least once, "
                                                      "and its continuation should line up with the start of its text rather than the margin.\n"
                                                      "Plain **strong** and `code` text.\n")},
                                 {520, 360});
    ASSERT_TRUE(window);
    EditorView *view = editor(window);
    EXPECT_GT(view->contentHeight(), 5 * view->lineHeight());
    EXPECT_EQ(view->positionAt({view->gutterWidth() + 20, 6 + view->lineHeight() * 0.5}).line, 0);
    // The wrapped item continues at its own indentation.
    const TextPosition wrapped = view->positionAt({view->gutterWidth() + 1, 6 + view->lineHeight() * 3.5});
    EXPECT_EQ(wrapped.line, 2);
    EXPECT_GT(wrapped.column, 2);

    window->showMainMenu();
    window->showPreferences();
    window->showShortcuts();
    TestSupport::wait(30);
}

TEST_F(AppTest, PreeditShowsAtTheCursor)
{
    EditorWindow *window = start({});
    ASSERT_TRUE(window);
    EditorView *view = editor(window);
    TestInput::typeText(window->window(), "ab");
    const double before = view->cursorRectangle().x;
    TextInputEvent preedit;
    preedit.preedit = "\xE4\xBD\xA0\xE5\xA5\xBD";
    preedit.preeditCursorBegin = int(preedit.preedit.size());
    preedit.preeditCursorEnd = preedit.preeditCursorBegin;
    TestInput::inputMethod(window->window(), preedit);
    frame(window);
    EXPECT_GT(view->cursorRectangle().x, before);
    EXPECT_EQ(view->document()->text(), "ab");
    TextInputEvent commit;
    commit.commit = "\xE4\xBD\xA0\xE5\xA5\xBD";
    TestInput::inputMethod(window->window(), commit);
    frame(window);
    EXPECT_EQ(view->document()->text(), "ab\xE4\xBD\xA0\xE5\xA5\xBD");
}

TEST_F(AppTest, CloseLastWindow)
{
    for (const bool modified : {false, true}) {
        for (const bool restore : {true, false}) {
            SCOPED_TRACE(std::to_string(modified) + std::to_string(restore));
            m_settings->restoreSession = restore;
            m_app = std::make_unique<EditorApplication>(*m_settings);
            std::error_code error;
            fs::remove_all(Session::dataDirectory(), error);
            EditorWindow *window = start({writeFile("last.txt", "text\n")});
            ASSERT_TRUE(window);
            if (modified)
                window->currentPage()->document().insertText("more ");
            window->requestClose();
            if (modified && !restore) {
                // Unsaved changes are asked about first: Don't Save.
                bool found = false;
                for (Window *w : Lingmo::ui::Application::instance()->windows()) {
                    if (w != &window->window() && !w->isHidden())
                        found = true;
                }
                EXPECT_TRUE(found);
                EXPECT_FALSE(m_app->isQuitting());
                m_app->quit();
            }
            EXPECT_TRUE(m_app->isQuitting());
            if (restore) {
                const std::vector<WindowState> session = Session::load();
                ASSERT_EQ(session.size(), 1u);
                EXPECT_EQ(session.front().tabs.front().unsaved, modified);
            }
        }
    }
}

TEST_F(AppTest, LargeFilePerformance)
{
    std::string data;
    data.reserve(25 * 1024 * 1024);
    for (int i = 0; i < 400000; ++i)
        data += "    if (value_" + std::to_string(i) + " > limit) { total += compute(value, \"text\"); } // note\n";
    const std::string path = writeFile("large.c", data);
    EditorWindow *window = start({});
    ASSERT_TRUE(window);
    Lingmo::base::ElapsedTimer timer;
    timer.start();
    m_app->openPaths({path});
    EditorView *view = editor(window);
    Document &document = *view->document();
    ASSERT_TRUE(document.waitForLoaded(60000));
    const double loadMs = std::chrono::duration<double, std::milli>(timer.elapsed()).count();
    window->window().grab();
    const double firstMs = std::chrono::duration<double, std::milli>(timer.elapsed()).count();
    ASSERT_EQ(document.lineCount(), 400001);

    document.setCursor({200000, 10});
    view->ensureCursorVisible();
    window->window().grab();
    timer.start();
    for (int i = 0; i < 50; ++i) {
        TestInput::typeText(window->window(), "a");
        window->window().grab();
    }
    const double typingMs = std::chrono::duration<double, std::milli>(timer.elapsed()).count();
    timer.start();
    for (int i = 0; i < 200; ++i) {
        document.insertText("b");
        view->updateNow();
    }
    const double editMs = std::chrono::duration<double, std::milli>(timer.elapsed()).count();
    timer.start();
    for (int i = 0; i < 20; ++i) {
        TestInput::typeKey(window->window(), key::Return);
        window->window().grab();
    }
    const double enterMs = std::chrono::duration<double, std::milli>(timer.elapsed()).count();
    timer.start();
    for (int i = 0; i < 50; ++i) {
        view->setContentPosition(view->contentX(), view->contentY() + view->geometry().height);
        window->window().grab();
    }
    const double scrollMs = std::chrono::duration<double, std::milli>(timer.elapsed()).count();
    timer.start();
    view->scrollToLine(399000);
    window->window().grab();
    const double jumpMs = std::chrono::duration<double, std::milli>(timer.elapsed()).count();
    std::printf("large file (window): load %.0f ms, first frame %.0f ms, typing %.1f ms/char (%.2f ms without "
                "rendering), enter %.1f ms, scroll %.1f ms/screen, jump %.0f ms\n",
                loadMs, firstMs, typingMs / 50, editMs / 200, enterMs / 20, scrollMs / 50, jumpMs);
    EXPECT_LT(typingMs / 50, 50);
    EXPECT_LT(scrollMs / 50, 100);
}

TEST_F(AppTest, SaveAllAndSaveChanges)
{
    const std::string one = writeFile("one.txt", "one\n");
    const std::string two = writeFile("two.txt", "two\n");
    EditorWindow *window = start({one, two});
    ASSERT_TRUE(window);
    ASSERT_EQ(window->tabCount(), 2);
    for (EditorPage *page : window->pages())
        page->document().insertText("more ");
    window->saveAll();
    for (EditorPage *page : window->pages())
        EXPECT_FALSE(page->document().isModified());
    EXPECT_EQ(readFile(one), "more one\n");
    EXPECT_EQ(readFile(two), "more two\n");

    // Return saves in the dialog that closing a modified tab asks.
    window->pages().front()->document().insertText("again ");
    window->closeTab(0);
    Window *dialog = openDialog(*m_app);
    ASSERT_TRUE(dialog);
    TestInput::setKeyboardFocus(*dialog, true);
    TestInput::typeKey(*dialog, key::Return);
    TestSupport::wait(5);
    EXPECT_EQ(window->tabCount(), 1);
    EXPECT_EQ(readFile(one), "more again one\n");
}

TEST_F(AppTest, QuitDialogSkipsPagesClosedMeanwhile)
{
    m_settings->restoreSession = false;
    const std::string first = writeFile("first.txt", "first\n");
    const std::string second = writeFile("second.txt", "second\n");
    EditorWindow *window = start({first});
    ASSERT_TRUE(window);
    window->currentPage()->document().insertText("edit ");
    EditorWindow::Options options;
    options.initialTabs = {TabState{}};
    options.initialTabs.front().path = second;
    options.initialTabs.front().draftId = "second";
    EditorWindow *other = m_app->newWindow(std::move(options));
    other->currentPage()->document().waitForLoaded();
    other->currentPage()->document().insertText("edit ");

    // Both pages are asked about; the other window then closes its page without saving.
    window->quitApplication();
    Window *dialog = openDialog(*m_app);
    ASSERT_TRUE(dialog);
    other->closeTab(0, false);
    TestSupport::wait(5);
    EXPECT_EQ(m_app->windowCount(), 1);

    TestInput::setKeyboardFocus(*dialog, true);
    TestInput::typeKey(*dialog, key::Return);
    TestSupport::wait(5);
    EXPECT_EQ(readFile(first), "edit first\n");
    EXPECT_EQ(readFile(second), "second\n");
    EXPECT_TRUE(m_app->isQuitting());
}

TEST_F(AppTest, LanguageChangeKeepsTheDocuments)
{
    const std::string path = writeFile("language.txt", "apple banana apple\ncherry apple\n");
    EditorWindow *window = start({path});
    ASSERT_TRUE(window);
    EditorPage *page = window->currentPage();
    Document &document = page->document();
    document.insertText("edit ");
    TestInput::typeKey(window->window(), key::letter('f'), ControlModifier);
    TestInput::typeText(window->window(), "apple");
    TestSupport::wait(80);
    const std::string text = document.text();
    const TextRange selection = document.selection();
    EditorPage *untitled = window->newTab();
    const std::string untitledTitle = untitled->title();
    window->closeTab(0);
    Window *dialog = openDialog(*m_app);
    ASSERT_TRUE(dialog);
    const View *question = dialog->contentView();
    const StatusBar *status = &page->statusBar();
    const EditorView *view = &page->editor();

    Lingmo::ui::Application::instance()->languageChanged.emit();
    frame(window);
    ASSERT_EQ(window->tabCount(), 2);
    EXPECT_EQ(window->pages().front(), page);
    EXPECT_EQ(window->currentPage(), untitled);
    EXPECT_EQ(untitled->title(), untitledTitle);
    EXPECT_EQ(&page->editor(), view);
    EXPECT_NE(&page->statusBar(), status);
    EXPECT_EQ(document.text(), text);
    EXPECT_TRUE(document.isModified());
    EXPECT_TRUE(document.canUndo());
    EXPECT_EQ(document.selection(), selection);
    ASSERT_TRUE(page->findBarIfMade());
    EXPECT_TRUE(page->findBarIfMade()->isVisible());
    EXPECT_EQ(document.searchText(), "apple");
    // The question still waits for its answer, made again in its window.
    EXPECT_EQ(openDialog(*m_app), dialog);
    EXPECT_NE(dialog->contentView(), question);

    TestInput::setKeyboardFocus(*dialog, true);
    TestInput::typeKey(*dialog, key::Return);
    TestSupport::wait(5);
    EXPECT_EQ(window->tabCount(), 1);
    EXPECT_EQ(readFile(path), text);
}

TEST_F(AppTest, FilesArriveAfterTheirTargetCloses)
{
    const std::string late = writeFile("late.txt", "late\n");
    EditorWindow *window = start({writeFile("a.txt", "a\n"), writeFile("b.txt", "b\n")});
    ASSERT_TRUE(window);
    ASSERT_EQ(window->tabCount(), 2);

    // Dropped on the text of a tab that closes before the source answers.
    SlowFileOffer onText;
    EditorView *view = editor(window);
    const PointF middle = at(view, {view->geometry().width / 2, view->geometry().height / 2});
    TestInput::dragEnter(window->window(), middle, &onText);
    EXPECT_EQ(TestInput::drop(window->window()), CopyAction);
    ASSERT_TRUE(onText.asked());
    window->closeTab(window->currentIndex(), false);
    TestSupport::wait(5);
    onText.answer("file://" + late + "\n");
    EXPECT_EQ(window->tabCount(), 1);

    // Dropped on the tab bar of a window that closes before the source answers.
    EditorWindow *other = m_app->newWindow();
    TestInput::configure(other->window(), 900, 640, {true});
    frame(other);
    SlowFileOffer onTabs;
    View *header = other->headerContent();
    // The free stretch before the menu button.
    TestInput::dragEnter(other->window(), at(header, {header->geometry().width - 56, header->geometry().height / 2}),
                         &onTabs);
    EXPECT_EQ(TestInput::drop(other->window()), CopyAction);
    ASSERT_TRUE(onTabs.asked());
    other->requestClose();
    TestSupport::wait(5);
    EXPECT_EQ(m_app->windowCount(), 1);
    onTabs.answer("file://" + late + "\n");
    EXPECT_EQ(window->tabCount(), 1);
    EXPECT_FALSE(m_app->findOpenDocument(late).has_value());
}
