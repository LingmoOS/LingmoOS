// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testutil.h"

#include "model/foldermodel.h"
#include "views/folderpage.h"
#include "views/folderview.h"
#include "views/folderwindow.h"
#include "views/renameeditor.h"

#include "base/url.h"
#include "ui/application.h"
#include "ui/test_input.h"
#include "ui/text_area.h"
#include "ui/window.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <memory>

using namespace Lingmo;
using fmtest::runUntil;

extern ui::Application* s_app;

namespace {

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

} // namespace

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

static void dragBy(ui::Window& window, ui::PointF from, ui::PointF delta)
{
    const ui::PointF to = {from.x + delta.x, from.y + delta.y};
    ui::TestInput::movePointer(window, from);
    ui::TestInput::press(window, from);
    ui::TestInput::movePointer(window, to);
    ui::TestInput::release(window, to);
}

// A maximized window still moves from the path bar, as from the rest of the
// header; a click there still opens the editor.
TEST(HeaderTest, PathBarDragsMoveTheWindow)
{
    base::EventLoop& loop = s_app->loop();
    fmtest::TemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    std::filesystem::create_directories(directory.filePath("a/b"));
    const std::string url = base::url::fromLocalFile(directory.filePath("a/b"));

    WindowHost host;
    fm::FolderPage::Start start;
    start.url = url;
    fm::FolderWindow folderWindow(&host, start);
    ui::Window& window = folderWindow.window();
    folderWindow.show();
    fm::FolderModel& model = folderWindow.currentPage()->model();
    ASSERT_TRUE(runUntil(loop, [&] { return model.status() == fm::FolderModel::Status::Ready; }));
    window.setMaximized(true);
    window.grab();
    ASSERT_TRUE(window.states().maximized);

    ui::View* pathBar = findView(window.contentView(), "pathBar");
    ui::View* editor = findView(window.contentView(), "pathEditor");
    ASSERT_NE(pathBar, nullptr);
    ASSERT_NE(editor, nullptr);
    const double middle = pathBar->geometry().height / 2;
    const ui::PointF empty = pathBar->mapToWindow({pathBar->geometry().width - 10, middle});
    const ui::PointF crumb = pathBar->mapToWindow({20, middle});

    dragBy(window, empty, {0, 20});
    EXPECT_EQ(ui::TestInput::moveRequests(window), 1);
    window.grab();
    EXPECT_FALSE(editor->isVisible());

    dragBy(window, crumb, {30, 0});
    EXPECT_EQ(ui::TestInput::moveRequests(window), 2);
    fmtest::runFor(loop, 100);
    EXPECT_EQ(folderWindow.currentPage()->currentUrl(), directory.filePath("a/b"));

    ui::TestInput::click(window, empty);
    window.grab();
    EXPECT_TRUE(editor->isVisible());
    EXPECT_EQ(ui::TestInput::moveRequests(window), 2);
}

// A window can go in the turn its tabs changed: what the tab bar left for the
// next turn must not reach it.
TEST(HeaderTest, WindowGoesRightAfterItsTabsChange)
{
    base::EventLoop& loop = s_app->loop();
    fmtest::TemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const std::string url = base::url::fromLocalFile(directory.path());

    WindowHost host;
    fm::FolderPage::Start start;
    start.url = url;
    auto folderWindow = std::make_unique<fm::FolderWindow>(&host, start);
    folderWindow->show();
    fmtest::runFor(loop, 50);

    folderWindow->openTab(url);
    EXPECT_EQ(folderWindow->tabCount(), 2);
    folderWindow.reset();
    fmtest::runFor(loop, 50);
}

// A new session language takes the text again where the user was: the tabs,
// the folder and its history, the selection and an open rename with its text.
TEST(HeaderTest, LanguageChangeKeepsWhereTheUserWas)
{
    base::EventLoop& loop = s_app->loop();
    fmtest::TemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    std::filesystem::create_directories(directory.filePath("a"));
    ASSERT_TRUE(fmtest::writeFile(directory.filePath("a/notes.txt")));
    ASSERT_TRUE(fmtest::writeFile(directory.filePath("a/other.txt")));
    const std::string other = base::url::fromLocalFile(directory.filePath("a/other.txt"));

    WindowHost host;
    fm::FolderPage::Start start;
    start.url = base::url::fromLocalFile(directory.path());
    fm::FolderWindow folderWindow(&host, start);
    ui::Window& window = folderWindow.window();
    folderWindow.show();
    folderWindow.openTab(start.url);
    folderWindow.setCurrentTab(0);
    fm::FolderPage* page = folderWindow.currentPage();
    fm::FolderModel& model = page->model();
    page->openUrl(directory.filePath("a"));
    ASSERT_TRUE(runUntil(loop, [&] { return model.status() == fm::FolderModel::Status::Ready && model.count() == 2; }));
    window.grab();
    const int row = model.rowForUrl(other);
    ASSERT_GE(row, 0);
    fm::FolderView* view = page->view();
    view->setCurrentIndex(row);
    model.setSelected(row);
    window.grab();
    view->rename();
    ASSERT_TRUE(view->isRenaming());
    auto* editor = static_cast<fm::RenameEditor*>(findView(view, "renameEditor"));
    ASSERT_NE(editor, nullptr);
    auto* field = static_cast<ui::TextArea*>(editor->children().front().get());
    field->setText("draft.txt");

    ui::Application::instance()->languageChanged.emit();
    fmtest::runFor(loop, 100);
    ASSERT_TRUE(runUntil(loop, [&] { return model.status() == fm::FolderModel::Status::Ready; }));
    window.grab();

    EXPECT_EQ(folderWindow.tabCount(), 2);
    EXPECT_EQ(folderWindow.currentTab(), 0);
    EXPECT_EQ(folderWindow.currentPage(), page);
    EXPECT_EQ(page->currentUrl(), directory.filePath("a"));
    EXPECT_TRUE(model.canGoBack());
    EXPECT_EQ(model.count(), 2);
    EXPECT_EQ(model.selectedUrls(), std::vector<std::string> {other});
    EXPECT_EQ(page->view(), view);
    EXPECT_TRUE(view->isRenaming());
    EXPECT_EQ(findView(view, "renameEditor"), editor);
    EXPECT_EQ(field->text(), "draft.txt");
}
