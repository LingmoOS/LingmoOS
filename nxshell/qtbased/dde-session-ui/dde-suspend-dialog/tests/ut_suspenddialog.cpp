// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "suspenddialog.h"
#undef private

#include <QTest>

#include <gtest/gtest.h>

class UT_SuspendDialog : public testing::Test
{
public:
    void SetUp() override
    {
        widget = new SuspendDialog(QRect());
        manager = new Manager();
    }

    void TearDown() override
    {
        delete widget;
        widget = nullptr;

        delete manager;
        manager = nullptr;
    }

public:
    SuspendDialog *widget = nullptr;
    Manager *manager = nullptr;
};

TEST_F(UT_SuspendDialog, coverageTest)
{
    widget->m_timer.setInterval(10);
    QTest::qWait(200);
}
