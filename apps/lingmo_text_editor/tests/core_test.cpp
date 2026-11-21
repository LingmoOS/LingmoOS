// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "document.h"
#include "fileoptions.h"
#include "recentfiles.h"
#include "searcher.h"
#include "session.h"
#include "syntax.h"
#include "textbuffer.h"
#include "textcodec.h"
#include "undostack.h"

#include "base/event_loop.h"
#include "base/file.h"
#include "base/settings.h"
#include "test_support.h"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <map>
#include <sys/stat.h>
#include <unistd.h>

namespace fs = std::filesystem;

static std::ostream &operator<<(std::ostream &out, const TextPosition &p)
{
    return out << "{" << p.line << ", " << p.column << "}";
}

static std::ostream &operator<<(std::ostream &out, const TextRange &r)
{
    return out << "{" << r.start << ", " << r.end << "}";
}

class CoreTest : public ::testing::Test
{
protected:
    std::string path(const std::string &name) const { return TestSupport::directory() + "/" + name; }

    std::string writeFile(const std::string &name, std::string_view data) const
    {
        const std::string file = path(name);
        std::error_code error;
        fs::create_directories(fs::path(file).parent_path(), error);
        std::string temporary = file + ".tmp";
        if (!Lingmo::base::file::writeAtomically(temporary, data))
            return {};
        fs::rename(temporary, file, error);
        return file;
    }

    static std::string readFile(const std::string &file)
    {
        return Lingmo::base::file::read(file).value_or(std::string("<missing>"));
    }

    static std::string encode(std::string_view text, const char *encoding)
    {
        return TextCodec::encode({std::string(text)}, encoding, false, LineEnding::Unix);
    }
};

static TextPosition at(int line, int column)
{
    return {line, column};
}

TEST_F(CoreTest, BufferEdits)
{
    TextBuffer buffer;
    EXPECT_EQ(buffer.lineCount(), 1);
    EXPECT_EQ(buffer.insert({0, 0}, "hello\nworld"), at(1, 5));
    EXPECT_EQ(buffer.lineCount(), 2);
    EXPECT_EQ(buffer.length(), 11);
    buffer.insert({0, 5}, ",\nbig");
    EXPECT_EQ(buffer.text(), "hello,\nbig\nworld");
    buffer.remove({{0, 5}, {2, 0}});
    EXPECT_EQ(buffer.text(), "helloworld");
    EXPECT_EQ(buffer.length(), 10);
    buffer.setText("a\nb\n");
    EXPECT_EQ(buffer.lineCount(), 3);
    EXPECT_EQ(buffer.length(), 4);
}

TEST_F(CoreTest, BufferRanges)
{
    TextBuffer buffer;
    buffer.setText("one\ntwo\nthree");
    EXPECT_EQ(buffer.text({{0, 1}, {2, 2}}), "ne\ntwo\nth");
    EXPECT_EQ(buffer.offset({2, 1}), 9);
    EXPECT_EQ(buffer.position(9), at(2, 1));
    EXPECT_EQ(buffer.clamp({5, 9}), at(2, 5));
    EXPECT_EQ(TextBuffer::endOf({1, 2}, "x\nyz"), at(2, 2));

    // Positions are bytes and never land inside a character.
    buffer.setText("a中b");
    EXPECT_EQ(buffer.line(0).size(), 5u);
    EXPECT_EQ(buffer.clamp({0, 2}), at(0, 1));
    EXPECT_EQ(buffer.clamp({0, 4}), at(0, 4));
}

struct DetectionCase
{
    const char *name;
    std::string data;
    const char *encoding;
    bool bom;
};

TEST_F(CoreTest, EncodingDetection)
{
    const std::string chinese = "这是一个简单的中文文本文件，用来测试编码检测。\n第二行也是中文。\n";
    const std::string japanese = "これは日本語のテキストです。エンコーディングを確認します。\n";
    const std::string western = "Café crème, naïve façade, résumé.\nÜber straße.\n";
    const std::vector<DetectionCase> cases{
        {"ascii", "plain text\n", "UTF-8", false},
        {"utf-8", chinese, "UTF-8", false},
        {"utf-8 bom", "\xEF\xBB\xBFhi", "UTF-8", true},
        {"utf-16le bom", std::string("\xFF\xFEh\0i\0", 6), "UTF-16LE", true},
        {"utf-16be bom", std::string("\xFE\xFF\0h\0i", 6), "UTF-16BE", true},
        {"utf-16le", encode("hello world\nmore text\n", "UTF-16LE"), "UTF-16LE", false},
        {"gb18030", encode(chinese, "GB18030"), "GB18030", false},
        {"big5", encode("這是一個繁體中文的文字檔案，用來測試。\n", "Big5"), "Big5", false},
        {"shift_jis", encode(japanese, "Shift_JIS"), "Shift_JIS", false},
        {"latin-1", encode(western, "ISO-8859-1"), "windows-1252", false},
    };
    for (const DetectionCase &c : cases) {
        SCOPED_TRACE(c.name);
        bool detectedBom = !c.bom;
        EXPECT_EQ(TextCodec::detect(c.data, &detectedBom), c.encoding);
        EXPECT_EQ(detectedBom, c.bom);
        const DecodedText decoded = TextCodec::decode(c.data);
        EXPECT_FALSE(decoded.malformed);
        EXPECT_FALSE(decoded.binary);
    }
    EXPECT_EQ(TextCodec::decode(encode(chinese, "GB18030")).lines[0], "这是一个简单的中文文本文件，用来测试编码检测。");

    // Longer than the sample, which ends inside a character.
    std::string large;
    while (large.size() < 300 * 1024)
        large += "\xE2\x82\xAC";
    EXPECT_EQ(TextCodec::detect(large), "UTF-8");
}

TEST_F(CoreTest, LocaleEncoding)
{
    EXPECT_EQ(TextCodec::localeEncoding("zh_CN"), "GB18030");
    EXPECT_EQ(TextCodec::localeEncoding("zh_TW"), "Big5");
    EXPECT_EQ(TextCodec::localeEncoding("ja_JP"), "Shift_JIS");
    EXPECT_EQ(TextCodec::localeEncoding("ru_RU"), "windows-1251");
    EXPECT_EQ(TextCodec::localeEncoding("en_US"), "windows-1252");
}

