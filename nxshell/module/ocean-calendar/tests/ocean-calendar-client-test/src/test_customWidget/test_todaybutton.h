// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_TODAYBUTTON_H
#define TEST_TODAYBUTTON_H

#include <QObject>
#include <gtest/gtest.h>
#include "customWidget/todaybutton.h"

class test_todaybutton : public::QObject, public::testing::Test
{
    Q_OBJECT
public:
    test_todaybutton();
    void SetUp() override;
    void TearDown() override;
protected:
    CTodayButton *mTodayButton = nullptr;
};

#endif // TEST_TODAYBUTTON_H
