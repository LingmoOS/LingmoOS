// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testutil.h"

#include "core/shortcutconvention.h"
#include "desktop/desktopview.h"
#include "desktop/desktopwindow.h"
#include "model/foldermodel.h"
#include "views/folderpage.h"
#include "views/folderwindow.h"
#include "views/folderview.h"
#include "views/renameeditor.h"

#include "base/dbus.h"
#include "base/url.h"
#include "ui/application.h"
#include "ui/drag.h"
#include "ui/keys.h"
#include "ui/text_area.h"
#include "ui/window.h"

#include <gtest/gtest.h>

#include <filesystem>

using namespace Lingmo;
using fmtest::runUntil;

namespace {

class Host : public fm::FolderModelHost
{
public:
    void openFolders(const std::vector<std::string>&) override {}
    void showProperties(const fm::FileItemList&) override { ++properties; }
    void showOpenWith(const std::string&) override {}
    void confirmDelete(const std::vector<std::string>&) override {}
    void confirmEmptyTrash() override {}
    void confirmLaunch(const std::string&) override {}
    void notify(const std::string&) override {}

    int properties = 0;
};

class WindowHost : public fm::FileManagerHost
{
public:
    fm::FolderWindow* openFolderWindow(const fm::FolderPage::Start&, ui::Window*) override { return nullptr; }
    void openFolders(const std::vector<std::string>&) override {}
    void showProperties(const fm::FileItemList&, ui::Window*) override {}
    void showOpenWith(const std::string&, ui::Window*) override {}
    void confirmDelete(const std::vector<std::string>&, ui::Window*) override {}
    void confirmEmptyTrash(ui::Window*) override {}
    void confirmLaunch(const std::string&, ui::Window*) override {}
    void windowClosed(fm::FolderWindow*) override {}
};

// The session's keyboard settings, as org.lingmo.Keyboard1 publishes them.
class Keyboard
{
public:
    explicit Keyboard(base::dbus::Connection* bus)
        : m_object(bus, "/org/lingmo/Keyboard1", "org.lingmo.Keyboard1")
    {
        m_object.addProperty("shortcutConvention", [this] { return m_convention; });
        m_object.addSignal("shortcutConventionChanged", "");
        m_object.publish();
        bus->requestName("org.lingmo.Keyboard1");
    }

    void set(const std::string& convention)
    {
        m_convention = convention;
        m_object.emitSignal("shortcutConventionChanged");
    }

private:
    base::dbus::Object m_object;
    std::string m_convention = "control";
};

} // namespace

extern ui::Application* s_app;

static ui::View* findView(ui::View* view, const std::string& name)
{
    if (view->name() == name)
        return view;
    for (const auto& child : view->children()) {
        if (ui::View* found = findView(child.get(), name))
            return found;
    }
    return nullptr;
}

static bool press(fm::FolderPage& page, uint32_t key, uint32_t modifiers)
{
    return page.view()->sendKey(ui::KeyEvent {.key = key, .modifiers = modifiers});
}