TEST_F(CoreTest, BinaryDetection)
{
    EXPECT_TRUE(TextCodec::isBinary(std::string("ELF\0\1\2", 6)));
    EXPECT_FALSE(TextCodec::isBinary("text\twith tabs\r\n"));
    EXPECT_TRUE(TextCodec::decode(std::string("abc\0def", 7)).binary);
    // UTF-16 is full of NUL bytes and is still text.
    EXPECT_FALSE(TextCodec::decode(std::string("\xFF\xFEh\0i\0", 6)).binary);

    const DecodedText broken = TextCodec::decode("ok \xC3 broken", "UTF-8");
    EXPECT_TRUE(broken.malformed);
    EXPECT_EQ(broken.lines[0], "ok \xEF\xBF\xBD broken");
}

TEST_F(CoreTest, LineEndings)
{
    struct Case
    {
        const char *text;
        LineEnding ending;
        bool mixed;
        size_t lines;
    };
    const std::vector<Case> cases{
        {"a\nb\n", LineEnding::Unix, false, 3},
        {"a\r\nb\r\n", LineEnding::Windows, false, 3},
        {"a\rb", LineEnding::ClassicMac, false, 2},
        {"a\r\nb\r\nc\nd", LineEnding::Windows, true, 4},
        {"single", LineEnding::Unix, false, 1},
    };
    for (const Case &c : cases) {
        SCOPED_TRACE(c.text);
        LineEnding dominant;
        bool isMixed;
        const std::vector<std::string> result = TextCodec::splitLines(c.text, &dominant, &isMixed);
        EXPECT_EQ(result.size(), c.lines);
        EXPECT_EQ(dominant, c.ending);
        EXPECT_EQ(isMixed, c.mixed);
        for (const std::string &line : result)
            EXPECT_EQ(line.find('\r'), std::string::npos);
    }
}

TEST_F(CoreTest, EncodeRoundTrip)
{
    const std::vector<std::string> lines{"第一行", "second", ""};
    for (const char *name : {"UTF-8", "UTF-16LE", "GB18030", "Big5"}) {
        SCOPED_TRACE(name);
        bool lossy = true;
        const std::string bytes = TextCodec::encode(lines, name, false, LineEnding::Windows, &lossy);
        EXPECT_FALSE(lossy);
        const DecodedText decoded = TextCodec::decode(bytes, name);
        EXPECT_EQ(decoded.lines, lines);
        EXPECT_EQ(decoded.lineEnding, LineEnding::Windows);
    }
    EXPECT_EQ(TextCodec::encode({"x"}, "UTF-8", true, LineEnding::Unix), "\xEF\xBB\xBFx");
    EXPECT_EQ(TextCodec::encode({"x"}, "UTF-16LE", true, LineEnding::Unix), std::string("\xFF\xFEx\0", 4));
}

TEST_F(CoreTest, LossyEncoding)
{
    bool lossy = false;
    TextCodec::encode({"中文"}, "ISO-8859-1", false, LineEnding::Unix, &lossy);
    EXPECT_TRUE(lossy);
    TextCodec::encode({"abc"}, "ISO-8859-1", false, LineEnding::Unix, &lossy);
    EXPECT_FALSE(lossy);
}

TEST_F(CoreTest, UndoGroupsWords)
{
    Document document;
    for (const char c : std::string("hello world"))
        document.insertText(std::string(1, c));
    EXPECT_EQ(document.text(), "hello world");
    document.undo();
    EXPECT_EQ(document.text(), "hello ");
    document.undo();
    EXPECT_EQ(document.text(), "");
    document.redo();
    document.redo();
    EXPECT_EQ(document.text(), "hello world");

    // Backspaces group by the word too.
    for (int i = 0; i < 6; ++i)
        document.deleteBackward(false);
    EXPECT_EQ(document.text(), "hello");
    document.undo();
    EXPECT_EQ(document.text(), "hello ");
}

TEST_F(CoreTest, UndoCleanState)
{
    const std::string file = writeFile("clean.txt", "abc\n");
    Document document;
    document.load(file);
    ASSERT_TRUE(document.waitForLoaded());
    EXPECT_FALSE(document.isModified());
    document.setCursor({0, 3});
    document.insertText("d");
    EXPECT_TRUE(document.isModified());
    document.undo();
    EXPECT_FALSE(document.isModified());
    document.redo();
    EXPECT_TRUE(document.save());
    EXPECT_FALSE(document.isModified());
    // Typing right after a save starts a new group, so undo reaches the saved text.
    document.insertText("e");
    document.undo();
    EXPECT_FALSE(document.isModified());
    EXPECT_EQ(document.text(), "abcd\n");
    document.undo();
    EXPECT_TRUE(document.isModified());
    // A new edit after undoing past the save point makes it unreachable.
    document.insertText("x");
    document.undo();
    EXPECT_TRUE(document.isModified());
}

TEST_F(CoreTest, UndoKeepsRedoAcrossEmptyGroups)
{
    Document document;
    document.setText("abc");
    document.setCursor({0, 3});
    document.insertText("d");
    document.undo();
    // Nothing to unindent and nothing to replace.
    document.indentSelection(true);
    document.setSearch("x", false, false, false, false);
    EXPECT_EQ(document.replaceAll("y"), 0);
    EXPECT_TRUE(document.canRedo());
    document.redo();
    EXPECT_EQ(document.text(), "abcd");
}

TEST_F(CoreTest, UndoRestoresSelection)
{
    Document document;
    document.setText("alpha beta");
    document.setCursor({0, 10}, {0, 6});
    document.insertText("gamma");
    EXPECT_EQ(document.text(), "alpha gamma");
    document.undo();
    EXPECT_EQ(document.text(), "alpha beta");
    EXPECT_EQ(document.cursor(), at(0, 10));
    EXPECT_EQ(document.anchor(), at(0, 6));
}

TEST_F(CoreTest, SearchPlain)
{
    TextBuffer buffer;
    buffer.setText("Foo bar foo\nbaz FOO");
    Searcher searcher;
    searcher.setPattern("foo", {});
    EXPECT_EQ(searcher.count(buffer), 3);
    EXPECT_EQ(searcher.find(buffer, {0, 1}, false), (TextRange{{0, 8}, {0, 11}}));
    EXPECT_EQ(searcher.find(buffer, {0, 8}, true), (TextRange{{0, 0}, {0, 3}}));
    searcher.setPattern("foo", {true, false, false});
    EXPECT_EQ(searcher.count(buffer), 1);

    // Without case by Unicode folding: non-ASCII letters too; offsets are bytes.
    buffer.setText("Ärger ärger");
    searcher.setPattern("ärger", {});
    EXPECT_EQ(searcher.count(buffer), 2);
    EXPECT_EQ(searcher.find(buffer, {0, 2}, false), (TextRange{{0, 7}, {0, 13}}));
}

