// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ut_dutil.h"

#include <QTest>

#include "util/dtimeunitformatter.h"
#include "util/ddisksizeformatter.h"

DCORE_USE_NAMESPACE

void ut_DUtil::SetUpTestCase()
{
    //qDebug() << "*****************" << __FUNCTION__;
}

void ut_DUtil::TearDownTestCase()
{
    //qDebug() << "*****************" << __FUNCTION__;
}

void ut_DUtil::SetUp()
{
    QDir dir("/tmp/etc/");
    if (!dir.exists())
        dir.mkdir("/tmp/etc/");
}
void ut_DUtil::TearDown()
{
    QDir dir("/tmp/etc/");
    if (dir.exists())
        dir.remove("/tmp/etc/");
}


TEST_F(ut_DUtil, testTimeFormatter)
{
    const DTimeUnitFormatter timeFormatter;

    // 3600 seconds == 1 hour
    const auto r0 = timeFormatter.format(3600, DTimeUnitFormatter::Seconds);
    ASSERT_TRUE(qFuzzyCompare(r0.first, 1) && r0.second == DTimeUnitFormatter::Hour);

    // 86400 seconds == 1 day
    const auto r1 = timeFormatter.format(86400, DTimeUnitFormatter::Seconds);
    ASSERT_TRUE(qFuzzyCompare(r1.first, 1) && r1.second == DTimeUnitFormatter::Day);

    // 129600 seconds == 1.5 day
    const auto r3 = timeFormatter.format(129600, DTimeUnitFormatter::Seconds);
    ASSERT_TRUE(qFuzzyCompare(1.5, r3.first) && r3.second == DTimeUnitFormatter::Day);

    // 1.5 day == 36 hours
    const auto r4 = timeFormatter.formatAs(1.5, DTimeUnitFormatter::Day, DTimeUnitFormatter::Hour);
    ASSERT_TRUE(qFuzzyCompare(r4, 36));
}

TEST_F(ut_DUtil, testTimeFormatterList)
{
    const DTimeUnitFormatter timeFormatter;

    // 135120.5 Minutes == 93 days + 20 hours + 30 seconds
    const auto r = timeFormatter.formatAsUnitList(135120.5, DTimeUnitFormatter::Minute);
    ASSERT_TRUE(qFuzzyCompare(r[0].first, 93) && r[0].second == DTimeUnitFormatter::Day);
    ASSERT_TRUE(qFuzzyCompare(r[1].first, 20) && r[1].second == DTimeUnitFormatter::Hour);
    ASSERT_TRUE(qFuzzyCompare(r[2].first, 30) && r[2].second == DTimeUnitFormatter::Seconds);
}

TEST_F(ut_DUtil, testDiskFormatter)
{
    const DDiskSizeFormatter diskFormatter1000 = DDiskSizeFormatter();

    // 1000 K == 1 M
    const auto i0 = diskFormatter1000.format(1000, DDiskSizeFormatter::K);
    ASSERT_TRUE(qFuzzyCompare(i0.first, 1) && i0.second == DDiskSizeFormatter::M);

    // 1000 K == 1000000 B
    const auto i1 = diskFormatter1000.formatAs(1000, DDiskSizeFormatter::K, DDiskSizeFormatter::B);
    ASSERT_TRUE(qFuzzyCompare(i1, 1000000));
}

TEST_F(ut_DUtil, testDiskFormatterList)
{
    const DDiskSizeFormatter diskFormatter = DDiskSizeFormatter();

    // 1351223412.1234 KB == 1 TB + 351 GB + 223 MB + 412 KB + 123.4 B
    const auto r = diskFormatter.formatAsUnitList(1351223412.1234, DDiskSizeFormatter::K);
    ASSERT_TRUE(qFuzzyCompare(r[0].first, 1) && r[0].second == DDiskSizeFormatter::T);
    ASSERT_TRUE(qFuzzyCompare(r[1].first, 351) && r[1].second == DDiskSizeFormatter::G);
    ASSERT_TRUE(qFuzzyCompare(r[2].first, 223) && r[2].second == DDiskSizeFormatter::M);
    ASSERT_TRUE(qFuzzyCompare(r[3].first, 412) && r[3].second == DDiskSizeFormatter::K);

    // TODO: test failed
    //    Q_ASSERT(r[4].first == 123.4 && r[4].second == DiskSizeFormatter::B);
}

TEST_F(ut_DUtil, testDiskFormatter1024)
{
    const DDiskSizeFormatter diskFormatter = DDiskSizeFormatter().rate(1024);

    // 1024 K == 1 M
    const auto d0 = diskFormatter.format(1024, DDiskSizeFormatter::K);
    ASSERT_TRUE(qFuzzyCompare(d0.first, 1) && d0.second == DDiskSizeFormatter::M);

    // 100000000000 B == 93.13225746154785 G
    const auto d1 = diskFormatter.format(100000000000, DDiskSizeFormatter::B);
    ASSERT_TRUE(qFuzzyCompare(93.13225746154785, d1.first) && d1.second == DDiskSizeFormatter::G);

    // 100000000000 B == 0.09094947017729282 T
    const auto d2 = diskFormatter.formatAs(100000000000, DDiskSizeFormatter::B, DDiskSizeFormatter::T);
    ASSERT_TRUE(qFuzzyCompare(0.09094947017729282, d2));
}

