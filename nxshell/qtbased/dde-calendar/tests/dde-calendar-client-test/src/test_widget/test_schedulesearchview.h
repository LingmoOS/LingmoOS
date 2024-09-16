// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_SCHEDULESEARCHVIEW_H
#define TEST_SCHEDULESEARCHVIEW_H

#include "widget/schedulesearchview.h"
#include "gtest/gtest.h"
#include <QObject>

class test_schedulesearchview : public QObject
    , public ::testing::Test
{
public:
    test_schedulesearchview();
    ~test_schedulesearchview();
    void SetUp() override;
    void TearDown() override;

protected:
    CScheduleSearchView *mScheduleSearchView = nullptr;
    CScheduleSearchDateItem *mScheduleSearchDateItem = nullptr;
    CScheduleListWidget *mScheduleListWidget = nullptr;
    CScheduleSearchItem *mScheduleSearchItem = nullptr;
};

#endif // TEST_SCHEDULESEARCHVIEW_H