TEST(ShortcutTest, CommandKeysWorkOnlyUnderTheCommandConvention)
{
    base::EventLoop& loop = s_app->loop();
    base::dbus::Connection* bus = base::dbus::Connection::session(&loop);
    ASSERT_NE(bus, nullptr);
    Keyboard keyboard(bus);
    fmtest::TemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    std::filesystem::create_directories(directory.filePath("a/b"));
    const std::string a = base::url::fromLocalFile(directory.filePath("a"));
    const std::string b = base::url::fromLocalFile(directory.filePath("a/b"));
    const std::string root = base::url::fromLocalFile(directory.path());

    Host host;
    fm::FolderPage::Start start;
    start.url = b;
    fm::FolderPage page(&host, start);
    fm::FolderModel& model = page.model();
    ASSERT_TRUE(runUntil(loop, [&] { return model.status() == fm::FolderModel::Status::Ready; }));
    // The convention is read from the bus once its owner is known.
    fmtest::runFor(loop, 200);
    ASSERT_FALSE(fm::shortcuts::commandKeys());

    // Under the Ctrl convention Ctrl+Backspace is still Backspace: up a folder.
    press(page, ui::key::BackSpace, ui::ControlModifier);
    ASSERT_TRUE(runUntil(loop, [&] { return model.resolvedUrl() == a; }));

    keyboard.set("command");
    ASSERT_TRUE(runUntil(loop, [] { return fm::shortcuts::commandKeys(); }));
    ASSERT_TRUE(runUntil(loop, [&] { return model.status() == fm::FolderModel::Status::Ready; }));

    // Command+Backspace deletes, so the folder stays.
    press(page, ui::key::BackSpace, ui::ControlModifier);
    fmtest::runFor(loop, 100);
    EXPECT_EQ(model.resolvedUrl(), a);

    press(page, ui::key::Up, ui::ControlModifier);
    EXPECT_TRUE(runUntil(loop, [&] { return model.resolvedUrl() == root; }));

    press(page, '[', ui::ControlModifier);
    EXPECT_TRUE(runUntil(loop, [&] { return model.resolvedUrl() == a; }));
    press(page, ']', ui::ControlModifier);
    EXPECT_TRUE(runUntil(loop, [&] { return model.resolvedUrl() == root; }));

    ASSERT_TRUE(runUntil(loop, [&] { return model.rowForUrl(a) >= 0; }));
    model.setSelected(model.rowForUrl(a));
    press(page, 'i', ui::ControlModifier);
    EXPECT_EQ(host.properties, 1);
    press(page, ui::key::Down, ui::ControlModifier);
    EXPECT_TRUE(runUntil(loop, [&] { return model.resolvedUrl() == a; }));

    const bool hidden = model.showHiddenFiles();
    press(page, '>', ui::ControlModifier | ui::ShiftModifier);
    EXPECT_NE(model.showHiddenFiles(), hidden);

    // Plain Backspace still goes up a folder.
    press(page, ui::key::BackSpace, ui::NoModifier);
    EXPECT_TRUE(runUntil(loop, [&] { return model.resolvedUrl() == root; }));
    keyboard.set("control");
    runUntil(loop, [] { return !fm::shortcuts::commandKeys(); });
}


TEST(ShortcutTest, F2RenamesTheCurrentItem)
{
    base::EventLoop& loop = s_app->loop();
    fmtest::TemporaryDir directory;
    fmtest::writeFile(directory.filePath("notes.txt"));
    Host host;
    fm::FolderPage::Start start;
    start.url = base::url::fromLocalFile(directory.path());
    ui::Window window;
    window.resize({900, 580});
    auto owned = std::make_unique<fm::FolderPage>(&host, start);
    fm::FolderPage& page = *owned;
    window.setContentView(std::move(owned));
    window.show();
    fm::FolderModel& model = page.model();
    ASSERT_TRUE(runUntil(loop, [&] { return model.status() == fm::FolderModel::Status::Ready && model.count() == 1; }));
    window.grab();
    page.view()->setCurrentIndex(0);
    model.setSelected(0);
    window.grab();
    press(page, 0xffbf, ui::NoModifier);
    EXPECT_TRUE(page.view()->isRenaming());
    // It stays open: nothing takes the keys back from it.
    fmtest::runFor(loop, 300);
    window.grab();
    EXPECT_TRUE(page.view()->isRenaming());
}

