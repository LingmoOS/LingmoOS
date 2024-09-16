// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_monthgraphiview.h"
#include "../third-party_stub/stub.h"
#include "../dialog_stub.h"
#include "widget/monthWidget/monthscheduleview.h"

static void monthgraphiview_stub_void()
{
}

test_monthgraphiview::test_monthgraphiview()
{
    cMonthGraphiview = new CMonthGraphicsview();
}

test_monthgraphiview::~test_monthgraphiview()
{
    delete cMonthGraphiview;
}

//void CMonthGraphiview::setTheMe(int type)
TEST_F(test_monthgraphiview, setTheMe)
{
    int type = 1;
    cMonthGraphiview->setTheMe();
    cMonthGraphiview->setTheMe(type);
}

//void CMonthGraphiview::setDate(const QVector<QDate> &showDate)
TEST_F(test_monthgraphiview, setDate)
{
    QVector<QDate> showDate;
    QDate date1(2020, 10, 1);
    QDate date2(2020, 12, 1);
    for (int i = 0; i < 42; ++i) {
        if (i % 2 == 0) {
            showDate.append(date1);
        } else {
            showDate.append(date2);
        }
    }
    std::cout << showDate.size() << std::endl;
    cMonthGraphiview->setDate(showDate);
}

//void CMonthGraphiview::setLunarInfo(const QMap<QDate, CaHuangLiDayInfo> &lunarCache)
TEST_F(test_monthgraphiview, setLunarInfo)
{
    QMap<QDate, CaHuangLiDayInfo> lunarCache = QMap<QDate, CaHuangLiDayInfo> {};
    cMonthGraphiview->setLunarInfo(lunarCache);
}

//void CMonthGraphiview::setLunarVisible(bool visible)
TEST_F(test_monthgraphiview, setLunarVisible)
{
    bool visible = false;
    cMonthGraphiview->setLunarVisible(visible);
}

//void CMonthGraphiview::setScheduleInfo(const QMap<QDate, QVector<ScheduleDataInfo> > &info)
TEST_F(test_monthgraphiview, setScheduleInfo)
{
    QMap<QDate, QVector<ScheduleDataInfo> > info = QMap<QDate, QVector<ScheduleDataInfo> > {};
    cMonthGraphiview->setScheduleInfo(info);
}

//void CMonthGraphiview::setSelectSearchSchedule(const ScheduleDataInfo &scheduleInfo)
TEST_F(test_monthgraphiview, setSelectSearchSchedule)
{
    ScheduleDataInfo scheduleInfo = ScheduleDataInfo{};
    cMonthGraphiview->setSelectSearchSchedule(scheduleInfo);
}

//QPointF CMonthGraphiview::getItemPos(const QPoint &p, const QRectF &itemRect)
TEST_F(test_monthgraphiview, getItemPos)
{
    QPoint p(100, 100);
    QRectF itemRect;
    cMonthGraphiview->getItemPos(p, itemRect);
}

//QDateTime CMonthGraphiview::getPosDate(const QPoint &p)
TEST_F(test_monthgraphiview, getPosDate_01)
{
    QPoint p(100, 100);
    cMonthGraphiview->getPosDate(p);
}

TEST_F(test_monthgraphiview, getPosDate_02)
{
    QPoint p(-100, -1);
    cMonthGraphiview->getPosDate(p);
}

TEST_F(test_monthgraphiview, getPosDate_03)
{
    QPoint p(-1, -1);
    cMonthGraphiview->setSceneRect(-2, -2, 100, 100);
    cMonthGraphiview->getPosDate(p);
}

////void CMonthGraphiview::upDateInfoShow(const CMonthGraphiview::DragStatus &status, const ScheduleDataInfo &info)
TEST_F(test_monthgraphiview, upDateInfoShow_01)
{
    Stub stub;
    stub.set(ADDR(CWeekScheduleView, addData), monthgraphiview_stub_void);
    stub.set(ADDR(CWeekScheduleView, changeDate), monthgraphiview_stub_void);
    ScheduleDataInfo info = ScheduleDataInfo{};
    cMonthGraphiview->upDateInfoShow(CMonthGraphicsview::IsCreate, info);
    cMonthGraphiview->upDateInfoShow(CMonthGraphicsview::ChangeBegin, info);
    cMonthGraphiview->upDateInfoShow(CMonthGraphicsview::ChangeEnd, info);
    cMonthGraphiview->upDateInfoShow(CMonthGraphicsview::ChangeWhole, info);
    cMonthGraphiview->upDateInfoShow(CMonthGraphicsview::NONE, info);

}