TEST_F(CoreTest, SearchWholeWordsAndCase)
{
    TextBuffer buffer;
    buffer.setText("cat concat cat_ cat.");
    Searcher searcher;
    searcher.setPattern("cat", {false, true, false});
    const std::vector<TextRange> matches = searcher.matchesInLine(buffer.line(0), 0);
    ASSERT_EQ(matches.size(), 2u);
    EXPECT_EQ(matches.at(1).start.column, 16);
    // Regular expression characters are literal without the option.
    searcher.setPattern("cat.", {});
    EXPECT_EQ(searcher.count(buffer), 1);
    // Words are Unicode words: a Han character is a word character.
    buffer.setText("中cat cat");
    searcher.setPattern("cat", {false, true, false});
    EXPECT_EQ(searcher.count(buffer), 1);
}

TEST_F(CoreTest, SearchRegexReplace)
{
    Document document;
    document.setText("x=1, y=22");
    document.setSearch("(\\w)=(\\d+)", false, false, true, false);
    EXPECT_TRUE(document.searchValid());
    EXPECT_EQ(document.matchCount(), 2);
    EXPECT_EQ(document.replaceAll("$2:\\1"), 2);
    EXPECT_EQ(document.text(), "1:x, 22:y");

    document.setSearch("(unclosed", false, false, true, false);
    EXPECT_FALSE(document.searchValid());
    EXPECT_FALSE(document.searchError().empty());

    // An empty match does not stop the next one.
    document.setText("ab");
    document.setSearch("x*", false, false, true, false);
    EXPECT_EQ(document.matchCount(), 0);
    document.setSearch("b|", false, false, true, false);
    EXPECT_EQ(document.matchCount(), 1);

    // Captures of the match found past an empty one at the same place.
    document.setText("y");
    document.setSearch("x*|(y)", false, false, true, false);
    EXPECT_EQ(document.replaceAll("[$1]"), 1);
    EXPECT_EQ(document.text(), "[y]");

    // Captures of a match that looks ahead into the next match's text.
    document.setText("aaa");
    document.setSearch("a(?=a)", false, false, true, false);
    EXPECT_EQ(document.replaceAll("[$0]"), 2);
    EXPECT_EQ(document.text(), "[a][a]a");
}

TEST_F(CoreTest, SearchScopeAndWrap)
{
    TextBuffer buffer;
    buffer.setText("a a\na a\na a");
    Searcher searcher;
    searcher.setPattern("a", {});
    bool wrapped = false;
    EXPECT_EQ(searcher.find(buffer, {2, 3}, false, {}, &wrapped), (TextRange{{0, 0}, {0, 1}}));
    EXPECT_TRUE(wrapped);
    const TextRange scope{{1, 0}, {1, 3}};
    EXPECT_EQ(searcher.count(buffer, scope), 2);
    EXPECT_EQ(searcher.find(buffer, {1, 3}, false, scope, &wrapped), (TextRange{{1, 0}, {1, 1}}));
    EXPECT_TRUE(wrapped);

    Document document;
    document.setText("a a\na a\na a");
    document.setCursor({1, 3}, {1, 0});
    document.setSearch("a", false, false, false, true);
    EXPECT_EQ(document.matchCount(), 2);
    EXPECT_EQ(document.replaceAll("bb"), 2);
    EXPECT_EQ(document.text(), "a a\nbb bb\na a");
}

TEST_F(CoreTest, ReplaceAllIsOneStep)
{
    Document document;
    document.setText("one two one\none");
    document.setSearch("one", false, false, false, false);
    EXPECT_EQ(document.replaceAll("1"), 3);
    EXPECT_EQ(document.text(), "1 two 1\n1");
    document.undo();
    EXPECT_EQ(document.text(), "one two one\none");

    document.setCursor({0, 0});
    EXPECT_TRUE(document.findNext());
    EXPECT_EQ(document.selection(), (TextRange{{0, 0}, {0, 3}}));
    EXPECT_EQ(document.matchIndex(), 1);
    EXPECT_TRUE(document.replaceCurrent("uno"));
    EXPECT_EQ(document.text(), "uno two one\none");
    EXPECT_EQ(document.selection(), (TextRange{{0, 8}, {0, 11}}));
}

TEST_F(CoreTest, LoadAndSave)
{
    const std::string file = writeFile("load.txt", "line one\nline two\n");
    Document document;
    int loaded = 0;
    const auto connection = document.loaded.connect([&] { ++loaded; });
    document.load(file);
    EXPECT_TRUE(document.isLoading());
    ASSERT_TRUE(document.waitForLoaded());
    EXPECT_EQ(loaded, 1);
    EXPECT_EQ(document.lineCount(), 3);
    EXPECT_EQ(document.title(), "load.txt");
    EXPECT_EQ(document.encoding(), "UTF-8");

    document.setCursor({1, 8});
    document.insertText("!");
    EXPECT_TRUE(document.save());
    EXPECT_EQ(readFile(file), "line one\nline two!\n");

    // An untitled document is named after its first line.
    Document untitled;
    EXPECT_EQ(untitled.title(), "Untitled");
    untitled.insertText("Shopping list\nmilk");
    EXPECT_EQ(untitled.title(), "Shopping list");
    const std::string newPath = path("new.txt");
    EXPECT_TRUE(untitled.saveAs(newPath));
    EXPECT_EQ(untitled.filePath(), newPath);
    // New files end with a newline.
    EXPECT_EQ(readFile(newPath), "Shopping list\nmilk\n");

    Document longTitle;
    longTitle.insertText("A first line that is much longer than forty characters");
    EXPECT_EQ(longTitle.title(), "A first line that is much longer than f…");
}

TEST_F(CoreTest, SaveKeepsLineEndingsAndBom)
{
    const std::string file = writeFile("crlf.txt", "\xEF\xBB\xBF" "a\r\nb");
    Document document;
    document.load(file);
    ASSERT_TRUE(document.waitForLoaded());
    EXPECT_EQ(document.lineEnding(), int(LineEnding::Windows));
    EXPECT_TRUE(document.hasBom());
    document.setCursor({1, 1});
    document.insertText("c");
    EXPECT_TRUE(document.save());
    // No final newline was added: the file had none.
    EXPECT_EQ(readFile(file), "\xEF\xBB\xBF" "a\r\nbc");

    document.setLineEnding(int(LineEnding::Unix));
    document.setBom(false);
    EXPECT_TRUE(document.isModified());
    EXPECT_TRUE(document.save());
    EXPECT_EQ(readFile(file), "a\nbc");
}

