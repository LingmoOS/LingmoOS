// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "archives.h"

#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/file.h"
#include "base/paths.h"
#include "base/process.h"
#include "base/url.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <functional>
#include <sys/stat.h>
#include <thread>

using namespace Lingmo::base;
using dbus::Variant;
using dbus::VariantMap;
using fmtest::noise;
using fmtest::splitFile;
using fmtest::writeArchive;

static constexpr char kService[] = "org.lingmo.FileOperations";
static constexpr char kManagerPath[] = "/org/lingmo/FileOperations";
static constexpr char kManager[] = "org.lingmo.FileOperations.Manager1";
static constexpr char kTask[] = "org.lingmo.FileOperations.Task1";

namespace {

class FileOperationsDaemonTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        bus = dbus::Connection::open(dbus::Connection::Bus::Session, &loop);
        ASSERT_TRUE(bus);
        ASSERT_TRUE(daemon.start(FILEMANAGER_DAEMON));
        ASSERT_TRUE(waitFor([&] { return manager().call("CanUndo").value<bool>().has_value(); }, 3000));
    }

    void TearDown() override
    {
        daemon.terminate();
        EXPECT_TRUE(daemon.waitForFinished(std::chrono::seconds(3)));
    }

    dbus::Proxy manager(dbus::Connection* connection = nullptr)
    {
        return dbus::Proxy(connection ? connection : bus.get(), kService, kManagerPath, kManager);
    }

    dbus::Proxy task(const std::string& path) { return dbus::Proxy(bus.get(), kService, path, kTask); }

    static bool waitFor(const std::function<bool()>& condition, int milliseconds)
    {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(milliseconds);
        while (std::chrono::steady_clock::now() < deadline) {
            if (condition())
                return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        return condition();
    }

    std::optional<VariantMap> snapshot(const std::string& path) { return task(path).call("GetSnapshot").value<VariantMap>(); }

    static bool flag(const VariantMap& map, const std::string& key)
    {
        const auto found = map.find(key);
        return found != map.end() && found->second.value<bool>();
    }

    std::string submit(int32_t operation, const std::vector<std::string>& sources, const std::string& destination,
                       int32_t conflict, dbus::Connection* connection = nullptr)
    {
        const auto path = manager(connection).call("Submit", operation, sources, destination, conflict)
                                  .value<dbus::ObjectPath>();
        return path ? path->path : std::string();
    }

    std::string submitArchive(int32_t operation, const std::vector<std::string>& sources, const std::string& output,
                              dbus::Connection* connection = nullptr)
    {
        const auto path = manager(connection).call("SubmitArchive", operation, sources, output)
                                  .value<dbus::ObjectPath>();
        return path ? path->path : std::string();
    }

    VariantMap waitForFinish(const std::string& path, int milliseconds = 10000)
    {
        VariantMap result;
        waitFor([&] {
            const auto current = snapshot(path);
            if (current)
                result = *current;
            return current && flag(*current, "finished");
        }, milliseconds);
        return result;
    }

    // The id of the password request waiting, past the one given; 0 if none came.
    uint32_t waitForPasswordRequest(const std::string& path, uint32_t after = 0)
    {
        uint32_t request = 0;
        waitFor([&] {
            const auto current = snapshot(path);
            if (!current || !flag(*current, "waitingForPassword"))
                return false;
            request = current->at("passwordId").value<uint32_t>();
            return request != 0 && request != after;
        }, 5000);
        return request != after ? request : 0;
    }

    static std::vector<std::string> entries(const std::string& directory)
    {
        std::vector<std::string> names;
        for (const auto& entry : std::filesystem::directory_iterator(directory))
            names.push_back(entry.path().filename().string());
        std::sort(names.begin(), names.end());
        return names;
    }

    EventLoop loop;
    std::unique_ptr<dbus::Connection> bus;
    Process daemon;
};

} // namespace

