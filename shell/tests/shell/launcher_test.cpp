// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <algorithm>

#include "base/file.h"
#include "base/settings.h"
#include "base/strings.h"
#include "dbus_test_client.h"
#include "loop_test_util.h"
#include "shell/common/app_launcher.h"
#include "shell/common/file_thread.h"
#include "shell/common/task_manager.h"
#include "shell/launcher/launcher.h"
#include "shell_test_util.h"
#include "temporary_directory.h"

using namespace Lingmo;
namespace dbus = base::dbus;

namespace {

class LauncherTest : public testing::Test
{
protected:
    LauncherTest()
    {
        applications.addSimple("zeta", "Zeta");
        applications.addSimple("alpha", "Alpha", "", "GenericName=Web Browser\n");
        applications.addSimple("mid", "Middle", "", "Comment=\xc3\x89" "cran\n");
        applications.addSimple("console", "Console", "", "Terminal=true\n");
    }

    void start(const std::string &settings = {})
    {
        if (!settings.empty())
            directory.write("shell.conf", settings);
        launcher = std::make_unique<shell::Launcher>(dbus::Connection::session(core::mainLoop()), tasks, starter,
                                                     nullptr, directory.filePath("shell.conf"));
    }

    shell::LauncherModel &model() { return launcher->applications(); }

    std::vector<std::string> ids()
    {
        std::vector<std::string> list;
        for (const auto &item : model().rows())
            list.push_back(item.id);
        return list;
    }

    test::TemporaryDirectory directory;
    test::FakeApplications applications;
    test::FakeTaskWindows windows;
    shell::TaskManager tasks{windows, applications, [](int64_t) { return shell::ProcessCommand(); }};
    shell::ApplicationLauncher starter{nullptr};
    std::unique_ptr<shell::Launcher> launcher;
};

} // namespace

TEST_F(LauncherTest, SortsTheFirstListAndKeepsTheUsersOrderAfter)
{
    start();
    applications.scan();
    ASSERT_TRUE(test::runUntil([&] { return model().count() == 3; }));
    // Alphabetical in the user's language; terminal programs are left out.
    EXPECT_EQ(ids(), (std::vector<std::string>{"alpha", "mid", "zeta"}));
    EXPECT_FALSE(model().rows()[0].newInstalled);

    model().move(2, 0);
    EXPECT_EQ(ids(), (std::vector<std::string>{"zeta", "alpha", "mid"}));
    model().save();
    shell::waitForFileThread();

    const base::Settings settings(directory.filePath("shell.conf"));
    EXPECT_EQ(settings.stringListValue("Launcher/order"), (std::vector<std::string>{"zeta", "alpha", "mid"}));
    EXPECT_TRUE(settings.contains("Launcher/newAppIds"));

    // A new installation is appended and marked new.
    applications.addSimple("beta", "Beta");
    applications.scan();
    EXPECT_EQ(ids(), (std::vector<std::string>{"zeta", "alpha", "mid", "beta"}));
    EXPECT_TRUE(model().rows()[3].newInstalled);
    model().clearNewInstalled({"beta"});
    EXPECT_FALSE(model().rows()[3].newInstalled);
}

TEST_F(LauncherTest, ReadsTheStoredOrder)
{
    start("[Launcher]\norder=zeta, gone, mid\nnewAppIds=mid\n");
    applications.scan();
    ASSERT_TRUE(test::runUntil([&] { return model().count() == 3; }));
    EXPECT_EQ(ids(), (std::vector<std::string>{"zeta", "mid", "alpha"}));
    EXPECT_TRUE(model().rows()[1].newInstalled);
    EXPECT_TRUE(model().rows()[2].newInstalled);
    EXPECT_EQ(model().rows()[2].name, "Alpha");
}

TEST_F(LauncherTest, AnUnknownListIsAFirstStart)
{
    start("[Launcher]\nlist=@ByteArray(\\0\\0\\0\\x1)\n");
    applications.scan();
    ASSERT_TRUE(test::runUntil([&] { return model().count() == 3; }));
    EXPECT_EQ(ids(), (std::vector<std::string>{"alpha", "mid", "zeta"}));
    EXPECT_FALSE(model().rows()[0].newInstalled);
}

TEST_F(LauncherTest, SearchesNamesCommentsAndIds)
{
    start();
    applications.scan();
    ASSERT_TRUE(test::runUntil([&] { return model().count() == 3; }));
    model().search("  browser ");
    EXPECT_EQ(ids(), (std::vector<std::string>{"alpha"}));
    model().search("\xc3\xa9" "c");
    EXPECT_EQ(ids(), (std::vector<std::string>{"mid"}));
    // No moving while a search shows.
    model().move(0, 1);
    model().search("");
    EXPECT_EQ(ids(), (std::vector<std::string>{"alpha", "mid", "zeta"}));
}

