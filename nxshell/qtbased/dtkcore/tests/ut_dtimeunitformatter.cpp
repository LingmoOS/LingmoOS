// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "util/dtimeunitformatter.h"
#include "filesystem/dtrashmanager.h"

DCORE_USE_NAMESPACE

class ut_DTimeUnitFormatter : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
    DTimeUnitFormatter timeUnitFormatter;
};

void ut_DTimeUnitFormatter::SetUp()
{
}

void ut_DTimeUnitFormatter::TearDown()
{
}

TEST_F(ut_DTimeUnitFormatter, testDTimeUnitFormatterFormatAs)
{
    qreal result0 = timeUnitFormatter.formatAs(120, DTimeUnitFormatter::Seconds, DTimeUnitFormatter::Minute);
    ASSERT_TRUE(qFuzzyCompare(result0, 2));
    qreal result1 = timeUnitFormatter.formatAs(2, DTimeUnitFormatter::Minute, DTimeUnitFormatter::Seconds);
    ASSERT_TRUE(qFuzzyCompare(result1, 120));
    qreal result2 = timeUnitFormatter.formatAs(2, DTimeUnitFormatter::Seconds, DTimeUnitFormatter::Seconds);
    ASSERT_TRUE(qFuzzyCompare(result2, 2));
}

TEST_F(ut_DTimeUnitFormatter, testDTimeUnitFormatterFormat)
{
    QPair<double, int> result = timeUnitFormatter.format(120, DTimeUnitFormatter::Seconds);
    ASSERT_TRUE(qFuzzyCompare(result.first, 2));
    ASSERT_EQ(result.second, DTimeUnitFormatter::Minute);
}

TEST_F(ut_DTimeUnitFormatter, testDTimeUnitFormatterFormatAsUnitList)
{
    QList<QPair<double, int>> result = timeUnitFormatter.formatAsUnitList(121, DTimeUnitFormatter::Seconds);
    ASSERT_TRUE(qFuzzyCompare(result[0].first, 121));
    ASSERT_EQ(result[0].second, DTimeUnitFormatter::Seconds);
}

TEST_F(ut_DTimeUnitFormatter, testDTimeUnitFormatterUnitStr)
{
    ASSERT_EQ(timeUnitFormatter.unitStr(DTimeUnitFormatter::Seconds), "s");
    ASSERT_EQ(timeUnitFormatter.unitStr(DTimeUnitFormatter::Minute), "m");
    ASSERT_EQ(timeUnitFormatter.unitStr(DTimeUnitFormatter::Hour), "h");
    ASSERT_EQ(timeUnitFormatter.unitStr(DTimeUnitFormatter::Day), "d");
}
