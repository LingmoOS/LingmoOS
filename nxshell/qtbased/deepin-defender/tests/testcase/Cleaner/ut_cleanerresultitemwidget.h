// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_CLEANERRESULTITEMWIDGETTEST_H
#define UT_CLEANERRESULTITEMWIDGETTEST_H

#include <gtest/gtest.h>

#include <QObject>

class CleanerResultItemWidgetTest : public testing::Test
    , public QObject
{
public:
    CleanerResultItemWidgetTest();

protected:
    virtual void SetUp()
    {
    }
    virtual void TearDown()
    {
    }
};

#endif // UT_CLEANERRESULTITEMWIDGETTEST_H
