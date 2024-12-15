// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_TOUCHGESTUREOPERATION_H
#define TEST_TOUCHGESTUREOPERATION_H

#include <QObject>
#include <widget/touchgestureoperation.h>
#include <gtest/gtest.h>

class test_touchgestureoperation : public ::QObject
    , public ::testing::Test
{
public:
    test_touchgestureoperation();
    ~test_touchgestureoperation();

protected:
    touchGestureOperation *mTouchGestureOperation = nullptr;
};

#endif // TEST_TOUCHGESTUREOPERATION_H
