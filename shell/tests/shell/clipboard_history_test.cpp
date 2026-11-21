// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <filesystem>

#include <gtest/gtest.h>
#include <sys/stat.h>
#include <unistd.h>

#include "base/file.h"
#include "base/json.h"
#include "base/paths.h"
#include "clipboard/selection.h"
#include "clipboard/stored_source.h"
#include "loop_test_util.h"
#include "shell/clipboard/clipboard_history.h"
#include "shell/common/file_thread.h"
#include "shell_test_util.h"
#include "temporary_directory.h"

using namespace Lingmo;
using shell::ClipboardEntry;
using shell::ClipboardPayload;
using shell::ClipboardStore;

namespace {

// An index of one file entry with key "abc", and its body: a text and an
// image of three bytes.
constexpr const char *kIndex = R"([{"key":"abc","kind":2,"copied":1760000000123,"text":"/a\n/b",)"
                               R"("files":["/a","/b"],"imageType":""}])";
const std::string kBody = std::string("text/plain\n5\nhello") + "image/png\n3\n" + std::string("\0\1\xff", 3);

std::shared_ptr<clipboard::StoredSource> sourceWith(const std::vector<std::pair<std::string, std::string>> &formats)
{
    auto source = std::make_shared<clipboard::StoredSource>();
    for (const auto &[type, data] : formats) {
        source->offer(type);
        source->payloads[type] = data;
    }
    return source;
}

// Reads one type of the selection as a client would.
std::string receive(clipboard::DataSource &source, const std::string &type)
{
    int fds[2];
    EXPECT_EQ(pipe(fds), 0);
    source.send(type, fds[1]);
    std::string data;
    char buffer[4096];
    test::runUntil([&] {
        const ssize_t count = read(fds[0], buffer, sizeof(buffer));
        if (count > 0) {
            data.append(buffer, size_t(count));
            return false;
        }
        return count == 0;
    });
    close(fds[0]);
    return data;
}

} // namespace

TEST(ClipboardStoreTest, KeepsTextFilesAndImagesButNotPasswords)
{
    EXPECT_TRUE(ClipboardStore::formatsToKeep({"text/plain", "x-kde-passwordManagerHint"}).empty());
    EXPECT_EQ(ClipboardStore::formatsToKeep({"text/html", "UTF8_STRING", "text/plain", "text/uri-list", "image/png"}),
              (std::vector<std::string>{"text/uri-list", "text/plain"}));
    EXPECT_EQ(ClipboardStore::formatsToKeep({"image/jpeg", "image/png"}), (std::vector<std::string>{"image/png"}));
    EXPECT_EQ(ClipboardStore::formatsToKeep({"image/webp"}), (std::vector<std::string>{"image/webp"}));

    ClipboardEntry entry;
    ASSERT_TRUE(ClipboardStore::describe({{"text/uri-list", "# comment\r\nfile:///tmp/a%20b\r\nhttp://x\r\n"}}, &entry));
    EXPECT_EQ(entry.kind, ClipboardEntry::Kind::Files);
    EXPECT_EQ(entry.files, (std::vector<std::string>{"/tmp/a b"}));
    ASSERT_TRUE(ClipboardStore::describe({{"STRING", "caf\xe9"}}, &entry));
    EXPECT_EQ(entry.kind, ClipboardEntry::Kind::Text);
    EXPECT_EQ(entry.text, "caf\xc3\xa9");
    EXPECT_FALSE(ClipboardStore::describe({{"text/plain", "   "}}, &entry));
    ASSERT_TRUE(ClipboardStore::describe({{"image/png", "PNG"}}, &entry));
    EXPECT_EQ(entry.imageType, "image/png");
    EXPECT_EQ(entry.key.size(), 64u);
    EXPECT_NE(ClipboardStore::keyFor({{"text/plain", "a"}}), ClipboardStore::keyFor({{"text/plain", "b"}}));
}

