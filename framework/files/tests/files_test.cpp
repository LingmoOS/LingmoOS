// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fcntl.h>
#include <linux/capability.h>
#include <linux/fs.h>
#include <map>
#include <sched.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

#include "base/event_loop.h"
#include "base/locale.h"
#include "base/paths.h"
#include "base/strings.h"
#include "lingmo/files/directory_lister.h"
#include "lingmo/files/directory_model.h"
#include "lingmo/files/file_enumerator.h"
#include "lingmo/files/file_path.h"
#include "lingmo/files/file_watcher.h"
#include "lingmo/files/local_file_system.h"
#include "lingmo/files/mime_sniffer_p.h"
#include "lingmo/files/mime_types.h"
#include "lingmo/files/path_utils_p.h"
#include "lingmo/files/task.h"
#include "lingmo/files/trash_directory_p.h"
#include "lingmo/files/trash_monitor.h"
#include "testing.h"

using namespace Lingmo;
using namespace Lingmo::Files;
using namespace Lingmo::Files::testing;

static Url local(const std::string& path)
{
    return Url::fromLocalFile(path);
}

static std::string userTrash(const std::string& topdir)
{
    return topdir + "/.Trash-" + base::strings::number(::getuid());
}

static std::string trashId(const std::string& topdir, bool shared = false)
{
    return (shared ? "s" : "u") + base::strings::toHex(topdir);
}

static std::string escapedMountPoint(const std::string& point)
{
    std::string result;
    for (const char c : point) {
        if (c == '\\')
            result += "\\134";
        else if (c == ' ')
            result += "\\040";
        else
            result += c;
    }
    return result;
}

// Mount lines as the kernel writes them, spaces in mount points escaped, after one
// for /proc.
static void writeMountTable(const std::string& table, const std::vector<std::pair<std::string, std::string>>& mounts)
{
    std::string content = "22 1 0:21 / /proc rw,nosuid shared:5 - proc proc rw\n";
    int id = 30;

    for (const auto& [point, type] : mounts) {
        content += base::strings::number(id++) + " 1 8:1 / " + escapedMountPoint(point) + " rw,relatime shared:1 - "
            + type + " /dev/sda1 rw\n";
    }

    writeFile(table, content);
}

static std::vector<std::string> sorted(std::vector<std::string> list)
{
    std::sort(list.begin(), list.end());
    return list;
}

static bool writeProc(const char* path, const std::string& data)
{
    const int fd = ::open(path, O_WRONLY | O_CLOEXEC);
    if (fd < 0)
        return false;
    const bool written = ::write(fd, data.data(), data.size()) == ssize_t(data.size());
    ::close(fd);
    return written;
}

// The trash of these tests lives in a folder of their own, and no volume of
// this machine is listed, so no test touches its trash.
class FilesEnvironment : public ::testing::Environment
{
public:
    void SetUp() override
    {
        ASSERT_TRUE(data.isValid());
        internal::setHomeTrashRoot(data.path() + "/Trash");
        ASSERT_EQ(FileSystemUrl::trashRoot(), data.path() + "/Trash");

        mountTable = data.filePath("mountinfo");
        writeFile(mountTable, {});
        internal::setMountTable(mountTable);
    }

    TemporaryDirectory data;
    std::string mountTable;
};

static FilesEnvironment* s_environment =
    static_cast<FilesEnvironment*>(::testing::AddGlobalTestEnvironment(new FilesEnvironment));

class FilesTest : public ::testing::Test
{
protected:
    void TearDown() override
    {
        OperationContext ctx;
        LocalFileSystem::emptyTrash(ctx);
    }

    const std::string& mountTable() const { return s_environment->mountTable; }

    base::EventLoop loop;
};

// A model row that follows insertions and removals.
class PersistentRow
{
public:
    PersistentRow(DirectoryModel& model, int row)
        : m_row(row)
    {
        m_inserted = model.rowsInserted.connect([this](int first, int count) {
            if (m_row >= first)
                m_row += count;
        });
        m_removed = model.rowsRemoved.connect([this](int first, int count) {
            if (m_row >= first + count)
                m_row -= count;
            else if (m_row >= first)
                m_row = -1;
        });
        m_reset = model.reset.connect([this] {
            m_row = -1;
        });
    }

    bool isValid() const { return m_row >= 0; }
    int row() const { return m_row; }

private:
    int m_row;
    base::Subscription m_inserted;
    base::Subscription m_removed;
    base::Subscription m_reset;
};

// Checks that every model signal leaves the row count it announces.
class ModelTester
{
public:
    explicit ModelTester(DirectoryModel& model)
        : m_model(model)
        , m_rows(model.rowCount())
    {
        m_subscriptions.push_back(model.rowsInserted.connect([this](int first, int count) {
            EXPECT_GE(first, 0);
            EXPECT_LE(first, m_rows);
            EXPECT_GT(count, 0);
            m_rows += count;
            EXPECT_EQ(m_rows, m_model.rowCount());
        }));
        m_subscriptions.push_back(model.rowsRemoved.connect([this](int first, int count) {
            EXPECT_GE(first, 0);
            EXPECT_LE(first + count, m_rows);
            EXPECT_GT(count, 0);
            m_rows -= count;
            EXPECT_EQ(m_rows, m_model.rowCount());
        }));
        m_subscriptions.push_back(model.dataChanged.connect([this](int first, int last) {
            EXPECT_GE(first, 0);
            EXPECT_LE(first, last);
            EXPECT_LT(last, m_model.rowCount());
            EXPECT_EQ(m_rows, m_model.rowCount());
        }));
        m_subscriptions.push_back(model.reset.connect([this] {
            m_rows = m_model.rowCount();
        }));
    }

private:
    DirectoryModel& m_model;
    int m_rows;
    std::vector<base::Subscription> m_subscriptions;
};

// Counts a signal's emissions and keeps their arguments.
template<typename... Args>
class Spy
{
public:
    explicit Spy(base::Signal<Args...>& signal)
    {
        m_subscription = signal.connect([this](Args... args) {
            calls.push_back(std::tuple<std::decay_t<Args>...>(args...));
        });
    }

    size_t count() const { return calls.size(); }

    std::vector<std::tuple<std::decay_t<Args>...>> calls;

private:
    base::Subscription m_subscription;
};

TEST_F(FilesTest, PathsAndUrls)
{
    EXPECT_EQ(FilePath("/one/two").parent().bytes(), "/one");
    EXPECT_EQ(FilePath("/one").child("a% b").bytes(), "/one/a% b");
    EXPECT_EQ(FilePath(std::string("/a/\xff", 4)).bytes(), std::string("/a/\xff", 4));

    EXPECT_EQ(errorCode([] { FilePath(std::string("x\0y", 3)); }), EINVAL);
    EXPECT_EQ(errorCode([] { FilePath("/a").child("../x"); }), EINVAL);
    EXPECT_EQ(errorCode([] { FileSystemUrl(Url("https://example.com/file")); }), ENOTSUP);
    EXPECT_EQ(errorCode([] { FileSystemUrl(Url("file://remote/tmp/a")); }), ENOTSUP);
    EXPECT_EQ(errorCode([] { FileSystemUrl(Url("trash:/../a")); }), EINVAL);
    EXPECT_EQ(errorCode([] { FileSystemUrl(Url("trash:/%2E%2E/a")); }), EINVAL);
    EXPECT_EQ(errorCode([] { FileSystemUrl(Url("trash:/a")); }), EINVAL);
    EXPECT_EQ(errorCode([] { FileSystemUrl(Url("trash:/u2F/a")); }), EINVAL);
    EXPECT_EQ(errorCode([] { FileSystemUrl(Url("trash:/u2e2e/a")); }), EINVAL);
    EXPECT_EQ(errorCode([] { FileSystemUrl(Url("trash:/u2f2e2e/a")); }), EINVAL);
    EXPECT_EQ(errorCode([] { FileSystemUrl(Url("trash:/x2f/a")); }), EINVAL);
    EXPECT_EQ(FileSystemUrl(Url("trash:/home/a/b")).localPath(), FileSystemUrl::trashRoot() + "/files/a/b");
    EXPECT_EQ(FileSystemUrl(Url("trash:/" + trashId("/media/a b") + "/c")).localPath(),
              userTrash("/media/a b") + "/files/c");
    EXPECT_EQ(FileSystemUrl(Url("trash:/" + trashId("/", true) + "/c")).localPath(),
              "/.Trash/" + base::strings::number(::getuid()) + "/files/c");

    EXPECT_EQ(FileSystemUrl(local("/tmp/a % # é")).localPath(), "/tmp/a % # é");
    EXPECT_EQ(upUrl(local("/tmp/a")), local("/tmp"));

    // URLs decode when read and percent-encode when written.
    EXPECT_EQ(local("/tmp/a % # é").toString(), "file:///tmp/a%20%25%20%23%20%C3%A9");
    EXPECT_EQ(Url("file:///tmp/a%20b").path(), "/tmp/a b");
    EXPECT_EQ(Url("file:///tmp/a%20b"), local("/tmp/a b"));
    EXPECT_EQ(Url("trash:/home/x").toString(), "trash:/home/x");
    EXPECT_EQ(Url("trash:/home/x").fileName(), "x");
    EXPECT_TRUE(Url("file://localhost/tmp").isLocalFile());
    EXPECT_EQ(Url("file://localhost/tmp").toLocalFile(), "/tmp");
    EXPECT_EQ(local("/tmp/a/").strippedTrailingSlash(), local("/tmp/a"));
    EXPECT_EQ(local("/tmp/a/b").folder(), local("/tmp/a"));
    EXPECT_TRUE(Url().isEmpty());
    EXPECT_FALSE(Url().isValid());
}

TEST_F(FilesTest, EnumerationAndMetadata)
{
    TemporaryDirectory dir;
    writeFile(dir.filePath("file.txt"), "hello");
    writeFile(dir.filePath(".hidden"));
    ASSERT_EQ(::mkdir(dir.filePath("sub").c_str(), 0755), 0);
    ASSERT_EQ(::symlink("missing", dir.filePath("broken").c_str()), 0);

    const auto items = LocalFileSystem::enumerate(local(dir.path()));
    EXPECT_EQ(items.size(), 4u);
    EXPECT_EQ(LocalFileSystem::enumerate(local(dir.path()), false).size(), 3u);

    FileItem item(local(dir.filePath("file.txt")));
    EXPECT_EQ(item.size(), 5u);
    EXPECT_TRUE(item.isFile());
    EXPECT_TRUE(item.time(FileItem::ModificationTime).has_value());
    EXPECT_EQ(item.mimetype(), "text/plain");
    EXPECT_TRUE(FileItem(local(dir.filePath("broken"))).isLink());

    EXPECT_EQ(errorCode([&] { LocalFileSystem::enumerate(local(dir.filePath("missing"))); }), ENOENT);
}

TEST_F(FilesTest, CopyRecursiveSymlinksAndPermissions)
{
    TemporaryDirectory dir;
    const auto source = dir.filePath("source");
    ASSERT_TRUE(makePath(source + "/a/b"));
    writeFile(source + "/a/b/file", std::string("hello\0world", 11));
    ASSERT_EQ(::chmod((source + "/a/b/file").c_str(), 0751), 0);
    ASSERT_EQ(::symlink("..", (source + "/a/loop").c_str()), 0);
    ASSERT_EQ(::symlink("absent", (source + "/broken").c_str()), 0);
    ASSERT_EQ(::symlink("/etc/passwd", (source + "/external").c_str()), 0);

    OperationContext ctx;
    const auto destination = dir.filePath("target");
    EXPECT_EQ(LocalFileSystem::copy(source, destination, ConflictPolicy::Fail, ctx), destination);
    EXPECT_EQ(readFile(destination + "/a/b/file"), readFile(source + "/a/b/file"));

    struct stat st;
    ASSERT_EQ(::lstat((destination + "/a/b/file").c_str(), &st), 0);
    EXPECT_EQ(st.st_mode & 0777, mode_t(0751));
    ASSERT_EQ(::lstat((destination + "/broken").c_str(), &st), 0);
    EXPECT_TRUE(S_ISLNK(st.st_mode));
    EXPECT_TRUE(isSymbolicLink(destination + "/a/loop"));

    LocalFileSystem::remove(destination, ctx);
    EXPECT_FALSE(exists(destination));
    EXPECT_TRUE(exists("/etc/passwd"));
    EXPECT_TRUE(exists(source + "/a/b/file"));
}

TEST_F(FilesTest, Conflicts)
{
    TemporaryDirectory dir;
    const auto source = dir.filePath("source.txt"), target = dir.filePath("target.txt");
    writeFile(source, "new");
    writeFile(target, "old");

    OperationContext ctx;
    EXPECT_EQ(errorCode([&] { LocalFileSystem::copy(source, target, ConflictPolicy::Fail, ctx); }), EEXIST);
    EXPECT_EQ(readFile(target), "old");

    EXPECT_TRUE(LocalFileSystem::copy(source, target, ConflictPolicy::Skip, ctx).empty());

    EXPECT_EQ(LocalFileSystem::copy(source, target, ConflictPolicy::KeepBoth, ctx), dir.filePath("target (1).txt"));
    EXPECT_EQ(readFile(dir.filePath("target (1).txt")), "new");

    const auto moved = dir.filePath("moved.txt");
    writeFile(moved, "moved");
    EXPECT_EQ(LocalFileSystem::move(moved, target, ConflictPolicy::KeepBoth, ctx), dir.filePath("target (2).txt"));
    EXPECT_EQ(readFile(dir.filePath("target (2).txt")), "moved");
    EXPECT_FALSE(exists(moved));

    EXPECT_EQ(LocalFileSystem::copy(source, target, ConflictPolicy::Replace, ctx), target);
    EXPECT_EQ(readFile(target), "new");

    for (const std::string& name : entries(dir.path()))
        EXPECT_FALSE(name.starts_with(".lingmo-")) << name;
}