// An item selected without becoming current, as a band selects it, is where the arrow keys start.
TEST(ShortcutTest, ArrowKeysMoveFromTheSelection)
{
    base::EventLoop& loop = s_app->loop();
    fmtest::TemporaryDir directory;
    fmtest::writeFile(directory.filePath("a.txt"));
    fmtest::writeFile(directory.filePath("b.txt"));
    fmtest::writeFile(directory.filePath("c.txt"));
    fmtest::writeFile(directory.filePath("d.txt"));
    Host host;
    fm::FolderPage::Start start;
    start.url = base::url::fromLocalFile(directory.path());
    ui::Window window;
    window.resize({900, 580});
    auto owned = std::make_unique<fm::FolderPage>(&host, start);
    fm::FolderPage& page = *owned;
    window.setContentView(std::move(owned));
    window.show();
    fm::FolderModel& model = page.model();
    ASSERT_TRUE(runUntil(loop, [&] { return model.status() == fm::FolderModel::Status::Ready && model.count() == 4; }));
    window.grab();
    const auto selected = [&] {
        const std::vector<int> rows = model.selectedRows();
        return rows.size() == 1 ? model.item(rows.front())->name : std::string();
    };

    ASSERT_EQ(page.view()->currentIndex(), -1);
    model.updateSelection({model.rowForUrl(base::url::fromLocalFile(directory.filePath("c.txt")))}, false, false);
    press(page, ui::key::Left, ui::NoModifier);
    EXPECT_EQ(selected(), "b.txt");

    model.updateSelection({model.rowForUrl(base::url::fromLocalFile(directory.filePath("d.txt")))}, false, false);
    press(page, ui::key::Left, ui::NoModifier);
    EXPECT_EQ(selected(), "c.txt");

    model.updateSelection({0, 1}, false, false);
    press(page, ui::key::Right, ui::NoModifier);
    EXPECT_EQ(selected(), "c.txt");
}

// A file that comes in while a name is typed moves the rows; the file renamed stays the one edited.
TEST(ShortcutTest, RenameFollowsItsFileWhenRowsMove)
{
    base::EventLoop& loop = s_app->loop();
    fmtest::TemporaryDir directory;
    fmtest::writeFile(directory.filePath("b.txt"));
    Host host;
    fm::FolderPage::Start start;
    start.url = base::url::fromLocalFile(directory.path());
    ui::Window window;
    window.resize({900, 580});
    auto owned = std::make_unique<fm::FolderPage>(&host, start);
    fm::FolderPage& page = *owned;
    window.setContentView(std::move(owned));
    window.show();
    fm::FolderModel& model = page.model();
    ASSERT_TRUE(runUntil(loop, [&] { return model.status() == fm::FolderModel::Status::Ready && model.count() == 1; }));
    window.grab();
    page.view()->setCurrentIndex(0);
    model.setSelected(0);
    window.grab();
    press(page, 0xffbf, ui::NoModifier);
    ASSERT_TRUE(page.view()->isRenaming());

    fmtest::writeFile(directory.filePath("a.txt"));
    model.refresh();
    ASSERT_TRUE(runUntil(loop, [&] { return model.count() == 2; }));
    ASSERT_EQ(model.rowForUrl(base::url::fromLocalFile(directory.filePath("b.txt"))), 1);

    std::string renamed;
    base::Subscription subscription = model.renaming.connect([&](std::string from, std::string) { renamed = from; });
    auto* editor = static_cast<fm::RenameEditor*>(findView(page.view(), "renameEditor"));
    ASSERT_NE(editor, nullptr);
    static_cast<ui::TextArea*>(editor->children().front().get())->setText("c.txt");
    editor->commit();
    EXPECT_EQ(renamed, base::url::fromLocalFile(directory.filePath("b.txt")));
}

