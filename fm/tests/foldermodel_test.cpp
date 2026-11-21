// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "archives.h"
#include "testutil.h"

#include "core/preferences.h"
#include "model/foldermodel.h"
#include "ops/operations.h"

#include "base/dbus.h"
#include "base/process.h"
#include "base/url.h"
#include "ui/application.h"
#include "ui/clipboard.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <map>

using namespace Lingmo;
using fm::FolderModel;
using fmtest::runUntil;
using fmtest::TemporaryDir;
using fmtest::writeFile;

extern ui::Application* s_app;

static base::EventLoop& loop()
{
    return s_app->loop();
}

static std::vector<std::string> names(const FolderModel& folder)
{
    std::vector<std::string> result;
    for (int row = 0; row < folder.count(); ++row)
        result.push_back(folder.item(row)->name);
    return result;
}

static bool ready(const FolderModel& folder)
{
    return folder.status() == FolderModel::Status::Ready;
}

static std::string url(const std::string& path)
{
    return base::url::fromLocalFile(path);
}

TEST(FolderModelTest, NamesSortNumerically)
{
    TemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    for (const char* name : {"img10.png", "img2.png", "img1.png", "img100.png", "Img3.png"})
        ASSERT_TRUE(writeFile(directory.filePath(name)));
    const std::vector<std::string> expected = {"img1.png", "img2.png", "Img3.png", "img10.png", "img100.png"};

    FolderModel folder;
    folder.setSortMode(fm::Preferences::Name);
    folder.setSortDesc(false);
    folder.setUrl(directory.path());
    ASSERT_TRUE(runUntil(loop(), [&] { return ready(folder) && folder.count() == 5; }));
    EXPECT_EQ(names(folder), expected);

    std::vector<std::string> reversed(expected.rbegin(), expected.rend());
    folder.setSortDesc(true);
    EXPECT_TRUE(runUntil(loop(), [&] { return names(folder) == reversed; }));
}

// A refresh merges changes into the rows: what did not change keeps its item.
TEST(FolderModelTest, RefreshKeepsUnchangedItems)
{
    TemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    std::map<std::string, std::string> expected;
    for (int i = 0; i < 800; ++i) {
        char name[16];
        std::snprintf(name, sizeof(name), "%04d.txt", i);
        expected[name] = std::to_string(i);
        ASSERT_TRUE(writeFile(directory.filePath(name), expected[name]));
    }

    FolderModel folder;
    folder.setSortMode(fm::Preferences::Name);
    folder.setSortDesc(false);
    folder.setUrl(directory.path());
    ASSERT_TRUE(runUntil(loop(), [&] { return ready(folder) && folder.count() == int(expected.size()); }, 15000));

    for (int round = 0; round < 3; ++round) {
        std::map<std::string, fm::FileItemPtr> previous;
        for (int row = 0; row < folder.count(); ++row)
            previous[folder.item(row)->name] = folder.item(row);

        std::vector<std::string> keys;
        for (const auto& [name, contents] : expected)
            keys.push_back(name);
        std::map<std::string, bool> changed;
        for (size_t i = 0; i < keys.size(); ++i) {
            const std::string& name = keys[i];
            if ((round == 0 && i < 300) || (round > 0 && i % 3 == 0)) {
                std::filesystem::remove(directory.filePath(name));
                expected.erase(name);
            } else if (i % 5 == 0) {
                expected[name] += "-updated";
                changed[name] = true;
                ASSERT_TRUE(writeFile(directory.filePath(name), expected[name]));
            }
        }
        for (int i = 0; i < 270; ++i) {
            char name[32];
            std::snprintf(name, sizeof(name), "new-%d-%04d.txt", round, i);
            expected[name] = std::string(size_t(i % 31 + 1), 'x');
            ASSERT_TRUE(writeFile(directory.filePath(name), expected[name]));
        }

        bool completed = false;
        const base::Subscription subscription = folder.listingCompleted.connect([&] { completed = true; });
        folder.refresh();
        ASSERT_TRUE(runUntil(loop(), [&] { return completed && ready(folder) && folder.count() == int(expected.size()); },
                             15000));

        int row = 0;
        for (const auto& [name, contents] : expected) {
            const fm::FileItemPtr item = folder.item(row);
            ASSERT_EQ(item->name, name);
            EXPECT_EQ(item->size, contents.size());
            EXPECT_EQ(folder.rowForUrl(url(directory.filePath(name))), row);
            const auto before = previous.find(name);
            // Unchanged files keep their item, so views keep what they made for it.
            if (before != previous.end() && !changed.count(name))
                EXPECT_EQ(before->second, item) << name;
            ++row;
        }
    }
}