TEST_F(FilesTest, DirectoryConflictPreservesDestination)
{
    TemporaryDirectory dir;
    ASSERT_TRUE(makePath(dir.filePath("source/sub")));
    ASSERT_TRUE(makePath(dir.filePath("target/sub")));
    writeFile(dir.filePath("target/sub/keep"));
    OperationContext ctx;
    writeFile(dir.filePath("source/sub/new"));

    LocalFileSystem::copy(dir.filePath("source"), dir.filePath("target"), ConflictPolicy::Replace, ctx);

    EXPECT_TRUE(exists(dir.filePath("target/sub/new")));
    EXPECT_TRUE(exists(dir.filePath("target/sub/keep")));
}

TEST_F(FilesTest, SelfAndDescendantProtection)
{
    TemporaryDirectory dir;
    writeFile(dir.filePath("file"));
    ASSERT_EQ(::mkdir(dir.filePath("sub").c_str(), 0755), 0);
    ASSERT_EQ(::symlink(dir.path().c_str(), dir.filePath("alias").c_str()), 0);

    OperationContext ctx;
    EXPECT_EQ(errorCode([&] {
                  LocalFileSystem::copy(dir.filePath("file"), dir.filePath("file"), ConflictPolicy::Replace, ctx);
              }),
              EINVAL);

    EXPECT_EQ(errorCode([&] {
                  LocalFileSystem::copy(dir.path(), dir.filePath("sub/copy"), ConflictPolicy::Fail, ctx);
              }),
              EINVAL);

    EXPECT_EQ(errorCode([&] {
                  LocalFileSystem::move(dir.path(), dir.filePath("alias/copy"), ConflictPolicy::Fail, ctx);
              }),
              EINVAL);

    EXPECT_EQ(errorCode([&] { LocalFileSystem::remove("/", ctx); }), EINVAL);
}

TEST_F(FilesTest, ReplacingAFolderThatHoldsTheSourceIsRefused)
{
    TemporaryDirectory dir;
    ASSERT_TRUE(makePath(dir.filePath("b/b/b")));
    writeFile(dir.filePath("b/b/b/inner"), "inner");
    writeFile(dir.filePath("b/b/file"), "file");
    writeFile(dir.filePath("b/keep"), "keep");

    OperationContext ctx;
    EXPECT_EQ(errorCode([&] {
                  LocalFileSystem::copy(dir.filePath("b/b"), dir.filePath("b"), ConflictPolicy::Replace, ctx);
              }),
              EINVAL);

    EXPECT_EQ(errorCode([&] {
                  LocalFileSystem::move(dir.filePath("b/b"), dir.filePath("b"), ConflictPolicy::Replace, ctx);
              }),
              EINVAL);

    EXPECT_EQ(readFile(dir.filePath("b/b/b/inner")), "inner");
    EXPECT_EQ(readFile(dir.filePath("b/b/file")), "file");
    EXPECT_EQ(readFile(dir.filePath("b/keep")), "keep");
    EXPECT_FALSE(exists(dir.filePath("b/file")));

    EXPECT_EQ(LocalFileSystem::move(dir.filePath("b/b"), dir.filePath("b"), ConflictPolicy::KeepBoth, ctx),
              dir.filePath("b (1)"));
    EXPECT_EQ(readFile(dir.filePath("b (1)/b/inner")), "inner");

    for (const std::string& name : entries(dir.path()))
        EXPECT_FALSE(name.starts_with(".lingmo-")) << name;
}

TEST_F(FilesTest, DestinationSymlinkIsNotFollowed)
{
    TemporaryDirectory dir;
    writeFile(dir.filePath("source"), "new");
    writeFile(dir.filePath("victim"), "unchanged");
    ASSERT_EQ(::symlink("victim", dir.filePath("target").c_str()), 0);

    OperationContext ctx;
    LocalFileSystem::copy(dir.filePath("source"), dir.filePath("target"), ConflictPolicy::Replace, ctx);

    EXPECT_EQ(readFile(dir.filePath("victim")), "unchanged");
    EXPECT_EQ(readFile(dir.filePath("target")), "new");
    EXPECT_FALSE(isSymbolicLink(dir.filePath("target")));
}

TEST_F(FilesTest, CancellationCleansStaging)
{
    TemporaryDirectory dir;
    writeFile(dir.filePath("source"), std::string(8 * 1024 * 1024, 'x'));
    writeFile(dir.filePath("target"), "original");

    OperationContext ctx;
    ctx.progress = [&](uint64_t bytes, uint64_t) {
        if (bytes >= 1024 * 1024)
            ctx.canceled = true;
    };

    EXPECT_EQ(errorCode([&] {
                  LocalFileSystem::copy(dir.filePath("source"), dir.filePath("target"), ConflictPolicy::Replace, ctx);
              }),
              ECANCELED);

    EXPECT_EQ(readFile(dir.filePath("target")), "original");
    EXPECT_EQ(fileSize(dir.filePath("source")), 8 * 1024 * 1024);
    for (const std::string& name : entries(dir.path()))
        EXPECT_FALSE(name.starts_with(".lingmo-")) << name;
}

TEST_F(FilesTest, CancelBeforeMutation)
{
    TemporaryDirectory dir;
    writeFile(dir.filePath("source"));

    OperationContext ctx;
    ctx.canceled = true;
    EXPECT_EQ(errorCode([&] {
                  LocalFileSystem::move(dir.filePath("source"), dir.filePath("target"), ConflictPolicy::Fail, ctx);
              }),
              ECANCELED);

    EXPECT_EQ(errorCode([&] { LocalFileSystem::remove(dir.filePath("source"), ctx); }), ECANCELED);

    EXPECT_TRUE(exists(dir.filePath("source")));
}

TEST_F(FilesTest, SizeDoesNotFollowSymlinks)
{
    TemporaryDirectory dir;
    ASSERT_EQ(::mkdir(dir.filePath("sub").c_str(), 0755), 0);
    writeFile(dir.filePath("sub/file"), std::string(50, 'x'));
    ASSERT_EQ(::symlink("..", dir.filePath("sub/loop").c_str()), 0);

    OperationContext ctx;
    EXPECT_EQ(LocalFileSystem::directorySize(dir.path(), ctx), 52u);
    EXPECT_EQ(ctx.bytes, 52u);

    ctx.canceled = true;
    EXPECT_EQ(errorCode([&] { LocalFileSystem::directorySize(dir.path(), ctx); }), ECANCELED);
}

TEST_F(FilesTest, SparseLargeFile)
{
    TemporaryDirectory dir;
    const std::string source = dir.filePath("source");
    constexpr int64_t length = (int64_t(5) << 30) + 17;
    {
        const int fd = ::open(source.c_str(), O_WRONLY | O_CREAT | O_CLOEXEC, 0644);
        ASSERT_GE(fd, 0);
        ASSERT_EQ(::ftruncate(fd, length), 0);
        ASSERT_EQ(::pwrite(fd, "tail", 4, length - 4), 4);
        ::close(fd);
    }

    OperationContext ctx;
    LocalFileSystem::copy(source, dir.filePath("target"), ConflictPolicy::Fail, ctx);
    EXPECT_EQ(fileSize(dir.filePath("target")), length);
    EXPECT_EQ(ctx.bytes, uint64_t(length));

    const int fd = ::open(dir.filePath("target").c_str(), O_RDONLY | O_CLOEXEC);
    ASSERT_GE(fd, 0);
    char buffer[4];
    ASSERT_EQ(::pread(fd, buffer, 4, length - 4), 4);
    EXPECT_EQ(std::string(buffer, 4), "tail");
    ASSERT_EQ(::pread(fd, buffer, 4, 0), 4);
    EXPECT_EQ(std::string(buffer, 4), std::string(4, '\0'));
    ::close(fd);
}

TEST_F(FilesTest, DataAroundHolesCopiesExactly)
{
    TemporaryDirectory dir;
    std::string head(9 * 1024 * 1024 + 3, '\0');
    std::string tail(5 * 1024 * 1024 + 11, '\0');
    for (size_t i = 0; i < head.size(); ++i)
        head[i] = char(i * 7 + i / 4099);
    for (size_t i = 0; i < tail.size(); ++i)
        tail[i] = char(i * 13 + i / 8191);

    constexpr int64_t gap = 64 * 1024 * 1024;
    const std::string source = dir.filePath("source");
    {
        const int fd = ::open(source.c_str(), O_WRONLY | O_CREAT | O_CLOEXEC, 0644);
        ASSERT_GE(fd, 0);
        ASSERT_EQ(::write(fd, head.data(), head.size()), ssize_t(head.size()));
        ASSERT_EQ(::pwrite(fd, tail.data(), tail.size(), off_t(head.size()) + gap), ssize_t(tail.size()));
        ::close(fd);
    }

    const int64_t length = int64_t(head.size()) + gap + int64_t(tail.size());
    uint64_t previous = 0;
    OperationContext ctx;
    ctx.progress = [&](uint64_t bytes, uint64_t) {
        EXPECT_GE(bytes, previous);
        previous = bytes;
    };

    LocalFileSystem::copy(source, dir.filePath("target"), ConflictPolicy::Fail, ctx);
    EXPECT_EQ(ctx.bytes, uint64_t(length));

    const std::string copied = readFile(dir.filePath("target"));
    ASSERT_EQ(int64_t(copied.size()), length);
    EXPECT_TRUE(copied.compare(0, head.size(), head) == 0);
    EXPECT_TRUE(copied.compare(head.size(), size_t(gap), std::string(size_t(gap), '\0')) == 0);
    EXPECT_TRUE(copied.compare(copied.size() - tail.size(), tail.size(), tail) == 0);

    struct stat st;
    ASSERT_EQ(::stat(dir.filePath("target").c_str(), &st), 0);
    EXPECT_LT(st.st_blocks * 512, length - gap / 2);
}

TEST_F(FilesTest, SpecialFilesRejected)
{
    TemporaryDirectory dir;
    ASSERT_EQ(::mkfifo(dir.filePath("fifo").c_str(), 0600), 0);

    OperationContext ctx;
    EXPECT_EQ(errorCode([&] {
                  LocalFileSystem::copy(dir.filePath("fifo"), dir.filePath("target"), ConflictPolicy::Fail, ctx);
              }),
              ENOTSUP);

    EXPECT_FALSE(exists(dir.filePath("target")));
}

TEST_F(FilesTest, RemoveStopsAtMounts)
{
    TemporaryDirectory dir;
    ASSERT_TRUE(makePath(dir.filePath("tree/mount")));
    writeFile(dir.filePath("tree/file"));

    // A private user and mount namespace lets the child mount without privileges.
    const uid_t uid = ::getuid();
    const gid_t gid = ::getgid();
    const pid_t pid = ::fork();
    ASSERT_GE(pid, 0);
    if (pid == 0) {
        if (::unshare(CLONE_NEWUSER | CLONE_NEWNS) != 0)
            ::_exit(77);

        // Creating files needs the group mapped as well, which needs setgroups denied first.
        if (!writeProc("/proc/self/uid_map", "0 " + std::to_string(uid) + " 1")
            || !writeProc("/proc/self/setgroups", "deny")
            || !writeProc("/proc/self/gid_map", "0 " + std::to_string(gid) + " 1"))
            ::_exit(77);

        if (::mount("none", dir.filePath("tree/mount").c_str(), "tmpfs", 0, nullptr) != 0)
            ::_exit(77);

        try {
            writeFile(dir.filePath("tree/mount/kept"));
            OperationContext ctx;
            const int code = errorCode([&] { LocalFileSystem::remove(dir.filePath("tree"), ctx); });
            ::_exit(code == EBUSY && exists(dir.filePath("tree/mount/kept")) ? 0 : 1);
        } catch (...) {
            ::_exit(2);
        }
    }

    int status = 0;
    ASSERT_EQ(::waitpid(pid, &status, 0), pid);
    ASSERT_TRUE(WIFEXITED(status));
    if (WEXITSTATUS(status) == 77)
        GTEST_SKIP() << "Unprivileged user namespaces are unavailable";
    EXPECT_EQ(WEXITSTATUS(status), 0);
}

