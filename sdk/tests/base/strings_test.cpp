// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <algorithm>

#include "base/strings.h"

using namespace Lingmo::Base;
using namespace Lingmo::Base::strings;

TEST(StringsTest, Case)
{
    EXPECT_EQ(toAsciiLower("eDP-1 Ä"), "edp-1 Ä");
    EXPECT_EQ(toAsciiUpper("dp-2"), "DP-2");
    EXPECT_TRUE(equalsIgnoreAsciiCase("Apple", "aPPLE"));
    EXPECT_FALSE(equalsIgnoreAsciiCase("Apple", "Apples"));
    EXPECT_EQ(compare("a", "B", CaseSensitivity::Insensitive), -1);
    EXPECT_EQ(compare("a", "B"), 1);
    EXPECT_EQ(compare("abc", "abc"), 0);
    EXPECT_TRUE(startsWith("eDP-1", "edp", CaseSensitivity::Insensitive));
    EXPECT_FALSE(startsWith("eDP-1", "edp"));
    EXPECT_TRUE(endsWith("file.PNG", ".png", CaseSensitivity::Insensitive));
    EXPECT_TRUE(contains("Apple Keyboard", "keyboard", CaseSensitivity::Insensitive));
    EXPECT_EQ(indexOf("abcABC", "C", 3, CaseSensitivity::Insensitive), 5u);
    EXPECT_EQ(indexOf("abc", "x"), std::string_view::npos);

    const std::vector<std::string> list{"Foo", "bar"};
    EXPECT_TRUE(contains(list, "foo", CaseSensitivity::Insensitive));
    EXPECT_FALSE(contains(list, "foo"));

    std::vector<std::string> names{"b", "C", "a"};
    std::sort(names.begin(), names.end(), LessIgnoreAsciiCase());
    EXPECT_EQ(names, (std::vector<std::string>{"a", "b", "C"}));
}

TEST(StringsTest, TrimAndSimplify)
{
    EXPECT_EQ(trimmed("  a b \t\n"), "a b");
    EXPECT_EQ(trimmed(" \t "), "");
    EXPECT_EQ(simplified("  a \t b\n\nc  "), "a b c");
}

TEST(StringsTest, SplitAndJoin)
{
    EXPECT_EQ(split("a,,b", ','), (std::vector<std::string>{"a", "", "b"}));
    EXPECT_EQ(split("a,,b,", ',', SplitBehavior::SkipEmptyParts), (std::vector<std::string>{"a", "b"}));
    EXPECT_EQ(split("", ','), (std::vector<std::string>{""}));
    EXPECT_EQ(split("a::b", "::"), (std::vector<std::string>{"a", "b"}));
    EXPECT_EQ(splitWhitespace("  one two\tthree "), (std::vector<std::string>{"one", "two", "three"}));
    EXPECT_EQ(splitViews("x/y", '/').size(), 2u);
    const std::vector<std::string> parts{"a", "b", "c"};
    EXPECT_EQ(join(parts, ", "), "a, b, c");
    const std::vector<std::string_view> views{"x"};
    EXPECT_EQ(join(views, "-"), "x");
    EXPECT_EQ(join(std::vector<std::string>{}, "-"), "");
}

TEST(StringsTest, Section)
{
    EXPECT_EQ(section("card0/DP-1", '/', -1), "DP-1");
    EXPECT_EQ(section("key@port", '@', 0, 0), "key");
    EXPECT_EQ(section("a,b,c,d", ',', 1, 2), "b,c");
    EXPECT_EQ(section("a,b,c", ',', 1), "b,c");
    EXPECT_EQ(section("a,b,c", ',', 5), "");
    EXPECT_EQ(section("Name,Room", ',', 0, 0), "Name");
    EXPECT_EQ(section("plain", '_', 0, 0), "plain");
}

TEST(StringsTest, Replace)
{
    EXPECT_EQ(replaceAll("a-b-c", "-", "::"), "a::b::c");
    EXPECT_EQ(replaceAll("aaa", "aa", "b"), "ba");
    EXPECT_EQ(replaceAll("abc", "", "x"), "abc");
}

TEST(StringsTest, ParseNumbers)
{
    EXPECT_EQ(toNumber<int>("42"), 42);
    EXPECT_EQ(toNumber<int>(" -42 "), -42);
    EXPECT_EQ(toNumber<int>("+7"), 7);
    EXPECT_EQ(toNumber<int>("2147483647"), INT_MAX);
    EXPECT_EQ(toNumber<int>("-2147483648"), INT_MIN);
    EXPECT_EQ(toNumber<int>("2147483648"), std::nullopt);
    EXPECT_EQ(toNumber<int>("-2147483649"), std::nullopt);
    EXPECT_EQ(toNumber<int64_t>("-9223372036854775808"), INT64_MIN);
    EXPECT_EQ(toNumber<uint64_t>("18446744073709551615"), UINT64_MAX);
    EXPECT_EQ(toNumber<unsigned>("-1"), std::nullopt);
    EXPECT_EQ(toNumber<unsigned>("-0"), 0u);
    EXPECT_EQ(toNumber<int>("12a"), std::nullopt);
    EXPECT_EQ(toNumber<int>(""), std::nullopt);
    EXPECT_EQ(toNumber<int>("+-1"), std::nullopt);
    EXPECT_EQ(toNumber<int>("ff", 16), 255);
    EXPECT_EQ(toNumber<int>("0x1F", 0), 31);
    EXPECT_EQ(toNumber<int>("010", 0), 8);
    EXPECT_EQ(toNumber<int>("0b101", 0), 5);
    EXPECT_EQ(toNumber<int>("0", 0), 0);
    EXPECT_EQ(toNumber<uint8_t>("256"), std::nullopt);

    EXPECT_EQ(toDouble("1.5"), 1.5);
    EXPECT_EQ(toDouble(" -2e3 "), -2000);
    EXPECT_EQ(toDouble("+0.25"), 0.25);
    EXPECT_EQ(toDouble("1.5x"), std::nullopt);
    EXPECT_EQ(toDouble("+-1"), std::nullopt);
    EXPECT_EQ(toDouble(""), std::nullopt);
}

