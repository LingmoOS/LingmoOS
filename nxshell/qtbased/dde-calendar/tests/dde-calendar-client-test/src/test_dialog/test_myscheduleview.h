// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_MYSCHEDULEVIEW_H
#define TEST_MYSCHEDULEVIEW_H

#include <QObject>
#include <gtest/gtest.h>
#include "dialog/myscheduleview.h"
class test_myscheduleview : public::QObject, public::testing::Test
{
public:
    test_myscheduleview();
    ~test_myscheduleview();
protected:
    CMyScheduleView *mScheduleView = nullptr;
};

#endif // TEST_MYSCHEDULEVIEW_H