TEST_F(FilesTest, RenameToANewCaseWhereNamesIgnoreCase)
{
    TemporaryDirectory dir;
    ASSERT_TRUE(makePath(dir.filePath("mount")));

    const uid_t uid = ::getuid();
    const gid_t gid = ::getgid();
    const pid_t pid = ::fork();
    ASSERT_GE(pid, 0);
    if (pid == 0) {
        if (::unshare(CLONE_NEWUSER | CLONE_NEWNS) != 0)
            ::_exit(77);

        if (!writeProc("/proc/self/uid_map", "0 " + std::to_string(uid) + " 1")
            || !writeProc("/proc/self/setgroups", "deny")
            || !writeProc("/proc/self/gid_map", "0 " + std::to_string(gid) + " 1"))
            ::_exit(77);

        if (::mount("none", dir.filePath("mount").c_str(), "tmpfs", 0, "casefold") != 0)
            ::_exit(77);

        const std::string folder = dir.filePath("mount/folder");
        if (::mkdir(folder.c_str(), 0700) != 0)
            ::_exit(2);

        const int fd = ::open(folder.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
        int flags = 0;
        if (fd < 0 || ::ioctl(fd, FS_IOC_GETFLAGS, &flags) != 0)
            ::_exit(77);
        flags |= FS_CASEFOLD_FL;
        if (::ioctl(fd, FS_IOC_SETFLAGS, &flags) != 0)
            ::_exit(77);
        ::close(fd);

        try {
            writeFile(folder + "/photo.JPG", "photo");
            if (::mkdir((folder + "/album").c_str(), 0700) != 0)
                ::_exit(2);

            OperationContext ctx;
            const bool file = LocalFileSystem::move(folder + "/photo.JPG", folder + "/photo.jpg", ConflictPolicy::Ask, ctx)
                == folder + "/photo.jpg";
            const bool directory =
                LocalFileSystem::move(folder + "/album", folder + "/Album", ConflictPolicy::Ask, ctx) == folder + "/Album";
            auto names = entries(folder);
            std::sort(names.begin(), names.end());
            ::_exit(file && directory && names == std::vector<std::string>{"Album", "photo.jpg"}
                            && readFile(folder + "/photo.jpg") == "photo"
                        ? 0
                        : 1);
        } catch (...) {
            ::_exit(3);
        }
    }

    int status = 0;
    ASSERT_EQ(::waitpid(pid, &status, 0), pid);
    ASSERT_TRUE(WIFEXITED(status));
    if (WEXITSTATUS(status) == 77)
        GTEST_SKIP() << "No filesystem that ignores case can be mounted here";
    EXPECT_EQ(WEXITSTATUS(status), 0);
}

TEST_F(FilesTest, MovingAHardLinkOntoItsTwinIsRefused)
{
    TemporaryDirectory dir;
    writeFile(dir.filePath("a"), "data");
    ASSERT_EQ(::link(dir.filePath("a").c_str(), dir.filePath("b").c_str()), 0);

    OperationContext ctx;
    EXPECT_EQ(errorCode([&] {
                  LocalFileSystem::move(dir.filePath("a"), dir.filePath("b"), ConflictPolicy::Replace, ctx);
              }),
              EINVAL);
    EXPECT_TRUE(exists(dir.filePath("a")));
    EXPECT_TRUE(exists(dir.filePath("b")));
}

TEST_F(FilesTest, PermissionErrors)
{
    if (::geteuid() == 0)
        GTEST_SKIP() << "Permission checks require an unprivileged user";

    TemporaryDirectory dir;
    writeFile(dir.filePath("source"));
    ASSERT_EQ(::mkdir(dir.filePath("blocked").c_str(), 0755), 0);
    ASSERT_EQ(::chmod(dir.filePath("blocked").c_str(), 0500), 0);

    OperationContext ctx;
    EXPECT_EQ(errorCode([&] {
                  LocalFileSystem::copy(dir.filePath("source"), dir.filePath("blocked/target"), ConflictPolicy::Fail,
                                        ctx);
              }),
              EACCES);

    ASSERT_EQ(::chmod(dir.filePath("source").c_str(), 0000), 0);
    EXPECT_EQ(errorCode([&] {
                  LocalFileSystem::copy(dir.filePath("source"), dir.filePath("target"), ConflictPolicy::Fail, ctx);
              }),
              EACCES);

    ASSERT_EQ(::chmod(dir.filePath("blocked").c_str(), 0700), 0);
    ASSERT_EQ(::chmod(dir.filePath("source").c_str(), 0600), 0);
}

TEST_F(FilesTest, TrashRestoreAndConflicts)
{
    TemporaryDirectory dir;
    const auto path = dir.filePath("é % #\n.txt");
    writeFile(path, "original");

    OperationContext ctx;
    const auto trashed = LocalFileSystem::trash(local(path), ctx);
    EXPECT_FALSE(exists(path));
    EXPECT_EQ(readFile(FileSystemUrl(trashed).localPath()), "original");

    writeFile(path, "replacement");
    EXPECT_EQ(errorCode([&] { LocalFileSystem::restore(trashed, ConflictPolicy::Fail, ctx); }), EEXIST);

    EXPECT_TRUE(exists(FileSystemUrl(trashed).localPath()));
    EXPECT_EQ(LocalFileSystem::restore(trashed, ConflictPolicy::Replace, ctx), local(path));
    EXPECT_EQ(readFile(path), "original");

    EXPECT_TRUE(LocalFileSystem::enumerate(Url("trash:/")).empty());
    EXPECT_FALSE(exists(FileSystemUrl::trashRoot() + "/info/" + trashed.fileName() + ".trashinfo"));
}

TEST_F(FilesTest, TrashRecursiveDuplicateAndEmpty)
{
    TemporaryDirectory dir;
    ASSERT_TRUE(makePath(dir.filePath("tree/sub")));
    writeFile(dir.filePath("tree/sub/file"));

    OperationContext ctx;
    const auto first = LocalFileSystem::trash(local(dir.filePath("tree")), ctx);
    ASSERT_TRUE(makePath(dir.filePath("tree/sub")));
    const auto second = LocalFileSystem::trash(local(dir.filePath("tree")), ctx);
    EXPECT_FALSE(first == second);
    EXPECT_EQ(LocalFileSystem::enumerate(Url("trash:/")).size(), 2u);

    LocalFileSystem::emptyTrash(ctx);
    EXPECT_TRUE(LocalFileSystem::enumerate(Url("trash:/")).empty());
    EXPECT_TRUE(entries(FileSystemUrl::trashRoot() + "/info").empty());
}

// Sizes other implementations cached for trashed folders go with the folders.
TEST_F(FilesTest, TrashForgetsCachedDirectorySizes)
{
    TemporaryDirectory dir;
    ASSERT_TRUE(makePath(dir.filePath("a folder")));
    ASSERT_TRUE(makePath(dir.filePath("kept")));

    OperationContext ctx;
    const auto folder = LocalFileSystem::trash(local(dir.filePath("a folder")), ctx);
    const auto kept = LocalFileSystem::trash(local(dir.filePath("kept")), ctx);

    const auto sizes = FileSystemUrl::trashRoot() + "/directorysizes";
    writeFile(sizes, "4096 1760000000000 a%20folder\n8192 1760000000000 kept\n");

    EXPECT_EQ(LocalFileSystem::restore(folder, ConflictPolicy::Fail, ctx), local(dir.filePath("a folder")));
    EXPECT_EQ(readFile(sizes), "8192 1760000000000 kept\n");

    const auto again = LocalFileSystem::trash(local(dir.filePath("a folder")), ctx);
    writeFile(sizes, "4096 1760000000000 a%20folder\n8192 1760000000000 kept\n");
    LocalFileSystem::remove(again, ctx);
    EXPECT_FALSE(exists(FileSystemUrl(again).localPath()));
    EXPECT_EQ(readFile(sizes), "8192 1760000000000 kept\n");

    LocalFileSystem::emptyTrash(ctx);
    EXPECT_FALSE(exists(FileSystemUrl(kept).localPath()));
    EXPECT_FALSE(exists(sizes));
}

TEST_F(FilesTest, LongTrashNameAndOrphanMetadata)
{
    TemporaryDirectory dir;
    const auto path = dir.filePath(std::string(250, 'a'));
    writeFile(path);

    OperationContext ctx;
    const auto trashed = LocalFileSystem::trash(local(path), ctx);
    EXPECT_EQ(LocalFileSystem::restore(trashed, ConflictPolicy::Fail, ctx), local(path));

    writeFile(FileSystemUrl::trashRoot() + "/info/orphan.trashinfo", "[Trash Info]\nPath=/missing\n");

    LocalFileSystem::emptyTrash(ctx);
    EXPECT_TRUE(entries(FileSystemUrl::trashRoot() + "/info").empty());
}

TEST_F(FilesTest, RestoreRecreatesFolder)
{
    TemporaryDirectory dir;
    const auto path = dir.filePath("gone/inner/file");
    ASSERT_TRUE(makePath(dir.filePath("gone/inner")));
    writeFile(path);

    OperationContext ctx;
    const auto trashed = LocalFileSystem::trash(local(path), ctx);
    TemporaryDirectory::removeAll(dir.filePath("gone"));
    EXPECT_EQ(LocalFileSystem::restore(trashed, ConflictPolicy::Fail, ctx), local(path));
    EXPECT_TRUE(exists(path));
}

TEST_F(FilesTest, MalformedTrashInfo)
{
    TemporaryDirectory dir;
    writeFile(dir.filePath("file"));

    OperationContext ctx;
    const auto url = LocalFileSystem::trash(local(dir.filePath("file")), ctx);
    writeFile(FileSystemUrl::trashRoot() + "/info/" + url.fileName() + ".trashinfo", "[Trash Info]\nPath=relative\n");

    EXPECT_EQ(errorCode([&] { LocalFileSystem::restore(url, ConflictPolicy::Fail, ctx); }), EINVAL);
    EXPECT_TRUE(exists(FileSystemUrl(url).localPath()));
}

// An info file that is a pipe, or a link to one, leaves the item without details
// instead of blocking the listing.
TEST_F(FilesTest, TrashInfoThatIsNoFile)
{
    TemporaryDirectory dir;
    writeFile(dir.filePath("piped"));
    writeFile(dir.filePath("linked"));
    writeFile(dir.filePath("plain"));

    OperationContext ctx;
    const auto piped = LocalFileSystem::trash(local(dir.filePath("piped")), ctx);
    const auto linked = LocalFileSystem::trash(local(dir.filePath("linked")), ctx);
    const auto plain = LocalFileSystem::trash(local(dir.filePath("plain")), ctx);

    const auto info = FileSystemUrl::trashRoot() + "/info/";
    ASSERT_EQ(::unlink((info + piped.fileName() + ".trashinfo").c_str()), 0);
    ASSERT_EQ(::mkfifo((info + piped.fileName() + ".trashinfo").c_str(), 0600), 0);
    ASSERT_EQ(::unlink((info + linked.fileName() + ".trashinfo").c_str()), 0);
    ASSERT_EQ(::symlink((info + piped.fileName() + ".trashinfo").c_str(),
                        (info + linked.fileName() + ".trashinfo").c_str()),
              0);

    const pid_t pid = ::fork();
    ASSERT_GE(pid, 0);
    if (pid == 0) {
        ::alarm(10);

        try {
            std::map<std::string, FileItem> items;
            for (const auto& item : LocalFileSystem::enumerate(Url("trash:/")))
                items.emplace(item.name(), item);

            OperationContext child;
            const bool passed = items.size() == 3 && items[piped.fileName()].originalPath().empty()
                && items[linked.fileName()].originalPath().empty()
                && items[plain.fileName()].originalPath() == dir.filePath("plain")
                && FileItem(piped).originalPath().empty()
                && errorCode([&] { LocalFileSystem::restore(linked, ConflictPolicy::Fail, child); }) == EACCES;
            ::_exit(passed ? 0 : 1);
        } catch (...) {
            ::_exit(100);
        }
    }

    int status = 0;
    ASSERT_EQ(::waitpid(pid, &status, 0), pid);
    EXPECT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WIFEXITED(status) ? WEXITSTATUS(status) : -WTERMSIG(status), 0);

    LocalFileSystem::emptyTrash(ctx);
    EXPECT_TRUE(entries(info).empty());
}

