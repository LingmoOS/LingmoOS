// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "window.h"
#undef private

#include <QTest>

#include <gtest/gtest.h>

class UT_Window : public testing::Test
{
public:
    void SetUp() override
    {
        widget = new Window();
        adaptor = new LowPowerAdaptor(widget);
    }

    void TearDown() override
    {
        if (widget) {
            delete widget;
            widget = nullptr;
        }
    }

public:
    Window *widget = nullptr;
    LowPowerAdaptor *adaptor = nullptr;
};

TEST_F(UT_Window, coverageTest)
{
    widget->HideShowToRaise(true);
    widget->m_bSleepLock = false;
    QTest::qWait(500);
    adaptor->Raise();
    adaptor->Quit();
}