TEST_F(LauncherTest, CreatesPersistsAndSearchesFolders)
{
    start();
    applications.scan();
    ASSERT_TRUE(test::runUntil([&] { return model().count() == 3; }));
    ASSERT_TRUE(model().makeFolder("alpha", "mid"));
    ASSERT_EQ(2, model().count());
    const auto folder = std::find_if(model().rows().begin(), model().rows().end(),
                                     [](const shell::LauncherItem &item) { return item.folder; });
    ASSERT_NE(model().rows().end(), folder);
    const std::string folderId = folder->id;
    EXPECT_EQ(2u, model().folderItems(folderId).size());
    EXPECT_TRUE(model().renameFolder(folderId, "Work"));
    const auto renamed = std::find_if(model().rows().begin(), model().rows().end(),
                                      [&](const shell::LauncherItem &item) { return item.id == folderId; });
    ASSERT_NE(model().rows().end(), renamed);
    EXPECT_EQ("Work", renamed->name);
    EXPECT_FALSE(model().renameFolder(folderId, "   "));
    EXPECT_FALSE(model().makeFolder(folderId, "zeta"));
    model().search("browser");
    ASSERT_EQ(1, model().count());
    EXPECT_EQ("alpha", model().rows().front().id);
    model().search({});
    model().save();
    shell::waitForFileThread();

    launcher.reset();
    start();
    applications.scan();
    ASSERT_TRUE(test::runUntil([&] { return model().count() == 2; }));
    EXPECT_EQ(2u, model().folderItems(folderId).size());
    ASSERT_TRUE(model().removeFromFolder(folderId, "alpha"));
    EXPECT_EQ(-1, model().findById(folderId));
    EXPECT_GE(model().findById("mid"), 0);
    EXPECT_GE(model().findById("alpha"), 0);

    ASSERT_TRUE(model().makeFolder("alpha", "mid"));
    const auto nested = std::find_if(model().allItems().begin(), model().allItems().end(),
                                     [](const shell::LauncherItem &item) { return item.folder; });
    ASSERT_NE(model().allItems().end(), nested);
    const std::string nestedId = nested->id;
    ASSERT_TRUE(model().makeFolder("zeta", nestedId));
    EXPECT_EQ(3u, model().folderItems(nestedId).size());
    ASSERT_TRUE(model().removeFromFolder(nestedId, "alpha"));
    EXPECT_EQ(2u, model().folderItems(nestedId).size());
    ASSERT_TRUE(model().dissolveFolder(nestedId));
    EXPECT_EQ(-1, model().findById(nestedId));
    EXPECT_GE(model().findById("alpha"), 0);
    EXPECT_GE(model().findById("mid"), 0);
    EXPECT_GE(model().findById("zeta"), 0);
}

TEST_F(LauncherTest, PagesFollowTheList)
{
    start();
    applications.scan();
    ASSERT_TRUE(test::runUntil([&] { return model().count() == 3; }));
    shell::LauncherPageModel page(model());
    page.setLimitCount(2);
    EXPECT_EQ(page.count(), 2);
    EXPECT_EQ(page.item(0)->id, "alpha");
    page.setStartIndex(2);
    EXPECT_EQ(page.count(), 1);
    EXPECT_EQ(page.item(0)->id, "zeta");
    EXPECT_EQ(page.sourceRow(0), 2);

    page.setStartIndex(0);
    int moves = 0;
    auto subscription = page.rowMoved.connect([&](int, int) { ++moves; });
    model().move(1, 0);
    EXPECT_EQ(page.item(0)->id, "mid");
    EXPECT_EQ(moves, 1);
}

TEST_F(LauncherTest, ShowsHidesAndRemembersOpenedApplications)
{
    applications.scan();
    start();
    ASSERT_TRUE(launcher->registerService());
    int shown = 0;
    launcher->showWindow = [&] { ++shown; };
    launcher->hideWindow = [&] { --shown; };

    test::DBusTestClient session;
    session.call("org.lingmo.Launcher", "/Launcher", "org.lingmo.Launcher", "toggle");
    EXPECT_TRUE(launcher->isShown());
    EXPECT_EQ(shown, 1);
    // A drag keeps it up when it loses activation.
    launcher->setDragging(true);
    launcher->deactivated();
    EXPECT_TRUE(launcher->isShown());
    launcher->setDragging(false);
    launcher->deactivated();
    EXPECT_FALSE(launcher->isShown());

    shell::TaskWindow window;
    window.appId = "zeta";
    windows.add(window);
    EXPECT_EQ(launcher->takeOpenedApplications(), (std::vector<std::string>{"zeta"}));
    EXPECT_TRUE(launcher->takeOpenedApplications().empty());
}