TEST_F(FilesTest, VolumeTrashListsRestoresAndEmpties)
{
    TemporaryDirectory dir;
    const auto volume = dir.filePath("disk one");
    const auto automount = dir.filePath("auto");
    ASSERT_TRUE(makePath(userTrash(volume) + "/files"));
    ASSERT_TRUE(makePath(userTrash(volume) + "/info"));
    ASSERT_TRUE(makePath(userTrash(automount) + "/files"));
    writeFile(userTrash(automount) + "/files/unseen");
    const auto share = dir.filePath("share");
    ASSERT_TRUE(makePath(userTrash(share) + "/files"));
    writeFile(userTrash(share) + "/files/remote");
    const auto bound = dir.filePath("bound");
    ASSERT_EQ(::symlink(volume.c_str(), bound.c_str()), 0);

    // Looking into an automount would mount it, and into a share could hang; a volume
    // reached twice has one trash.
    writeMountTable(mountTable(), {{volume, "ext4"}, {bound, "ext4"}, {automount, "autofs"}, {share, "nfs"}});
    struct EmptyTable
    {
        std::string table;
        ~EmptyTable() { writeFile(table, {}); }
    } emptyTable{mountTable()};

    writeFile(userTrash(volume) + "/files/old.txt", "old");
    writeFile(userTrash(volume) + "/info/old.txt.trashinfo",
              "[Trash Info]\nPath=docs/old%20one.txt\nDeletionDate=2026-01-02T03:04:05\n");
    // Left by a tool that wrote no info file.
    writeFile(userTrash(volume) + "/files/orphan");

    writeFile(dir.filePath("local"));
    OperationContext ctx;
    const auto trashed = LocalFileSystem::trash(local(dir.filePath("local")), ctx);
    EXPECT_EQ(trashed.path(), "/home/local");

    std::map<std::string, FileItem> items;
    for (const auto& item : LocalFileSystem::enumerate(Url("trash:/")))
        items.emplace(item.name(), item);
    EXPECT_EQ(items.size(), 3u);

    struct tm expected = {};
    expected.tm_year = 2026 - 1900;
    expected.tm_mon = 0;
    expected.tm_mday = 2;
    expected.tm_hour = 3;
    expected.tm_min = 4;
    expected.tm_sec = 5;
    expected.tm_isdst = -1;
    const int64_t deletion = int64_t(::mktime(&expected)) * 1000;

    const auto old = items["old.txt"];
    EXPECT_EQ(old.url().path(), '/' + trashId(volume) + "/old.txt");
    EXPECT_EQ(old.originalPath(), volume + "/docs/old one.txt");
    EXPECT_EQ(old.deletionTime(), deletion);
    EXPECT_EQ(readFile(old.localPath()), "old");
    EXPECT_EQ(FileItem(old.url()).originalPath(), old.originalPath());
    EXPECT_EQ(FileItem(old.url()).deletionTime(), old.deletionTime());

    const auto orphan = items["orphan"];
    EXPECT_TRUE(orphan.originalPath().empty());
    EXPECT_FALSE(orphan.deletionTime().has_value());

    // One trash lists alone, with the same details.
    const auto alone = LocalFileSystem::enumerate(upUrl(orphan.url()));
    EXPECT_EQ(alone.size(), 2u);
    for (const auto& item : alone)
        EXPECT_EQ(item.originalPath(), items[item.name()].originalPath());

    const auto home = items["local"];
    EXPECT_EQ(home.url(), trashed);
    EXPECT_EQ(home.originalPath(), dir.filePath("local"));
    ASSERT_TRUE(home.deletionTime().has_value());
    EXPECT_LT(std::abs(*home.deletionTime() / 1000 - int64_t(::time(nullptr))), 60);

    EXPECT_EQ(errorCode([&] { LocalFileSystem::restore(orphan.url(), ConflictPolicy::Fail, ctx); }), ENOENT);

    ASSERT_TRUE(makePath(volume + "/docs"));
    EXPECT_EQ(LocalFileSystem::restore(old.url(), ConflictPolicy::Fail, ctx), local(volume + "/docs/old one.txt"));
    EXPECT_EQ(readFile(volume + "/docs/old one.txt"), "old");
    EXPECT_FALSE(exists(userTrash(volume) + "/info/old.txt.trashinfo"));

    LocalFileSystem::emptyTrash(ctx);
    EXPECT_TRUE(LocalFileSystem::enumerate(Url("trash:/")).empty());
    EXPECT_TRUE(entries(userTrash(volume) + "/files").empty());
    EXPECT_TRUE(entries(FileSystemUrl::trashRoot() + "/info").empty());
    EXPECT_TRUE(exists(userTrash(automount) + "/files/unseen"));
    EXPECT_TRUE(exists(userTrash(share) + "/files/remote"));
}

// A volume's trash names places on that volume only: an item whose info file points
// elsewhere stays listed, with no original location, and is not put back.
TEST_F(FilesTest, VolumeTrashKeepsItemsOnVolume)
{
    TemporaryDirectory dir;
    const auto volume = dir.filePath("stick");
    const auto outside = dir.filePath("outside");
    ASSERT_TRUE(makePath(userTrash(volume) + "/files"));
    ASSERT_TRUE(makePath(userTrash(volume) + "/info"));
    ASSERT_TRUE(makePath(outside));
    ASSERT_TRUE(makePath(dir.filePath("stick2")));
    ASSERT_EQ(::symlink(outside.c_str(), (volume + "/link").c_str()), 0);

    writeMountTable(mountTable(), {{volume, "ext4"}});
    struct EmptyTable
    {
        std::string table;
        ~EmptyTable() { writeFile(table, {}); }
    } emptyTable{mountTable()};

    const std::vector<std::pair<std::string, std::string>> infos = {
        {"away", outside + "/away"},
        {"sibling", dir.filePath("stick2/sibling")},
        {"climb", volume + "/../outside/climb"},
        {"relativeClimb", "docs/../../outside/relativeClimb"},
        {"linked", "link/linked"},
        {"inside", volume + "/docs/inside"},
        {"relative", "notes/relative"},
    };
    for (const auto& [name, path] : infos) {
        writeFile(userTrash(volume) + "/files/" + name, name);
        writeFile(userTrash(volume) + "/info/" + name + ".trashinfo", "[Trash Info]\nPath=" + path + '\n');
    }

    std::map<std::string, FileItem> items;
    for (const auto& item : LocalFileSystem::enumerate(Url("trash:/")))
        items.emplace(item.name(), item);
    ASSERT_EQ(items.size(), infos.size());
    EXPECT_TRUE(items["away"].originalPath().empty());
    EXPECT_TRUE(items["sibling"].originalPath().empty());
    EXPECT_TRUE(items["climb"].originalPath().empty());
    EXPECT_TRUE(items["relativeClimb"].originalPath().empty());
    EXPECT_EQ(items["linked"].originalPath(), volume + "/link/linked");
    EXPECT_EQ(items["inside"].originalPath(), volume + "/docs/inside");
    EXPECT_EQ(items["relative"].originalPath(), volume + "/notes/relative");

    const auto failure = [&](const std::string& name) {
        try {
            OperationContext ctx;
            LocalFileSystem::restore(items[name].url(), ConflictPolicy::Replace, ctx);
        } catch (const Failure& failure) {
            return failure.error;
        }
        return Error();
    };

    for (const std::string name : {"away", "sibling", "climb", "relativeClimb"}) {
        const Error error = failure(name);
        EXPECT_EQ(error.code, EINVAL) << name;
        EXPECT_EQ(error.path, name);
        EXPECT_EQ(error.message, "The original location is not known");
        EXPECT_EQ(readFile(userTrash(volume) + "/files/" + name), name);
        EXPECT_TRUE(exists(userTrash(volume) + "/info/" + name + ".trashinfo"));
    }

    const Error linked = failure("linked");
    EXPECT_EQ(linked.code, EXDEV);
    EXPECT_EQ(linked.message, "The original location is outside the volume it was deleted on");
    EXPECT_TRUE(exists(userTrash(volume) + "/files/linked"));

    EXPECT_TRUE(entries(outside).empty());
    EXPECT_FALSE(exists(dir.filePath("stick2/sibling")));

    OperationContext ctx;
    EXPECT_EQ(LocalFileSystem::restore(items["inside"].url(), ConflictPolicy::Fail, ctx), local(volume + "/docs/inside"));
    EXPECT_EQ(readFile(volume + "/docs/inside"), "inside");
    EXPECT_EQ(LocalFileSystem::restore(items["relative"].url(), ConflictPolicy::Fail, ctx),
              local(volume + "/notes/relative"));
    EXPECT_EQ(readFile(volume + "/notes/relative"), "relative");

    // The home trash is the user's own: any absolute path goes back.
    writeFile(dir.filePath("home"), "home");
    const auto trashed = LocalFileSystem::trash(local(dir.filePath("home")), ctx);
    writeFile(FileSystemUrl::trashRoot() + "/info/" + trashed.fileName() + ".trashinfo",
              "[Trash Info]\nPath=" + outside + "/home\n");
    EXPECT_EQ(LocalFileSystem::restore(trashed, ConflictPolicy::Fail, ctx), local(outside + "/home"));
    EXPECT_EQ(readFile(outside + "/home"), "home");

    LocalFileSystem::emptyTrash(ctx);
}

TEST_F(FilesTest, TrashMonitorSeesVolumes)
{
    TemporaryDirectory dir;
    const auto volume = dir.filePath("volume");
    ASSERT_TRUE(makePath(volume));
    writeMountTable(mountTable(), {{volume, "ext4"}});
    struct EmptyTable
    {
        std::string table;
        ~EmptyTable() { writeFile(table, {}); }
    } emptyTable{mountTable()};

    writeFile(dir.filePath("local"));
    OperationContext ctx;
    LocalFileSystem::trash(local(dir.filePath("local")), ctx);

    TrashMonitor monitor;
    EXPECT_TRUE(waitFor(loop, [&] { return !monitor.isEmpty(); }));

    LocalFileSystem::emptyTrash(ctx);
    EXPECT_TRUE(waitFor(loop, [&] { return monitor.isEmpty(); }));

    // The first item trashed on a volume creates its trash.
    ASSERT_TRUE(makePath(userTrash(volume) + "/files"));
    writeFile(userTrash(volume) + "/files/stick");
    EXPECT_TRUE(waitFor(loop, [&] { return !monitor.isEmpty(); }));

    LocalFileSystem::emptyTrash(ctx);
    EXPECT_TRUE(waitFor(loop, [&] { return monitor.isEmpty(); }));

    writeFile(userTrash(volume) + "/files/again");
    EXPECT_TRUE(waitFor(loop, [&] { return !monitor.isEmpty(); }));
}

// Every file made in a memory-backed folder or a system place would wake the
// watcher, and none of them holds a trash.
TEST_F(FilesTest, TrashWatchesOnlyVolumesThatCanHoldOne)
{
    if (::getuid() == 0)
        GTEST_SKIP() << "Permissions do not apply to root";

    TemporaryDirectory dir;
    const auto memory = dir.filePath("tmp");
    const auto locked = dir.filePath("credentials");
    const auto stick = dir.filePath("media/u/stick");
    const auto data = dir.filePath("mnt/data");
    const auto runtime = "/run/user/" + std::to_string(::getuid());
    for (const auto& path : {memory, locked, stick, data})
        ASSERT_TRUE(makePath(path));
    ASSERT_EQ(::chmod(locked.c_str(), 0), 0);
    struct Restore
    {
        std::string path;
        ~Restore() { ::chmod(path.c_str(), 0700); }
    } restore{locked};

    writeMountTable(mountTable(), {{"/", "ext4"},
                                   {memory, "tmpfs"},
                                   {runtime, "fuse.app"},
                                   {locked, "ext4"},
                                   {stick, "vfat"},
                                   {data, "ext4"}});
    struct EmptyTable
    {
        std::string table;
        ~EmptyTable() { writeFile(table, {}); }
    } emptyTable{mountTable()};

    std::vector<std::string> points;
    internal::trashDirectories(&points);
    EXPECT_EQ(sorted(points), sorted({"/", stick, data}));

    std::vector<std::string> paths;
    internal::WatchEntries watched;
    internal::trashWatches(&paths, &watched);
    EXPECT_FALSE(watched.contains("/"));
    EXPECT_FALSE(watched.contains("/proc"));
    EXPECT_FALSE(watched.contains(memory));
    EXPECT_FALSE(watched.contains(runtime));
    EXPECT_FALSE(watched.contains(locked));
    EXPECT_TRUE(watched.contains(stick));
    EXPECT_TRUE(watched.contains(data));

    EXPECT_FALSE(internal::volumeTakesTrash(memory));
    EXPECT_FALSE(internal::volumeTakesTrash(runtime));
    EXPECT_TRUE(internal::volumeTakesTrash(stick));
}

// A trash folder that cannot be watched is looked for again less and less often;
// each look reads the mount table.
TEST_F(FilesTest, TrashMonitorBacksOffFailingWatch)
{
    TemporaryDirectory dir;
    writeFile(dir.filePath("local"));
    OperationContext ctx;
    LocalFileSystem::trash(local(dir.filePath("local")), ctx);

    const auto files = FileSystemUrl::trashRoot() + "/files";
    ASSERT_EQ(::chmod(files.c_str(), 0), 0);
    struct Restore
    {
        std::string path;
        ~Restore() { ::chmod(path.c_str(), 0700); }
    } restore{files};

    const int inotify = ::inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    ASSERT_GE(inotify, 0);
    ASSERT_GE(::inotify_add_watch(inotify, mountTable().c_str(), IN_OPEN), 0);

    // Repeated reads of the table merge into one event until the queue is read.
    int looks = 0;
    {
        TrashMonitor monitor;
        for (int i = 0; i < 95; ++i) {
            wait(loop, 100ms);

            inotify_event events[64];
            if (::read(inotify, events, sizeof(events)) > 0)
                ++looks;
        }
    }
    ::close(inotify);

    // At the start, after two seconds and after four more.
    EXPECT_GE(looks, 2);
    EXPECT_LE(looks, 3);
}