TEST(FolderModelTest, SortingKeepsTheSelection)
{
    TemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    for (const char* name : {"a.txt", "b.txt", "c.txt", "d.txt"})
        ASSERT_TRUE(writeFile(directory.filePath(name)));

    FolderModel folder;
    folder.setSortMode(fm::Preferences::Name);
    folder.setSortDesc(false);
    folder.setUrl(directory.path());
    ASSERT_TRUE(runUntil(loop(), [&] { return ready(folder) && folder.count() == 4; }));
    folder.setSelected(folder.rowForUrl(url(directory.filePath("b.txt"))));
    folder.toggleSelected(folder.rowForUrl(url(directory.filePath("d.txt"))));

    folder.setSortDesc(true);
    ASSERT_TRUE(runUntil(loop(), [&] { return folder.item(0)->name == "d.txt"; }));
    std::vector<std::string> selected = folder.selectedUrls();
    std::sort(selected.begin(), selected.end());
    EXPECT_EQ(selected, (std::vector<std::string> {url(directory.filePath("b.txt")), url(directory.filePath("d.txt"))}));
}

TEST(FolderModelTest, SearchingClearsTheSelection)
{
    TemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    for (const char* name : {"a.txt", "b.txt"})
        ASSERT_TRUE(writeFile(directory.filePath(name)));

    FolderModel folder;
    folder.setUrl(directory.path());
    ASSERT_TRUE(runUntil(loop(), [&] { return ready(folder) && folder.count() == 2; }));
    folder.setSelected(folder.rowForUrl(url(directory.filePath("a.txt"))));
    ASSERT_EQ(folder.selectedPath(), directory.filePath("a.txt"));

    int changes = 0;
    const base::Subscription subscription = folder.selectionChanged.connect([&] { ++changes; });
    folder.setSearchText("b");
    EXPECT_GT(changes, 0);
    EXPECT_TRUE(folder.selectedPath().empty());
}

TEST(FolderModelTest, OpeningAFileShowsItsFolder)
{
    TemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ASSERT_TRUE(writeFile(directory.filePath("file.txt")));

    FolderModel folder;
    folder.setUrl(directory.filePath("file.txt"));
    ASSERT_TRUE(runUntil(loop(), [&] { return ready(folder) && folder.count() == 1; }));
    EXPECT_EQ(folder.resolvedUrl(), url(directory.path()));
    EXPECT_FALSE(folder.canGoBack());

    std::vector<std::string> errors;
    const base::Subscription subscription
            = folder.notification.connect([&](const std::string& message) { errors.push_back(message); });
    folder.setUrl(directory.filePath("missing"));
    EXPECT_TRUE(runUntil(loop(), [&] { return !errors.empty(); }));
}

TEST(FolderModelTest, BackAndForwardKeepViewPositions)
{
    TemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    std::filesystem::create_directories(directory.filePath("a/b"));
    const std::string root = url(directory.path());
    const std::string a = url(directory.filePath("a"));
    const std::string b = url(directory.filePath("a/b"));

    FolderModel folder;
    int leaves = 0;
    const base::Subscription subscription
            = folder.aboutToLeaveFolder.connect([&] { folder.setViewPosition(++leaves * 100); });

    folder.setUrl(root);
    EXPECT_TRUE(std::isnan(folder.viewPosition()));
    folder.setUrl(a);
    folder.setUrl(b);

    folder.goBack();
    EXPECT_EQ(folder.resolvedUrl(), a);
    EXPECT_TRUE(folder.canGoBack());
    EXPECT_TRUE(folder.canGoForward());
    EXPECT_EQ(folder.viewPosition(), 300.0);

    folder.goBack();
    EXPECT_EQ(folder.resolvedUrl(), root);
    EXPECT_FALSE(folder.canGoBack());
    EXPECT_EQ(folder.viewPosition(), 200.0);

    folder.goForward();
    EXPECT_EQ(folder.resolvedUrl(), a);
    EXPECT_EQ(folder.viewPosition(), 500.0);
    folder.goForward();
    EXPECT_EQ(folder.resolvedUrl(), b);
    EXPECT_FALSE(folder.canGoForward());
    EXPECT_EQ(folder.viewPosition(), 400.0);

    folder.goBack();
    folder.setUrl(root);
    EXPECT_FALSE(folder.canGoForward());
    EXPECT_TRUE(std::isnan(folder.viewPosition()));
}

