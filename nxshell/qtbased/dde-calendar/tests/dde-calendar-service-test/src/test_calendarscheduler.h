// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CALENDARSCHEDULER_H
#define TEST_CALENDARSCHEDULER_H

#include "calendarscheduler.h"
#include <QObject>
#include <gtest/gtest.h>

class test_calendarscheduler: public::testing::Test
{
public:
    test_calendarscheduler();

    virtual void SetUp()
    {
        mCalendar = new CalendarScheduler();
    }

    virtual void TearDown()
    {
        delete mCalendar;
        mCalendar = nullptr;
    }
protected:
    CalendarScheduler *mCalendar = nullptr;
};

#endif // TEST_CALENDARSCHEDULER_H
