// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../deb-installer/view/widgets/ShowInstallInfoTextEdit.h"
#include <stub.h>

#include <QGestureEvent>
#include <QMouseEvent>
#include <QScrollBar>

#include <gtest/gtest.h>

Qt::MouseEventSource stud_source()
{
    return Qt::MouseEventSynthesizedByQt;
}

class ut_showInstallInfoTextEdit_Test : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() { m_infoTextEdit = new ShowInstallInfoTextEdit; }
    void TearDown() { delete m_infoTextEdit; }

    ShowInstallInfoTextEdit *m_infoTextEdit = nullptr;
};

TEST_F(ut_showInstallInfoTextEdit_Test, ShowInstallInfoTextEdit_UT_slideGesture)
{
    int value = m_infoTextEdit->verticalScrollBar()->value();
    m_infoTextEdit->slideGesture(5);
    EXPECT_EQ(0 + value, m_infoTextEdit->verticalScrollBar()->value());
}

QString ut_selectedText()
{
    return "//";
}

TEST_F(ut_showInstallInfoTextEdit_Test, ShowInstallInfoTextEdit_UT_onSelectionArea)
{
    m_infoTextEdit->m_gestureAction = ShowInstallInfoTextEdit::GA_tap;
    Stub stub;
    stub.set(ADDR(QTextCursor, selectedText), ut_selectedText);
    m_infoTextEdit->onSelectionArea();
    EXPECT_EQ(ShowInstallInfoTextEdit::GA_tap, m_infoTextEdit->m_gestureAction);
    EXPECT_EQ("//", m_infoTextEdit->textCursor().selectedText());
}

TEST(FlashTween_TEST, FlashTween_UT_start)
{
    FlashTween flash;
    EXPECT_EQ(flash.sinusoidalEaseOut(1.0, 1.0, 1.0, 1.0), 1.0 * sin(1.0 / 1.0 * (3.14 / 2)) + 1.0);
    flash.start(0, 0, 1, 0, 0);
    flash.start(0, 0, 1, 1, 0);
    flash.m_timer->start(5);
    usleep(5000);
    flash.m_timer->stop();
}

int states = 1;

int state()
{
    return states;
}

TEST_F(ut_showInstallInfoTextEdit_Test, ShowInstallInfoTextEdit_UT_tapGestureTriggered)
{
    Stub stub;
    stub.set(ADDR(QGesture, state), state);

    QTapGesture *tap = new QTapGesture;
    tap->setPosition(QPointF(0, 0));
    QTapAndHoldGesture *tapAndHold = new QTapAndHoldGesture;
    while (states <= 4) {
        m_infoTextEdit->tapGestureTriggered(tap);
        if (states != 2 && states != 4)
            m_infoTextEdit->tapAndHoldGestureTriggered(tapAndHold);
        states++;
    }
    EXPECT_FALSE(m_infoTextEdit->m_slideContinue);
    EXPECT_EQ(ShowInstallInfoTextEdit::GA_slide, m_infoTextEdit->m_gestureAction);
    delete tap;
    delete tapAndHold;
}

TEST_F(ut_showInstallInfoTextEdit_Test, ShowInstallInfoTextEdit_UT_gestureEvent)
{
    QGesture *gesture = new QGesture();

    QGestureEvent gestureEvent(QList<QGesture *>{gesture, gesture});
    EXPECT_TRUE(m_infoTextEdit->gestureEvent(&gestureEvent));
    delete gesture;
}

TEST_F(ut_showInstallInfoTextEdit_Test, ShowInstallInfoTextEdit_UT_mouseReleaseEvent)
{
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease,
                             QPoint(0, 0),
                             QPoint(0, 0),
                             QPoint(0, 0),
                             Qt::LeftButton,
                             Qt::LeftButton,
                             Qt::NoModifier,
                             Qt::MouseEventSynthesizedByQt);
    m_infoTextEdit->m_gestureAction = ShowInstallInfoTextEdit::GA_slide;
    m_infoTextEdit->mouseReleaseEvent(&releaseEvent);
    EXPECT_EQ(ShowInstallInfoTextEdit::GA_null, m_infoTextEdit->m_gestureAction);
}

TEST_F(ut_showInstallInfoTextEdit_Test, ShowInstallInfoTextEdit_UT_mouseMoveEvent)
{
    QMouseEvent moveEvent(QEvent::MouseMove, QPoint(0, 0), QPoint(10, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_infoTextEdit->mouseMoveEvent(&moveEvent);
    ASSERT_EQ(0.0, m_infoTextEdit->m_lastMousepos);
}

TEST_F(ut_showInstallInfoTextEdit_Test, ShowInstallInfoTextEdit_UT_mouseMoveEvent_source)
{
    Stub stub;
    stub.set(ADDR(QMouseEvent, source), stud_source);

    m_infoTextEdit->m_gestureAction = ShowInstallInfoTextEdit::GA_slide;
    QMouseEvent moveEvent(QEvent::MouseMove, QPoint(0, 0), QPoint(10, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_infoTextEdit->mouseMoveEvent(&moveEvent);
    ASSERT_EQ(0.0, m_infoTextEdit->m_lastMousepos);
}