TEST(FolderModelTest, BackForwardAndUpRestoreTheSelection)
{
    TemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    std::filesystem::create_directories(directory.filePath("a/b"));
    std::filesystem::create_directories(directory.filePath("c"));
    ASSERT_TRUE(writeFile(directory.filePath("a/file.txt")));
    const std::string root = url(directory.path());
    const std::string a = url(directory.filePath("a"));
    const std::string b = url(directory.filePath("a/b"));
    const std::string file = url(directory.filePath("a/file.txt"));
    using Urls = std::vector<std::string>;

    FolderModel folder;
    folder.setUrl(root);
    ASSERT_TRUE(runUntil(loop(), [&] { return ready(folder) && folder.rowForUrl(a) >= 0; }));
    folder.setSelected(folder.rowForUrl(a));
    folder.setUrl(a);
    ASSERT_TRUE(runUntil(loop(), [&] { return ready(folder) && folder.rowForUrl(b) >= 0; }));
    folder.setSelected(folder.rowForUrl(b));
    folder.setUrl(b);
    ASSERT_TRUE(runUntil(loop(), [&] { return ready(folder); }));

    folder.goBack();
    EXPECT_TRUE(runUntil(loop(), [&] { return folder.selectedUrls() == Urls {b}; }));
    folder.clearSelection();
    ASSERT_TRUE(runUntil(loop(), [&] { return folder.rowForUrl(file) >= 0; }));
    folder.setSelected(folder.rowForUrl(file));

    folder.goBack();
    EXPECT_TRUE(runUntil(loop(), [&] { return folder.selectedUrls() == Urls {a}; }));

    folder.goForward();
    EXPECT_TRUE(runUntil(loop(), [&] { return folder.selectedUrls() == Urls {file}; }));

    folder.goForward();
    ASSERT_TRUE(runUntil(loop(), [&] { return ready(folder) && folder.resolvedUrl() == b; }));
    folder.up();
    EXPECT_TRUE(runUntil(loop(), [&] { return folder.resolvedUrl() == a; }));
    EXPECT_TRUE(runUntil(loop(), [&] { return folder.selectedUrls() == Urls {b}; }));
}

// The menus follow the selection: a file, a folder, and the folder itself.
TEST(FolderModelTest, ContextMenuActionsFollowTheSelection)
{
    TemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ASSERT_TRUE(writeFile(directory.filePath("notes.txt")));
    std::filesystem::create_directories(directory.filePath("folder"));

    FolderModel folder;
    folder.setUrl(directory.path());
    ASSERT_TRUE(runUntil(loop(), [&] { return ready(folder) && folder.count() == 2; }));

    folder.setSelected(folder.rowForUrl(url(directory.filePath("notes.txt"))));
    folder.prepareContextMenu();
    EXPECT_TRUE(folder.action("open").visible);
    EXPECT_TRUE(folder.action("openWith").visible);
    EXPECT_TRUE(folder.action("copy").enabled);
    EXPECT_TRUE(folder.action("rename").visible);
    EXPECT_EQ(folder.action("compress").text, "Compress “notes.txt”");
    EXPECT_FALSE(folder.action("pasteInto").visible);

    folder.clearSelection();
    folder.setSelected(folder.rowForUrl(url(directory.filePath("folder"))));
    folder.prepareContextMenu();
    EXPECT_FALSE(folder.action("open").visible);
    EXPECT_FALSE(folder.action("openWith").visible);
    EXPECT_TRUE(folder.action("pasteInto").visible);

    folder.setSelected(folder.rowForUrl(url(directory.filePath("notes.txt"))));
    folder.prepareContextMenu();
    EXPECT_FALSE(folder.action("pasteInto").visible);

    folder.clearSelection();
    folder.prepareContextMenu();
    EXPECT_TRUE(folder.action("newFolder").enabled);
    EXPECT_TRUE(folder.action("newTextFile").visible);
}


// The clipboard's URLs while it holds a cut; empty otherwise.
static std::vector<std::string> cutUrls()
{
    const auto types = ui::Clipboard::mimeTypes();
    if (std::find(types.begin(), types.end(), "application/x-lingmo-cutselection") == types.end())
        return {};
    std::vector<std::string> urls;
    ui::Clipboard::requestData("text/uri-list", [&](std::optional<std::string> data) {
        ui::MimeData mime;
        mime.setData("text/uri-list", data.value_or(""));
        urls = mime.urls();
    });
    return urls;
}