TEST(ClipboardStoreTest, ReadsAndWritesItsHistory)
{
    test::TemporaryDirectory directory;
    directory.write("history/index.json", kIndex);
    directory.write("history/abc", kBody);
    // A body no entry names goes; what this store never writes, such as an
    // older history's index, is left alone.
    const std::string orphan(64, 'e');
    directory.write("history/" + orphan, kBody);
    directory.write("history/index", "left over");

    ClipboardStore store(directory.filePath("history"), 2);
    ASSERT_TRUE(test::runUntil([&] { return store.isLoaded(); }));
    ASSERT_EQ(store.entries().size(), 1u);
    EXPECT_EQ(store.entries()[0].key, "abc");
    EXPECT_EQ(store.entries()[0].kind, ClipboardEntry::Kind::Files);
    EXPECT_EQ(store.entries()[0].files, (std::vector<std::string>{"/a", "/b"}));
    EXPECT_EQ(store.entries()[0].copied, 1760000000123LL);
    EXPECT_FALSE(std::filesystem::exists(directory.filePath("history/" + orphan)));
    EXPECT_TRUE(std::filesystem::exists(directory.filePath("history/index")));
    // Its own place, not the older ~/.config/lingmo/clipboard-history.
    EXPECT_EQ(ClipboardStore::defaultDirectory(), base::paths::dataHome() + "/lingmo/clipboard-history");

    std::optional<ClipboardPayload> body;
    store.payload("abc", [&](ClipboardPayload payload) { body = payload; });
    ASSERT_TRUE(test::runUntil([&] { return body.has_value(); }));
    ASSERT_EQ(body->size(), 2u);
    EXPECT_EQ((*body)[0], std::make_pair(std::string("text/plain"), std::string("hello")));
    EXPECT_EQ((*body)[1], std::make_pair(std::string("image/png"), std::string("\0\1\xff", 3)));

    ASSERT_TRUE(store.add({{"text/plain", "one"}}));
    ASSERT_TRUE(test::runUntil([&] { return store.entries().size() == 2; }));
    EXPECT_EQ(store.entries()[0].text, "one");
    const std::string key = store.entries()[0].key;
    struct stat info = {};
    ASSERT_EQ(stat(store.bodyPath(key).c_str(), &info), 0);
    EXPECT_EQ(info.st_mode & 0777, 0600u);
    ASSERT_EQ(stat(directory.filePath("history").c_str(), &info), 0);
    EXPECT_EQ(info.st_mode & 0777, 0700u);

    // The same copy again moves its entry up; past the limit the oldest goes.
    store.add({{"text/plain", "two"}});
    ASSERT_TRUE(test::runUntil([&] { return store.entries()[0].text == "two"; }));
    EXPECT_EQ(store.entries().size(), 2u);
    EXPECT_EQ(store.entries()[1].text, "one");
    store.add({{"text/plain", "one"}});
    EXPECT_EQ(store.entries()[0].text, "one");
    shell::waitForFileThread();
    EXPECT_FALSE(std::filesystem::exists(directory.filePath("history/abc")));

    // The index on disk, newest first; the body in the same format it was read.
    const auto index = base::json::Value::parse(
            base::file::read(directory.filePath("history/index.json")).value_or(""));
    ASSERT_TRUE(index && index->isArray());
    ASSERT_EQ(index->size(), 2u);
    EXPECT_EQ((*index)[0]["key"].toString(), key);
    EXPECT_EQ((*index)[0]["kind"].toInt(), 0);
    EXPECT_TRUE((*index)[0]["copied"].isNumber());
    EXPECT_EQ((*index)[0]["text"].toString(), "one");
    EXPECT_EQ(base::file::read(store.bodyPath(key)).value_or(""), "text/plain\n3\none");
    EXPECT_EQ(ClipboardStore::readBody(store.bodyPath(key)),
              (ClipboardPayload{{"text/plain", "one"}}));
    // A damaged body reads as nothing.
    directory.write("damaged", "text/plain\n9\nshort");
    EXPECT_TRUE(ClipboardStore::readBody(directory.filePath("damaged")).empty());

    store.remove(key);
    EXPECT_EQ(store.entries().size(), 1u);
    store.clear();
    EXPECT_TRUE(store.entries().empty());
    shell::waitForFileThread();
    EXPECT_FALSE(std::filesystem::exists(store.bodyPath(key)));
}

