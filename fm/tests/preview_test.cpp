// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testutil.h"

#include "preview/imageinfo.h"
#include "preview/previewcontroller.h"

#include "base/url.h"

#include <gtest/gtest.h>
#include <unicode/ucnv.h>

using fm::PreviewFile;
using fmtest::TemporaryDir;

static PreviewFile describe(const TemporaryDir& directory, const std::string& name, const std::string& contents)
{
    const std::string path = directory.filePath(name);
    fmtest::writeFile(path, contents);
    return fm::describeFile(Lingmo::base::url::fromLocalFile(path));
}

static std::string encode(const std::string& utf8, const char* encoding)
{
    UErrorCode status = U_ZERO_ERROR;
    std::string out(utf8.size() * 4 + 16, '\0');
    const int32_t length = ucnv_convert(encoding, "UTF-8", out.data(), int32_t(out.size()), utf8.data(),
                                        int32_t(utf8.size()), &status);
    out.resize(U_SUCCESS(status) ? size_t(length) : 0);
    return out;
}

TEST(PreviewTest, TextIsDecoded)
{
    TemporaryDir directory;
    const std::string sample = "\u7B2C\u4E00\u884C first\n\tsecond\n";
    std::string crlf;
    for (char c : sample)
        crlf += c == '\n' ? std::string("\r\n") : std::string(1, c);
    const struct
    {
        const char* name;
        std::string contents;
        std::string text;
    } cases[] = {
        {"notes.txt", sample, sample},
        {"notes.txt", crlf, sample},
        {"notes.txt", "\xff\xfe" + encode(sample, "UTF-16LE"), sample},
        {"main.cpp", "int main() {}\n", "int main() {}\n"},
        {"README", "Plain words.\n", "Plain words.\n"},
        {"notes.txt", encode("\u8FD9\u662F\u4E00\u4E2A\u7528 GBK \u7F16\u7801\u4FDD\u5B58\u7684\u4E2D\u6587\u6587\u672C\u6587\u4EF6\u3002\n", "GB18030"),
         "\u8FD9\u662F\u4E00\u4E2A\u7528 GBK \u7F16\u7801\u4FDD\u5B58\u7684\u4E2D\u6587\u6587\u672C\u6587\u4EF6\u3002\n"},
        {"notes.txt", encode("\u9019\u662F\u4E00\u500B\u7528\u7E41\u9AD4\u4E2D\u6587\u5BEB\u7684\u6587\u4EF6\uFF0C\u4F7F\u7528 Big5 \u7DE8\u78BC\u4FDD\u5B58\u3002\n", "Big5"),
         "\u9019\u662F\u4E00\u500B\u7528\u7E41\u9AD4\u4E2D\u6587\u5BEB\u7684\u6587\u4EF6\uFF0C\u4F7F\u7528 Big5 \u7DE8\u78BC\u4FDD\u5B58\u3002\n"},
        {"notes.txt", encode("\u3053\u308C\u306F Shift_JIS \u3067\u4FDD\u5B58\u3055\u308C\u305F\u65E5\u672C\u8A9E\u306E\u30C6\u30AD\u30B9\u30C8\u3067\u3059\u3002\n", "Shift_JIS"),
         "\u3053\u308C\u306F Shift_JIS \u3067\u4FDD\u5B58\u3055\u308C\u305F\u65E5\u672C\u8A9E\u306E\u30C6\u30AD\u30B9\u30C8\u3067\u3059\u3002\n"},
        {"notes.txt", encode("\uC774 \uD30C\uC77C\uC740 EUC-KR \uC778\uCF54\uB529\uC73C\uB85C \uC800\uC7A5\uB41C \uD55C\uAD6D\uC5B4 \uD14D\uC2A4\uD2B8\uC785\uB2C8\uB2E4.\n", "EUC-KR"),
         "\uC774 \uD30C\uC77C\uC740 EUC-KR \uC778\uCF54\uB529\uC73C\uB85C \uC800\uC7A5\uB41C \uD55C\uAD6D\uC5B4 \uD14D\uC2A4\uD2B8\uC785\uB2C8\uB2E4.\n"},
        {"notes.txt", encode("Привет, это текст в кодировке Windows-1251.\n", "windows-1251"),
         "Привет, это текст в кодировке Windows-1251.\n"},
        {"notes.txt", encode("Café, naïve façade, résumé.\n", "windows-1252"), "Café, naïve façade, résumé.\n"},
    };
    for (const auto& entry : cases) {
        const PreviewFile file = describe(directory, entry.name, entry.contents);
        EXPECT_EQ(file.kind, PreviewFile::Kind::Text) << entry.text;
        EXPECT_EQ(file.text, entry.text);
        EXPECT_FALSE(file.truncated);
    }
}