TEST_F(FileOperationsDaemonTest, OperationOutlivesSubmittingClient)
{
    file::TemporaryDirectory source(paths::runtimeDirectory(), "source");
    file::TemporaryDirectory destination(paths::runtimeDirectory(), "destination");
    const std::string payload(16 * 1024 * 1024, 'x');
    ASSERT_TRUE(file::writeAtomically(source.path() + "/payload.bin", payload));

    std::string path;
    {
        auto submitter = dbus::Connection::open(dbus::Connection::Bus::Session, &loop);
        path = submit(0, {url::fromLocalFile(source.path() + "/payload.bin")}, url::fromLocalFile(destination.path()),
                      4, submitter.get());
        ASSERT_FALSE(path.empty());
    }

    VariantMap result;
    ASSERT_TRUE(waitFor([&] {
        const auto current = snapshot(path);
        if (current)
            result = *current;
        return current && flag(*current, "finished");
    }, 10000));
    EXPECT_EQ(result["errorCode"].value<int32_t>(), 0);
    struct stat st;
    ASSERT_EQ(::stat((destination.path() + "/payload.bin").c_str(), &st), 0);
    EXPECT_EQ(uint64_t(st.st_size), payload.size());
}

TEST_F(FileOperationsDaemonTest, LiveTasksAreDiscoverable)
{
    file::TemporaryDirectory directory(paths::runtimeDirectory(), "conflict");
    ASSERT_TRUE(file::writeAtomically(directory.path() + "/source", "new"));
    ASSERT_TRUE(file::writeAtomically(directory.path() + "/destination", "old"));

    const std::string path = submit(5, {url::fromLocalFile(directory.path() + "/source")},
                                    url::fromLocalFile(directory.path() + "/destination"), 4);
    ASSERT_FALSE(path.empty());
    ASSERT_TRUE(waitFor([&] {
        const auto current = snapshot(path);
        return current && flag(*current, "waitingForConflict");
    }, 3000));

    VariantMap waiting = *snapshot(path);
    const VariantMap progress = waiting["progress"].value<VariantMap>();
    ASSERT_FALSE(progress.empty());
    EXPECT_TRUE(flag(progress, "totalsKnown"));
    EXPECT_EQ(progress.at("totalItems").value<uint64_t>(), 1u);

    const auto paths = manager().call("TaskPaths").value<std::vector<std::string>>();
    ASSERT_TRUE(paths);
    EXPECT_NE(std::find(paths->begin(), paths->end(), path), paths->end());

    EXPECT_FALSE(task(path).call("ResolveConflict", waiting["conflictId"].value<uint32_t>(), int32_t(1), false).isError());
    EXPECT_TRUE(waitFor([&] {
        const auto current = snapshot(path);
        return current && flag(*current, "finished");
    }, 3000));
}

TEST_F(FileOperationsDaemonTest, FinishedTasksLeaveTheBus)
{
    file::TemporaryDirectory directory(paths::runtimeDirectory(), "mkdir");
    const std::string created = directory.path() + "/created";
    const std::string path = submit(4, {}, url::fromLocalFile(created), 0);
    ASSERT_FALSE(path.empty());

    // A client attaching just after the task ended still reads its result.
    ASSERT_TRUE(waitFor([&] {
        const auto current = snapshot(path);
        return current && flag(*current, "finished");
    }, 3000));
    struct stat st;
    ASSERT_EQ(::stat(created.c_str(), &st), 0);
    EXPECT_TRUE(S_ISDIR(st.st_mode));

    EXPECT_TRUE(waitFor([&] { return !snapshot(path); }, 15000));
}

