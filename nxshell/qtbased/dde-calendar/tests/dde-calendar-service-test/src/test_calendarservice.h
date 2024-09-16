// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CALENDARSERVICE_H
#define TEST_CALENDARSERVICE_H

#include "calendarservice.h"
#include <QObject>
#include <gtest/gtest.h>

class test_calendarservice: public::testing::Test
{
public:
    test_calendarservice();

    virtual void SetUp()
    {
        mService = new CalendarService();
    }

    virtual void TearDown()
    {
        delete mService;
        mService = nullptr;
    }
protected:
    CalendarService *mService = nullptr;
};

#endif // TEST_CALENDARSERVICE_H
