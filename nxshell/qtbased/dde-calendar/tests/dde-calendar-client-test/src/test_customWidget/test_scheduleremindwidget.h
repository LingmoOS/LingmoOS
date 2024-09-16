// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_SCHEDULEREMINDWIDGET_H
#define TEST_SCHEDULEREMINDWIDGET_H

#include <QObject>
#include <gtest/gtest.h>
#include "customWidget/scheduleRemindWidget.h"

class test_scheduleremindwidget : public::QObject, public::testing::Test
{
public:
    test_scheduleremindwidget();
    ~test_scheduleremindwidget();
protected:
    ScheduleRemindWidget *mScheduleRemindWidget = nullptr;
    CenterWidget *mCenterWidget = nullptr;
};

#endif // TEST_SCHEDULEREMINDWIDGET_H
