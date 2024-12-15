// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "monitorindicator.h"

#include <QApplication>
#include <QMimeData>

class UT_MonitorIndicator : public testing::Test
{
public:
    void SetUp() override
    {
        // widget = new MonitorIndicator();
    }

    void TearDown() override
    {
//        delete widget;
//        widget = nullptr;
    }

public:
    MonitorIndicator *widget = nullptr;
};

TEST_F(UT_MonitorIndicator, coverageTest)
{

}
