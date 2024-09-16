// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QWidget>
#include <gtest/gtest.h>
#include <DGuiApplicationHelper>
#include <QMouseEvent>

#include "stub.h"
#include "addr_pri.h"
#include "../../../src/dde-dock-plugins/recordtime/timewidget.h"

DGUI_USE_NAMESPACE
ACCESS_PRIVATE_FUN(TimeWidget, void(QMouseEvent *e), mousePressEvent);
ACCESS_PRIVATE_FUN(TimeWidget, void(QPaintEvent *e), paintEvent);
ACCESS_PRIVATE_FUN(TimeWidget, void(QMouseEvent *e), mouseReleaseEvent);
ACCESS_PRIVATE_FUN(TimeWidget, void(QMouseEvent *e), mouseMoveEvent);
ACCESS_PRIVATE_FUN(TimeWidget, void(QEvent *e), leaveEvent);
ACCESS_PRIVATE_FUN(TimeWidget, void(), onTimeout);
ACCESS_PRIVATE_FUN(TimeWidget, void(int), onPositionChanged);
ACCESS_PRIVATE_FUN(TimeWidget, void(), createCacheFile);

ACCESS_PRIVATE_FIELD(TimeWidget, int, m_position)
ACCESS_PRIVATE_FIELD(TimeWidget, QPixmap, m_pixmap)
namespace  {

class TestTimeWidget : public testing::Test
{

public:

    void SetUp() override
    {
        m_timeWidget = new TimeWidget();
    }
    void TearDown() override
    {
        if (nullptr != m_timeWidget)
            delete m_timeWidget;
    }

public:
    TimeWidget *m_timeWidget;
};
}

TEST_F(TestTimeWidget, enabled)
{
    EXPECT_EQ(true, m_timeWidget->enabled());
}

TEST_F(TestTimeWidget, start)
{
    m_timeWidget->start();
}

TEST_F(TestTimeWidget, stop)
{
    m_timeWidget->stop();
}

TEST_F(TestTimeWidget, sizeHint)
{
    qDebug() << m_timeWidget->sizeHint();
    EXPECT_LT(0, m_timeWidget->sizeHint().width());
    EXPECT_EQ(22, m_timeWidget->sizeHint().height());
}

TEST_F(TestTimeWidget, onTimeout)
{
    call_private_fun::TimeWidgetonTimeout(*m_timeWidget);
}

TEST_F(TestTimeWidget, onPositionChanged)
{
    call_private_fun::TimeWidgetonPositionChanged(*m_timeWidget, 1);
}

TEST_F(TestTimeWidget, paintEvent)
{
    QPaintEvent *e = new QPaintEvent(QRect());
    access_private_field::TimeWidgetm_position(*m_timeWidget) = 0;
    call_private_fun::TimeWidgetpaintEvent(*m_timeWidget, e);

    access_private_field::TimeWidgetm_position(*m_timeWidget) = 1;
    call_private_fun::TimeWidgetpaintEvent(*m_timeWidget, e);

    access_private_field::TimeWidgetm_position(*m_timeWidget) = 0;
    access_private_field::TimeWidgetm_pixmap(*m_timeWidget) = QPixmap();
    call_private_fun::TimeWidgetpaintEvent(*m_timeWidget, e);
    delete  e;
}

TEST_F(TestTimeWidget, mousePressEvent)
{
    QMouseEvent *ev = new QMouseEvent(QEvent::MouseButtonPress, QPoint(-10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    access_private_field::TimeWidgetm_position(*m_timeWidget) = 0;
    call_private_fun::TimeWidgetmousePressEvent(*m_timeWidget, ev);
    delete  ev;
}

TEST_F(TestTimeWidget, mouseReleaseEvent)
{
    QMouseEvent *ev = new QMouseEvent(QEvent::MouseButtonRelease, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    call_private_fun::TimeWidgetmouseReleaseEvent(*m_timeWidget, ev);
    delete  ev;
}

TEST_F(TestTimeWidget, mouseMoveEvent)
{
    QMouseEvent *ev = new QMouseEvent(QEvent::MouseMove, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    call_private_fun::TimeWidgetmouseMoveEvent(*m_timeWidget, ev);
    delete  ev;
}


TEST_F(TestTimeWidget, leaveEvent)
{
    QEvent *ev = new QMouseEvent(QEvent::MouseMove, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    call_private_fun::TimeWidgetleaveEvent(*m_timeWidget, ev);
    delete  ev;
}
TEST_F(TestTimeWidget, isWaylandProtocol)
{
    m_timeWidget->isWaylandProtocol();
}

TEST_F(TestTimeWidget, createCacheFile)
{
    call_private_fun::TimeWidgetcreateCacheFile(*m_timeWidget);
}