TEST_F(CoreTest, SaveThroughSymlinkKeepsPermissions)
{
    const std::string target = writeFile("target.sh", "echo hi\n");
    ::chmod(target.c_str(), 0700);
    const std::string link = path("link.sh");
    fs::create_symlink(target, link);
    Document document;
    document.load(link);
    ASSERT_TRUE(document.waitForLoaded());
    EXPECT_EQ(document.language(), "sh");
    document.setCursor({0, 7});
    document.insertText("!");
    EXPECT_TRUE(document.save());
    EXPECT_TRUE(fs::is_symlink(link));
    struct stat info;
    ASSERT_EQ(::stat(target.c_str(), &info), 0);
    EXPECT_TRUE(info.st_mode & S_IXUSR);
    EXPECT_EQ(readFile(target), "echo hi!\n");
}

TEST_F(CoreTest, SaveThroughDanglingSymlink)
{
    const std::string target = path("not-yet.txt");
    const std::string link = path("dangling.txt");
    fs::create_symlink(target, link);
    Document document;
    document.setText("made");
    EXPECT_TRUE(document.saveAs(link));
    EXPECT_TRUE(fs::is_symlink(link));
    EXPECT_EQ(readFile(target), "made\n");
}

TEST_F(CoreTest, ExternalChangeReloadsUnmodified)
{
    const std::string file = writeFile("watched.txt", "first\nsecond\n");
    Document document;
    document.load(file);
    ASSERT_TRUE(document.waitForLoaded());
    document.setCursor({1, 3});
    TestSupport::wait(50);
    writeFile("watched.txt", "first\nchanged here\n");
    EXPECT_TRUE(TestSupport::waitUntil([&] { return document.buffer().line(1) == "changed here"; }, 5000));
    EXPECT_FALSE(document.isModified());
    EXPECT_EQ(document.cursor(), at(1, 3));
    EXPECT_EQ(document.notice(), int(Document::NoNotice));
}

TEST_F(CoreTest, ExternalChangeAfterOtherDocumentCloses)
{
    // Documents share one inotify instance; the last one open keeps its watch.
    const std::string file = writeFile("shared watch.txt", "first\n");
    Document document;
    document.load(file);
    ASSERT_TRUE(document.waitForLoaded());
    {
        Document other;
        other.load(file);
        ASSERT_TRUE(other.waitForLoaded());
    }
    TestSupport::wait(50);
    writeFile("shared watch.txt", "changed\n");
    EXPECT_TRUE(TestSupport::waitUntil([&] { return document.buffer().line(0) == "changed"; }, 5000));
}

TEST_F(CoreTest, ExternalChangeReloadIsUndoable)
{
    const std::string file = writeFile("history.txt", "first\nsecond\n");
    Document document;
    document.load(file);
    ASSERT_TRUE(document.waitForLoaded());
    document.setCursor({0, 5});
    document.insertText("!");
    EXPECT_TRUE(document.save());
    TestSupport::wait(50);
    writeFile("history.txt", "first!\nchanged\n");
    EXPECT_TRUE(TestSupport::waitUntil([&] { return document.buffer().line(1) == "changed"; }, 5000));
    EXPECT_FALSE(document.isModified());

    document.undo();
    EXPECT_EQ(document.buffer().line(1), "second");
    EXPECT_TRUE(document.isModified());
    document.undo();
    EXPECT_EQ(document.buffer().line(0), "first");
    document.redo();
    document.redo();
    EXPECT_EQ(document.buffer().line(1), "changed");
    EXPECT_FALSE(document.isModified());
}

TEST_F(CoreTest, ExternalChangeWhileModified)
{
    const std::string file = writeFile("conflict.txt", "mine\n");
    Document document;
    document.load(file);
    ASSERT_TRUE(document.waitForLoaded());
    document.setCursor({0, 4});
    document.insertText("!");
    TestSupport::wait(50);
    writeFile("conflict.txt", "theirs\n");
    EXPECT_TRUE(TestSupport::waitUntil([&] { return document.notice() == Document::ChangedOnDisk; }, 5000));
    EXPECT_EQ(document.buffer().line(0), "mine!");

    document.keepLocalVersion();
    EXPECT_EQ(document.notice(), int(Document::NoNotice));
    document.undo();
    // The disk copy is someone else's now: back at the loaded text is still modified.
    EXPECT_TRUE(document.isModified());

    document.reload();
    ASSERT_TRUE(document.waitForLoaded());
    EXPECT_EQ(document.buffer().line(0), "theirs");
    EXPECT_FALSE(document.isModified());
}

TEST_F(CoreTest, ExternalDelete)
{
    const std::string file = writeFile("doomed.txt", "bye\n");
    Document document;
    document.load(file);
    ASSERT_TRUE(document.waitForLoaded());
    fs::remove(file);
    EXPECT_TRUE(TestSupport::waitUntil([&] { return document.notice() == Document::Deleted; }, 5000));
    EXPECT_TRUE(document.isModified());
    EXPECT_TRUE(document.save());
    EXPECT_TRUE(fs::exists(file));
    EXPECT_FALSE(document.isModified());
}

TEST_F(CoreTest, ReopenWithEncoding)
{
    const std::string text = "编码测试";
    const std::string file = writeFile("gbk.txt", encode(text, "GB18030"));
    Document document;
    document.load(file, "UTF-8");
    ASSERT_TRUE(document.waitForLoaded());
    EXPECT_EQ(document.notice(), int(Document::Malformed));
    document.reloadWithEncoding("GB18030");
    ASSERT_TRUE(document.waitForLoaded());
    EXPECT_EQ(document.buffer().line(0), text);
    EXPECT_EQ(document.encoding(), "GB18030");
    EXPECT_EQ(document.notice(), int(Document::NoNotice));

    // Saving with another encoding converts the file.
    document.setEncoding("UTF-8");
    EXPECT_TRUE(document.isModified());
    EXPECT_TRUE(document.save());
    EXPECT_EQ(readFile(file), text);

    // A character the encoding lacks stops the save instead of being lost.
    document.setEncoding("ISO-8859-1");
    EXPECT_FALSE(document.save());
    EXPECT_EQ(document.notice(), int(Document::LossyEncoding));
    EXPECT_TRUE(document.saveAsUtf8());
}