TEST(StringsTest, FormatNumbers)
{
    EXPECT_EQ(number(-42), "-42");
    EXPECT_EQ(number(255u, 16), "ff");
    EXPECT_EQ(number(uint64_t(UINT64_MAX)), "18446744073709551615");
    EXPECT_EQ(number(1.5), "1.5");
    EXPECT_EQ(number(0.1 + 0.2), "0.3");
    EXPECT_EQ(number(1234567.0), "1.23457e+06");
    EXPECT_EQ(number(2.0), "2");
    EXPECT_EQ(number(1.25, 'f', 1), "1.2");
    EXPECT_EQ(number(1.0 / 3, 'f', 3), "0.333");
    EXPECT_EQ(number(0.1 + 0.2, 'g', -1), "0.30000000000000004");
}

TEST(StringsTest, Utf8Validation)
{
    EXPECT_TRUE(isValidUtf8("plain"));
    EXPECT_TRUE(isValidUtf8("中文 \xEF\xBF\xBD 😀"));
    EXPECT_FALSE(isValidUtf8("\xC0\x80"));        // overlong
    EXPECT_FALSE(isValidUtf8("\xED\xA0\x80"));    // surrogate
    EXPECT_FALSE(isValidUtf8("\xF4\x90\x80\x80")); // past U+10FFFF
    EXPECT_FALSE(isValidUtf8("\xE4\xB8"));        // truncated
    EXPECT_FALSE(isValidUtf8("\x80"));

    // Each maximal subpart becomes one U+FFFD.
    EXPECT_EQ(sanitizedUtf8("a\xE4\xB8z"), "a\xEF\xBF\xBDz");
    EXPECT_EQ(sanitizedUtf8("\xC0\x80"), "\xEF\xBF\xBD\xEF\xBF\xBD");
    EXPECT_EQ(sanitizedUtf8("\xF0\x9F\x98"), "\xEF\xBF\xBD");
    EXPECT_EQ(sanitizedUtf8("ok"), "ok");
}

TEST(StringsTest, Utf16And32)
{
    const std::string text = "aé中😀";
    const std::u16string utf16 = utf8ToUtf16(text);
    EXPECT_EQ(utf16, u"aé中😀");
    EXPECT_EQ(utf16.size(), 5u);
    EXPECT_EQ(utf16Length(text), 5u);
    EXPECT_EQ(utf16ToUtf8(utf16), text);
    EXPECT_EQ(utf8ToUtf32(text), U"aé中😀");
    EXPECT_EQ(utf32ToUtf8(U"aé中😀"), text);

    // A lone surrogate becomes U+FFFD.
    const char16_t lone[] = {u'a', char16_t(0xD800), u'b', 0};
    EXPECT_EQ(utf16ToUtf8(lone), "a\xEF\xBF\xBD" "b");
    EXPECT_EQ(utf32ToUtf8(std::u32string(1, char32_t(0x110000))), "\xEF\xBF\xBD");
    EXPECT_EQ(latin1ToUtf8("caf\xE9"), "café");

    std::string appended;
    appendUtf8(appended, U'😀');
    EXPECT_EQ(appended, "😀");
}

TEST(StringsTest, Hex)
{
    EXPECT_EQ(toHex(std::string("\x00\xff\x10", 3)), "00ff10");
    EXPECT_EQ(fromHex("00FFffffffffff00"), std::string("\x00\xff\xff\xff\xff\xff\xff\x00", 8));
    EXPECT_EQ(fromHex("abc"), std::nullopt);
    EXPECT_EQ(fromHex("zz"), std::nullopt);
    EXPECT_EQ(fromHex(""), std::string());
}

TEST(StringsTest, UnicodeUpperCase)
{
    EXPECT_EQ(toUpper("é"), "É");
    EXPECT_EQ(toUpper("ß"), "SS");
    EXPECT_EQ(toUpper("ŉ"), "ʼN");
    EXPECT_EQ(toUpper("ﬃ"), "FFI");
    EXPECT_EQ(toUpper("ΐ"), "\u0399\u0308\u0301");
    // Root locale: no dotted capital I for i.
    EXPECT_EQ(toUpper("istanbul"), "ISTANBUL");
    EXPECT_EQ(toUpper("ǆ"), "Ǆ");
    EXPECT_EQ(toUpper("𐐨"), "𐐀");
    EXPECT_EQ(toUpper("中文 a1!"), "中文 A1!");
    EXPECT_EQ(toUpper(""), "");
    EXPECT_EQ(toUpper("a\xFF"), "A\xEF\xBF\xBD");
}
