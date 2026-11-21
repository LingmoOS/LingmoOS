// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <unistd.h>

#include "base/file.h"
#include "base/settings.h"

using namespace Lingmo::Base;

// The reference file for the values below, byte for byte.
static constexpr const char* kReferenceFile = R"([General]
top=plain

[Deep]
a\b=nested

[%General]
inGroup=1

[Odd%20Key]
semi%3Bcolon%3D=w
with%20space=v

[Values]
at=@@start
bool=false
control=a\x1\x62\x2\x39
double=0.07772229381443299
double2=1.5
double3=1e+20
double4=100
empty=
emptyList=@Invalid()
escapes=tab\there\nnew\\back
int=42
list=a, b c, "d,e", " lead", @@x
negative=-7
oneList=solo
point=@Point(3 -4)
quote="say \"hi\"; ok = yes"
rect=@Rect(1 2 30 40)
trailing="space "
unicode=中文 é

[WindowMaximized]
Main=true

[WindowSize]
Main=@Size(800 600)
)";

namespace {

class SettingsTest : public testing::Test
{
protected:
    void SetUp() override
    {
        char directory[] = "/tmp/settings-test-XXXXXX";
        ASSERT_NE(mkdtemp(directory), nullptr);
        m_directory = directory;
    }
    void TearDown() override { std::filesystem::remove_all(m_directory); }

    std::string path(const std::string& name) const { return m_directory + "/" + name; }

    std::string m_directory;
};

} // namespace

TEST_F(SettingsTest, WritesWhatQSettingsWrites)
{
    {
        Settings settings(path("our.conf"));
        settings.setValue("top", "plain");
        settings.setValue("General/inGroup", 1);
        settings.setValue("WindowSize/Main", Settings::Size{800, 600});
        settings.setValue("WindowMaximized/Main", true);
        settings.setValue("Values/double", 0.07772229381443299);
        settings.setValue("Values/double2", 1.5);
        settings.setValue("Values/double3", 1e20);
        settings.setValue("Values/double4", 100.0);
        settings.setValue("Values/int", 42);
        settings.setValue("Values/negative", -7);
        settings.setValue("Values/bool", false);
        settings.setValue("Values/list", std::vector<std::string>{"a", "b c", "d,e", " lead", "@x"});
        settings.setValue("Values/emptyList", std::vector<std::string>{});
        settings.setValue("Values/oneList", std::vector<std::string>{"solo"});
        settings.setValue("Values/at", "@start");
        settings.setValue("Values/quote", "say \"hi\"; ok = yes");
        settings.setValue("Values/escapes", "tab\there\nnew\\back");
        settings.setValue("Values/unicode", "中文 é");
        settings.setValue("Values/control", std::string("a\x01" "b\x02" "9"));
        settings.setValue("Values/point", Settings::Point{3, -4});
        settings.setValue("Values/rect", Settings::Rect{1, 2, 30, 40});
        settings.setValue("Values/trailing", "space ");
        settings.setValue("Odd Key/with space", "v");
        settings.setValue("Odd Key/semi;colon=", "w");
        settings.setValue("Deep/a/b", "nested");
        settings.setValue("Values/empty", "");
    }
    EXPECT_EQ(file::read(path("our.conf")), kReferenceFile);
}

// Empty items keep their place, written and read.
TEST_F(SettingsTest, ListsKeepEmptyItems)
{
    const std::vector<std::string> names{"", "", "Music"};
    const std::vector<std::string> trailing{"a", ""};
    const std::vector<std::string> allEmpty{"", ""};
    {
        Settings settings(path("lists.conf"));
        settings.setValue("Desks/Names", names);
        settings.setValue("Desks/Trailing", trailing);
        settings.setValue("Desks/AllEmpty", allEmpty);
    }
    EXPECT_EQ(file::read(path("lists.conf")), "[Desks]\nAllEmpty=, \nNames=, , Music\nTrailing=a, \n");
    Settings settings(path("lists.conf"));
    EXPECT_EQ(settings.stringListValue("Desks/Names"), names);
    EXPECT_EQ(settings.stringListValue("Desks/Trailing"), trailing);
    EXPECT_EQ(settings.stringListValue("Desks/AllEmpty"), allEmpty);
}