TEST_F(FileOperationsDaemonTest, ConflictOfVanishedSubmitterIsCanceled)
{
    file::TemporaryDirectory directory(paths::runtimeDirectory(), "vanished");
    const std::string source = directory.path() + "/source";
    const std::string destination = directory.path() + "/destination";
    ASSERT_TRUE(file::writeAtomically(source, "new"));
    ASSERT_TRUE(file::writeAtomically(destination, "old"));

    std::string path;
    {
        auto submitter = dbus::Connection::open(dbus::Connection::Bus::Session, &loop);
        path = submit(5, {url::fromLocalFile(source)}, url::fromLocalFile(destination), 4, submitter.get());
        ASSERT_FALSE(path.empty());
        ASSERT_TRUE(waitFor([&] {
            const auto current = snapshot(path);
            return current && flag(*current, "waitingForConflict");
        }, 3000));
        // The submitter leaves the bus without answering the conflict.
    }

    VariantMap result;
    ASSERT_TRUE(waitFor([&] {
        const auto current = snapshot(path);
        if (current)
            result = *current;
        return current && flag(*current, "finished");
    }, 3000));
    EXPECT_TRUE(flag(result, "canceled"));
    EXPECT_EQ(file::read(destination).value_or(""), "old");
    struct stat st;
    EXPECT_EQ(::stat(source.c_str(), &st), 0);
}

TEST_F(FileOperationsDaemonTest, UnknownConflictAnswerKeepsWaiting)
{
    file::TemporaryDirectory directory(paths::runtimeDirectory(), "unknown");
    const std::string source = directory.path() + "/source";
    const std::string destination = directory.path() + "/destination";
    ASSERT_TRUE(file::writeAtomically(source, "new"));
    ASSERT_TRUE(file::writeAtomically(destination, "old"));

    std::string path;
    {
        auto submitter = dbus::Connection::open(dbus::Connection::Bus::Session, &loop);
        path = submit(5, {url::fromLocalFile(source)}, url::fromLocalFile(destination), 4, submitter.get());
        ASSERT_FALSE(path.empty());
        ASSERT_TRUE(waitFor([&] {
            const auto current = snapshot(path);
            return current && flag(*current, "waitingForConflict");
        }, 3000));

        const uint32_t conflict = snapshot(path)->at("conflictId").value<uint32_t>();
        for (const int32_t policy : {int32_t(-1), int32_t(4), int32_t(99)})
            EXPECT_TRUE(task(path).call("ResolveConflict", conflict, policy, false).isError()) << policy;
        const auto current = snapshot(path);
        ASSERT_TRUE(current);
        EXPECT_TRUE(flag(*current, "waitingForConflict"));
        EXPECT_FALSE(flag(*current, "finished"));
    }

    // Still waiting, so the submitter leaving cancels it.
    VariantMap result;
    ASSERT_TRUE(waitFor([&] {
        const auto current = snapshot(path);
        if (current)
            result = *current;
        return current && flag(*current, "finished");
    }, 3000));
    EXPECT_TRUE(flag(result, "canceled"));
    EXPECT_EQ(file::read(destination).value_or(""), "old");
}

