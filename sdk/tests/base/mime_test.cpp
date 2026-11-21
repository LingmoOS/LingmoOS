// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <unistd.h>

#include "base/file.h"
#include "base/mime_apps.h"
#include "base/mime_database.h"
#include "base/paths.h"

using namespace Lingmo::Base;
namespace fs = std::filesystem;

namespace {

class MimeAppsTest : public testing::Test
{
protected:
    void SetUp() override
    {
        m_directory = fs::temp_directory_path() / ("lingmo-mimeapps-" + std::to_string(getpid()));
        fs::create_directories(m_directory);
    }
    void TearDown() override { fs::remove_all(m_directory); }

    std::string path(const std::string& name) const { return (m_directory / name).string(); }

    fs::path m_directory;
};

} // namespace

TEST(MimeDatabaseTest, ResolvesAliasesAndParents)
{
    // shared-mime-info has these on every system the desktop runs on.
    EXPECT_EQ(mime::canonicalName("application/x-pdf"), "application/pdf");
    EXPECT_EQ(mime::canonicalName("application/pdf"), "application/pdf");
    EXPECT_EQ(mime::canonicalName("no/such-type"), "");
    const std::vector<std::string> aliases = mime::aliases("application/pdf");
    EXPECT_NE(std::find(aliases.begin(), aliases.end(), "application/x-pdf"), aliases.end());
    EXPECT_EQ(mime::parents("text/x-csrc"), std::vector<std::string>{"text/plain"});
    // Without parents of its own: application/octet-stream, the fallback.
    EXPECT_EQ(mime::parents("image/png"), std::vector<std::string>{"application/octet-stream"});
    EXPECT_TRUE(mime::parents("inode/directory").empty());
    EXPECT_TRUE(mime::parents("application/octet-stream").empty());
}

TEST_F(MimeAppsTest, ListsLiveInTheConfigHome)
{
    EXPECT_EQ(mimeapps::userListPath(), paths::configHome() + "/mimeapps.list");
    const std::vector<std::string> lists = mimeapps::listPaths();
    ASSERT_FALSE(lists.empty());
    EXPECT_TRUE(lists.front().starts_with(paths::configHome()));
    EXPECT_TRUE(lists.front().ends_with("-mimeapps.list"));
}

TEST_F(MimeAppsTest, SetsDefaultAndUpdatesAssociations)
{
    const std::string list = path("mimeapps.list");
    ASSERT_TRUE(file::writeAtomically(list, "# kept\n[Default Applications]\ntext/plain=old.desktop;\n"
                                            "[Added Associations]\ntext/plain=old.desktop;new.desktop;\n"
                                            "[Removed Associations]\ntext/plain=new.desktop;other.desktop;\n"));
    ASSERT_TRUE(mimeapps::setDefault(list, "text/plain", "new.desktop"));
    // What MimeApps::setDefault in lingmo-framework writes, byte for byte: the
    // file's last line break makes an empty line, which the group keeps.
    EXPECT_EQ(file::read(list).value_or(""),
              "# kept\n[Default Applications]\ntext/plain=new.desktop;\n"
              "[Added Associations]\ntext/plain=new.desktop;old.desktop;\n"
              "[Removed Associations]\n\ntext/plain=other.desktop;\n");
    EXPECT_EQ(mimeapps::associations(list, "Added Associations", "text/plain"),
              (std::vector<std::string>{"new.desktop", "old.desktop"}));
    EXPECT_FALSE(mimeapps::setDefault(list, "text/plain", "bad;id"));
    EXPECT_FALSE(fs::exists(list + ".lock"));
}

TEST_F(MimeAppsTest, KeepsTheTerminalList)
{
    const std::string list = path("xdg-terminals.list");
    ASSERT_TRUE(file::writeAtomically(list, "# preferred\n-excluded.desktop\nfirst.desktop:new-window\nsecond.desktop\n"));
    EXPECT_EQ(mimeapps::defaultTerminal(list), "first.desktop");
    ASSERT_TRUE(mimeapps::setDefaultTerminal(list, "second.desktop"));
    EXPECT_EQ(file::read(list).value_or(""),
              "second.desktop\n# preferred\n-excluded.desktop\nfirst.desktop:new-window\n");
    EXPECT_EQ(mimeapps::defaultTerminal(list), "second.desktop");
}