// Cuts the named files of the source and pastes them into the destination;
// the move it submitted.
static fm::OperationTaskPtr cutAndPaste(FolderModel& source, const std::vector<std::string>& names,
                                        FolderModel& destination)
{
    source.clearSelection();
    for (const std::string& name : names)
        source.setSelected(source.rowForUrl(url(source.rootItem().path + "/" + name)));
    source.prepareContextMenu();
    source.cut();

    fm::OperationTaskPtr move;
    const base::Subscription added = fm::OperationsClient::instance().taskAdded.connect(
            [&](fm::OperationTaskPtr task) { move = task; });
    destination.paste();
    runUntil(loop(), [&] { return move != nullptr; });
    return move;
}

// A cut stays until its move is done: one that fails leaves it whole.
// A rename that failed leaves no expectation behind: the file whose name it
// asked for keeps its own row when the old file goes later.
TEST(FolderModelTest, FailedRenameLeavesTheTakenNameAlone)
{
    TemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ASSERT_TRUE(writeFile(directory.filePath("a")));
    ASSERT_TRUE(writeFile(directory.filePath("z")));

    FolderModel folder;
    folder.setSortMode(fm::Preferences::Name);
    folder.setSortDesc(false);
    folder.setUrl(directory.path());
    ASSERT_TRUE(runUntil(loop(), [&] { return ready(folder) && folder.count() == 2; }));

    bool failed = false;
    const base::Subscription renameFailed =
            folder.renameFailed.connect([&](const std::string&, const std::string&) { failed = true; });
    // The test bus has no operations daemon, so the rename fails.
    folder.rename(folder.rowForUrl(url(directory.filePath("a"))), "z");
    ASSERT_TRUE(runUntil(loop(), [&] { return failed; }));

    std::vector<std::string> removed;
    const base::Subscription changed = folder.rowsChanged.connect([&](const fm::ContentsChange& change) {
        removed.insert(removed.end(), change.removed.begin(), change.removed.end());
    });
    std::filesystem::remove(directory.filePath("a"));
    ASSERT_TRUE(runUntil(loop(), [&] { return folder.count() == 1; }));
    EXPECT_EQ(removed, std::vector<std::string> {url(directory.filePath("a"))});
    EXPECT_EQ(names(folder), std::vector<std::string> {"z"});
}

TEST(FolderModelTest, FailedMoveKeepsTheCut)
{
    TemporaryDir from;
    TemporaryDir to;
    ASSERT_TRUE(writeFile(from.filePath("a")));
    FolderModel source;
    source.setUrl(from.path());
    FolderModel destination;
    destination.setUrl(to.path());
    ASSERT_TRUE(runUntil(loop(), [&] { return ready(source) && source.count() == 1 && ready(destination); }));

    // The test bus has no operations daemon, so the move fails.
    const fm::OperationTaskPtr move = cutAndPaste(source, {"a"}, destination);
    ASSERT_TRUE(move);
    ASSERT_TRUE(runUntil(loop(), [&] { return move->isFinished(); }));
    EXPECT_NE(move->error(), 0);
    EXPECT_EQ(cutUrls(), std::vector<std::string> {url(from.filePath("a"))});
    ui::Clipboard::clear();
}

// A move cancelled at a conflict leaves the cut holding what it did not
// move; one that succeeds clears it.
TEST(FolderModelTest, CutFollowsWhatTheMoveLeft)
{
    fm::OperationsClient::instance();
    base::Process daemon;
    ASSERT_TRUE(daemon.start(FILEMANAGER_DAEMON));
    auto bus = base::dbus::Connection::open(base::dbus::Connection::Bus::Session, &loop());
    ASSERT_TRUE(bus);
    const base::dbus::Proxy manager(bus.get(), "org.lingmo.FileOperations", "/org/lingmo/FileOperations",
                                    "org.lingmo.FileOperations.Manager1");
    ASSERT_TRUE(runUntil(loop(), [&] { return manager.call("CanUndo").value<bool>().has_value(); }));

    TemporaryDir from;
    TemporaryDir to;
    TemporaryDir other;
    ASSERT_TRUE(writeFile(from.filePath("a"), "a"));
    ASSERT_TRUE(writeFile(from.filePath("b"), "new b"));
    ASSERT_TRUE(writeFile(to.filePath("b"), "old b"));
    FolderModel source;
    source.setUrl(from.path());
    FolderModel destination;
    destination.setUrl(to.path());
    FolderModel elsewhere;
    elsewhere.setUrl(other.path());
    ASSERT_TRUE(runUntil(loop(), [&] {
        return ready(source) && source.count() == 2 && ready(destination) && ready(elsewhere);
    }));

    const fm::OperationTaskPtr cancelled = cutAndPaste(source, {"a", "b"}, destination);
    ASSERT_TRUE(cancelled);
    ASSERT_TRUE(runUntil(loop(), [&] { return cancelled->waitingForConflict(); }));
    EXPECT_EQ(cutUrls().size(), 2u);
    cancelled->cancel();
    ASSERT_TRUE(runUntil(loop(), [&] { return cancelled->isFinished(); }));
    EXPECT_TRUE(std::filesystem::exists(to.filePath("a")));
    EXPECT_EQ(cutUrls(), std::vector<std::string> {url(from.filePath("b"))});

    fm::OperationTaskPtr moved;
    const base::Subscription added = fm::OperationsClient::instance().taskAdded.connect(
            [&](fm::OperationTaskPtr task) { moved = task; });
    elsewhere.paste();
    ASSERT_TRUE(runUntil(loop(), [&] { return moved && moved->isFinished(); }));
    EXPECT_EQ(moved->error(), 0);
    EXPECT_TRUE(std::filesystem::exists(other.filePath("b")));
    EXPECT_TRUE(ui::Clipboard::mimeTypes().empty());

    daemon.terminate();
    EXPECT_TRUE(daemon.waitForFinished(std::chrono::seconds(3)));
}