// An answer counts only for the conflict it names: a repeated answer, or one
// to a conflict already answered, never reaches the next conflict.
TEST_F(FileOperationsDaemonTest, StaleConflictAnswerIsRefused)
{
    file::TemporaryDirectory directory(paths::runtimeDirectory(), "stale");
    const std::string target = directory.path() + "/target";
    ASSERT_EQ(::mkdir(target.c_str(), 0700), 0);
    for (const std::string name : {"a", "b"}) {
        ASSERT_TRUE(file::writeAtomically(directory.path() + "/" + name, "new " + name));
        ASSERT_TRUE(file::writeAtomically(target + "/" + name, "old " + name));
    }

    const std::string path = submit(0, {url::fromLocalFile(directory.path() + "/a"),
                                        url::fromLocalFile(directory.path() + "/b")},
                                    url::fromLocalFile(target), 4);
    ASSERT_FALSE(path.empty());
    const auto waitingAt = [&](const std::string& name) {
        uint32_t conflict = 0;
        const bool waiting = waitFor([&] {
            const auto current = snapshot(path);
            if (!current || !flag(*current, "waitingForConflict")
                || current->at("conflictPath").value<std::string>() != target + "/" + name)
                return false;
            conflict = current->at("conflictId").value<uint32_t>();
            return true;
        }, 3000);
        return waiting ? conflict : 0;
    };

    const uint32_t first = waitingAt("a");
    ASSERT_NE(first, 0u);
    EXPECT_TRUE(task(path).call("ResolveConflict", first + 1, int32_t(3), false).isError());
    EXPECT_FALSE(task(path).call("ResolveConflict", first, int32_t(3), false).isError());

    const uint32_t second = waitingAt("b");
    ASSERT_NE(second, 0u);
    EXPECT_NE(second, first);
    EXPECT_TRUE(task(path).call("ResolveConflict", first, int32_t(3), false).isError());
    const auto current = snapshot(path);
    ASSERT_TRUE(current);
    EXPECT_TRUE(flag(*current, "waitingForConflict"));
    EXPECT_FALSE(flag(*current, "finished"));
    EXPECT_EQ(file::read(target + "/b").value_or(""), "old b");

    EXPECT_FALSE(task(path).call("ResolveConflict", second, int32_t(1), false).isError());
    ASSERT_TRUE(waitFor([&] {
        const auto finished = snapshot(path);
        return finished && flag(*finished, "finished");
    }, 3000));
    EXPECT_EQ(file::read(target + "/a").value_or(""), "new a");
    EXPECT_EQ(file::read(target + "/b").value_or(""), "old b");
}

// A wrong password asks again under a new id, an answer to the old id is
// refused, and the right one extracts every entry.
TEST_F(FileOperationsDaemonTest, EncryptedZipAsksForPassword)
{
    for (const std::string encryption : {"zipcrypt", "aes256"}) {
        SCOPED_TRACE(encryption);
        file::TemporaryDirectory directory(paths::runtimeDirectory(), "encrypted");
        const std::string archive = directory.path() + "/secret.zip";
        ASSERT_TRUE(writeArchive(archive, "zip", {{"secret/a.txt", "alpha"}, {"secret/b.txt", "beta"}},
                                 "zip:encryption=" + encryption, "right"));

        const std::string output = directory.path() + "/secret";
        const std::string path = submitArchive(1, {archive}, output);
        ASSERT_FALSE(path.empty());

        const uint32_t first = waitForPasswordRequest(path);
        ASSERT_NE(first, 0u);
        EXPECT_FALSE(flag(*snapshot(path), "passwordRetry"));
        EXPECT_TRUE(task(path).call("AnswerPassword", first + 1, std::string("right")).isError());
        EXPECT_FALSE(task(path).call("AnswerPassword", first, std::string("wrong")).isError());

        const uint32_t second = waitForPasswordRequest(path, first);
        ASSERT_NE(second, 0u);
        EXPECT_TRUE(flag(*snapshot(path), "passwordRetry"));
        EXPECT_TRUE(task(path).call("AnswerPassword", first, std::string("right")).isError());
        EXPECT_FALSE(task(path).call("AnswerPassword", second, std::string("right")).isError());

        // Every entry has the same password, so it is asked once.
        const VariantMap result = waitForFinish(path);
        EXPECT_TRUE(flag(result, "finished"));
        EXPECT_FALSE(flag(result, "waitingForPassword"));
        EXPECT_EQ(result.at("errorCode").value<int32_t>(), 0) << result.at("errorString").value<std::string>();
        EXPECT_EQ(file::read(output + "/secret/a.txt").value_or(""), "alpha");
        EXPECT_EQ(file::read(output + "/secret/b.txt").value_or(""), "beta");
    }
}

