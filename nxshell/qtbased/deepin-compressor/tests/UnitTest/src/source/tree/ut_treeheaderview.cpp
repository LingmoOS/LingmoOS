// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treeheaderview.h"

#include "gtest/src/stub.h"

#include <QTest>
#include <QSignalSpy>
#include <DApplication>

#include <gtest/gtest.h>

/*******************************函数打桩************************************/
// 对QHeaderView的sectionSizeFromContents进行打桩
QSize qHeaderView_sectionSizeFromContents_stub(int logicalIndex)
{
    return QSize(30, 30);
}
/*******************************函数打桩************************************/


class UT_PreviousLabel : public ::testing::Test
{
public:
    UT_PreviousLabel(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new PreviousLabel;
//        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    PreviousLabel *m_tester;
};

TEST_F(UT_PreviousLabel, initTest)
{

}

TEST_F(UT_PreviousLabel, test_setPrePath)
{
    m_tester->setPrePath("123");
    qInfo() << m_tester->text();
    EXPECT_EQ(m_tester->text(), "     .. Back to: /");
}

TEST_F(UT_PreviousLabel, test_paintEvent)
{
    QRect f;
    QPaintEvent *paint = new QPaintEvent(f);
    m_tester->paintEvent(paint);
    delete paint;
}

TEST_F(UT_PreviousLabel, test_mouseDoubleClickEvent)
{
    QSignalSpy spy(m_tester, SIGNAL(doubleClickedSignal()));
    QTest::mouseDClick(m_tester, Qt::LeftButton, Qt::KeyboardModifiers(), QPoint());
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_PreviousLabel, test_enterEvent)
{
    QEvent *event = new QEvent(QEvent::Enter);
    m_tester->enterEvent(event);
    delete event;
    EXPECT_EQ(m_tester->focusIn_, true);
}

TEST_F(UT_PreviousLabel, test_leaveEvent)
{
    QEvent *event = new QEvent(QEvent::Leave);
    m_tester->leaveEvent(event);
    delete event;
    EXPECT_EQ(m_tester->focusIn_, false);
}

TEST_F(UT_PreviousLabel, test_focusInEvent)
{
    QFocusEvent *event = new QFocusEvent(QEvent::FocusIn);
    m_tester->focusInEvent(event);
    delete event;
    EXPECT_EQ(m_tester->focusIn_, true);
}

TEST_F(UT_PreviousLabel, test_focusOutEvent)
{
    QFocusEvent *event = new QFocusEvent(QEvent::FocusOut);
    m_tester->focusOutEvent(event);
    delete event;
    EXPECT_EQ(m_tester->focusIn_, false);
}

TEST_F(UT_PreviousLabel, test_keyPressEvent)
{
    QSignalSpy spy(m_tester, SIGNAL(doubleClickedSignal()));
    QTest::keyPress(m_tester, Qt::Key_Enter);
    EXPECT_EQ(spy.count(), 1);
    QTest::keyPress(m_tester, Qt::Key_Tab);
}



class UT_TreeHeaderView : public ::testing::Test
{
public:
    UT_TreeHeaderView(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new TreeHeaderView(Qt::Horizontal);
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    TreeHeaderView *m_tester;
};

TEST_F(UT_TreeHeaderView, initTest)
{

}

TEST_F(UT_TreeHeaderView, test_sizeHint)
{
    typedef QSize(*fptr)(QHeaderView *, int);
    fptr A_foo = (fptr)(&QHeaderView::sectionSizeFromContents);   // 获取虚函数地址
    Stub stub;
    stub.set(A_foo, qHeaderView_sectionSizeFromContents_stub);

    EXPECT_EQ(m_tester->sizeHint().width(), 30);
}

TEST_F(UT_TreeHeaderView, test_getpreLbl)
{
    EXPECT_EQ(m_tester->getpreLbl(), m_tester->m_pPreLbl);
}

TEST_F(UT_TreeHeaderView, test_setPreLblVisible_001)
{
    m_tester->setPreLblVisible(true);
    EXPECT_EQ(m_tester->height(), 76);
}

TEST_F(UT_TreeHeaderView, test_setPreLblVisible_002)
{
    m_tester->setPreLblVisible(false);
    EXPECT_EQ(m_tester->height(), 38);
}

TEST_F(UT_TreeHeaderView, test_resizeEvent)
{
    QResizeEvent *event = new QResizeEvent(QSize(100, 30), QSize(50, 30));
    m_tester->resizeEvent(event);
    delete event;
    EXPECT_EQ(m_tester->m_pPreLbl->width(), 80);
}