// A tmpfs in a private user and mount namespace stands in for a second drive.
TEST_F(FilesTest, TrashOnAnotherFilesystem)
{
    TemporaryDirectory dir;
    const auto volume = dir.filePath("volume");
    ASSERT_TRUE(makePath(volume));
    const auto table = dir.filePath("mountinfo");

    const uid_t uid = ::getuid();
    const gid_t gid = ::getgid();
    const pid_t pid = ::fork();
    ASSERT_GE(pid, 0);
    if (pid == 0) {
        if (::unshare(CLONE_NEWUSER | CLONE_NEWNS) != 0)
            ::_exit(77);

        if (!writeProc("/proc/self/uid_map", std::to_string(uid) + ' ' + std::to_string(uid) + " 1")
            || !writeProc("/proc/self/setgroups", "deny")
            || !writeProc("/proc/self/gid_map", std::to_string(gid) + ' ' + std::to_string(gid) + " 1"))
            ::_exit(77);

        if (::mount("none", volume.c_str(), "tmpfs", 0, "mode=0755") != 0)
            ::_exit(77);

        // Without capabilities, permissions hold as they do outside the namespace.
        __user_cap_header_struct header{_LINUX_CAPABILITY_VERSION_3, 0};
        __user_cap_data_struct capabilities[2] = {};
        if (::syscall(SYS_capset, &header, capabilities) != 0)
            ::_exit(76);

        internal::setMountTable(table);

        // Exits with the number of the first check that fails.
        int step = 0;
        const auto expect = [&](bool passed) {
            ++step;
            if (!passed)
                ::_exit(step);
        };

        try {
            OperationContext ctx;
            const auto trash = userTrash(volume);
            const auto shared = volume + "/.Trash/" + std::to_string(uid);
            const auto mode = [](const std::string& path) {
                struct stat st;
                return ::stat(path.c_str(), &st) == 0 ? st.st_mode & 07777 : 0;
            };

            // A remote filesystem takes no trash, so its files go home.
            writeMountTable(table, {{volume, "nfs"}});
            writeFile(volume + "/remote");
            const auto remote = LocalFileSystem::trash(local(volume + "/remote"), ctx);
            expect(remote.path() == "/home/remote");
            expect(!exists(volume + "/remote") && !exists(trash));
            expect(LocalFileSystem::restore(remote, ConflictPolicy::Fail, ctx) == local(volume + "/remote"));
            writeMountTable(table, {{volume, "ext4"}});

            // A volume that takes no trash sends its files home.
            expect(makePath(volume + "/sub"));
            expect(::chmod(volume.c_str(), 0555) == 0);
            writeFile(volume + "/sub/sent");
            const auto sent = LocalFileSystem::trash(local(volume + "/sub/sent"), ctx);
            expect(sent.path() == "/home/sent");
            expect(!exists(volume + "/sub/sent") && !exists(trash));
            expect(::chmod(volume.c_str(), 0755) == 0);

            writeFile(volume + "/sub/file", "data");
            const auto url = LocalFileSystem::trash(local(volume + "/sub/file"), ctx);
            expect(url.path() == '/' + trashId(volume) + "/file");
            expect(readFile(trash + "/files/file") == "data");
            expect(readFile(trash + "/info/file.trashinfo").starts_with("[Trash Info]\nPath=sub/file\nDeletionDate="));
            expect(mode(trash) == 0700);

            const auto items = LocalFileSystem::enumerate(Url("trash:/"));
            expect(items.size() == 2);
            const auto item = std::find_if(items.cbegin(), items.cend(), [&](const FileItem& entry) {
                return entry.url() == url;
            });
            expect(item != items.cend() && item->originalPath() == volume + "/sub/file"
                   && item->deletionTime().has_value());

            expect(LocalFileSystem::restore(url, ConflictPolicy::Fail, ctx) == local(volume + "/sub/file"));
            expect(readFile(volume + "/sub/file") == "data");
            expect(!exists(trash + "/info/file.trashinfo"));

            // $topdir/.Trash serves only as a sticky folder that is no link and takes
            // the user's folder.
            expect(::mkdir((volume + "/.Trash").c_str(), 0755) == 0);
            const auto plain = LocalFileSystem::trash(local(volume + "/sub/file"), ctx);
            expect(plain.path() == '/' + trashId(volume) + "/file");

            expect(::chmod((volume + "/.Trash").c_str(), 01555) == 0);
            writeFile(volume + "/sub/second");
            const auto unwritable = LocalFileSystem::trash(local(volume + "/sub/second"), ctx);
            expect(unwritable.path() == '/' + trashId(volume) + "/second");

            expect(::chmod((volume + "/.Trash").c_str(), 01777) == 0);
            writeFile(volume + "/sub/third");
            const auto sticky = LocalFileSystem::trash(local(volume + "/sub/third"), ctx);
            expect(sticky.path() == '/' + trashId(volume, true) + "/third");
            expect(exists(shared + "/files/third") && mode(shared) == 0700);

            expect(::rename((volume + "/.Trash").c_str(), (volume + "/.Trash-real").c_str()) == 0);
            expect(::symlink(".Trash-real", (volume + "/.Trash").c_str()) == 0);
            writeFile(volume + "/sub/fourth");
            const auto linked = LocalFileSystem::trash(local(volume + "/sub/fourth"), ctx);
            expect(linked.path() == '/' + trashId(volume) + "/fourth");
            expect(::unlink((volume + "/.Trash").c_str()) == 0);
            expect(::rename((volume + "/.Trash-real").c_str(), (volume + "/.Trash").c_str()) == 0);

            // Trashing a trash into itself is refused.
            expect(errorCode([&] { LocalFileSystem::trash(local(trash + "/files/fourth"), ctx); }) == EINVAL);

            expect(LocalFileSystem::enumerate(Url("trash:/")).size() == 5);
            LocalFileSystem::emptyTrash(ctx);
            expect(LocalFileSystem::enumerate(Url("trash:/")).empty());
            expect(entries(trash + "/info").empty() && entries(shared + "/info").empty());
            expect(entries(FileSystemUrl::trashRoot() + "/files").empty());
        } catch (...) {
            ::_exit(100);
        }

        ::_exit(0);
    }

    int status = 0;
    ASSERT_EQ(::waitpid(pid, &status, 0), pid);
    ASSERT_TRUE(WIFEXITED(status));
    if (WEXITSTATUS(status) == 77)
        GTEST_SKIP() << "Unprivileged user namespaces are unavailable";
    EXPECT_EQ(WEXITSTATUS(status), 0);
}

// A volume's trash made after the listing opened appears in it.
TEST_F(FilesTest, TrashListingFollowsVolumes)
{
    TemporaryDirectory dir;
    const auto volume = dir.filePath("volume");
    ASSERT_TRUE(makePath(volume));
    writeMountTable(mountTable(), {{volume, "ext4"}});
    struct EmptyTable
    {
        std::string table;
        ~EmptyTable() { writeFile(table, {}); }
    } emptyTable{mountTable()};

    DirectoryLister lister;
    DirectoryModel model;
    model.setDirLister(&lister);
    Spy listed(lister.completed);
    lister.openUrl(Url("trash:/"));
    EXPECT_TRUE(waitFor(loop, [&] { return listed.count() == 1; }));
    EXPECT_EQ(model.rowCount(), 0);

    ASSERT_TRUE(makePath(userTrash(volume) + "/files"));
    writeFile(userTrash(volume) + "/files/first");
    EXPECT_TRUE(waitFor(loop, [&] { return model.rowCount() == 1; }));
    EXPECT_EQ(model.itemForRow(0).originalPath(), "");

    writeFile(userTrash(volume) + "/files/second");
    EXPECT_TRUE(waitFor(loop, [&] { return model.rowCount() == 2; }));

    writeFile(dir.filePath("local"));
    OperationContext ctx;
    LocalFileSystem::trash(local(dir.filePath("local")), ctx);
    EXPECT_TRUE(waitFor(loop, [&] { return model.rowCount() == 3; }));

    const int row = model.rowForUrl(Url("trash:/home/local"));
    ASSERT_GE(row, 0);
    EXPECT_EQ(model.itemForRow(row).originalPath(), dir.filePath("local"));
    EXPECT_TRUE(model.itemForRow(row).deletionTime().has_value());
}

TEST_F(FilesTest, WatcherAndStableModel)
{
    TemporaryDirectory dir;
    writeFile(dir.filePath("first"));
    DirectoryLister lister;
    DirectoryModel model;
    model.setDirLister(&lister);
    ModelTester tester(model);

    Spy listed(lister.completed);
    lister.openUrl(local(dir.path()));
    EXPECT_TRUE(waitFor(loop, [&] { return model.rowCount() == 1; }));

    PersistentRow first(model, model.rowForUrl(local(dir.filePath("first"))));
    writeFile(dir.filePath("second"));
    EXPECT_TRUE(waitFor(loop, [&] { return model.rowCount() == 2; }));
    ASSERT_TRUE(first.isValid());
    EXPECT_EQ(model.itemForRow(first.row()).name(), "first");

    writeFile(dir.filePath("first"), "modified contents");
    EXPECT_TRUE(waitFor(loop, [&] { return model.itemForRow(first.row()).size() == 17; }));

    ASSERT_EQ(::rename(dir.filePath("second").c_str(), dir.filePath("renamed").c_str()), 0);
    EXPECT_TRUE(waitFor(loop, [&] { return model.rowForUrl(local(dir.filePath("renamed"))) >= 0; }));

    ASSERT_EQ(::unlink(dir.filePath("renamed").c_str()), 0);
    EXPECT_TRUE(waitFor(loop, [&] { return model.rowCount() == 1; }));

    writeFile(dir.filePath(".hidden"));
    lister.setShowHiddenFiles(true);
    lister.emitChanges();
    EXPECT_TRUE(waitFor(loop, [&] { return model.rowCount() == 2; }));

    EXPECT_GE(listed.count(), 4u);
}

TEST_F(FilesTest, SearchFindsNamesAtAnyDepth)
{
    TemporaryDirectory dir;
    ASSERT_TRUE(makePath(dir.filePath("Projects/Report Drafts/old")));
    ASSERT_TRUE(makePath(dir.filePath(".cache/report")));
    writeFile(dir.filePath("annual report.txt"));
    writeFile(dir.filePath("Projects/REPORT final.pdf"));
    writeFile(dir.filePath("Projects/Report Drafts/old/report-1.txt"));
    writeFile(dir.filePath("Projects/notes.txt"));
    writeFile(dir.filePath(".cache/report/report.bin"));
    ASSERT_EQ(::symlink(dir.filePath("Projects").c_str(), dir.filePath("Projects/loop").c_str()), 0);

    DirectoryLister lister;
    DirectoryModel model;
    model.setDirLister(&lister);
    ModelTester tester(model);
    Spy listed(lister.completed);

    lister.openUrl(local(dir.filePath("Projects")));
    EXPECT_TRUE(waitFor(loop, [&] { return listed.count() == 1; }));

    const auto names = [&] {
        std::vector<std::string> result;
        for (const auto& item : lister.items())
            result.push_back(item.localPath().substr(dir.path().size() + 1));
        return sorted(result);
    };

    // Every word, in any case; hidden folders are left out and links not followed.
    lister.search(local(dir.path()), "  report ");
    EXPECT_TRUE(lister.isSearching());
    EXPECT_TRUE(waitFor(loop, [&] { return listed.count() == 2; }));
    EXPECT_EQ(names(), std::vector<std::string>({"Projects/REPORT final.pdf", "Projects/Report Drafts",
                                                 "Projects/Report Drafts/old/report-1.txt", "annual report.txt"}));
    EXPECT_EQ(lister.url(), local(dir.filePath("Projects")));
    EXPECT_EQ(model.rowCount(), 4);

    lister.search(local(dir.path()), "report TXT");
    EXPECT_TRUE(waitFor(loop, [&] { return listed.count() == 3; }));
    EXPECT_EQ(names(), std::vector<std::string>({"Projects/Report Drafts/old/report-1.txt", "annual report.txt"}));

    lister.setShowHiddenFiles(true);
    lister.emitChanges();
    EXPECT_TRUE(waitFor(loop, [&] { return listed.count() == 4; }));
    EXPECT_EQ(names(), std::vector<std::string>({"Projects/Report Drafts/old/report-1.txt", "annual report.txt"}));

    lister.search(local(dir.path()), "report");
    EXPECT_TRUE(waitFor(loop, [&] { return listed.count() == 5; }));
    const auto all = names();
    EXPECT_NE(std::find(all.cbegin(), all.cend(), ".cache/report/report.bin"), all.cend());

    // No text lists the directory again.
    lister.search({}, " ");
    EXPECT_FALSE(lister.isSearching());
    EXPECT_TRUE(waitFor(loop, [&] { return listed.count() == 6; }));
    EXPECT_EQ(names(), std::vector<std::string>({"Projects/REPORT final.pdf", "Projects/Report Drafts",
                                                 "Projects/loop", "Projects/notes.txt"}));
}

TEST_F(FilesTest, SearchStopsWhenAFolderOpens)
{
    TemporaryDirectory dir;
    ASSERT_TRUE(makePath(dir.filePath("inner")));
    writeFile(dir.filePath("inner/match"));

    DirectoryLister lister;
    Spy listed(lister.completed);
    lister.search(local(dir.path()), "match");
    lister.openUrl(local(dir.filePath("inner")));
    EXPECT_FALSE(lister.isSearching());
    EXPECT_TRUE(waitFor(loop, [&] { return listed.count() == 1; }));
    ASSERT_EQ(lister.items().size(), 1u);
    EXPECT_EQ(lister.items().front().name(), "match");
}

TEST_F(FilesTest, RefreshDetectsChangedContents)
{
    TemporaryDirectory dir;
    // Without an extension the type comes from the contents.
    writeFile(dir.filePath("changed"), "%PDF-1.4\n");
    writeFile(dir.filePath("kept"), "%PDF-1.4\n");

    DirectoryLister lister;
    Spy listed(lister.completed);
    lister.openUrl(local(dir.path()));
    EXPECT_TRUE(waitFor(loop, [&] { return listed.count() == 1; }));

    const auto typeOf = [&](const std::string& name) {
        for (const auto& item : lister.items()) {
            if (item.name() == name)
                return item.mimetype();
        }
        return std::string();
    };
    EXPECT_EQ(typeOf("changed"), "application/pdf");
    EXPECT_EQ(typeOf("kept"), "application/pdf");

    writeFile(dir.filePath("changed"), "#!/bin/sh\necho\n");
    EXPECT_TRUE(waitFor(loop, [&] { return typeOf("changed") == "application/x-shellscript"; }));
    EXPECT_EQ(typeOf("kept"), "application/pdf");
}

