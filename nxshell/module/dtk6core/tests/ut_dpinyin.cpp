// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "util/dpinyin.h"
#include <algorithm>
DCORE_USE_NAMESPACE

class ut_DPinyin : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
};

void ut_DPinyin::SetUp()
{
}

void ut_DPinyin::TearDown()
{
}

TEST_F(ut_DPinyin, testChinese2Pinyin)
{
    auto pinyin = Chinese2Pinyin("你好, world");

    ASSERT_EQ(pinyin, "ni3hao3, world");
}

TEST_F(ut_DPinyin, pinyin)
{
    bool ok = false;
    QString words("深度音乐");
    QStringList tones = {"shēndùyīnlè", "shēndùyīnyuè", "shēndùyīnyào",
                         "shēndùyīnlào", "shēnduóyīnlè", "shēnduóyīnyuè",
                         "shēnduóyīnyào", "shēnduóyīnlào"};
    QStringList noneTones = {"shenduyinle", "shenduyinyue", "shenduyinyao",
                             "shenduyinlao", "shenduoyinle", "shenduoyinyue",
                             "shenduoyinyao", "shenduoyinlao"};
    QStringList numTones = {"shen1du4yin1le4", "shen1du4yin1yue4", "shen1du4yin1yao4",
                            "shen1du4yin1lao4", "shen1duo2yin1le4", "shen1duo2yin1yue4",
                            "shen1duo2yin1yao4", "shen1duo2yin1lao4"};

    QStringList py = pinyin(words, TS_ToneNum, &ok);
    ASSERT_TRUE(ok);
    bool isPermutation = std::is_permutation(py.begin() , py.end(), numTones.begin());
    ASSERT_TRUE(isPermutation);

    py = pinyin(words, TS_NoneTone, &ok);

    ASSERT_TRUE(ok);
    isPermutation = std::is_permutation(py.begin() , py.end(), noneTones.begin());
    ASSERT_TRUE(isPermutation);

    py = pinyin(words, TS_Tone, &ok);
    ASSERT_TRUE(ok);
    isPermutation = std::is_permutation(py.begin() , py.end(), tones.begin());
    ASSERT_TRUE(isPermutation);
}

TEST_F(ut_DPinyin, firstLetters)
{
    QStringList letters = {"sdyl", "sdyy"};
    QString words("深度音乐");
    QStringList ls = firstLetters(words);

    bool isPermutation = std::is_permutation(ls.begin() , ls.end(), letters.begin());
    ASSERT_TRUE(isPermutation);
}

