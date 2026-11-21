// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testutil.h"

#include "chooser/filechooser.h"
#include "core/preferences.h"

#include "base/json.h"
#include "base/url.h"
#include "ui/application.h"

#include <gtest/gtest.h>

#include <filesystem>

using namespace Lingmo;
using base::json::Value;
using fm::FileChooser;
using fmtest::runUntil;
using fmtest::TemporaryDir;
using fmtest::writeFile;

extern ui::Application* s_app;

static Value request(const std::string& json, const std::string& folder = {})
{
    Value value = *Value::parse(json);
    if (!folder.empty())
        value.insert("current_folder", folder);
    return value;
}

static int rowFor(FileChooser& chooser, const std::string& name)
{
    fm::FolderModel& folder = chooser.folder();
    for (int row = 0; row < folder.count(); ++row) {
        if (folder.item(row)->name == name)
            return row;
    }
    return -1;
}

static bool listed(FileChooser& chooser, int count)
{
    return chooser.folder().status() == fm::FolderModel::Status::Ready && chooser.folder().count() == count;
}

TEST(FileChooserTest, OpenMultipleAndFilter)
{
    TemporaryDir directory;
    writeFile(directory.filePath("first.txt"));
    writeFile(directory.filePath("second.txt"));
    writeFile(directory.filePath("photo.png"));
    std::filesystem::create_directory(directory.filePath("Documents"));
    FileChooser chooser(request(R"({"mode":"OpenFile", "multiple":true,
        "filters":[{"name":"Text", "rules":[{"type":0,"value":"*.txt"}]}]})",
                                directory.path()));
    ASSERT_TRUE(runUntil(s_app->loop(), [&] { return listed(chooser, 3); }));
    EXPECT_EQ(rowFor(chooser, "photo.png"), -1);
    EXPECT_GE(rowFor(chooser, "Documents"), 0);
    chooser.folder().setSelected(rowFor(chooser, "first.txt"));
    chooser.folder().toggleSelected(rowFor(chooser, "second.txt"));

    std::vector<Value> results;
    const base::Subscription subscription
            = chooser.finished.connect([&](const Value& result) { results.push_back(result); });
    chooser.accept();
    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0]["response"].toInt(), 0);
    EXPECT_EQ(results[0]["uris"].size(), 2u);
    chooser.cancel();
    EXPECT_EQ(results.size(), 1u);
}

TEST(FileChooserTest, NamesSortNumerically)
{
    TemporaryDir directory;
    for (const char* name : {"page10.txt", "page2.txt", "page1.txt"})
        writeFile(directory.filePath(name));
    FileChooser chooser(request(R"({"mode":"OpenFile"})", directory.path()));
    chooser.folder().setSortMode(fm::Preferences::Name);
    chooser.folder().setSortDesc(false);
    ASSERT_TRUE(runUntil(s_app->loop(), [&] { return listed(chooser, 3) && rowFor(chooser, "page1.txt") == 0; }));
    EXPECT_EQ(rowFor(chooser, "page2.txt"), 1);
    EXPECT_EQ(rowFor(chooser, "page10.txt"), 2);
}

TEST(FileChooserTest, MimeAndGlobAreAlternatives)
{
    TemporaryDir directory;
    writeFile(directory.filePath("document.txt"));
    writeFile(directory.filePath("photo.png"));
    writeFile(directory.filePath("archive.zip"));
    FileChooser chooser(request(R"({"mode":"OpenFile", "filters":[{"name":"Documents and images", "rules":[
        {"type":0,"value":"*.txt"},{"type":1,"value":"image/png"}]}]})",
                                directory.path()));
    ASSERT_TRUE(runUntil(s_app->loop(), [&] { return listed(chooser, 2); }));
    EXPECT_GE(rowFor(chooser, "document.txt"), 0);
    EXPECT_GE(rowFor(chooser, "photo.png"), 0);
}