// The sniffer's types by name and by contents; the expected types were
// recorded from a reference implementation on the laptop for the same files,
// its own executable standing in for the program.
TEST_F(FilesTest, SniffsTypesByNameAndContents)
{
    TemporaryDirectory dir;
    const std::string elf = readFile("/proc/self/exe").substr(0, 20000);
    const std::string transportStream =
        std::string(1, 0x47) + std::string(187, '\0') + std::string(1, 0x47) + std::string(187, '\0');
    const std::vector<std::tuple<std::string, std::string, std::string>> files = {
        {"program", elf, "application/x-executable"},
        {"libsample.so.6", elf, "application/x-sharedlib"},
        {"page.1", "plain words\n", "application/x-troff-man"},
        {"script", "#!/bin/sh\necho\n", "application/x-shellscript"},
        {"empty", {}, "application/x-zerosize"},
        {"empty.ts", {}, "text/vnd.trolltech.linguist"},
        {"strings.ts", "<?xml version=\"1.0\"?>\n<TS version=\"2.1\"></TS>\n", "text/vnd.trolltech.linguist"},
        {"video.ts", transportStream, "text/vnd.trolltech.linguist"},
        {"function.m", "function y = f(x)\nend\n", "text/x-matlab"},
        {"README", "read me\n", "text/x-readme"},
        {"Makefile", "all:\n", "text/x-makefile"},
        {"PHOTO.JPG", "\xff\xd8\xff\xe0", "image/jpeg"},
        {"source.C", "int main() {}\n", "text/x-c++src"},
        {"binary", std::string("\x00\x01\x02\x03", 4), "application/octet-stream"},
        {"document", "%PDF-1.4\n", "application/pdf"},
        {"notes~", "backup\n", "application/x-trash"},
        {"123.vdr", "recording", "video/mpeg"},
        {"callgrind.out.1", "events: Ir\n", "application/x-troff-man"},
        {"x.anim5", "zz", "video/x-anim"},
        {"photo.jpeg.txt", "hello\n", "text/plain"},
        {"\xc3\x84RGER.TXT", "x\n", "text/plain"},
        {"archive.tar.gz", "\x1f\x8b", "application/x-compressed-tar"},
    };

    for (const auto& [name, data, type] : files) {
        const std::string path = dir.filePath(name);
        writeFile(path, data);
        EXPECT_EQ(name + ' ' + internal::MimeSniffer::self()->mimeTypeForFile(path), name + ' ' + type);
    }
}

// A first listing detects types on several threads.
TEST_F(FilesTest, ListingDetectsEveryType)
{
    TemporaryDirectory dir;
    for (int i = 0; i < 400; ++i)
        writeFile(dir.filePath(std::to_string(i)), i % 2 ? "%PDF-1.4\n" : "#!/bin/sh\n");

    DirectoryLister lister;
    Spy listed(lister.completed);
    lister.openUrl(local(dir.path()));
    EXPECT_TRUE(waitFor(loop, [&] { return listed.count() == 1; }));
    EXPECT_EQ(lister.items().size(), 400u);

    for (const auto& item : lister.items()) {
        EXPECT_EQ(item.name() + ' ' + item.mimetype(),
                  item.name() + (std::stoi(item.name()) % 2 ? " application/pdf" : " application/x-shellscript"));
    }
}

// The name an item keeps is the one its URL decodes to.
TEST_F(FilesTest, ListedNamesMatchTheirUrls)
{
    TemporaryDirectory dir;
    const std::vector<std::string> names = {"a%20b", "100%",       "c#d",         "e?f",     "g h",      "[x]",
                                            "semi;colon", "back\\slash", "\xc3\xbc", "x\xff", "plus+sign"};
    for (const std::string& name : names)
        ASSERT_EQ(::close(::open(dir.filePath(name).c_str(), O_CREAT | O_WRONLY | O_CLOEXEC, 0644)), 0);

    DirectoryLister lister;
    Spy listed(lister.completed);
    lister.openUrl(local(dir.path()));
    EXPECT_TRUE(waitFor(loop, [&] { return listed.count() == 1; }));
    EXPECT_EQ(lister.items().size(), names.size());

    for (const auto& item : lister.items()) {
        EXPECT_EQ(item.name(), item.url().fileName());
        EXPECT_EQ(item.localPath().substr(item.localPath().rfind('/') + 1), item.name());
        // The URL reads back as the same item.
        EXPECT_EQ(Url(item.url().toString()), item.url());
    }
}

// A folder this large builds its items on several threads.
TEST_F(FilesTest, LargeListingHasEveryFileOnce)
{
    TemporaryDirectory dir;
    constexpr int Count = 20500;
    for (int i = 0; i < Count; ++i)
        ASSERT_EQ(::close(::open(dir.filePath(std::to_string(i)).c_str(), O_CREAT | O_WRONLY | O_CLOEXEC, 0644)), 0);

    DirectoryLister lister;
    Spy listed(lister.completed);
    lister.openUrl(local(dir.path()));
    EXPECT_TRUE(waitFor(loop, [&] { return listed.count() == 1; }, 30000ms));

    std::vector<bool> seen(Count, false);
    for (const auto& item : lister.items()) {
        const int index = std::stoi(item.name());
        EXPECT_FALSE(seen[size_t(index)]);
        seen[size_t(index)] = true;
        EXPECT_EQ(item.url(), local(dir.filePath(item.name())));
        EXPECT_EQ(item.localPath(), dir.filePath(item.name()));
    }
    EXPECT_EQ(std::count(seen.cbegin(), seen.cend(), false), 0);
}

// Files added or removed in one place leave the other rows where they are.
TEST_F(FilesTest, ModelFollowsChangesBetweenUnchangedRows)
{
    TemporaryDirectory dir;
    for (int i = 0; i < 60; ++i)
        writeFile(dir.filePath(std::to_string(i)));

    DirectoryLister lister;
    DirectoryModel model;
    model.setDirLister(&lister);
    ModelTester tester(model);
    Spy listed(lister.completed);
    lister.openUrl(local(dir.path()));
    EXPECT_TRUE(waitFor(loop, [&] { return listed.count() == 1; }));

    const auto sortedUrls = [](const FileItemList& items) {
        std::vector<std::string> urls;
        for (const auto& item : items)
            urls.push_back(item.url().toString());
        return sorted(urls);
    };

    PersistentRow kept(model, model.rowForUrl(local(dir.filePath("30"))));
    ASSERT_TRUE(kept.isValid());

    writeFile(dir.filePath("added"));
    EXPECT_TRUE(waitFor(loop, [&] { return model.rowCount() == 61; }));
    EXPECT_EQ(sortedUrls(model.items()), sortedUrls(lister.items()));

    ASSERT_EQ(::unlink(dir.filePath("10").c_str()), 0);
    EXPECT_TRUE(waitFor(loop, [&] { return model.rowCount() == 60; }));
    EXPECT_EQ(sortedUrls(model.items()), sortedUrls(lister.items()));

    ASSERT_TRUE(kept.isValid());
    EXPECT_EQ(model.itemForRow(kept.row()).name(), "30");
    EXPECT_GE(model.rowForUrl(local(dir.filePath("added"))), 0);
    EXPECT_LT(model.rowForUrl(local(dir.filePath("10"))), 0);
}

// Names are kept as the bytes they are, so invalid UTF-8 names that decode to
// the same text stay apart; one row each either way.
TEST_F(FilesTest, ModelKeepsRowsForCollidingNames)
{
    TemporaryDirectory dir;
    const std::vector<std::string> names = {"a\xff", "a\xfe", "b"};
    for (const std::string& name : names)
        ASSERT_EQ(::close(::open(dir.filePath(name).c_str(), O_CREAT | O_WRONLY | O_CLOEXEC, 0644)), 0);

    DirectoryLister lister;
    DirectoryModel model;
    model.setDirLister(&lister);
    ModelTester tester(model);
    Spy listed(lister.completed);
    lister.openUrl(local(dir.path()));
    EXPECT_TRUE(waitFor(loop, [&] { return listed.count() == 1; }));
    EXPECT_EQ(model.rowCount(), 3);

    for (int round = 0; round < 4; ++round) {
        const size_t before = listed.count();
        writeFile(dir.filePath("new" + std::to_string(round)));
        EXPECT_TRUE(waitFor(loop, [&] { return listed.count() > before; }));
        EXPECT_TRUE(waitFor(loop, [&] { return model.rowCount() == int(lister.items().size()); }));
        EXPECT_EQ(model.rowCount(), 4 + round);
    }
}

TEST_F(FilesTest, WatcherIgnoresUnrelatedSiblings)
{
    TemporaryDirectory directory;
    const auto watched = directory.filePath("watched");
    ASSERT_TRUE(makePath(watched));

    FileWatcher watcher;
    Spy changed(watcher.changed);
    EXPECT_TRUE(watcher.watch(watched));

    writeFile(directory.filePath("unrelated"));
    wait(loop, 80ms);
    EXPECT_EQ(changed.count(), 0u);

    writeFile(watched + "/inside");
    EXPECT_TRUE(waitFor(loop, [&] { return changed.count() == 1; }));
}

// A kept watch must not outlive its directory: one recreated at the path is watched anew.
TEST_F(FilesTest, WatcherFollowsRecreatedDirectory)
{
    TemporaryDirectory directory;
    const auto watched = directory.filePath("watched");
    ASSERT_TRUE(makePath(watched));

    FileWatcher watcher;
    Spy changed(watcher.changed);
    EXPECT_TRUE(watcher.watch(watched));

    TemporaryDirectory::removeAll(watched);
    ASSERT_TRUE(makePath(watched));
    EXPECT_TRUE(waitFor(loop, [&] { return changed.count() > 0; }));
    EXPECT_TRUE(watcher.watch(watched));

    wait(loop, 80ms);
    changed.calls.clear();
    writeFile(watched + "/inside");
    EXPECT_TRUE(waitFor(loop, [&] { return changed.count() > 0; }));
}

TEST_F(FilesTest, WatcherDirectoryRemoval)
{
    TemporaryDirectory dir;
    const auto path = dir.filePath("a/b");
    ASSERT_TRUE(makePath(path));
    writeFile(dir.filePath("sibling"));

    DirectoryLister lister;
    DirectoryModel model;
    model.setDirLister(&lister);
    Spy listed(lister.completed);
    Spy failed(lister.error);
    lister.openUrl(local(path));
    EXPECT_TRUE(waitFor(loop, [&] { return listed.count() == 1; }));

    TemporaryDirectory::removeAll(dir.filePath("a"));
    EXPECT_TRUE(waitFor(loop, [&] { return lister.url() == local(dir.path()); }));
    EXPECT_TRUE(waitFor(loop, [&] { return model.rowCount() == 1; }));
    EXPECT_EQ(model.itemForRow(0).name(), "sibling");
    EXPECT_EQ(failed.count(), 0u);
}

TEST_F(FilesTest, MissingDirectoryReportsOnce)
{
    TemporaryDirectory dir;
    const auto path = dir.filePath("missing");

    DirectoryLister lister;
    Spy failed(lister.error);
    lister.openUrl(local(path));
    EXPECT_TRUE(waitFor(loop, [&] { return failed.count() == 1; }));

    lister.emitChanges();
    wait(loop, 2500ms);
    EXPECT_EQ(failed.count(), 1u);
    EXPECT_EQ(lister.url(), local(path));
}

TEST_F(FilesTest, StaleListingAndDestruction)
{
    TemporaryDirectory dir;
    ASSERT_TRUE(makePath(dir.filePath("a")));
    ASSERT_TRUE(makePath(dir.filePath("b")));
    for (int i = 0; i < 100; ++i)
        writeFile(dir.filePath("a/" + std::to_string(i)));
    writeFile(dir.filePath("b/only"));

    DirectoryLister lister;
    DirectoryModel model;
    model.setDirLister(&lister);

    lister.openUrl(local(dir.filePath("a")));
    lister.openUrl(local(dir.filePath("b")));
    EXPECT_TRUE(waitFor(loop, [&] { return model.rowCount() == 1; }));
    EXPECT_EQ(model.itemForRow(0).name(), "only");

    auto pending = std::make_unique<DirectoryLister>();
    pending->openUrl(local(dir.filePath("a")));
    pending.reset();
}

