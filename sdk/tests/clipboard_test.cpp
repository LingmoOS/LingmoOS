// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <algorithm>

#include "ui/clipboard.h"

using namespace Lingmo::UI;

static bool contains(const std::vector<std::string>& types, const std::string& type)
{
    return std::find(types.begin(), types.end(), type) != types.end();
}

TEST(ClipboardTest, MimeDataKeepsTypesInOrder)
{
    MimeData data;
    data.setUrls({"file:///home/a%20b.txt", "file:///tmp/c"});
    data.setData("x-special/gnome-copied-files", "cut\nfile:///home/a%20b.txt\nfile:///tmp/c");
    data.setText("/home/a b.txt\n/tmp/c");
    EXPECT_EQ(data.formats(), (std::vector<std::string>{"text/uri-list", "x-special/gnome-copied-files",
                                                       "text/plain;charset=utf-8"}));
    EXPECT_EQ(*data.data("text/uri-list"), "file:///home/a%20b.txt\r\nfile:///tmp/c\r\n");
    EXPECT_EQ(data.urls(), (std::vector<std::string>{"file:///home/a%20b.txt", "file:///tmp/c"}));
    data.setText("replaced");
    EXPECT_EQ(data.formats().size(), 3u);
    EXPECT_EQ(data.text(), "replaced");
}

TEST(ClipboardTest, UriListSkipsCommentsAndBlankLines)
{
    MimeData data;
    data.setData("text/uri-list", "# a comment\r\nfile:///a\r\n\r\nfile:///b\n");
    EXPECT_EQ(data.urls(), (std::vector<std::string>{"file:///a", "file:///b"}));
}

TEST(ClipboardTest, TextUnderAnyTextType)
{
    MimeData data;
    data.setData("UTF8_STRING", "legacy");
    EXPECT_EQ(data.text(), "legacy");
    data.setData("text/plain", "plain");
    EXPECT_EQ(data.text(), "plain");
}

TEST(ClipboardTest, ClipboardAndSelectionAreSeparate)
{
    std::vector<ClipboardMode> changes;
    const auto subscription = Clipboard::changed().connect([&](ClipboardMode mode) { changes.push_back(mode); });
    Clipboard::setText("copied");
    Clipboard::setText("selected", ClipboardMode::Selection);
    EXPECT_EQ(changes, (std::vector<ClipboardMode>{ClipboardMode::Clipboard, ClipboardMode::Selection}));
    EXPECT_TRUE(Clipboard::supportsSelection());

    std::string clipboard;
    std::string selection;
    Clipboard::requestText([&](std::string text) { clipboard = std::move(text); });
    Clipboard::requestText([&](std::string text) { selection = std::move(text); }, ClipboardMode::Selection);
    EXPECT_EQ(clipboard, "copied");
    EXPECT_EQ(selection, "selected");
    EXPECT_TRUE(Clipboard::hasText());
    EXPECT_TRUE(Clipboard::hasText(ClipboardMode::Selection));
    EXPECT_TRUE(Clipboard::ownsData(ClipboardMode::Selection));

    Clipboard::clear(ClipboardMode::Selection);
    EXPECT_FALSE(Clipboard::hasText(ClipboardMode::Selection));
    EXPECT_TRUE(Clipboard::mimeTypes(ClipboardMode::Selection).empty());
    EXPECT_TRUE(Clipboard::hasText());
}

TEST(ClipboardTest, SeveralTypesAndTextAliases)
{
    MimeData data;
    data.setUrls({"file:///tmp/c"});
    data.setData("x-special/gnome-copied-files", "copy\nfile:///tmp/c");
    data.setText("/tmp/c");
    Clipboard::setMimeData(data);
    const std::vector<std::string> types = Clipboard::mimeTypes();
    EXPECT_TRUE(contains(types, "text/uri-list"));
    EXPECT_TRUE(contains(types, "x-special/gnome-copied-files"));
    EXPECT_TRUE(contains(types, "text/plain"));
    EXPECT_TRUE(contains(types, "UTF8_STRING"));

    std::optional<std::string> copied;
    Clipboard::requestData("x-special/gnome-copied-files", [&](std::optional<std::string> bytes) { copied = bytes; });
    EXPECT_EQ(copied, "copy\nfile:///tmp/c");
    std::optional<std::string> plain;
    Clipboard::requestData("text/plain", [&](std::optional<std::string> bytes) { plain = bytes; });
    EXPECT_EQ(plain, "/tmp/c");
    bool missingAnswered = false;
    Clipboard::requestData("image/png", [&](std::optional<std::string> bytes) {
        missingAnswered = true;
        EXPECT_FALSE(bytes.has_value());
    });
    EXPECT_TRUE(missingAnswered);
}