// Reference types measured on the laptop, for these files.
TEST(MimeTest, DetectsTypesOfFiles)
{
    const fs::path directory = fs::temp_directory_path() / ("lingmo-mime-detect-" + std::to_string(getpid()));
    fs::create_directories(directory / "folder");
    const auto write = [&directory](const std::string& name, std::string_view contents) {
        ASSERT_TRUE(file::writeAtomically((directory / name).string(), contents));
    };
    write("notes.txt", "hello\nworld\n");
    write("readme_noext", "plain words\n");
    write("blob", std::string_view("\0\1\2\3binary", 10));
    write("picture", std::string_view("\x89PNG\r\n\x1a\n\0\0\0\rIHDR", 16));
    write("picture.txt", std::string_view("\x89PNG\r\n\x1a\n\0\0\0\rIHDR", 16));
    write("empty", "");
    write("code.C", "int main() {}\n");
    write("code.c", "int main() {}\n");
    write("lang.ts", "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<!DOCTYPE TS>\n<TS version=\"2.1\">\n</TS>\n");
    write("video.ts", std::string_view("G@\0\x10\0\0", 6));
    write("app.desktop", "[Desktop Entry]\nType=Application\n");
    write("doc", "%PDF-1.4\n");
    write("script", "#!/bin/sh\necho hi\n");
    write("Makefile", "all:\n");

    struct Expected
    {
        const char* name;
        const char* type;
        const char* icon;
        const char* genericIcon;
    };
    const Expected expected[] = {
        {"Makefile", "text/x-makefile", "text-x-makefile", "text-x-generic"},
        {"app.desktop", "application/x-desktop", "application-x-desktop", "text-x-generic"},
        {"blob", "application/octet-stream", "application-octet-stream", "application-x-generic"},
        {"code.C", "text/x-c++src", "text-x-c++src", "text-x-generic"},
        {"code.c", "text/x-csrc", "text-x-csrc", "text-x-generic"},
        {"doc", "application/pdf", "application-pdf", "x-office-document"},
        {"empty", "application/x-zerosize", "application-x-zerosize", "application-x-generic"},
        {"folder", "inode/directory", "inode-directory", "folder"},
        {"lang.ts", "text/vnd.trolltech.linguist", "text-vnd.trolltech.linguist", "text-x-generic"},
        {"notes.txt", "text/plain", "text-plain", "text-x-generic"},
        {"picture", "image/png", "image-png", "image-x-generic"},
        {"picture.txt", "text/plain", "text-plain", "text-x-generic"},
        {"readme_noext", "text/x-readme", "text-x-readme", "text-x-generic"},
        {"script", "application/x-shellscript", "application-x-shellscript", "text-x-script"},
        {"video.ts", "text/vnd.trolltech.linguist", "text-vnd.trolltech.linguist", "text-x-generic"},
    };
    for (const Expected& entry : expected) {
        const std::string type = mime::typeForFile((directory / entry.name).string());
        EXPECT_EQ(type, entry.type) << entry.name;
        EXPECT_EQ(mime::iconName(type), entry.icon) << entry.name;
        EXPECT_EQ(mime::genericIconName(type), entry.genericIcon) << entry.name;
    }
    fs::remove_all(directory);

    EXPECT_EQ(mime::typeForFileName("a.tar.gz"), "application/x-compressed-tar");
    EXPECT_EQ(mime::typeForFileName("Makefile"), "text/x-makefile");
    EXPECT_EQ(mime::typeForFileName("x.C"), "text/x-c++src");
    EXPECT_EQ(mime::typeForFileName("x.c"), "text/x-csrc");
    EXPECT_EQ(mime::typeForFileName("X.PNG"), "image/png");
    EXPECT_EQ(mime::typeForFileName("foo"), "application/octet-stream");
    EXPECT_EQ(mime::typeForFileName("a.ts"), "text/vnd.trolltech.linguist");
    EXPECT_TRUE(mime::inherits("text/x-csrc", "text/plain"));
    EXPECT_FALSE(mime::inherits("text/plain", "text/x-csrc"));
}