// The desktop takes the file view's keys as the browser does: Ctrl+A selects every icon.
TEST(ShortcutTest, DesktopTakesTheFileKeys)
{
    base::EventLoop& loop = s_app->loop();
    fmtest::TemporaryDir directory;
    fmtest::writeFile(directory.filePath("a.txt"));
    fmtest::writeFile(directory.filePath("b.txt"));
    WindowHost host;
    fm::DesktopWindow desktop(nullptr, host);
    fm::FolderModel& model = desktop.model();
    model.setUrl(base::url::fromLocalFile(directory.path()));
    ASSERT_TRUE(runUntil(loop, [&] { return model.status() == fm::FolderModel::Status::Ready && model.count() == 2; }));
    desktop.window()->grab();
    EXPECT_TRUE(desktop.view()->sendKey(ui::KeyEvent {.key = 'a', .modifiers = ui::ControlModifier}));
    EXPECT_EQ(model.selectionCount(), 2);
    desktop.view()->sendKey(ui::KeyEvent {.key = ui::key::Escape});
    EXPECT_EQ(model.selectionCount(), 0);
}

// A new file made on the desktop lands on the cell asked for and opens for renaming there.
TEST(ShortcutTest, DesktopRenamesANewDocumentOnItsCell)
{
    base::EventLoop& loop = s_app->loop();
    fmtest::TemporaryDir directory;
    fmtest::writeFile(directory.filePath("a.txt"));
    WindowHost host;
    fm::DesktopWindow desktop(nullptr, host);
    fm::FolderModel& model = desktop.model();
    model.setUrl(base::url::fromLocalFile(directory.path()));
    desktop.window()->resize({1280, 800});
    desktop.window()->show();
    ASSERT_TRUE(runUntil(loop, [&] { return model.status() == fm::FolderModel::Status::Ready && model.count() == 1; }));
    desktop.window()->grab();
    fm::DesktopGridView& view = *desktop.view();
    ASSERT_GT(view.columns(), 2);

    // The press that opened the menu was on cell 1.
    const ui::RectF cell = view.cellRect(1);
    model.setNewDocumentPosition(int(cell.x + cell.width / 2), int(cell.y + cell.height / 2), 1);
    model.newTextFile();
    model.refresh();
    ASSERT_TRUE(runUntil(loop, [&] { return view.isRenaming(); }));
    EXPECT_EQ(view.positioner().cellOfUrl(base::url::fromLocalFile(directory.filePath("New Text"))), 1);
    EXPECT_EQ(view.currentIndex(), 1);
}

// An icon dragged to another cell of the desktop shows there once the drag
// ends: the drop moves it while the drag still blanks it.
TEST(ShortcutTest, DesktopIconShowsWhereItWasDropped)
{
    base::EventLoop& loop = s_app->loop();
    fmtest::TemporaryDir directory;
    fmtest::writeFile(directory.filePath("a.txt"));
    WindowHost host;
    fm::DesktopWindow desktop(nullptr, host);
    fm::FolderModel& model = desktop.model();
    model.setUrl(base::url::fromLocalFile(directory.path()));
    desktop.window()->resize({1280, 800});
    desktop.window()->show();
    ASSERT_TRUE(runUntil(loop, [&] { return model.status() == fm::FolderModel::Status::Ready && model.count() == 1; }));
    desktop.window()->grab();
    fm::DesktopGridView& view = *desktop.view();
    ASSERT_GT(view.columns(), 2);
    const std::string url = base::url::fromLocalFile(directory.filePath("a.txt"));
    const auto shownCells = [&] {
        std::vector<ui::RectF> cells;
        for (const auto& child : view.contentView()->children()) {
            if (child->isVisible())
                cells.push_back(child->geometry());
        }
        return cells;
    };

    model.setSelected(0);
    model.setDragging(true);
    EXPECT_TRUE(shownCells().empty());

    const ui::RectF target = view.cellRect(view.rows() * 2);
    model.drop({url}, -1, ui::MoveAction, int(target.x + target.width / 2), int(target.y + target.height / 2));
    model.setDragging(false);
    EXPECT_EQ(view.positioner().cellOfUrl(url), view.rows() * 2);
    const std::vector<ui::RectF> shown = shownCells();
    ASSERT_EQ(shown.size(), 1u);
    EXPECT_EQ(shown.front(), target);
}