TEST_F(CoreTest, TooLarge)
{
    const std::string file = path("huge.bin");
    {
        Lingmo::base::file::TemporaryFile placeholder(TestSupport::directory());
        ASSERT_TRUE(placeholder.isValid());
        ASSERT_EQ(::ftruncate(placeholder.fd(), Document::maximumFileSize + 1), 0);
        fs::rename(placeholder.path(), file);
    }
    Document document;
    document.load(file);
    ASSERT_TRUE(document.waitForLoaded());
    EXPECT_EQ(document.notice(), int(Document::TooLarge));
    fs::remove(file);
}

TEST_F(CoreTest, SaveRefusesOverwritingExternalChange)
{
    const std::string file = writeFile("race.txt", "base\n");
    Document document;
    document.load(file);
    ASSERT_TRUE(document.waitForLoaded());
    document.setCursor({0, 4});
    document.insertText(" mine");
    // Changed on disk just before the save, before any watcher can tell.
    writeFile("race.txt", "theirs, longer\n");
    EXPECT_FALSE(document.save());
    EXPECT_EQ(document.notice(), int(Document::ChangedOnDisk));
    EXPECT_EQ(readFile(file), "theirs, longer\n");
    document.keepLocalVersion();
    EXPECT_TRUE(document.save());
    EXPECT_EQ(readFile(file), "base mine\n");
}

TEST_F(CoreTest, SaveRefusesLosingMalformedBytes)
{
    const std::string file = writeFile("bad.txt", "ok \xff here\n");
    Document document;
    document.load(file);
    ASSERT_TRUE(document.waitForLoaded());
    EXPECT_EQ(document.encoding(), "windows-1252");
    document.reloadWithEncoding("UTF-8");
    ASSERT_TRUE(document.waitForLoaded());
    EXPECT_EQ(document.notice(), int(Document::Malformed));
    document.setCursor({0, 0});
    document.insertText("x");
    EXPECT_FALSE(document.save());
    EXPECT_EQ(document.notice(), int(Document::MalformedSave));
    EXPECT_EQ(readFile(file), "ok \xff here\n");
    // Another encoding cannot bring them back either.
    document.setEncoding("UTF-16LE");
    EXPECT_FALSE(document.save());
    EXPECT_TRUE(document.saveReplacingMalformed());
    EXPECT_FALSE(document.isModified());
}

TEST_F(CoreTest, SaveInReadOnlyFolder)
{
    const std::string file = writeFile("locked/inside.txt", "a\n");
    const std::string folder = fs::path(file).parent_path().string();
    ::chmod(folder.c_str(), 0500);
    Document document;
    document.load(file);
    ASSERT_TRUE(document.waitForLoaded());
    document.setCursor({0, 1});
    document.insertText("b");
    const bool saved = document.save();
    ::chmod(folder.c_str(), 0700);
    EXPECT_TRUE(saved);
    EXPECT_EQ(readFile(file), "ab\n");
}

TEST_F(CoreTest, SaveUnwritableFileKeepsText)
{
    const std::string file = writeFile("readonly.txt", "keep\n");
    ::chmod(file.c_str(), 0400);
    Document document;
    document.load(file);
    ASSERT_TRUE(document.waitForLoaded());
    EXPECT_FALSE(document.isWritable());
    EXPECT_EQ(document.notice(), int(Document::ReadOnly));
    document.setCursor({0, 4});
    document.insertText("!");
    EXPECT_FALSE(document.save());
    EXPECT_TRUE(document.isModified());
    EXPECT_EQ(document.buffer().line(0), "keep!");
    ::chmod(file.c_str(), 0600);
    EXPECT_EQ(readFile(file), "keep\n");
}

TEST_F(CoreTest, FailedSaveLeavesWhitespace)
{
    writeFile("trim/.editorconfig", "[*]\ntrim_trailing_whitespace = true\n");
    const std::string file = writeFile("trim/locked.txt", "keep\n");
    ::chmod(file.c_str(), 0400);
    Document document;
    document.load(file);
    ASSERT_TRUE(document.waitForLoaded());
    document.setCursor({0, 4});
    document.insertText("  ");
    EXPECT_FALSE(document.save());
    EXPECT_EQ(document.buffer().line(0), "keep  ");
    document.undo();
    EXPECT_EQ(document.buffer().line(0), "keep");
    EXPECT_FALSE(document.isModified());
    ::chmod(file.c_str(), 0600);
}

TEST_F(CoreTest, DeletedFileKeepsDraft)
{
    const std::string file = writeFile("gone.txt", "text\n");
    std::string id;
    {
        Document document;
        id = document.draftId();
        document.load(file);
        ASSERT_TRUE(document.waitForLoaded());
        document.setCursor({0, 4});
        document.insertText(" unsaved");
        document.writeDraftNow();
    }
    fs::remove(file);

    // As a restored session does: the file is gone, the draft is not.
    Document restored(id);
    restored.load(file);
    restored.restoreDraft();
    ASSERT_TRUE(restored.waitForLoaded());
    EXPECT_EQ(restored.buffer().line(0), "text unsaved");
    EXPECT_TRUE(restored.isModified());
    EXPECT_EQ(restored.notice(), int(Document::Deleted));
    EXPECT_TRUE(fs::exists(restored.draftPath()));
    EXPECT_TRUE(restored.save());
    EXPECT_TRUE(fs::exists(file));
}

TEST_F(CoreTest, DraftWritesAreOrdered)
{
    Document document;
    document.insertText("one");
    document.writeDraftAsync();
    document.insertText(" two");
    document.writeDraftNow();
    EXPECT_EQ(readFile(document.draftPath()), "one two");
    struct stat info;
    ASSERT_EQ(::stat(document.draftPath().c_str(), &info), 0);
    EXPECT_EQ(info.st_mode & 0777, 0600u);
    document.removeDraft();
    EXPECT_FALSE(fs::exists(document.draftPath()));
    ASSERT_EQ(::stat(fs::path(document.draftPath()).parent_path().c_str(), &info), 0);
    EXPECT_EQ(info.st_mode & 0077, 0u);
}

TEST_F(CoreTest, CorruptSessionKeepsDrafts)
{
    const std::string broken = writeFile("broken/session.json", "{\"windows\": [");
    bool ok = true;
    EXPECT_TRUE(Session::load(broken, &ok).empty());
    EXPECT_FALSE(ok);
    Session::load(path("missing.json"), &ok);
    EXPECT_TRUE(ok);

    // Fresh orphans stay while they may still be wanted.
    const std::string drafts = path("orphans");
    writeFile("orphans/fresh.txt", "x");
    Session::removeOrphanDrafts(drafts, {}, 7);
    EXPECT_TRUE(fs::exists(drafts + "/fresh.txt"));
}

