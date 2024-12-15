// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QRect>
#include <QPixmap>
#include <QDebug>
#include "../../src/utils/screengrabber.h"
#include "../../src/utils.h"
#include "stub.h"


using namespace testing;

class ScreenGrabberTest:public testing::Test{

public:
    Stub stub;
    ScreenGrabber screenGrabber;
    virtual void SetUp() override{
        std::cout << "start ScreenGrabberTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end ScreenGrabberTest" << std::endl;
        Utils::isWaylandMode = false;
    }
};

bool waylandDectected_stub(void *obj)
{
    return true;
}

TEST_F(ScreenGrabberTest, grabEntireDesktop)
{
    bool ok = false;
    QRect rect(0, 0, 600, 400);
    QPixmap pix = screenGrabber.grabEntireDesktop(ok, rect, 0);
    qDebug() << pix.rect();
    EXPECT_EQ(true, ok);
}
TEST_F(ScreenGrabberTest, grabEntireDesktop_wayland)
{
    Utils::isWaylandMode = true;
    bool ok = true;
    QRect rect;
    QPixmap pix = screenGrabber.grabEntireDesktop(ok, rect, 0);
    qDebug() << pix.rect();
    EXPECT_EQ(false, ok);
}