TEST(PreviewTest, BinaryContentIsNotText)
{
    TemporaryDir directory;
    const PreviewFile file = describe(directory, "data.txt", std::string("abc\0def\n", 8));
    EXPECT_EQ(file.kind, PreviewFile::Kind::Generic);
    EXPECT_TRUE(file.text.empty());
}

TEST(PreviewTest, LongTextEndsOnAWholeLine)
{
    TemporaryDir directory;
    const std::string line = std::string(99, 'x') + '\n';
    std::string contents;
    for (int i = 0; i < 4000; ++i)
        contents += line;
    const PreviewFile file = describe(directory, "long.txt", contents);
    EXPECT_EQ(file.kind, PreviewFile::Kind::Text);
    EXPECT_TRUE(file.truncated);
    EXPECT_LE(file.text.size(), 256u * 1024);
    EXPECT_GT(file.text.size(), 250u * 1024);
    EXPECT_TRUE(file.text.ends_with(line));
}

TEST(PreviewTest, FoldersCountTheirItems)
{
    TemporaryDir directory;
    fmtest::writeFile(directory.filePath("a"));
    fmtest::writeFile(directory.filePath(".hidden"));
    const PreviewFile file = fm::describeFile(Lingmo::base::url::fromLocalFile(directory.path()));
    EXPECT_TRUE(file.folder);
    EXPECT_EQ(file.itemCount, 2u);
    EXPECT_EQ(file.icon, "folder");
}

TEST(PreviewTest, MissingAndRemoteFilesSayWhy)
{
    TemporaryDir directory;
    EXPECT_FALSE(fm::describeFile(Lingmo::base::url::fromLocalFile(directory.filePath("gone"))).message.empty());
    EXPECT_FALSE(fm::describeFile("sftp://host/file").message.empty());
}

TEST(PreviewTest, ImageHeadersGiveSizeAndMotion)
{
    TemporaryDir directory;
    // A 3x2 PNG header; only the IHDR is read.
    const std::string png = std::string("\x89PNG\r\n\x1a\n", 8) + std::string("\0\0\0\x0dIHDR", 8)
            + std::string("\0\0\0\x03\0\0\0\x02\x08\x06\0\0\0", 13) + std::string(4, '\0')
            + std::string("\0\0\0\0IEND", 8);
    fmtest::writeFile(directory.filePath("a.png"), png);
    fm::ImageInfo info = fm::readImageInfo(directory.filePath("a.png"), "image/png");
    EXPECT_TRUE(info.readable);
    EXPECT_EQ(info.width, 3);
    EXPECT_EQ(info.height, 2);
    EXPECT_FALSE(info.animated);

    fmtest::writeFile(directory.filePath("a.svg"), "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 40 30\"/>");
    info = fm::readImageInfo(directory.filePath("a.svg"), "image/svg+xml");
    EXPECT_TRUE(info.vector);
    EXPECT_EQ(info.width, 40);
    EXPECT_EQ(info.height, 30);

    // Two image descriptors make a GIF move.
    std::string gif = std::string("GIF89a", 6) + std::string("\x05\0\x04\0\0\0\0", 7);
    for (int frame = 0; frame < 2; ++frame)
        gif += std::string("\x2c\0\0\0\0\x05\0\x04\0\0", 10) + std::string("\x02\x02\x44\x01\0", 5);
    gif += ";";
    fmtest::writeFile(directory.filePath("a.gif"), gif);
    info = fm::readImageInfo(directory.filePath("a.gif"), "image/gif");
    EXPECT_EQ(info.width, 5);
    EXPECT_EQ(info.height, 4);
    EXPECT_TRUE(info.animated);
}