TEST(FileChooserTest, GlobIgnoresCase)
{
    TemporaryDir directory;
    writeFile(directory.filePath("IMG_0001.JPG"));
    writeFile(directory.filePath("notes.txt"));
    FileChooser chooser(request(R"({"mode":"OpenFile", "filters":[{"name":"Images",
        "rules":[{"type":0,"value":"*.jpg"}]}]})",
                                directory.path()));
    ASSERT_TRUE(runUntil(s_app->loop(), [&] { return listed(chooser, 1); }));
    EXPECT_GE(rowFor(chooser, "IMG_0001.JPG"), 0);
}

TEST(FileChooserTest, SelectCurrentFolder)
{
    TemporaryDir directory;
    writeFile(directory.filePath("hidden-from-chooser.txt"));
    FileChooser chooser(request(R"({"mode":"OpenFile", "directory":true})", directory.path()));
    ASSERT_TRUE(runUntil(s_app->loop(), [&] { return listed(chooser, 0); }));
    std::vector<Value> results;
    const base::Subscription subscription
            = chooser.finished.connect([&](const Value& result) { results.push_back(result); });
    chooser.accept();
    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0]["uris"][0].toString(), base::url::fromLocalFile(directory.path()));
}

TEST(FileChooserTest, SaveRequiresOverwriteConfirmation)
{
    TemporaryDir directory;
    const std::string existing = directory.filePath("report.txt");
    writeFile(existing, "example\n");
    Value save = *Value::parse(R"({"mode":"SaveFile"})");
    save.insert("current_file", existing);
    FileChooser chooser(save);
    EXPECT_EQ(chooser.fileName(), "report.txt");
    EXPECT_EQ(chooser.directory(), directory.path());

    int confirmations = 0;
    std::vector<Value> results;
    const base::Subscription asked = chooser.confirmOverwrite.connect([&](const std::string&) { ++confirmations; });
    const base::Subscription done = chooser.finished.connect([&](const Value& result) { results.push_back(result); });
    chooser.accept(true);
    EXPECT_EQ(confirmations, 1);
    EXPECT_TRUE(results.empty());
    chooser.accept(true);
    EXPECT_EQ(results.size(), 1u);
    // The dialog never writes the file itself.
    std::ifstream file(existing);
    std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    EXPECT_EQ(contents, "example\n");
}

TEST(FileChooserTest, RejectInvalidNameAndMissingSelection)
{
    TemporaryDir directory;
    FileChooser save(request(R"({"mode":"SaveFile", "current_name":"../escape"})", directory.path()));
    int finished = 0;
    const base::Subscription subscription = save.finished.connect([&](const Value&) { ++finished; });
    save.accept();
    EXPECT_EQ(finished, 0);
    EXPECT_FALSE(save.error().empty());

    FileChooser open(request(R"({"mode":"OpenFile"})", directory.path()));
    ASSERT_TRUE(runUntil(s_app->loop(), [&] { return listed(open, 0); }));
    open.accept();
    EXPECT_FALSE(open.error().empty());
}

TEST(FileChooserTest, SaveFilesAvoidsCollisions)
{
    TemporaryDir directory;
    writeFile(directory.filePath("report.txt"));
    FileChooser chooser(request(R"({"mode":"SaveFiles", "files":["report.txt","report.txt"]})", directory.path()));
    std::vector<Value> results;
    const base::Subscription subscription
            = chooser.finished.connect([&](const Value& result) { results.push_back(result); });
    chooser.accept();
    ASSERT_EQ(results.size(), 1u);
    const Value& uris = results[0]["uris"];
    ASSERT_EQ(uris.size(), 2u);
    EXPECT_NE(uris[0].toString(), uris[1].toString());
    EXPECT_NE(uris[0].toString().find("report%20(1).txt"), std::string::npos);
    EXPECT_FALSE(std::filesystem::exists(directory.filePath("report (1).txt")));
}

TEST(FileChooserTest, InvalidRequestsAreRefused)
{
    EXPECT_FALSE(FileChooser::isValidRequest(*Value::parse(R"({"mode":"Delete"})")));
    EXPECT_TRUE(FileChooser::isValidRequest(*Value::parse(R"({"mode":"SaveFiles"})")));
}
