// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CALENDARHUANGLI_H
#define TEST_CALENDARHUANGLI_H

#include "calendarhuangli.h"
#include "gtest/gtest.h"
#include <QObject>

class test_calendarhuangli : public QObject, public::testing::Test
{
public:
    test_calendarhuangli();
    ~test_calendarhuangli();
protected:
    CalendarHuangLi *calendarHuangLi = nullptr;
};

#endif // TEST_CALENDARHUANGLI_H