// A search for "te" over a typical set of applications, in the en_SG locale:
// name, generic name, comment and desktop id, as substrings ignoring case,
// in the user's order.
TEST(LauncherSearchTest, MatchesTheQtLauncherOnTheReferenceApplications)
{
    struct App {
        const char *id;
        const char *name;
        const char *genericName;
        const char *comment;
    };
    const App apps[] = {
        {"io.bassi.Amberol", "Amberol", "Music Player", ""},
        {"chromium", "Chromium Web Browser", "Web Browser", "Access the Internet"},
        {"org.gnome.dfeet", "D-Feet", "D-Bus Debugger", "Debug D-Bus applications"},
        {"lingmo-filemanager", "Files", "File Manager", "Browse and manage your files"},
        {"com.mitchellh.ghostty", "Ghostty", "", "A terminal emulator"},
        {"google-chrome", "Google Chrome", "Web Browser", "Access the Internet"},
        {"org.gnome.Loupe", "Image Viewer", "", ""},
        {"libreoffice-startcenter", "LibreOffice", "Office",
         "Launch applications to create text documents, spreadsheets, presentations, drawings, formulas, and "
         "databases, or open recently used documents."},
        {"libreoffice-calc", "LibreOffice Calc", "Spreadsheet",
         "Perform calculations, analyze information and manage lists in spreadsheets."},
        {"libreoffice-draw", "LibreOffice Draw", "Drawing Program", "Create and edit drawings, flow charts and logos."},
        {"libreoffice-impress", "LibreOffice Impress", "Presentation",
         "Create and edit presentations for slideshows, meetings and Web pages."},
        {"libreoffice-math", "LibreOffice Math", "Formula Editor", "Create and edit scientific formulas and equations."},
        {"libreoffice-writer", "LibreOffice Writer", "Word Processor",
         "Create and edit text and graphics in letters, reports, documents and Web pages."},
        {"com.obsproject.Studio", "OBS Studio", "Streaming/Recording Software",
         "Free and Open Source Streaming/Recording Software"},
        {"lingmo-screenshot", "Screenshot", "", "Capture or record the screen, a window or a selected portion"},
        {"lingmo-settings", "Settings", "", ""},
        {"dev.lizardbyte.app.Sunshine", "Sunshine", "", "Self-hosted game stream host for Moonlight"},
        {"lingmo-terminal", "Terminal", "", ""},
        {"org.gnome.TextEditor", "Text Editor", "Text Editor", "View and edit text files"},
        {"vlc", "VLC media player", "Media player", "Read, capture, broadcast your multimedia streams"},
        {"lingmo-system-monitor", "System Monitor", "", "View resource usage and manage running processes"},
        {"lingmo-video-player", "Video Player", "", "Play videos and other media files"},
        {"lingmo-text-editor", "Text Editor", "", "Edit plain text files"},
        {"org.keepassxc.KeePassXC", "KeePassXC", "Password Manager",
         "Community-driven port of the Windows application \xe2\x80\x9cKeePass Password Safe\xe2\x80\x9d"},
        {"lingmo-calendar", "Calendar", "", "Keep track of events and appointments"},
        {"org.gnome.Cheese", "Cheese", "Webcam Booth", "Take photos and videos with your webcam, with fun graphical effects"},
        {"org.gnome.SoundRecorder", "Sound Recorder", "", "Record sound via the microphone and play it back"},
        {"ai.storyteller.photocraft", "PhotoCraft", "Image Editor", "Edit photos and layered PSD documents"},
        {"firefox", "Firefox", "Web Browser", "Fast and private browser"},
    };
    test::TemporaryDirectory directory;
    test::FakeApplications applications;
    std::string order;
    for (const App &app : apps) {
        std::string extra;
        if (*app.genericName)
            extra += std::string("GenericName=") + app.genericName + "\n";
        if (*app.comment)
            extra += std::string("Comment=") + app.comment + "\n";
        applications.addSimple(app.id, app.name, "", extra);
        order += std::string(order.empty() ? "" : ", ") + app.id;
    }
    directory.write("shell.conf", "[Launcher]\norder=" + order + "\n");
    test::FakeTaskWindows windows;
    shell::TaskManager tasks{windows, applications, [](int64_t) { return shell::ProcessCommand(); }};
    shell::ApplicationLauncher starter{nullptr};
    shell::Launcher launcher(nullptr, tasks, starter, nullptr, directory.filePath("shell.conf"));
    applications.scan();
    ASSERT_TRUE(test::runUntil([&] { return launcher.applications().count() == int(std::size(apps)); }));

    launcher.applications().search("te");
    std::vector<std::string> found;
    for (const auto &item : launcher.applications().rows())
        found.push_back(item.id);
    EXPECT_EQ(found, (std::vector<std::string>{
                             "chromium", "lingmo-filemanager", "com.mitchellh.ghostty", "google-chrome",
                             "libreoffice-startcenter", "libreoffice-draw", "libreoffice-impress", "libreoffice-math",
                             "libreoffice-writer", "lingmo-screenshot", "lingmo-settings",
                             "dev.lizardbyte.app.Sunshine", "lingmo-terminal", "org.gnome.TextEditor",
                             "lingmo-system-monitor", "lingmo-video-player", "lingmo-text-editor",
                             "lingmo-calendar", "ai.storyteller.photocraft", "firefox"}));
}
