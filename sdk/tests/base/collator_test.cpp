// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <thread>
#include <vector>

#include "base/collator.h"

using namespace Lingmo::Base;

static std::vector<std::string> sorted(std::vector<std::string> names, const Collator& collator)
{
    std::sort(names.begin(), names.end(), collator);
    return names;
}

TEST(CollatorTest, NumericModeOrdersNumbersByValue)
{
    Collator collator(Locale("en_US"));
    EXPECT_GT(collator.compare("Episode 2", "Episode 10"), 0);
    collator.setNumericMode(true);
    EXPECT_LT(collator.compare("Episode 2", "Episode 10"), 0);
    EXPECT_EQ(sorted({"Episode 10.mkv", "Episode 1.mkv", "Episode 2.mkv", "Episode 09.mkv"}, collator),
              (std::vector<std::string>{"Episode 1.mkv", "Episode 2.mkv", "Episode 09.mkv", "Episode 10.mkv"}));
}

TEST(CollatorTest, CaseInsensitiveUsesSecondaryStrength)
{
    Collator collator(Locale("en_US"));
    EXPECT_LT(collator.compare("abc", "ABC"), 0);
    collator.setCaseSensitive(false);
    EXPECT_EQ(collator.compare("abc", "ABC"), 0);
    // Accents still count at secondary strength.
    EXPECT_NE(collator.compare("resume", "r\xC3\xA9sum\xC3\xA9"), 0);
}

TEST(CollatorTest, IgnorePunctuationSkipsSpacesAndSymbols)
{
    Collator collator(Locale("en_US"));
    EXPECT_NE(collator.compare("a-b", "ab"), 0);
    collator.setIgnorePunctuation(true);
    EXPECT_EQ(collator.compare("a-b", "ab"), 0);
}

TEST(CollatorTest, EmptyStringsSortFirst)
{
    const Collator collator(Locale("en_US"));
    EXPECT_EQ(collator.compare("", ""), 0);
    EXPECT_LT(collator.compare("", "a"), 0);
    EXPECT_GT(collator.compare("a", ""), 0);
}

TEST(CollatorTest, CLocaleComparesBytes)
{
    Collator collator(Locale("C"));
    EXPECT_LT(collator.compare("B", "a"), 0);
    collator.setCaseSensitive(false);
    EXPECT_GT(collator.compare("B", "a"), 0);
    EXPECT_EQ(collator.compare("ABC", "abc"), 0);
}

TEST(CollatorTest, ChineseFollowsPinyin)
{
    const Collator collator(Locale("zh_CN"));
    // "Bei" before "Shang" in pinyin order.
    EXPECT_LT(collator.compare("\xE5\x8C\x97\xE4\xBA\xAC", "\xE4\xB8\x8A\xE6\xB5\xB7"), 0);
}

TEST(CollatorTest, SortKeysOrderAsCompare)
{
    Collator collator(Locale("en_US"));
    collator.setNumericMode(true);
    collator.setCaseSensitive(false);
    const std::vector<std::string> names = {"file 10", "File 9", "file 1", "FILE 100", "file 9b", "\xC3\xA9t\xC3\xA9"};
    for (const std::string& a : names) {
        for (const std::string& b : names) {
            const int byKey = collator.sortKey(a).compare(collator.sortKey(b));
            const int byCompare = collator.compare(a, b);
            EXPECT_EQ(byKey < 0, byCompare < 0) << a << " vs " << b;
            EXPECT_EQ(byKey == 0, byCompare == 0) << a << " vs " << b;
        }
    }
}

TEST(CollatorTest, CopiesKeepOptionsAndChangeAlone)
{
    Collator numeric(Locale("en_US"));
    numeric.setNumericMode(true);
    EXPECT_LT(numeric.compare("2", "10"), 0);
    Collator copy = numeric;
    EXPECT_LT(copy.compare("2", "10"), 0);
    copy.setNumericMode(false);
    EXPECT_GT(copy.compare("2", "10"), 0);
    EXPECT_LT(numeric.compare("2", "10"), 0);
}

TEST(CollatorTest, SharedAcrossThreads)
{
    Collator collator(Locale("en_US"));
    collator.setNumericMode(true);
    std::vector<std::thread> threads;
    std::vector<int> failures(4, 0);
    for (int t = 0; t < 4; ++t) {
        threads.emplace_back([&, t] {
            for (int i = 0; i < 2000; ++i) {
                if (collator.compare("Episode 2", "Episode 10") >= 0)
                    ++failures[t];
            }
        });
    }
    for (std::thread& thread : threads)
        thread.join();
    for (int count : failures)
        EXPECT_EQ(count, 0);
}
