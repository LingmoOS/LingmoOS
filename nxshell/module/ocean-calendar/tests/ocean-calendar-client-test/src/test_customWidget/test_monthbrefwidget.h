// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_MONTHBREFWIDGET_H
#define TEST_MONTHBREFWIDGET_H

#include <QObject>
#include <gtest/gtest.h>
#include "customWidget/monthbrefwidget.h"

class test_monthbrefwidget : public::QObject, public::testing::Test
{
public:
    test_monthbrefwidget();
    ~test_monthbrefwidget();
    void SetUp() override;
    void TearDown() override;
protected:
    MonthBrefWidget *mMonthBrefWidget = nullptr;
    CMonthDayRect *mMonthDayRect = nullptr;
};

#endif // TEST_MONTHBREFWIDGET_H