TEST_F(test_monthgraphiview, slideEvent_01)
{
    QPointF pos(1, 1);
    QPointF stopPos(2, 2);
    cMonthGraphiview->slideEvent(pos, stopPos);
}

TEST_F(test_monthgraphiview, mouseDoubleClickEvent_01)
{
    QMouseEvent event(QEvent::MouseButtonDblClick, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    cMonthGraphiview->mouseDoubleClickEvent(&event);
}

TEST_F(test_monthgraphiview, mouseDoubleClickEvent_02)
{
    QMouseEvent event(QEvent::MouseButtonDblClick, QPointF(0, 0), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    cMonthGraphiview->mouseDoubleClickEvent(&event);
}

TEST_F(test_monthgraphiview, mouseDoubleClickEvent_03)
{
    QMouseEvent event(QEvent::MouseButtonDblClick, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    cMonthGraphiview->mouseDoubleClickEvent(&event);
}

TEST_F(test_monthgraphiview, resizeEvent_01)
{
    QResizeEvent event(QSize(50, 50), QSize(100, 100));
    cMonthGraphiview->resizeEvent(&event);
}

TEST_F(test_monthgraphiview, changeEvent_01)
{
    QEvent event(QEvent::FontChange);
    cMonthGraphiview->changeEvent(&event);
}

TEST_F(test_monthgraphiview, wheelEvent_01)
{
    QWheelEvent event(QPointF(1, 1), 0, Qt::LeftButton, Qt::NoModifier);
    cMonthGraphiview->wheelEvent(&event);
}

TEST_F(test_monthgraphiview, updateBackgroundShowItem_01)
{
    cMonthGraphiview->updateBackgroundShowItem();
}

TEST_F(test_monthgraphiview, setDragPixmap_01)
{
    QDrag drag(cMonthGraphiview);
    CMonthScheduleItem item(QRect(0, 0, 10, 10));
    cMonthGraphiview->setDragPixmap(&drag, &item);
}

TEST_F(test_monthgraphiview, MeetCreationConditions_01)
{
    QDateTime date;
    EXPECT_TRUE(cMonthGraphiview->MeetCreationConditions(date));
}

TEST_F(test_monthgraphiview, IsEqualtime_01)
{
    QDateTime date;
    EXPECT_TRUE(cMonthGraphiview->IsEqualtime(date, date));
}

TEST_F(test_monthgraphiview, JudgeIsCreate_01)
{
    EXPECT_FALSE(cMonthGraphiview->JudgeIsCreate(QPointF(1, 1)));
}

TEST_F(test_monthgraphiview, RightClickToCreate_01)
{
    QPoint pos(1, 1);
    cMonthGraphiview->RightClickToCreate(nullptr, pos);
}

TEST_F(test_monthgraphiview, MoveInfoProcess_01)
{
    QPointF pos(1, 1);
    ScheduleDataInfo info;
    cMonthGraphiview->MoveInfoProcess(info, pos);
}

TEST_F(test_monthgraphiview, MoveInfoProcess_02)
{
    QPointF pos(1, -1);
    ScheduleDataInfo info;
    cMonthGraphiview->MoveInfoProcess(info, pos);
}

TEST_F(test_monthgraphiview, getDragScheduleInfoBeginTime_02)
{
    QPointF pos(1, -1);
    QDateTime time;
    cMonthGraphiview->m_InfoEndTime = time;
    EXPECT_EQ(cMonthGraphiview->getDragScheduleInfoBeginTime(time), time);
}

TEST_F(test_monthgraphiview, getDragScheduleInfoEndTime_02)
{
    QPointF pos(1, -1);
    QDateTime time;
    cMonthGraphiview->m_InfoEndTime = time;
    EXPECT_EQ(cMonthGraphiview->getDragScheduleInfoEndTime(time), time);
}

TEST_F(test_monthgraphiview, slotCreate_02)
{
    Stub stub;
    calendarDDialogExecStub(stub);

    QPointF pos(1, -1);
    QDateTime time;
    cMonthGraphiview->m_InfoEndTime = time;
    cMonthGraphiview->slotCreate(time);
}