TEST_F(CoreTest, ReplaceAllThenTypingUndoesSeparately)
{
    Document document;
    document.setText("a a a");
    document.setSearch("a", false, false, false, false);
    EXPECT_EQ(document.replaceAll("b"), 3);
    document.setCursor({0, 5});
    document.insertText("c");
    EXPECT_EQ(document.text(), "b b bc");
    document.undo();
    EXPECT_EQ(document.text(), "b b b");
    document.undo();
    EXPECT_EQ(document.text(), "a a a");
    document.redo();
    document.redo();
    EXPECT_EQ(document.text(), "b b bc");
}

TEST_F(CoreTest, EncodingRoundTripAll)
{
    // Text each encoding can hold: ASCII plus a character of its own script.
    const std::map<std::string, std::string> samples{
        {"GB18030", "中文"}, {"Big5", "中文"}, {"Big5-HKSCS", "中文"},
        {"Shift_JIS", "日本"}, {"EUC-JP", "日本"}, {"EUC-KR", "한국"},
        {"windows-1251", "Привет"}, {"KOI8-R", "Привет"}, {"KOI8-U", "Привіт"},
        {"windows-1253", "Γεια"}, {"windows-1255", "שלום"}, {"windows-1256", "سلام"},
        {"windows-874", "สวัสดี"}, {"windows-1258", "Đà"},
    };
    for (const EncodingInfo &info : TextCodec::encodings()) {
        SCOPED_TRACE(info.name);
        ASSERT_TRUE(TextCodec::isSupported(info.name));
        const auto sample = samples.find(info.name);
        const std::string special = sample == samples.end() ? "é" : sample->second;
        const std::vector<std::string> lines{"plain ascii", special, ""};
        bool lossy = true;
        const std::string bytes = TextCodec::encode(lines, info.name, false, LineEnding::Unix, &lossy);
        EXPECT_FALSE(lossy);
        const DecodedText decoded = TextCodec::decode(bytes, info.name);
        EXPECT_EQ(decoded.lines, lines);
        EXPECT_FALSE(decoded.malformed);
    }
}

TEST_F(CoreTest, EditingCommands)
{
    Document document;
    document.setText("a\nb\nc");
    document.setCursor({1, 0});
    document.moveLines(true);
    EXPECT_EQ(document.text(), "b\na\nc");
    EXPECT_EQ(document.cursor().line, 0);
    document.moveLines(false);
    document.moveLines(false);
    EXPECT_EQ(document.text(), "a\nc\nb");
    document.duplicateLines();
    EXPECT_EQ(document.text(), "a\nc\nb\nb");
    EXPECT_EQ(document.cursor().line, 3);
    document.deleteLines();
    EXPECT_EQ(document.text(), "a\nc\nb");
    document.setCursor({0, 0});
    document.deleteLines();
    EXPECT_EQ(document.text(), "c\nb");

    document.setText("hello wide world");
    document.setCursor({0, 10}, {0, 6});
    document.changeCase(Document::Upper);
    EXPECT_EQ(document.text(), "hello WIDE world");
    document.selectAll();
    document.changeCase(Document::Title);
    EXPECT_EQ(document.text(), "Hello Wide World");
    document.setText("straße");
    document.selectAll();
    document.changeCase(Document::Upper);
    EXPECT_EQ(document.text(), "STRASSE");

    document.setText("    indented");
    document.setCursor({0, 12});
    document.insertNewline(true);
    EXPECT_EQ(document.text(), "    indented\n    ");

    document.setText("abc");
    document.setCursor({0, 0});
    document.insertText("XY", true);
    EXPECT_EQ(document.text(), "XYc");
    document.setText("中文字");
    document.setCursor({0, 0});
    document.insertText("ab", true);
    EXPECT_EQ(document.text(), "ab字");
}

TEST_F(CoreTest, IndentCommands)
{
    Document document;
    document.setIndentationDefaults(true, 4);
    document.setText("a\nb\n\nc");
    document.setCursor({3, 1}, {0, 0});
    document.indentSelection(false);
    EXPECT_EQ(document.text(), "    a\n    b\n\n    c");
    document.indentSelection(true);
    EXPECT_EQ(document.text(), "a\nb\n\nc");
    document.undo();
    EXPECT_EQ(document.text(), "    a\n    b\n\n    c");

    // Backspace in leading spaces returns to the previous indentation stop.
    document.setText("      x");
    document.setCursor({0, 6});
    document.deleteBackward(false);
    EXPECT_EQ(document.text(), "    x");
}

TEST_F(CoreTest, WordMovement)
{
    Document document;
    document.setText("foo.bar  baz\nnext");
    EXPECT_EQ(document.wordBoundary({0, 0}, true), at(0, 3));
    EXPECT_EQ(document.wordBoundary({0, 3}, true), at(0, 4));
    EXPECT_EQ(document.wordBoundary({0, 7}, true), at(0, 12));
    EXPECT_EQ(document.wordBoundary({0, 12}, true), at(1, 0));
    EXPECT_EQ(document.wordBoundary({0, 9}, false), at(0, 4));
    EXPECT_EQ(document.wordAt({0, 5}), (TextRange{{0, 4}, {0, 7}}));
    document.setCursor({0, 12});
    document.deleteBackward(true);
    EXPECT_EQ(document.text(), "foo.bar  \nnext");

    // Letters of any script form words.
    document.setText("中文 text");
    EXPECT_EQ(document.wordBoundary({0, 0}, true), at(0, 6));
    EXPECT_EQ(document.wordAt({0, 3}), (TextRange{{0, 0}, {0, 6}}));
}

TEST_F(CoreTest, BracketMatching)
{
    Document document;
    document.setText("f(a, [b]) {\n  x\n}");
    EXPECT_EQ(document.matchingBracket({0, 1}), (TextRange{{0, 1}, {0, 8}}));
    EXPECT_EQ(document.matchingBracket({0, 9}), (TextRange{{0, 8}, {0, 1}}));
    EXPECT_EQ(document.matchingBracket({0, 10}), (TextRange{{0, 10}, {2, 0}}));
    EXPECT_EQ(document.matchingBracket({1, 1}).start.line, -1);
}

TEST_F(CoreTest, Statistics)
{
    Document document;
    document.setText("two words\n中文");
    const DocumentStatistics stats = document.statistics();
    EXPECT_EQ(stats.lines, 2);
    EXPECT_EQ(stats.words, 4);
    EXPECT_EQ(stats.characters, 12);
    EXPECT_EQ(stats.charactersWithoutSpaces, 10);
}