TEST_F(SettingsTest, ReadsWhatQSettingsWrites)
{
    ASSERT_TRUE(file::writeAtomically(path("qt.conf"), kReferenceFile));
    const Settings settings(path("qt.conf"));
    EXPECT_EQ(settings.stringValue("top"), "plain");
    EXPECT_EQ(settings.intValue("General/inGroup"), 1);
    EXPECT_EQ(settings.sizeValue("WindowSize/Main"), (Settings::Size{800, 600}));
    EXPECT_TRUE(settings.boolValue("WindowMaximized/Main"));
    EXPECT_DOUBLE_EQ(settings.doubleValue("Values/double"), 0.07772229381443299);
    EXPECT_DOUBLE_EQ(settings.doubleValue("Values/double3"), 1e20);
    EXPECT_EQ(settings.intValue("Values/negative"), -7);
    EXPECT_FALSE(settings.boolValue("Values/bool", true));
    EXPECT_EQ(settings.stringListValue("Values/list"), (std::vector<std::string>{"a", "b c", "d,e", " lead", "@x"}));
    EXPECT_TRUE(settings.contains("Values/emptyList"));
    EXPECT_EQ(settings.stringListValue("Values/emptyList", {"fallback"}), std::vector<std::string>{});
    EXPECT_EQ(settings.stringListValue("Values/oneList"), std::vector<std::string>{"solo"});
    EXPECT_EQ(settings.stringValue("Values/at"), "@start");
    EXPECT_EQ(settings.stringValue("Values/quote"), "say \"hi\"; ok = yes");
    EXPECT_EQ(settings.stringValue("Values/escapes"), "tab\there\nnew\\back");
    EXPECT_EQ(settings.stringValue("Values/unicode"), "中文 é");
    EXPECT_EQ(settings.stringValue("Values/control"), std::string("a\x01" "b\x02" "9"));
    EXPECT_EQ(settings.pointValue("Values/point"), (Settings::Point{3, -4}));
    EXPECT_EQ(settings.rectValue("Values/rect"), (Settings::Rect{1, 2, 30, 40}));
    EXPECT_EQ(settings.stringValue("Values/trailing"), "space ");
    EXPECT_EQ(settings.stringValue("Odd Key/semi;colon="), "w");
    EXPECT_EQ(settings.stringValue("Deep/a/b"), "nested");
    EXPECT_EQ(settings.stringValue("Values/missing", "x"), "x");
    EXPECT_EQ(settings.intValue("Values/quote", 5), 5);
    EXPECT_EQ(settings.childKeys(), std::vector<std::string>{"top"});
    EXPECT_EQ(settings.childGroups(), (std::vector<std::string>{"Deep", "General", "Odd Key", "Values",
                                                                  "WindowMaximized", "WindowSize"}));
}

TEST_F(SettingsTest, KeepsWhatItDoesNotUnderstand)
{
    const std::string original = "[General]\n"
                                 "blob=@Variant(\\0\\0\\0\\x7f\\0\\0\\0\\x18)\n"
                                 "bytes=@ByteArray(\\x1\\xff)\n"
                                 "; a comment QSettings drops as well\n"
                                 "plain=1\n";
    ASSERT_TRUE(file::writeAtomically(path("other.conf"), original));
    {
        Settings settings(path("other.conf"));
        EXPECT_EQ(settings.stringValue("bytes"), std::string("\x01\xff", 2));
        settings.setValue("plain", 2);
    }
    EXPECT_EQ(file::read(path("other.conf")), "[General]\n"
                                              "blob=@Variant(\\0\\0\\0\\x7f\\0\\0\\0\\x18)\n"
                                              "bytes=@ByteArray(\\x1\\xff)\n"
                                              "plain=2\n");
}

TEST_F(SettingsTest, SyncMergesWithOtherWriters)
{
    Settings ours(path("shared.conf"));
    ours.setValue("Ours/a", 1);
    ASSERT_TRUE(ours.sync());

    {
        Settings theirs(path("shared.conf"));
        theirs.setValue("Theirs/b", 2);
    }
    ours.setValue("Ours/c", 3);
    ours.remove("Ours/a");
    ASSERT_TRUE(ours.sync());

    const Settings check(path("shared.conf"));
    EXPECT_FALSE(check.contains("Ours/a"));
    EXPECT_EQ(check.intValue("Ours/c"), 3);
    EXPECT_EQ(check.intValue("Theirs/b"), 2);

    ours.remove("Ours");
    ASSERT_TRUE(ours.sync());
    EXPECT_EQ(Settings(path("shared.conf")).childGroups(), std::vector<std::string>{"Theirs"});
}

TEST_F(SettingsTest, NothingChangedWritesNothing)
{
    {
        Settings settings(path("untouched.conf"));
        EXPECT_FALSE(settings.contains("a"));
    }
    EXPECT_FALSE(std::filesystem::exists(path("untouched.conf")));
}

TEST_F(SettingsTest, ContinuesValuesAcrossWindowsLineBreaks)
{
    ASSERT_TRUE(file::writeAtomically(path("crlf.conf"), "[Group]\r\nlong=first \\\r\nsecond\r\nnext=x\r\n"));
    const Settings settings(path("crlf.conf"));
    EXPECT_EQ(settings.stringValue("Group/long"), "first second");
    EXPECT_EQ(settings.stringValue("Group/next"), "x");
}