// Opening any part of a split archive extracts the whole of it, named without
// the part's suffix; an encrypted one asks until the password fits.
TEST(FolderModelTest, ExtractsSplitAndEncryptedArchives)
{
    fm::OperationsClient::instance();
    base::Process daemon;
    ASSERT_TRUE(daemon.start(FILEMANAGER_DAEMON));
    auto bus = base::dbus::Connection::open(base::dbus::Connection::Bus::Session, &loop());
    ASSERT_TRUE(bus);
    const base::dbus::Proxy manager(bus.get(), "org.lingmo.FileOperations", "/org/lingmo/FileOperations",
                                    "org.lingmo.FileOperations.Manager1");
    ASSERT_TRUE(runUntil(loop(), [&] { return manager.call("CanUndo").value<bool>().has_value(); }));

    TemporaryDir directory;
    const std::string payload = fmtest::noise(300 * 1024);
    ASSERT_TRUE(fmtest::writeArchive(directory.filePath("data.7z"), "7zip", {{"noise.bin", payload}}));
    ASSERT_EQ(fmtest::splitFile(directory.filePath("data.7z"), 128 * 1024), 3);
    ASSERT_TRUE(fmtest::writeArchive(directory.filePath("secret.zip"), "zip", {{"a.txt", "alpha"}},
                                     "zip:encryption=aes256", "right"));

    FolderModel folder;
    folder.setUrl(directory.path());
    ASSERT_TRUE(runUntil(loop(), [&] { return ready(folder) && folder.count() == 4; }));
    const auto open = [&](const std::string& name) {
        const std::vector<std::string> listed = names(folder);
        const auto found = std::find(listed.begin(), listed.end(), name);
        ASSERT_NE(found, listed.end());
        folder.clearSelection();
        folder.setSelected(int(found - listed.begin()));
        folder.openSelected();
        EXPECT_TRUE(folder.archiveBusy());
    };

    open("data.7z.002");
    ASSERT_TRUE(runUntil(loop(), [&] { return !folder.archiveBusy(); }, 10000));
    std::ifstream extracted(directory.filePath("data/noise.bin"), std::ios::binary);
    EXPECT_TRUE(std::string(std::istreambuf_iterator<char>(extracted), std::istreambuf_iterator<char>()) == payload);

    open("secret.zip");
    ASSERT_TRUE(runUntil(loop(), [&] { return folder.archivePasswordId() != 0; }));
    const uint32_t first = folder.archivePasswordId();
    EXPECT_FALSE(folder.archivePasswordRetry());
    EXPECT_EQ(folder.archiveSourceName(), "secret.zip");
    folder.answerArchivePassword(first, "wrong");
    ASSERT_TRUE(runUntil(loop(), [&] {
        return folder.archivePasswordId() != 0 && folder.archivePasswordId() != first;
    }));
    EXPECT_TRUE(folder.archivePasswordRetry());
    folder.answerArchivePassword(folder.archivePasswordId(), "right");
    ASSERT_TRUE(runUntil(loop(), [&] { return !folder.archiveBusy(); }, 10000));
    std::ifstream secret(directory.filePath("secret/a.txt"));
    EXPECT_EQ(std::string(std::istreambuf_iterator<char>(secret), std::istreambuf_iterator<char>()), "alpha");

    daemon.terminate();
    EXPECT_TRUE(daemon.waitForFinished(std::chrono::seconds(3)));
}
