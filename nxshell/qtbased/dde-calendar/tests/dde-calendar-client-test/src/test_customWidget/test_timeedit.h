// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_TIMEEDIT_H
#define TEST_TIMEEDIT_H

#include <QObject>
#include <gtest/gtest.h>
#include "customWidget/timeedit.h"

class test_timeedit : public QObject
    , public testing::Test
{
public:
    test_timeedit();
    ~test_timeedit();
protected:
    CTimeEdit *mTimeEdit = nullptr;
};

#endif // TEST_TIMEEDIT_H