TEST_F(CoreTest, StatusBarCounts)
{
    // The status bar counts columns in characters, tabs expanded,
    // and the selection in UTF-16 units.
    Document document;
    document.setText("\t中😀x");
    document.setCursor({0, int(std::string("\t中😀").size())}, {0, 0});
    EXPECT_EQ(document.visualColumn(), 4 + 2 + 1);
    EXPECT_EQ(document.selectionLength(), 4);
    document.goToLine(1, 3);
    EXPECT_EQ(document.cursor(), at(0, 4));
}

TEST_F(CoreTest, EditorConfig)
{
    EXPECT_TRUE(EditorConfig::globMatches("*.py", "src/a.py"));
    EXPECT_TRUE(EditorConfig::globMatches("*.{c,h}", "x.h"));
    EXPECT_FALSE(EditorConfig::globMatches("*.{c,h}", "x.cpp"));
    EXPECT_TRUE(EditorConfig::globMatches("lib/**.js", "lib/a/b.js"));
    EXPECT_FALSE(EditorConfig::globMatches("/top.txt", "sub/top.txt"));

    writeFile("ec/.editorconfig",
              "root = true\n[*]\nindent_style = space\nindent_size = 2\n"
              "[*.mk]\nindent_style = tab\ntab_width = 8\n"
              "[sub/*.txt]\ntrim_trailing_whitespace = true\nend_of_line = crlf\n");
    writeFile("ec/sub/.editorconfig", "[*.txt]\ninsert_final_newline = false\n");
    const std::string file = writeFile("ec/sub/a.txt", "x  \ny\n");

    const EditorConfig config = EditorConfig::forFile(file);
    EXPECT_EQ(config.insertSpaces.value(), true);
    EXPECT_EQ(config.indentSize.value(), 2);
    EXPECT_TRUE(config.trimTrailingWhitespace.value());
    EXPECT_FALSE(config.insertFinalNewline.value());
    EXPECT_EQ(config.lineEnding.value(), LineEnding::Windows);

    const EditorConfig make = EditorConfig::forFile(path("ec/rules.mk"));
    EXPECT_EQ(make.insertSpaces.value(), false);
    EXPECT_EQ(make.tabWidth.value(), 8);

    Document document;
    document.load(file);
    ASSERT_TRUE(document.waitForLoaded());
    EXPECT_EQ(document.indentWidth(), 2);
    document.setCursor({1, 1});
    document.insertText("z ");
    EXPECT_TRUE(document.save());
    EXPECT_EQ(readFile(file), "x\nyz\n");
}

TEST_F(CoreTest, IndentationDetection)
{
    const std::vector<std::string> spaces{"a {", "  b {", "    c", "  }", "  d", "}"};
    const auto detected = FileOptions::detectIndentation(spaces);
    ASSERT_TRUE(detected);
    EXPECT_TRUE(detected->insertSpaces);
    EXPECT_EQ(detected->width, 2);

    const std::vector<std::string> tabs{"a", "\tb", "\t\tc", "\td"};
    EXPECT_FALSE(FileOptions::detectIndentation(tabs)->insertSpaces);
    EXPECT_FALSE(FileOptions::detectIndentation({"flat"}));
}

TEST_F(CoreTest, LanguageDetection)
{
    EXPECT_EQ(Syntax::detect("/x/main.cpp", ""), "cpp");
    EXPECT_EQ(Syntax::detect("/x/CMakeLists.txt", ""), "cmake");
    EXPECT_EQ(Syntax::detect("/x/notes.txt", ""), "");
    EXPECT_EQ(Syntax::detect("/x/script", "#!/usr/bin/env python3"), "python");
    EXPECT_EQ(Syntax::detect("", "#!/bin/bash"), "sh");
    EXPECT_EQ(Syntax::detect("Makefile", ""), "makefile");
    EXPECT_EQ(Syntax::detect("a.desktop", ""), "ini");
    EXPECT_EQ(Syntax::detect("README.md", ""), "markdown");
    EXPECT_EQ(Syntax::detect("x.LOG.1", ""), "log");

    // An untitled document follows its first line.
    Document document;
    document.insertText("#!/bin/sh\necho");
    EXPECT_EQ(document.language(), "sh");

    // Sorted by name, as the language menu lists them.
    const std::vector<const Language *> &languages = Syntax::languages();
    ASSERT_FALSE(languages.empty());
    EXPECT_EQ(Syntax::name(Syntax::id(languages.front())), "C");
    EXPECT_EQ(Syntax::name(""), "Plain Text");
}

TEST_F(CoreTest, HighlightCarriesState)
{
    const Language *c = Syntax::language("c");
    std::vector<SyntaxSpan> spans;
    Syntax::State state = Syntax::highlight(c, "int x; /* open", 0, &spans);
    EXPECT_NE(state, 0);
    EXPECT_EQ(spans.front().style, SyntaxStyle::Type);
    EXPECT_EQ(spans.back().style, SyntaxStyle::Comment);
    spans.clear();
    state = Syntax::highlight(c, "still */ return \"s\\n\";", state, &spans);
    EXPECT_EQ(state, 0);
    ASSERT_GE(spans.size(), 4u);
    EXPECT_EQ(spans.at(0).style, SyntaxStyle::Comment);
    EXPECT_EQ(spans.at(0).length, 8);
    EXPECT_EQ(spans.at(1).style, SyntaxStyle::Keyword);
    EXPECT_EQ(spans.at(2).style, SyntaxStyle::String);
    EXPECT_EQ(spans.at(3).style, SyntaxStyle::Escape);

    const Language *python = Syntax::language("python");
    spans.clear();
    state = Syntax::highlight(python, "x = \"\"\"doc", 0, &spans);
    EXPECT_NE(state, 0);
    spans.clear();
    EXPECT_EQ(Syntax::highlight(python, "end\"\"\" # c", state, &spans), 0);
    EXPECT_EQ(spans.back().style, SyntaxStyle::Comment);

    const Language *markdown = Syntax::language("markdown");
    spans.clear();
    Syntax::highlight(markdown, "a **bold** and `code`", 0, &spans);
    ASSERT_EQ(spans.size(), 2u);
    EXPECT_EQ(spans.at(0).style, SyntaxStyle::Strong);
    EXPECT_EQ(spans.at(1).style, SyntaxStyle::Code);

    // Spans are byte ranges.
    spans.clear();
    Syntax::highlight(c, "\"中\" x", 0, &spans);
    ASSERT_EQ(spans.size(), 1u);
    EXPECT_EQ(spans.at(0).start, 0);
    EXPECT_EQ(spans.at(0).length, 5);

    const Language *log = Syntax::language("log");
    spans.clear();
    Syntax::highlight(log, "2026-10-16 12:00:01 ERROR failed here", 0, &spans);
    ASSERT_EQ(spans.size(), 3u);
    EXPECT_EQ(spans.at(0).style, SyntaxStyle::Comment);
    EXPECT_EQ(spans.at(1).style, SyntaxStyle::Error);
    EXPECT_EQ(spans.at(2).style, SyntaxStyle::Error);
}

