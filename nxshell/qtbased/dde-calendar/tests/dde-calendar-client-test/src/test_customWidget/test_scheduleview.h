// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_SCHEDULEVIEW_H
#define TEST_SCHEDULEVIEW_H

#include <QObject>
#include <gtest/gtest.h>
#include "customWidget/scheduleview.h"

class test_scheduleview : public::QObject, public::testing::Test
{
public:
    test_scheduleview();
    ~test_scheduleview();
protected:
    CScheduleView *mScheduleView = nullptr;
};

#endif // TEST_SCHEDULEVIEW_H
