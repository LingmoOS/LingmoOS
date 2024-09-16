// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "BrowserWord.h"
#include "stub.h"

#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

#include <gtest/gtest.h>

using namespace deepin_reader;

/********测试BrowserWord***********/
class TestBrowserWord : public ::testing::Test
{
public:
    virtual void SetUp();

    virtual void TearDown();

protected:
    BrowserWord *m_tester = nullptr;
};
void TestBrowserWord::SetUp()
{
    m_tester = new BrowserWord(nullptr, Word("test", QRectF(0, 0, 20, 10)));
}

void TestBrowserWord::TearDown()
{
    delete m_tester;
}

/**********桩函数*************/
static bool isSelected_stub()
{
    return true;
}

static QString g_funcName;
void setPen_stub(Qt::PenStyle)
{
    g_funcName = __FUNCTION__;
}

void mousePressEvent_stub(QGraphicsSceneMouseEvent *)
{
    g_funcName = __FUNCTION__;
}
void mouseReleaseEvent_stub(QGraphicsSceneMouseEvent *)
{
    g_funcName = __FUNCTION__;
}

/*********测试用例**********/
TEST_F(TestBrowserWord, UT_BrowserWord_setScaleFactor_001)
{
    m_tester->m_scaleFactor = 1;
    m_tester->setScaleFactor(2);
    EXPECT_TRUE(qFuzzyCompare(m_tester->m_scaleFactor, 2));
}

TEST_F(TestBrowserWord, UT_BrowserWord_text_001)
{
    EXPECT_TRUE(m_tester->text() == "test");
}

TEST_F(TestBrowserWord, UT_BrowserWord_setSelectable_001)
{
    m_tester->m_selectable = false;
    m_tester->setSelectable(true);
    EXPECT_TRUE(m_tester->m_selectable);
}

TEST_F(TestBrowserWord, UT_BrowserWord_boundingRect_001)
{
    m_tester->m_scaleFactor = 2;
    EXPECT_TRUE(qFuzzyCompare(m_tester->boundingRect().width(), 42));
}

TEST_F(TestBrowserWord, UT_BrowserWord_boundingBox_001)
{
    EXPECT_TRUE(qFuzzyCompare(m_tester->boundingBox().width(), 20));
}

TEST_F(TestBrowserWord, UT_BrowserWord_paint_001)
{
    Stub s;
    s.set(ADDR(QGraphicsItem, isSelected), isSelected_stub);
    s.set(static_cast<void (QPainter::*)(Qt::PenStyle)>(ADDR(QPainter, setPen)), setPen_stub);

    QPainter *painter = new QPainter;
    QStyleOptionGraphicsItem *option = new QStyleOptionGraphicsItem;
    m_tester->paint(painter, option, nullptr);
    EXPECT_TRUE(g_funcName == "setPen_stub");
    delete painter;
    delete option;
}

TEST_F(TestBrowserWord, UT_BrowserWord_mousePressEvent_001)
{
    typedef void (*fptr)(QGraphicsItem *, QGraphicsSceneMouseEvent *);
    fptr A_foo = (fptr)(&QGraphicsItem::mousePressEvent);   //获取虚函数地址
    Stub s;
    s.set(A_foo, mousePressEvent_stub);

    QGraphicsSceneMouseEvent *e = new QGraphicsSceneMouseEvent;

    m_tester->m_selectable = true;
    m_tester->mousePressEvent(e);
    EXPECT_FALSE(g_funcName == "mousePressEvent_stub");


    m_tester->m_selectable = false;
    m_tester->mousePressEvent(e);
    EXPECT_TRUE(g_funcName == "mousePressEvent_stub");

    delete e;
}

TEST_F(TestBrowserWord, UT_BrowserWord_mouseReleaseEvent_001)
{
    typedef void (*fptr)(QGraphicsItem *, QGraphicsSceneMouseEvent *);
    fptr A_foo = (fptr)(&QGraphicsItem::mouseReleaseEvent);   //获取虚函数地址
    Stub s;
    s.set(A_foo, mouseReleaseEvent_stub);

    QGraphicsSceneMouseEvent *e = new QGraphicsSceneMouseEvent;

    m_tester->m_selectable = true;
    m_tester->mouseReleaseEvent(e);
    EXPECT_FALSE(g_funcName == "mouseReleaseEvent_stub");


    m_tester->m_selectable = false;
    m_tester->mouseReleaseEvent(e);
    EXPECT_TRUE(g_funcName == "mouseReleaseEvent_stub");

    delete e;
}
