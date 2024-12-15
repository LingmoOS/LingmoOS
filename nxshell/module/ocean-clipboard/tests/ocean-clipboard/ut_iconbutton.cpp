// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "clipboardmodel.h"

#include "iconbutton.h"

#include <QtTest>
#include <QSignalSpy>
#include <QDebug>

class TstIconButton : public testing::Test
{
public:
    void SetUp() override
    {
        btn = new IconButton();
    }

    void TearDown() override
    {
        delete btn;
        btn = nullptr;
    }

public:
    IconButton *btn = nullptr;
};

TEST_F(TstIconButton, coverageTest)
{
    btn->setText("abcdefghijklmnopqrstuvwxyz");
    btn->setFocusState(false);
    btn->setBackOpacity(-100);
    btn->setRadius(10000);
}

TEST_F(TstIconButton, method_test)
{
    btn->setBackOpacity(128);
    ASSERT_EQ(btn->backOpacity(), 128);
    btn->setBackOpacity(-1);
    ASSERT_EQ(btn->backOpacity(), 128);
    btn->setBackOpacity(0);
    ASSERT_EQ(btn->backOpacity(), 0);
    btn->setBackOpacity(256);
    ASSERT_EQ(btn->backOpacity(), 0);
    btn->setBackOpacity(255);
    ASSERT_EQ(btn->backOpacity(), 255);
    QTest::qWait(10);

    btn->setRadius(10);
    ASSERT_EQ(btn->radius(), 10);

    btn->setFocusState(true);
    ASSERT_TRUE(btn->focusState());

    btn->setFocusState(false);
    ASSERT_FALSE(btn->focusState());
    QTest::qWait(10);
}

TEST_F(TstIconButton, paintTest)
{
    // 触发paintEvent,看是否正常运行
    btn->show();
    QTest::qWait(10);

    // 模拟enterEvent
    QPoint pos = btn->pos();
    QEnterEvent enterE{pos, pos, btn->mapToGlobal(pos)};
    qApp->sendEvent(btn, &enterE);

    // 模拟leaveEvent
    QEvent leaveE(QEvent::Leave);
    qApp->sendEvent(btn, &leaveE);
}

TEST_F(TstIconButton, keyPressTest)
{
    QSignalSpy spy(btn, SIGNAL(clicked()));

    QTest::mouseClick(btn, Qt::LeftButton);
    ASSERT_TRUE(spy.count() == 1);

    QTest::mouseClick(btn, Qt::RightButton);
    ASSERT_TRUE(spy.count() == 1);

    QTest::mouseDClick(btn, Qt::RightButton);
    ASSERT_TRUE(spy.count() == 1);
}
