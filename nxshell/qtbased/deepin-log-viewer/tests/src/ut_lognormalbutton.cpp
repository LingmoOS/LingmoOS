// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lognormalbutton.h"

#include <gtest/gtest.h>
#include <stub.h>

#include <QKeyEvent>
#include <QDebug>
#include <QPaintEvent>
TEST(LogNormalButton_Constructor_UT, LogNormalButton_Constructor_UT_001)
{
    LogNormalButton *p = new LogNormalButton(nullptr);
    EXPECT_NE(p, nullptr);

    p->deleteLater();
}

TEST(LogNormalButton_Constructor_UT, LogNormalButton_Constructor_UT_002)
{
    LogNormalButton *p = new LogNormalButton("aa", nullptr);
    EXPECT_NE(p, nullptr);

    p->deleteLater();
}

TEST(LogNormalButton_Constructor_UT, LogNormalButton_Constructor_UT_003)
{
    LogNormalButton *p = new LogNormalButton(QIcon(""), "aa", nullptr);
    EXPECT_NE(p, nullptr);

    p->deleteLater();
}

class LogNormalButton_keyPressEvent_UT_Param
{
public:
    explicit LogNormalButton_keyPressEvent_UT_Param(int iKey)
    {
        key = iKey;
    }
    int key;
};

class LogNormalButton_keyPressEvent_UT : public ::testing::TestWithParam<LogNormalButton_keyPressEvent_UT_Param>
{
};

INSTANTIATE_TEST_CASE_P(LogNormalButton, LogNormalButton_keyPressEvent_UT, ::testing::Values(LogNormalButton_keyPressEvent_UT_Param(Qt::Key_Enter), LogNormalButton_keyPressEvent_UT_Param(Qt::Key_Return), LogNormalButton_keyPressEvent_UT_Param(Qt::Key_0)));

TEST_P(LogNormalButton_keyPressEvent_UT, LogNormalButton_keyPressEvent_UT_001)
{
    LogNormalButton_keyPressEvent_UT_Param param = GetParam();
    LogNormalButton *p = new LogNormalButton(nullptr);
    EXPECT_NE(p, nullptr);

    QKeyEvent keyEvent(QEvent::KeyPress, param.key, Qt::NoModifier);
    p->keyPressEvent(&keyEvent);
    p->deleteLater();
}

class LogNormalButton_keyReleaseEvent_UT_Param
{
public:
    explicit LogNormalButton_keyReleaseEvent_UT_Param(int iKey)
    {
        key = iKey;
    }
    int key;
};

class LogNormalButton_keyReleaseEvent_UT : public ::testing::TestWithParam<LogNormalButton_keyReleaseEvent_UT_Param>
{
};

INSTANTIATE_TEST_CASE_P(LogNormalButton, LogNormalButton_keyReleaseEvent_UT, ::testing::Values(LogNormalButton_keyReleaseEvent_UT_Param(Qt::Key_Enter), LogNormalButton_keyReleaseEvent_UT_Param(Qt::Key_Return), LogNormalButton_keyReleaseEvent_UT_Param(Qt::Key_0)));

TEST_P(LogNormalButton_keyReleaseEvent_UT, LogNormalButton_keyReleaseEvent_UT_001)
{
    LogNormalButton_keyReleaseEvent_UT_Param param = GetParam();
    LogNormalButton *p = new LogNormalButton(nullptr);
    EXPECT_NE(p, nullptr);

    QKeyEvent keyEvent(QEvent::KeyPress, param.key, Qt::NoModifier);
    p->keyReleaseEvent(&keyEvent);
    p->deleteLater();
}

//TEST(LogNormalButton_paintEvent_UT, LogNormalButton_paintEventr_UT)
//{
//    LogNormalButton *p = new LogNormalButton("aa", nullptr);
//    EXPECT_NE(p, nullptr);
//    p->paintEvent(new QPaintEvent(p->rect()));
//    p->deleteLater();
//}

class LogNormalButton_focusInEvent_UT_Param
{
public:
    explicit LogNormalButton_focusInEvent_UT_Param(Qt::FocusReason iReason)
        : reason(iReason)
    {
    }
    Qt::FocusReason reason;
};

class LogNormalButton_focusInEvent_UT : public ::testing::TestWithParam<LogNormalButton_focusInEvent_UT_Param>
{
};

INSTANTIATE_TEST_CASE_P(LogNormalButton, LogNormalButton_focusInEvent_UT, ::testing::Values(LogNormalButton_focusInEvent_UT_Param(Qt::ActiveWindowFocusReason), LogNormalButton_focusInEvent_UT_Param(Qt::NoFocusReason)));

TEST_P(LogNormalButton_focusInEvent_UT, LogNormalButton_focusInEvent_UT_001)
{
    LogNormalButton_focusInEvent_UT_Param param = GetParam();
    LogNormalButton *p = new LogNormalButton(nullptr);
    EXPECT_NE(p, nullptr);

    QFocusEvent *focusEvent = new QFocusEvent(QEvent::FocusIn, param.reason);
    p->focusInEvent(focusEvent);
    delete focusEvent;
    p->deleteLater();
}
