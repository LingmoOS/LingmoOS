// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../deb-installer/view/widgets/droundbgframe.h"

#include <stub.h>
#include <ut_Head.h>

#include <QDebug>
#include <QPaintEvent>

#include <gtest/gtest.h>

class DRoundBgFrame_UT : public UT_HEAD
{
public:
    // 添加日志
    static void SetUpTestCase() { qDebug() << "SetUpTestCase" << endl; }
    static void TearDownTestCase() { qDebug() << "TearDownTestCase" << endl; }
    void SetUp()  // TEST跑之前会执行SetUp
    {
        qDebug() << "SetUp" << endl;
    }
    void TearDown()  // TEST跑完之后会执行TearDown
    {
    }
    DRoundBgFrame *frame;
};

TEST_F(DRoundBgFrame_UT, paintEvent_UT)
{
    frame = new DRoundBgFrame(nullptr, 10, 10);
    QPaintEvent paint(QRect(frame->rect()));
    frame->paintEvent(&paint);
    ASSERT_EQ(10, frame->m_bgOffsetTop);
    ASSERT_EQ(10, frame->m_bgOffsetBottom);
    delete frame;
}