TEST_F(CoreTest, SyntaxStatesResume)
{
    TextBuffer buffer;
    std::vector<std::string> lines;
    for (int i = 0; i < 1000; ++i)
        lines.push_back("int a" + std::to_string(i) + ";");
    buffer.setLines(lines);
    SyntaxStates states;
    states.reset(Syntax::language("c"), buffer.lineCount());
    int last = -1;
    states.advance(buffer, 999, 1000, &last);
    EXPECT_EQ(states.validLines(), 1000);

    // An edit that keeps the end state leaves the lines below valid.
    buffer.remove({{10, 0}, {10, 3}});
    states.linesChanged(10, 1, 1);
    EXPECT_EQ(states.validLines(), 10);
    EXPECT_EQ(states.advance(buffer, 999, 1000, &last), -1);
    EXPECT_EQ(states.validLines(), 1000);

    // Opening a comment changes every line below.
    buffer.insert({10, 0}, "/*");
    states.linesChanged(10, 1, 1);
    const int first = states.advance(buffer, 999, 1000, &last);
    EXPECT_EQ(first, 11);
    EXPECT_EQ(last, 999);
    EXPECT_NE(states.before(500), 0);
}

TEST_F(CoreTest, SessionRoundTrip)
{
    const std::string file = path("session/session.json");
    std::vector<WindowState> windows(2);
    windows[0].tabs = {TabState{"/a.txt", "d1", false, 3, 4, 120.5, "GB18030", ""},
                       TabState{"", "d2", true, 0, 0, 0, "", "python"}};
    windows[0].current = 1;
    windows[1].tabs = {TabState{"", "d3", false, 0, 0, 0, "", ""}};
    ASSERT_TRUE(Session::save(windows, file));

    const std::vector<WindowState> loaded = Session::load(file);
    // The empty untitled tab is not brought back.
    ASSERT_EQ(loaded.size(), 1u);
    ASSERT_EQ(loaded.at(0).tabs.size(), 2u);
    EXPECT_EQ(loaded.at(0).current, 1);
    EXPECT_EQ(loaded.at(0).tabs.at(0).path, "/a.txt");
    EXPECT_EQ(loaded.at(0).tabs.at(0).line, 3);
    EXPECT_EQ(loaded.at(0).tabs.at(0).scroll, 120.5);
    EXPECT_EQ(loaded.at(0).tabs.at(0).encoding, "GB18030");
    EXPECT_TRUE(loaded.at(0).tabs.at(1).unsaved);
    EXPECT_EQ(loaded.at(0).tabs.at(1).language, "python");

    const std::string drafts = path("drafts-orphans");
    writeFile("drafts-orphans/keep.txt", "x");
    writeFile("drafts-orphans/old.txt", "y");
    writeFile("drafts-orphans/stale.txt.a1B2c3", "z");
    writeFile("drafts-orphans/current.txt.d4E5f6", "z");
    writeFile("drafts-orphans/other.txt.bak", "z");
    // Only a temporary from before this process started is stale.
    const auto before = fs::file_time_type::clock::now() - std::chrono::hours(1);
    fs::last_write_time(drafts + "/stale.txt.a1B2c3", before);
    fs::last_write_time(drafts + "/other.txt.bak", before);
    Session::removeOrphanDrafts(drafts, {"keep"});
    EXPECT_TRUE(fs::exists(drafts + "/keep.txt"));
    EXPECT_FALSE(fs::exists(drafts + "/old.txt"));
    EXPECT_FALSE(fs::exists(drafts + "/stale.txt.a1B2c3"));
    EXPECT_TRUE(fs::exists(drafts + "/current.txt.d4E5f6"));
    EXPECT_TRUE(fs::exists(drafts + "/other.txt.bak"));
}

TEST_F(CoreTest, DraftRestore)
{
    const std::string file = writeFile("drafted.txt", "saved text\n");
    std::string id;
    {
        Document document;
        id = document.draftId();
        document.load(file);
        ASSERT_TRUE(document.waitForLoaded());
        document.setCursor({0, 10});
        document.insertText(" plus edits");
        document.writeDraftNow();
        EXPECT_TRUE(fs::exists(document.draftPath()));
    }

    Document restored(id);
    restored.load(file);
    restored.restoreDraft();
    ASSERT_TRUE(restored.waitForLoaded());
    EXPECT_EQ(restored.buffer().line(0), "saved text plus edits");
    EXPECT_TRUE(restored.isModified());
    EXPECT_EQ(restored.notice(), int(Document::Restored));
    EXPECT_TRUE(restored.save());
    EXPECT_FALSE(fs::exists(restored.draftPath()));

    // An untitled draft comes back as an unsaved document.
    std::string untitledId;
    {
        Document untitled;
        untitledId = untitled.draftId();
        untitled.insertText("scratch");
        untitled.writeDraftNow();
    }
    Document back(untitledId);
    EXPECT_TRUE(back.restoreDraft());
    EXPECT_EQ(back.text(), "scratch");
    EXPECT_TRUE(back.isModified());
    back.removeDraft();
}

TEST_F(CoreTest, RecentFiles)
{
    const std::string store = path("text-editor.conf");
    const std::string a = writeFile("recent a.txt", "a");
    const std::string b = writeFile("recent-b.txt", "b");
    const std::string gone = path("recent-deleted.txt");
    {
        Lingmo::base::Settings settings(store);
        settings.setValue("Editor/wordWrap", false);
        settings.setValue("Recent/Files", std::vector<std::string>{gone});
    }

    RecentFiles recent(store);
    EXPECT_TRUE(recent.files().empty());
    recent.add(a);
    recent.add(b);
    recent.add(a);
    EXPECT_EQ(recent.files(), (std::vector<std::string>{a, b}));

    RecentFiles again(store);
    EXPECT_EQ(again.files(), (std::vector<std::string>{a, b}));
    again.clear();
    EXPECT_TRUE(again.files().empty());

    // The rest of the settings file stays.
    const Lingmo::base::Settings settings(store);
    EXPECT_FALSE(settings.contains("Recent/Files"));
    EXPECT_FALSE(settings.boolValue("Editor/wordWrap", true));
}