TEST_F(FileOperationsDaemonTest, CanceledPasswordRequestLeavesNothing)
{
    file::TemporaryDirectory directory(paths::runtimeDirectory(), "canceled");
    const std::string archive = directory.path() + "/secret.zip";
    ASSERT_TRUE(writeArchive(archive, "zip", {{"a.txt", "alpha"}}, "zip:encryption=aes256", "right"));

    const std::string path = submitArchive(1, {archive}, directory.path() + "/secret");
    ASSERT_FALSE(path.empty());
    ASSERT_NE(waitForPasswordRequest(path), 0u);
    EXPECT_FALSE(task(path).call("Cancel").isError());

    const VariantMap result = waitForFinish(path);
    EXPECT_TRUE(flag(result, "canceled"));
    EXPECT_EQ(entries(directory.path()), std::vector<std::string> {"secret.zip"});
}

TEST_F(FileOperationsDaemonTest, PasswordRequestOfVanishedSubmitterIsCanceled)
{
    file::TemporaryDirectory directory(paths::runtimeDirectory(), "vanished");
    const std::string archive = directory.path() + "/secret.zip";
    ASSERT_TRUE(writeArchive(archive, "zip", {{"a.txt", "alpha"}}, "zip:encryption=zipcrypt", "right"));

    std::string path;
    {
        auto submitter = dbus::Connection::open(dbus::Connection::Bus::Session, &loop);
        path = submitArchive(1, {archive}, directory.path() + "/secret", submitter.get());
        ASSERT_FALSE(path.empty());
        ASSERT_NE(waitForPasswordRequest(path), 0u);
    }

    const VariantMap result = waitForFinish(path, 3000);
    EXPECT_TRUE(flag(result, "canceled"));
    EXPECT_EQ(entries(directory.path()), std::vector<std::string> {"secret.zip"});
}

// Opening any part extracts the whole archive, from the first part on.
TEST_F(FileOperationsDaemonTest, SplitArchiveExtractsFromAnyPart)
{
    const std::string payload = noise(300 * 1024);
    for (const std::string format : {"7zip", "zip"}) {
        SCOPED_TRACE(format);
        file::TemporaryDirectory directory(paths::runtimeDirectory(), "split");
        const std::string archive = directory.path() + (format == "zip" ? "/data.zip" : "/data.7z");
        ASSERT_TRUE(writeArchive(archive, format, {{"data/noise.bin", payload}, {"data/note.txt", "note"}}));
        ASSERT_EQ(splitFile(archive, 128 * 1024), 3);

        const std::string output = directory.path() + "/data";
        const std::string path = submitArchive(1, {archive + ".002"}, output);
        ASSERT_FALSE(path.empty());
        const VariantMap result = waitForFinish(path);
        EXPECT_EQ(result.at("errorCode").value<int32_t>(), 0) << result.at("errorString").value<std::string>();
        EXPECT_TRUE(file::read(output + "/data/noise.bin").value_or("") == payload);
        EXPECT_EQ(file::read(output + "/data/note.txt").value_or(""), "note");
    }
}

TEST_F(FileOperationsDaemonTest, SplitArchiveWithoutFirstPartFails)
{
    file::TemporaryDirectory directory(paths::runtimeDirectory(), "partial");
    const std::string archive = directory.path() + "/data.7z";
    ASSERT_TRUE(writeArchive(archive, "7zip", {{"noise.bin", noise(300 * 1024)}}));
    ASSERT_EQ(splitFile(archive, 128 * 1024), 3);
    ASSERT_EQ(::unlink((archive + ".001").c_str()), 0);

    const std::string path = submitArchive(1, {archive + ".002"}, directory.path() + "/data");
    ASSERT_FALSE(path.empty());
    const VariantMap result = waitForFinish(path);
    EXPECT_NE(result.at("errorCode").value<int32_t>(), 0);
    EXPECT_EQ(result.at("errorString").value<std::string>(), "The first part of this multi-part archive is missing");
    EXPECT_EQ(entries(directory.path()), (std::vector<std::string> {"data.7z.002", "data.7z.003"}));
}