TEST_F(FilesTest, AsynchronousTasksAndPaste)
{
    TemporaryDirectory dir;
    ASSERT_TRUE(makePath(dir.filePath("target")));
    writeFile(dir.filePath("source"), "payload");

    Task* copyTask = paste({local(dir.filePath("source"))}, false, local(dir.filePath("target")));
    Spy copied(copyTask->result);
    EXPECT_TRUE(waitFor(loop, [&] { return copied.count() == 1; }));
    EXPECT_EQ(readFile(dir.filePath("target/source")), "payload");
    EXPECT_TRUE(exists(dir.filePath("source")));

    Task* moveTask = paste({local(dir.filePath("source"))}, true, local(dir.filePath("target")));
    Spy asked(moveTask->conflictRequested);
    Spy moved(moveTask->result);
    EXPECT_TRUE(waitFor(loop, [&] { return asked.count() == 1; }));
    EXPECT_EQ(std::get<1>(asked.calls.front()), dir.filePath("source"));
    EXPECT_EQ(std::get<2>(asked.calls.front()), dir.filePath("target/source"));
    EXPECT_TRUE(moveTask->resolveConflict(std::get<0>(asked.calls.front()), int(ConflictPolicy::KeepBoth)));
    EXPECT_TRUE(waitFor(loop, [&] { return moved.count() == 1; }));
    EXPECT_FALSE(exists(dir.filePath("source")));
    EXPECT_EQ(readFile(dir.filePath("target/source (1)")), "payload");

    Task rename({Operation::Rename, {local(dir.filePath("target/source (1)"))}, local(dir.filePath("renamed"))});
    Spy renamed(rename.result);
    rename.start();
    rename.start();
    EXPECT_TRUE(waitFor(loop, [&] { return renamed.count() == 1; }));
    EXPECT_EQ(rename.state(), Task::State::Succeeded);

    Task remove({Operation::Delete, {local(dir.filePath("target"))}});
    Spy removed(remove.result);
    remove.start();
    EXPECT_TRUE(waitFor(loop, [&] { return removed.count() == 1; }));
    EXPECT_FALSE(exists(dir.filePath("target")));
}

TEST_F(FilesTest, ConflictResolutionAppliesToAll)
{
    TemporaryDirectory dir;
    ASSERT_TRUE(makePath(dir.filePath("target")));
    writeFile(dir.filePath("a"), "new a");
    writeFile(dir.filePath("b"), "new b");
    writeFile(dir.filePath("target/a"), "old a");
    writeFile(dir.filePath("target/b"), "old b");

    Task task({Operation::Copy,
               {local(dir.filePath("a")), local(dir.filePath("b"))},
               local(dir.filePath("target")),
               ConflictPolicy::Ask});
    Spy conflict(task.conflictRequested);
    Spy finished(task.result);
    task.start();
    EXPECT_TRUE(waitFor(loop, [&] { return conflict.count() == 1; }));
    task.resolveConflict(std::get<0>(conflict.calls.front()), int(ConflictPolicy::Replace), true);

    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 1; }));
    EXPECT_EQ(conflict.count(), 1u);
    EXPECT_EQ(task.state(), Task::State::Succeeded);
    EXPECT_EQ(readFile(dir.filePath("target/a")), "new a");
    EXPECT_EQ(readFile(dir.filePath("target/b")), "new b");
}

// A second answer to a conflict, arriving once the worker waits at the next
// one but before that one is announced, must not answer it unseen.
TEST_F(FilesTest, DuplicateConflictAnswerIsRefused)
{
    TemporaryDirectory dir;
    ASSERT_TRUE(makePath(dir.filePath("target")));
    writeFile(dir.filePath("a"), "new a");
    writeFile(dir.filePath("b"), "new b");
    writeFile(dir.filePath("target/a"), "old a");
    writeFile(dir.filePath("target/b"), "old b");

    Task task({Operation::Copy,
               {local(dir.filePath("a")), local(dir.filePath("b"))},
               local(dir.filePath("target")),
               ConflictPolicy::Ask});
    Spy conflict(task.conflictRequested);
    Spy finished(task.result);
    task.start();
    ASSERT_TRUE(waitFor(loop, [&] { return conflict.count() == 1; }));
    const uint32_t first = std::get<0>(conflict.calls.front());
    EXPECT_FALSE(task.resolveConflict(first + 1, int(ConflictPolicy::Replace)));
    EXPECT_TRUE(task.resolveConflict(first, int(ConflictPolicy::Replace)));

    // The loop does not run here, so the next conflict is waiting but not yet announced.
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    EXPECT_FALSE(task.resolveConflict(first, int(ConflictPolicy::Replace)));

    ASSERT_TRUE(waitFor(loop, [&] { return conflict.count() == 2; }));
    const uint32_t second = std::get<0>(conflict.calls.back());
    EXPECT_NE(second, first);
    EXPECT_EQ(std::get<1>(conflict.calls.back()), dir.filePath("b"));
    EXPECT_EQ(finished.count(), 0u);
    EXPECT_TRUE(task.resolveConflict(second, int(ConflictPolicy::Skip)));
    EXPECT_FALSE(task.resolveConflict(second, int(ConflictPolicy::Replace)));

    ASSERT_TRUE(waitFor(loop, [&] { return finished.count() == 1; }));
    EXPECT_EQ(task.state(), Task::State::Succeeded);
    EXPECT_EQ(readFile(dir.filePath("target/a")), "new a");
    EXPECT_EQ(readFile(dir.filePath("target/b")), "old b");
}

TEST_F(FilesTest, PasteIntoOwnFolderDuplicatesWithoutAsking)
{
    TemporaryDirectory dir;
    writeFile(dir.filePath("source"), "payload");

    Task task({Operation::Copy, {local(dir.filePath("source"))}, local(dir.path()), ConflictPolicy::Ask});
    Spy conflict(task.conflictRequested);
    Spy finished(task.result);
    task.start();
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 1; }));

    EXPECT_EQ(conflict.count(), 0u);
    EXPECT_EQ(task.state(), Task::State::Succeeded);
    EXPECT_EQ(readFile(dir.filePath("source (1)")), "payload");
}

TEST_F(FilesTest, AsynchronousConflictAndCancel)
{
    TemporaryDirectory dir;
    ASSERT_TRUE(makePath(dir.filePath("target")));
    writeFile(dir.filePath("source"), "new");
    writeFile(dir.filePath("target/source"), "old");

    Task task({Operation::Copy, {local(dir.filePath("source"))}, local(dir.filePath("target")), ConflictPolicy::Ask});
    Spy conflict(task.conflictRequested);
    Spy finished(task.result);
    task.start();
    EXPECT_TRUE(waitFor(loop, [&] { return conflict.count() == 1; }));
    task.resolveConflict(std::get<0>(conflict.calls.front()), int(ConflictPolicy::KeepBoth));
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 1; }));
    EXPECT_EQ(task.state(), Task::State::Succeeded);
    EXPECT_EQ(readFile(dir.filePath("target/source (1)")), "new");

    Task canceled(task.request());
    Spy waiting(canceled.conflictRequested);
    Spy done(canceled.result);
    canceled.start();
    EXPECT_TRUE(waitFor(loop, [&] { return waiting.count() == 1; }));
    canceled.cancel();
    EXPECT_TRUE(waitFor(loop, [&] { return done.count() == 1; }));

    EXPECT_EQ(canceled.state(), Task::State::Canceled);
    EXPECT_EQ(readFile(dir.filePath("target/source")), "old");
}

TEST_F(FilesTest, ControllerTrashUndo)
{
    TemporaryDirectory dir;
    writeFile(dir.filePath("file"));
    TaskController controller;
    Spy finished(controller.taskFinished);

    controller.submit({Operation::Trash, {local(dir.filePath("file"))}});
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 1; }));
    EXPECT_TRUE(controller.isUndoAvailable());

    EXPECT_TRUE(controller.undo());
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 2; }));
    EXPECT_TRUE(exists(dir.filePath("file")));
    EXPECT_FALSE(controller.isUndoAvailable());
}

TEST_F(FilesTest, ControllerTrashRedo)
{
    TemporaryDirectory dir;
    writeFile(dir.filePath("file"));
    TaskController controller;
    Spy finished(controller.taskFinished);

    controller.submit({Operation::Trash, {local(dir.filePath("file"))}});
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 1; }));
    EXPECT_FALSE(controller.isRedoAvailable());

    EXPECT_TRUE(controller.undo());
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 2; }));
    EXPECT_TRUE(controller.isRedoAvailable());

    EXPECT_TRUE(controller.redo());
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 3; }));
    EXPECT_FALSE(exists(dir.filePath("file")));
    EXPECT_FALSE(controller.isRedoAvailable());
    EXPECT_TRUE(controller.isUndoAvailable());

    // A new operation after an undo forgets what could be redone.
    EXPECT_TRUE(controller.undo());
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 4; }));
    EXPECT_TRUE(controller.isRedoAvailable());
    writeFile(dir.filePath("other"));
    controller.submit({Operation::Trash, {local(dir.filePath("other"))}});
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 5; }));
    EXPECT_FALSE(controller.isRedoAvailable());
}

TEST_F(FilesTest, ControllerCopyUndo)
{
    TemporaryDirectory dir;
    ASSERT_TRUE(makePath(dir.filePath("target")));
    writeFile(dir.filePath("file"), "data");
    TaskController controller;
    Spy finished(controller.taskFinished);

    controller.submit({Operation::Copy, {local(dir.filePath("file"))}, local(dir.filePath("target"))});
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 1; }));
    EXPECT_TRUE(controller.undo());
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 2; }));
    EXPECT_FALSE(exists(dir.filePath("target/file")));
    EXPECT_TRUE(exists(dir.filePath("file")));
    EXPECT_EQ(LocalFileSystem::enumerate(Url("trash:/")).size(), 1u);

    // Redo brings the undone copy back from the trash.
    EXPECT_TRUE(controller.redo());
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 3; }));
    EXPECT_EQ(readFile(dir.filePath("target/file")), "data");
    EXPECT_TRUE(controller.isUndoAvailable());
    EXPECT_FALSE(controller.isRedoAvailable());
}

TEST_F(FilesTest, ControllerMergeHasNoUndo)
{
    TemporaryDirectory dir;
    ASSERT_TRUE(makePath(dir.filePath("source/folder")));
    ASSERT_TRUE(makePath(dir.filePath("target/folder")));
    writeFile(dir.filePath("source/folder/new"), "new");
    writeFile(dir.filePath("target/folder/old"), "old");
    TaskController controller;
    Spy finished(controller.taskFinished);

    // Undoing would take the folder's earlier contents along with the merged ones.
    for (const auto operation : {Operation::Copy, Operation::Move}) {
        controller.submit(
            {operation, {local(dir.filePath("source/folder"))}, local(dir.filePath("target")), ConflictPolicy::Replace});
        EXPECT_TRUE(waitFor(loop, [&] { return finished.count() > 0; }));
        finished.calls.clear();
        EXPECT_EQ(readFile(dir.filePath("target/folder/new")), "new");
        EXPECT_EQ(readFile(dir.filePath("target/folder/old")), "old");
        EXPECT_FALSE(controller.isUndoAvailable());
    }
}

TEST_F(FilesTest, ControllerMoveUndo)
{
    TemporaryDirectory dir;
    ASSERT_TRUE(makePath(dir.filePath("a")));
    ASSERT_TRUE(makePath(dir.filePath("b")));
    ASSERT_TRUE(makePath(dir.filePath("target")));
    writeFile(dir.filePath("a/one"), "one");
    writeFile(dir.filePath("b/two"), "two");
    writeFile(dir.filePath("target/two"), "old");
    TaskController controller;
    Spy finished(controller.taskFinished);
    std::string firstError = "unset";
    const base::Subscription errors = controller.taskFinished.connect([&](Task* task) {
        if (firstError == "unset")
            firstError = task->errorString();
    });

    controller.submit({Operation::Move,
                       {local(dir.filePath("a/one")), local(dir.filePath("b/two"))},
                       local(dir.filePath("target")),
                       ConflictPolicy::KeepBoth});
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 1; }));
    EXPECT_EQ(firstError, "");
    EXPECT_EQ(readFile(dir.filePath("target/two (1)")), "two");

    // Each item goes back to its own folder under its old name.
    EXPECT_TRUE(controller.undo());
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 2; }));
    EXPECT_EQ(readFile(dir.filePath("a/one")), "one");
    EXPECT_EQ(readFile(dir.filePath("b/two")), "two");
    EXPECT_EQ(readFile(dir.filePath("target/two")), "old");
    EXPECT_FALSE(exists(dir.filePath("target/two (1)")));

    EXPECT_TRUE(controller.redo());
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 3; }));
    EXPECT_EQ(readFile(dir.filePath("target/one")), "one");
    EXPECT_EQ(readFile(dir.filePath("target/two (1)")), "two");
}

TEST_F(FilesTest, ControllerRenameAndMkdirUndo)
{
    TemporaryDirectory dir;
    TaskController controller;
    Spy finished(controller.taskFinished);

    controller.submit({Operation::Mkdir, {}, local(dir.filePath("New Folder"))});
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 1; }));
    controller.submit({Operation::Rename, {local(dir.filePath("New Folder"))}, local(dir.filePath("Photos"))});
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 2; }));

    EXPECT_TRUE(controller.undo());
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 3; }));
    EXPECT_TRUE(isDirectory(dir.filePath("New Folder")));
    EXPECT_FALSE(exists(dir.filePath("Photos")));

    EXPECT_TRUE(controller.undo());
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 4; }));
    EXPECT_FALSE(exists(dir.filePath("New Folder")));
    EXPECT_FALSE(controller.isUndoAvailable());

    EXPECT_TRUE(controller.redo());
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 5; }));
    EXPECT_TRUE(controller.redo());
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 6; }));
    EXPECT_TRUE(isDirectory(dir.filePath("Photos")));
}