TEST(ClipboardMonitorTest, ReadsNewSelectionsAndKnowsItsOwn)
{
    clipboard::SelectionController selection;
    shell::ClipboardMonitor monitor(selection);
    std::vector<ClipboardPayload> copies;
    auto copied = monitor.copied.connect([&](const ClipboardPayload &payload) { copies.push_back(payload); });

    selection.setSource(sourceWith({{"text/html", "<b>x</b>"}, {"text/plain", "x"}}));
    ASSERT_TRUE(test::runUntil([&] { return copies.size() == 1; }));
    EXPECT_EQ(copies[0], (ClipboardPayload{{"text/plain", "x"}}));

    // A password manager's copy is not read.
    selection.setSource(sourceWith({{"text/plain", "secret"}, {"x-kde-passwordManagerHint", "secret"}}));
    test::runFor(std::chrono::milliseconds(20));
    EXPECT_EQ(copies.size(), 1u);

    // An entry put back on the clipboard offers every text alias and says
    // which entry it is.
    std::vector<std::string> shown;
    auto own = monitor.ownSelectionShown.connect([&](const std::string &key) { shown.push_back(key); });
    ASSERT_TRUE(monitor.setSelection({{"STRING", "caf\xe9"}}, "k1"));
    EXPECT_EQ(shown, (std::vector<std::string>{"k1"}));
    const auto source = selection.source();
    ASSERT_TRUE(source);
    EXPECT_EQ(receive(*source, "UTF8_STRING"), "caf\xc3\xa9");
    EXPECT_EQ(receive(*source, shell::ClipboardMonitor::kEntryType), "k1");
    EXPECT_EQ(receive(*source, shell::ClipboardMonitor::kServicesType), "1");
    EXPECT_EQ(copies.size(), 1u);
}

TEST(ClipboardHistoryTest, PastesAnEntryAfterReadingItsBody)
{
    test::TemporaryDirectory directory;
    clipboard::SelectionController selection;
    shell::ClipboardHistory history(nullptr, selection, directory.filePath("history"));
    ASSERT_TRUE(test::runUntil([&] { return history.store().isLoaded(); }));

    selection.setSource(sourceWith({{"text/plain", "first"}}));
    ASSERT_TRUE(test::runUntil([&] { return history.model().count() == 1; }));
    selection.setSource(sourceWith({{"text/uri-list", "file:///tmp/notes.txt\nfile:///tmp/b.png\n"}}));
    ASSERT_TRUE(test::runUntil([&] { return history.model().count() == 2; }));
    EXPECT_EQ(history.model().title(0), "2 Files");
    EXPECT_EQ(history.model().title(1), "first");
    EXPECT_EQ(shell::ClipboardHistoryModel::kind(history.model().entry(0)), "files");

    history.model().setFilterText("NOTES");
    EXPECT_EQ(history.model().count(), 1);
    history.model().setFilterText("");

    std::vector<std::string> calls;
    history.closePicker = [&] { calls.push_back("close"); };
    history.sendPaste = [&] { calls.push_back("paste"); };
    history.paste(1);
    ASSERT_TRUE(test::runUntil([&] { return calls.size() == 2; }));
    EXPECT_EQ(calls, (std::vector<std::string>{"close", "paste"}));
    EXPECT_EQ(history.model().title(0), "first");
    const auto source = selection.source();
    ASSERT_TRUE(source);
    EXPECT_EQ(receive(*source, "text/plain"), "first");
}
