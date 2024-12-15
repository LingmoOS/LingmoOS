// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "stub.h"
#include "addr_pri.h"

#include <QtTest>
#include <QColor>
#include <QDebug>
#include <gtest/gtest.h>
#include "../../src/widgets/zoomIndicator.h"


using namespace testing;
ACCESS_PRIVATE_FUN(ZoomIndicator, void(), paintGL);

class ZoomIndicatorTest: public testing::Test, public QObject
{

public:
    ZoomIndicator *zoomindicator;
    virtual void SetUp() override
    {
        std::cout << "start ZoomIndicatorTest" << std::endl;
        zoomindicator = new ZoomIndicator(nullptr);

    }

    virtual void TearDown() override
    {
        delete zoomindicator;
        std::cout << "end ZoomIndicatorTest" << std::endl;
    }
};

//TEST_F(ZoomIndicatorTest, ButtonClicked)
//{
//    zoomindicator->showMagnifier(QPoint(100, 100));
//}

TEST_F(ZoomIndicatorTest, paintEvent)
{
    //QPaintEvent *paintEvent = new QPaintEvent(QRect());
    call_private_fun::ZoomIndicatorpaintGL(*zoomindicator);

    //delete paintEvent;
}