TEST_F(FilesTest, ControllerFailedUndoIsDropped)
{
    TemporaryDirectory dir;
    writeFile(dir.filePath("file"));
    TaskController controller;
    Spy finished(controller.taskFinished);

    controller.submit({Operation::Rename, {local(dir.filePath("file"))}, local(dir.filePath("renamed"))});
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 1; }));
    ASSERT_EQ(::unlink(dir.filePath("renamed").c_str()), 0);

    Task* task = controller.undo();
    ASSERT_TRUE(task);
    int error = 0;
    bool done = false;
    task->whenFinished([&](Task* finishedTask) {
        error = finishedTask->error();
        done = true;
    });
    EXPECT_TRUE(waitFor(loop, [&] { return done; }));
    EXPECT_EQ(error, ENOENT);
    EXPECT_FALSE(controller.isUndoAvailable());
    EXPECT_FALSE(controller.isRedoAvailable());
}

TEST_F(FilesTest, ControllerEmptyTrashForgetsRestores)
{
    TemporaryDirectory dir;
    writeFile(dir.filePath("file"));
    writeFile(dir.filePath("other"));
    TaskController controller;
    Spy finished(controller.taskFinished);

    controller.submit({Operation::Trash, {local(dir.filePath("file"))}});
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 1; }));
    controller.submit({Operation::Rename, {local(dir.filePath("other"))}, local(dir.filePath("renamed"))});
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 2; }));
    controller.submit({Operation::EmptyTrash});
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 3; }));

    EXPECT_TRUE(controller.undo());
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 4; }));
    EXPECT_TRUE(exists(dir.filePath("other")));
    EXPECT_FALSE(controller.isUndoAvailable());
}

// The runtime directory is a tmpfs, another file system than the build tree's.
TEST_F(FilesTest, CrossDeviceMove)
{
    TemporaryDirectory source(".");
    TemporaryDirectory target(base::paths::runtimeDirectory());
    ASSERT_TRUE(source.isValid());
    if (!target.isValid())
        GTEST_SKIP() << "The runtime directory is not writable";

    struct stat from, to;
    ASSERT_EQ(::stat(source.path().c_str(), &from), 0);
    ASSERT_EQ(::stat(target.path().c_str(), &to), 0);
    if (from.st_dev == to.st_dev)
        GTEST_SKIP() << "The runtime directory is on the build tree's file system";

    ASSERT_TRUE(makePath(source.filePath("tree/sub")));
    writeFile(source.filePath("tree/sub/file"), std::string(4 * 1024 * 1024, 'x'));
    ASSERT_EQ(::symlink("sub/file", source.filePath("tree/link").c_str()), 0);

    OperationContext ctx;
    LocalFileSystem::move(source.filePath("tree"), target.filePath("tree"), ConflictPolicy::Fail, ctx);

    EXPECT_FALSE(exists(source.filePath("tree")));
    EXPECT_EQ(fileSize(target.filePath("tree/sub/file")), 4 * 1024 * 1024);
    EXPECT_TRUE(isSymbolicLink(target.filePath("tree/link")));

    OperationContext cancel;
    cancel.progress = [&](uint64_t bytes, uint64_t) {
        if (bytes)
            cancel.canceled = true;
    };

    EXPECT_EQ(errorCode([&] {
                  LocalFileSystem::move(target.filePath("tree"), source.filePath("tree"), ConflictPolicy::Fail, cancel);
              }),
              ECANCELED);

    EXPECT_TRUE(exists(target.filePath("tree/sub/file")));
    EXPECT_FALSE(exists(source.filePath("tree")));
}

TEST_F(FilesTest, ProgressIncludesTotalsAndCurrentPath)
{
    TemporaryDirectory dir;
    ASSERT_TRUE(makePath(dir.filePath("source/nested")));
    ASSERT_TRUE(makePath(dir.filePath("target")));
    writeFile(dir.filePath("source/nested/one"), std::string(4 * 1024 * 1024, 'x'));
    writeFile(dir.filePath("two"), std::string(2 * 1024 * 1024, 'y'));

    Task task({Operation::Copy, {local(dir.filePath("source")), local(dir.filePath("two"))}, local(dir.filePath("target"))});
    std::vector<TaskProgress> snapshots;
    const std::thread::id thread = std::this_thread::get_id();
    const base::Subscription progress = task.progressInfoChanged.connect([&] {
        EXPECT_EQ(std::this_thread::get_id(), thread);
        snapshots.push_back(task.progressInfo());
    });

    Spy done(task.result);
    task.start();
    EXPECT_TRUE(waitFor(loop, [&] { return done.count() == 1; }));

    EXPECT_EQ(task.state(), Task::State::Succeeded);
    ASSERT_FALSE(snapshots.empty());
    EXPECT_TRUE(snapshots.front().preparing);
    EXPECT_FALSE(snapshots.front().currentPath.empty());

    const auto last = task.progressInfo();
    EXPECT_FALSE(last.preparing);
    EXPECT_TRUE(last.totalsKnown);
    EXPECT_EQ(last.totalBytes, uint64_t(6 * 1024 * 1024));
    EXPECT_EQ(last.processedBytes, last.totalBytes);
    EXPECT_EQ(last.totalItems, 4u);
    EXPECT_EQ(last.processedItems, last.totalItems);

    uint64_t previous = 0;
    for (const auto& snapshot : snapshots) {
        if (snapshot.preparing)
            continue;
        EXPECT_GE(snapshot.processedBytes, previous);
        EXPECT_LE(snapshot.processedBytes, snapshot.totalBytes);
        previous = snapshot.processedBytes;
    }

    EXPECT_EQ(task.operationName(), "copy");
    EXPECT_EQ(task.destinationName(), "target");
}

TEST_F(FilesTest, CopyThatCannotFitFailsBeforeWriting)
{
    TemporaryDirectory target(base::paths::runtimeDirectory());
    if (!target.isValid())
        GTEST_SKIP() << "The runtime directory is not writable";

    struct statvfs fs;
    ASSERT_EQ(::statvfs(target.path().c_str(), &fs), 0);
    const int64_t available = int64_t(fs.f_bavail) * int64_t(fs.f_frsize);
    if (available > (int64_t(8) << 30))
        GTEST_SKIP() << "The runtime directory has too much free space to fill";

    // Kept on the build tree's disk, so the large source costs no memory.
    TemporaryDirectory source(".");
    ASSERT_TRUE(source.isValid());
    const int64_t length = available + 64 * 1024 * 1024;

    const int full = ::open(source.filePath("full").c_str(), O_WRONLY | O_CREAT | O_CLOEXEC, 0600);
    ASSERT_GE(full, 0);
    const int allocated = ::posix_fallocate(full, 0, length);
    ::close(full);
    if (allocated != 0)
        GTEST_SKIP() << "The build tree has no room for a source larger than the target";

    Task copy({Operation::Copy, {local(internal::absoluteFilePath(source.filePath("full")))}, local(target.path())});
    Spy failed(copy.result);
    copy.start();
    EXPECT_TRUE(waitFor(loop, [&] { return failed.count() == 1; }, 30000ms));
    EXPECT_EQ(copy.error(), ENOSPC);
    EXPECT_TRUE(entries(target.path()).empty());

    const std::string sparse = source.filePath("sparse");
    {
        const int fd = ::open(sparse.c_str(), O_WRONLY | O_CREAT | O_CLOEXEC, 0600);
        ASSERT_GE(fd, 0);
        ASSERT_EQ(::ftruncate(fd, length), 0);
        ASSERT_EQ(::pwrite(fd, "tail", 4, length - 4), 4);
        ::close(fd);
    }

    Task fits({Operation::Copy, {local(internal::absoluteFilePath(sparse))}, local(target.path())});
    Spy done(fits.result);
    fits.start();
    EXPECT_TRUE(waitFor(loop, [&] { return done.count() == 1; }, 30000ms));
    EXPECT_EQ(fits.state(), Task::State::Succeeded);
    EXPECT_EQ(fileSize(target.filePath("sparse")), length);
}

TEST_F(FilesTest, PreparationSharesCancellation)
{
    TemporaryDirectory dir;
    ASSERT_TRUE(makePath(dir.filePath("nested")));
    writeFile(dir.filePath("nested/file"));

    OperationContext task;
    OperationContext scan;
    scan.cancelFlag = &task.canceled;
    scan.currentItem = [&](const std::string&) {
        task.canceled = true;
    };

    EXPECT_EQ(errorCode([&] { FileEnumerator::size(dir.path(), scan); }), ECANCELED);
    EXPECT_TRUE(exists(dir.filePath("nested/file")));
}

TEST_F(FilesTest, TaskDestructionWhileWaiting)
{
    TemporaryDirectory dir;
    writeFile(dir.filePath("source"));
    writeFile(dir.filePath("target"));
    auto task = std::make_unique<Task>(
        TaskRequest{Operation::Rename, {local(dir.filePath("source"))}, local(dir.filePath("target")), ConflictPolicy::Ask, {}});

    Spy conflict(task->conflictRequested);
    task->start();
    EXPECT_TRUE(waitFor(loop, [&] { return conflict.count() == 1; }));

    task.reset();
    EXPECT_TRUE(exists(dir.filePath("source")));
}

TEST_F(FilesTest, AsynchronousErrorsAndMkdir)
{
    TemporaryDirectory dir;
    Task mkdirTask({Operation::Mkdir, {}, local(dir.filePath("folder"))});
    Spy made(mkdirTask.result);
    mkdirTask.start();
    EXPECT_TRUE(waitFor(loop, [&] { return made.count() == 1; }));
    EXPECT_EQ(mkdirTask.state(), Task::State::Succeeded);

    Task unsupported({Operation::Copy, {Url("https://example.invalid/file")}, local(dir.path())});
    Spy failed(unsupported.result);
    unsupported.start();
    EXPECT_TRUE(waitFor(loop, [&] { return failed.count() == 1; }));
    EXPECT_EQ(unsupported.error(), ENOTSUP);

    Task canceled({Operation::Mkdir, {}, local(dir.filePath("canceled"))});
    Spy done(canceled.result);
    canceled.cancel();
    canceled.start();
    EXPECT_TRUE(waitFor(loop, [&] { return done.count() == 1; }));
    EXPECT_EQ(canceled.state(), Task::State::Canceled);
    EXPECT_FALSE(exists(dir.filePath("canceled")));
}

TEST_F(FilesTest, LinkDrop)
{
    TemporaryDirectory dir;
    writeFile(dir.filePath("file"));
    ASSERT_TRUE(makePath(dir.filePath("target")));

    Spy finished(TaskController::instance()->taskFinished);
    drop({local(dir.filePath("file"))}, local(dir.filePath("target")), DropAction::Link);
    EXPECT_TRUE(waitFor(loop, [&] { return finished.count() == 1; }));
    EXPECT_TRUE(isSymbolicLink(dir.filePath("target/file")));
    EXPECT_TRUE(exists(dir.filePath("file")));
}

TEST_F(FilesTest, LinkDoesNotReplaceWhatItPointsTo)
{
    TemporaryDirectory dir;
    ASSERT_TRUE(makePath(dir.filePath("real")));
    writeFile(dir.filePath("real/file"), "data");
    ASSERT_EQ(::symlink("real", dir.filePath("alias").c_str()), 0);

    OperationContext ctx;
    EXPECT_EQ(errorCode([&] {
                  LocalFileSystem::link(dir.filePath("real/file"), dir.filePath("alias/file"), ConflictPolicy::Replace,
                                        ctx);
              }),
              EINVAL);

    EXPECT_FALSE(isSymbolicLink(dir.filePath("real/file")));
    EXPECT_EQ(readFile(dir.filePath("real/file")), "data");
    for (const std::string& name : entries(dir.filePath("real")))
        EXPECT_FALSE(name.starts_with(".lingmo-")) << name;
}

TEST_F(FilesTest, TypeDescriptionsAndIcons)
{
    TemporaryDirectory dir;
    writeFile(dir.filePath("notes.txt"), "words\n");
    ASSERT_TRUE(makePath(dir.filePath("folder")));

    FileItem text(local(dir.filePath("notes.txt")));
    text.prepareMetadata();
    EXPECT_EQ(text.mimetype(), "text/plain");
    EXPECT_FALSE(text.typeDescription().empty());
    // The shared-mime-info comment in English.
    if (base::Locale::system().language() == "en") {
        EXPECT_EQ(text.typeDescription(), "Plain text document");
        EXPECT_EQ(mime::description("inode/directory"), "Folder");
        EXPECT_EQ(mime::description("application/x-compressed-tar"), "Tar archive (gzip-compressed)");
    }
    EXPECT_EQ(text.iconName(), "text-plain");

    FileItem folder(local(dir.filePath("folder")));
    folder.prepareMetadata();
    EXPECT_EQ(folder.mimetype(), "inode/directory");
    EXPECT_EQ(folder.iconName(), "folder");

    FileItem home(local(base::paths::home()));
    home.prepareMetadata();
    EXPECT_EQ(home.iconName(), "folder-home");

    EXPECT_EQ(mime::canonicalName("application/x-pdf"), "application/pdf");
    EXPECT_TRUE(mime::inherits("application/x-shellscript", "text/plain"));
    EXPECT_EQ(mime::typeForFile(dir.filePath("folder")), "inode/directory");
}
