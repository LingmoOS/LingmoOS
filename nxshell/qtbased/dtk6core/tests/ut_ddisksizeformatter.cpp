// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "util/ddisksizeformatter.h"

DCORE_USE_NAMESPACE

class ut_DDiskSizeFormatter : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
    DDiskSizeFormatter diskSizeFormatter;
};

void ut_DDiskSizeFormatter::SetUp()
{
}

void ut_DDiskSizeFormatter::TearDown()
{
}

TEST_F(ut_DDiskSizeFormatter, testDDiskSizeFormatterFormatAs)
{
    diskSizeFormatter.rate(1024);
    qreal result0 = diskSizeFormatter.formatAs(2048, DDiskSizeFormatter::B, DDiskSizeFormatter::K);
    ASSERT_TRUE(qFuzzyCompare(result0, 2));
    qreal result1 = diskSizeFormatter.formatAs(2, DDiskSizeFormatter::K, DDiskSizeFormatter::B);
    ASSERT_TRUE(qFuzzyCompare(result1, 2048));
    qreal result2 = diskSizeFormatter.formatAs(2, DDiskSizeFormatter::K, DDiskSizeFormatter::K);
    ASSERT_TRUE(qFuzzyCompare(result2, 2));
}

TEST_F(ut_DDiskSizeFormatter, testDDiskSizeFormatterFormat)
{
    diskSizeFormatter.rate(1024);
    QPair<double, int> result = diskSizeFormatter.format(2048, DDiskSizeFormatter::B);
    ASSERT_TRUE(qFuzzyCompare(result.first, 2));
    ASSERT_EQ(result.second, DDiskSizeFormatter::K);
}

TEST_F(ut_DDiskSizeFormatter, testDDiskSizeFormatterFormatAsUnitList)
{
    diskSizeFormatter.rate(1024);
    QList<QPair<double, int>> result = diskSizeFormatter.formatAsUnitList(2049, DDiskSizeFormatter::K);
    ASSERT_TRUE(qFuzzyCompare(result[0].first, 2));
    ASSERT_EQ(result[0].second, DDiskSizeFormatter::M);
    ASSERT_TRUE(qFuzzyCompare(result[1].first, 1));
    ASSERT_EQ(result[1].second, DDiskSizeFormatter::K);
}

TEST_F(ut_DDiskSizeFormatter, testDDiskSizeFormatterUnitStr)
{
    ASSERT_EQ(diskSizeFormatter.unitStr(DDiskSizeFormatter::B), "B");
    ASSERT_EQ(diskSizeFormatter.unitStr(DDiskSizeFormatter::K), "KB");
    ASSERT_EQ(diskSizeFormatter.unitStr(DDiskSizeFormatter::M), "MB");
    ASSERT_EQ(diskSizeFormatter.unitStr(DDiskSizeFormatter::G), "GB");
    ASSERT_EQ(diskSizeFormatter.unitStr(DDiskSizeFormatter::T), "TB");
}
