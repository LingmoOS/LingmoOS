// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_schedulesearchview.h"
#include "../third-party_stub/stub.h"
#include "../calendar-basicstruct/src/utils.h"
#include "../third-party_stub/addr_pri.h"
#include "scheduleTask/cscheduledbus.h"
#include "scheduleTask/scheduletask.h"
#include "constants.h"
#include "../testscheduledata.h"
#include "../dialog_stub.h"
#include <QContextMenuEvent>

QVector<ScheduleDataInfo> getScheduleDInfo()
{
    QVector<ScheduleDataInfo> scheduleDate {};
    ScheduleDataInfo schedule1, schedule2, schedule3, schedule4, schedule5, scheduleFes;
    QDateTime currentDateTime = QDateTime::currentDateTime();

    schedule1.setID(1);
    schedule1.setBeginDateTime(currentDateTime);
    schedule1.setEndDateTime(currentDateTime.addSecs(60 * 60));
    schedule1.setTitleName("scheduleOne");
    schedule1.setAllDay(true);
    schedule1.setType(1);
    schedule1.setRecurID(0);

    schedule2.setID(2);
    schedule2.setBeginDateTime(currentDateTime.addDays(1));
    schedule2.setEndDateTime(currentDateTime.addDays(1).addSecs(60 * 60));
    schedule2.setTitleName("scheduleTwo");
    schedule2.setAllDay(true);
    schedule2.setType(2);
    schedule2.setRecurID(0);

    schedule3.setID(3);
    schedule3.setBeginDateTime(currentDateTime.addDays(2));
    schedule3.setEndDateTime(currentDateTime.addDays(2).addSecs(60 * 60));
    schedule3.setTitleName("scheduleThree");
    schedule3.setAllDay(false);
    schedule3.setType(3);
    schedule3.setRecurID(0);

    schedule4.setID(4);
    schedule4.setBeginDateTime(currentDateTime.addDays(3));
    schedule4.setEndDateTime(currentDateTime.addDays(3).addSecs(60 * 60));
    schedule4.setTitleName("scheduleFour");
    schedule4.setAllDay(false);
    schedule4.setType(1);
    schedule4.setRecurID(0);

    schedule5.setID(5);
    schedule5.setBeginDateTime(currentDateTime.addDays(4));
    schedule5.setEndDateTime(currentDateTime.addDays(4).addSecs(60 * 60));
    schedule5.setTitleName("scheduleFive");
    schedule5.setAllDay(false);
    schedule5.setType(2);
    schedule5.setRecurID(0);

    scheduleFes.setID(6);
    scheduleFes.setBeginDateTime(currentDateTime.addDays(5));
    scheduleFes.setEndDateTime(currentDateTime.addDays(5).addSecs(60 * 60));
    scheduleFes.setTitleName("scheduleFestival");
    scheduleFes.setAllDay(true);
    scheduleFes.setType(4);
    scheduleFes.setRecurID(0);

    scheduleDate.append(schedule1);
    scheduleDate.append(schedule2);
    scheduleDate.append(schedule3);
    scheduleDate.append(schedule4);
    scheduleDate.append(schedule5);
    scheduleDate.append(scheduleFes);
    return scheduleDate;
}

QMap<QDate, QVector<ScheduleDataInfo>> getMapScheduleDInfo(int getDays)
{
    QDate currentDate = QDate::currentDate();
    QVector<ScheduleDataInfo> scheduleInfo {};
    QMap<QDate, QVector<ScheduleDataInfo>> scheduleDateInof {};
    switch (getDays) {
    case 0: {
        scheduleInfo.append(getScheduleDInfo().at(0));
        scheduleDateInof[currentDate] = scheduleInfo;
    } break;
    case 1: {
        scheduleInfo.append(getScheduleDInfo().at(1));
        scheduleDateInof[currentDate.addDays(1)] = scheduleInfo;
    } break;
    case 2: {
        scheduleInfo.append(getScheduleDInfo().at(2));
        scheduleDateInof[currentDate.addDays(2)] = scheduleInfo;
    } break;
    default: {
        scheduleInfo.append(getScheduleDInfo().at(3));
        scheduleDateInof[currentDate.addDays(3)] = scheduleInfo;
        scheduleInfo.append(getScheduleDInfo().at(4));
        scheduleDateInof[currentDate.addDays(4)] = scheduleInfo;
        scheduleInfo.append(getScheduleDInfo().at(5));
        scheduleDateInof[currentDate.addDays(5)] = scheduleInfo;
    } break;
    }
    return scheduleDateInof;
}

bool stub_QueryJobs(const QString &key, QDateTime starttime, QDateTime endtime, QMap<QDate, QVector<ScheduleDataInfo>> &out)
{
    Q_UNUSED(key);
    int days = static_cast<int>(starttime.daysTo(endtime));

    switch (days) {
    case 0: {
        out = getMapScheduleDInfo(0);
    } break;
    case 1: {
        out = getMapScheduleDInfo(1);
    } break;
    case 2: {
        out = getMapScheduleDInfo(2);
    } break;
    default: {
        out = getMapScheduleDInfo(days);
    } break;
    }
    return true;
}

static QAction* schedulesearchview_stub_QMenu_exec()
{
    return nullptr;
}

test_schedulesearchview::test_schedulesearchview()
{
}

test_schedulesearchview::~test_schedulesearchview()
{
}

void test_schedulesearchview::SetUp()
{
    mScheduleSearchView = new CScheduleSearchView();
    mScheduleSearchDateItem = new CScheduleSearchDateItem();
    mScheduleListWidget = new CScheduleListWidget();
    mScheduleSearchItem = new CScheduleSearchItem();
}

void test_schedulesearchview::TearDown()
{
    delete mScheduleSearchView;
    mScheduleSearchView = nullptr;
    delete mScheduleSearchDateItem;
    mScheduleSearchDateItem = nullptr;
    delete mScheduleListWidget;
    mScheduleListWidget = nullptr;
    delete mScheduleSearchItem;
    mScheduleSearchItem = nullptr;
}

//void CScheduleSearchView::setTheMe(int type)
TEST_F(test_schedulesearchview, setTheMe)
{
    int type = 1;
    mScheduleSearchView->setTheMe(type);

    type = 2;
    mScheduleSearchView->setTheMe(type);
}

//void CScheduleSearchView::clearSearch()
TEST_F(test_schedulesearchview, clearSearch)
{
    ScheduleDataInfo schedule1;
    schedule1.setID(1);
    schedule1.setBeginDateTime(QDateTime(QDate(2021, 1, 1), QTime(0, 0, 0)));
    schedule1.setEndDateTime(QDateTime(QDate(2021, 1, 2), QTime(23, 59, 59)));
    schedule1.setTitleName("schedule test one");
    schedule1.setAllDay(true);
    schedule1.setType(1);
    schedule1.setRecurID(0);
    mScheduleSearchView->createItemWidget(schedule1, QDate(2021, 1, 1), 1);

    mScheduleSearchView->clearSearch();
}

//void CScheduleSearchView::setMaxWidth(const int w)
TEST_F(test_schedulesearchview, setMacWidth)
{
    mScheduleSearchView->setMaxWidth(44);
}

//void CScheduleSearchView::slotsetSearch()
TEST_F(test_schedulesearchview, slotsetSearch)
{
    //    mScheduleSearchView->slotsetSearch("jie");
}

//void CScheduleSearchView::createItemWidget(ScheduleDataInfo info, QDate date, int rtype)
TEST_F(test_schedulesearchview, createItemWidget)
{
    ScheduleDataInfo scheduleinof = getScheduleDInfo().first();
    mScheduleSearchView->createItemWidget(scheduleinof, QDate::currentDate(), 3);
}

//QListWidgetItem *CScheduleSearchView::createItemWidget(QDate date)
TEST_F(test_schedulesearchview, createItemWidgetDate)
{
    mScheduleSearchView->createItemWidget(QDate::currentDate());
}

//void CScheduleSearchView::slotSelectSchedule(const ScheduleDataInfo &scheduleInfo)
TEST_F(test_schedulesearchview, slotSelectSchedule)
{
    ScheduleDataInfo scheduleinof = getScheduleDInfo().first();
    mScheduleSearchView->slotSelectSchedule(scheduleinof);
}

//void CScheduleSearchView::updateSearch()
TEST_F(test_schedulesearchview, updateSearch)
{
    mScheduleSearchView->updateSearch();
}

//void CScheduleSearchDateItem::setBackgroundColor(QColor color1)
TEST_F(test_schedulesearchview, setBachgroundColor)
{
    QColor color1(240, 100, 100);
    mScheduleSearchDateItem->setBackgroundColor(color1);
}

//void CScheduleSearchDateItem::setText(QColor tcolor, QFont font)
TEST_F(test_schedulesearchview, setText)
{
    QColor color(240, 100, 100);
    QFont font;
    font.setWeight(QFont::Medium);
    font.setPixelSize(DDECalendar::FontSizeTwelve);
    mScheduleSearchDateItem->setText(color, font);
}

//void CScheduleSearchDateItem::setDate(QDate date)
TEST_F(test_schedulesearchview, setDate)
{
    mScheduleSearchDateItem->setDate(QDate::currentDate());
}

//void CScheduleSearchItem::setBackgroundColor(QColor color1)
TEST_F(test_schedulesearchview, setBackGroundColor)
{
    QColor color(240, 100, 100);
    mScheduleSearchItem->setBackgroundColor(color);
}

//void CScheduleSearchItem::setText(QColor tcolor, QFont font)
TEST_F(test_schedulesearchview, setItemText)
{
    QColor color(240, 100, 100);
    QFont font;
    font.setWeight(QFont::Medium);
    font.setPixelSize(DDECalendar::FontSizeTwelve);
    mScheduleSearchItem->setText(color, font);
}

//void CScheduleSearchItem::setTimeC(QColor tcolor, QFont font)
TEST_F(test_schedulesearchview, setTimeC)
{
    QColor color(240, 100, 100);
    QFont font;
    font.setWeight(QFont::Medium);
    font.setPixelSize(DDECalendar::FontSizeTwelve);
    mScheduleSearchItem->setTimeC(color, font);
}

//void CScheduleSearchItem::setData(ScheduleDataInfo vScheduleInfo, QDate date)
TEST_F(test_schedulesearchview, setItemDate)
{
    ScheduleDataInfo scheduleinfo = getScheduleDInfo().first();
    mScheduleSearchItem->setData(scheduleinfo, QDate::currentDate());
}

//void CScheduleSearchItem::setRoundtype(int rtype)
TEST_F(test_schedulesearchview, setRoundtype)
{
    mScheduleSearchItem->setRoundtype(3);
    mScheduleSearchItem->setRoundtype(2);
}

//void CScheduleSearchItem::setTheMe(int type)
TEST_F(test_schedulesearchview, setItemTheMe)
{
    mScheduleSearchItem->setTheMe(1);
    mScheduleSearchItem->setTheMe(2);
}

//void CScheduleSearchItem::slotEdit()
TEST_F(test_schedulesearchview, slotEdit_01)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    ScheduleDataInfo scheduleinfo = getScheduleDInfo().first();
    CScheduleSearchItem item;
    item.setData(scheduleinfo, QDate::currentDate());
    item.slotEdit();
}

//void CScheduleSearchItem::slotDelete()
TEST_F(test_schedulesearchview, slotDelete_01)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    ScheduleDataInfo scheduleinfo = getScheduleDInfo().first();
    CScheduleSearchItem item;
    item.setData(scheduleinfo, QDate::currentDate());
    item.slotDelete();
}

TEST_F(test_schedulesearchview, slotTimeFormatChanged_01)
{
    mScheduleSearchItem->slotTimeFormatChanged(1);
    EXPECT_EQ(mScheduleSearchItem->m_timeFormat, "hh:mm");
}

TEST_F(test_schedulesearchview, slotTimeFormatChanged_02)
{
    mScheduleSearchItem->slotTimeFormatChanged(0);
    EXPECT_EQ(mScheduleSearchItem->m_timeFormat, "h:mm");
}

TEST_F(test_schedulesearchview, slotSchotCutClicked_01)
{
    Stub stub;
    stub.set((QAction*(QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), schedulesearchview_stub_QMenu_exec);
    CScheduleSearchItem item;
    item.slotSchotCutClicked();
}

TEST_F(test_schedulesearchview, contextMenuEvent_01)
{
    Stub stub;
    stub.set((QAction*(QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), schedulesearchview_stub_QMenu_exec);
    CScheduleSearchItem item;

    QContextMenuEvent event(QContextMenuEvent::Mouse, QPoint());
    item.contextMenuEvent(&event);
}

TEST_F(test_schedulesearchview, mouseDoubleClickEvent_01)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    CScheduleSearchItem item;

    QMouseEvent event(QEvent::MouseButtonDblClick, QPointF(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    item.mouseDoubleClickEvent(&event);
}

TEST_F(test_schedulesearchview, mousePressEvent_01)
{
    CScheduleSearchItem item;
    QMouseEvent event(QEvent::MouseButtonDblClick, QPointF(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    item.mousePressEvent(&event);
    EXPECT_FALSE(item.m_tabFocus);
}

TEST_F(test_schedulesearchview, mouseReleaseEvent_01)
{
    CScheduleSearchItem item;
    QMouseEvent event(QEvent::MouseButtonDblClick, QPointF(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    item.mouseReleaseEvent(&event);
    EXPECT_EQ(item.m_mouseStatus, CScheduleSearchItem::M_HOVER);
}

TEST_F(test_schedulesearchview, enterEvent_01)
{
    CScheduleSearchItem item;
    QEvent event(QEvent::Enter);
    item.enterEvent(&event);
    EXPECT_EQ(item.m_mouseStatus, CScheduleSearchItem::M_HOVER);
}

TEST_F(test_schedulesearchview, focusOutEvent_01)
{
    CScheduleSearchItem item;
    QFocusEvent event(QEvent::FocusOut, Qt::TabFocusReason);
    item.focusOutEvent(&event);
}

TEST_F(test_schedulesearchview, focusInEvent_01)
{
    CScheduleSearchItem item;
    QFocusEvent event(QEvent::FocusIn, Qt::TabFocusReason);
    item.focusInEvent(&event);
}

TEST_F(test_schedulesearchview, keyPressEvent_01)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    CScheduleSearchItem item;
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    item.keyPressEvent(&event);
}

TEST_F(test_schedulesearchview, keyPressEvent_02)
{
    CScheduleSearchItem item;
    QKeyEvent event(QEvent::KeyPress, Qt::Key_M, Qt::AltModifier);
    item.keyPressEvent(&event);
}

//const ScheduleDataInfo &getData() const
TEST_F(test_schedulesearchview, getDate)
{
    mScheduleSearchItem->getData();
}

QMap<QDate, QVector<ScheduleDataInfo>> stub_getSearchScheduleInfo(void *obj, const QString &key, const QDateTime &startTime, const QDateTime &endTime)
{
    Q_UNUSED(obj)
    Q_UNUSED(key)
    Q_UNUSED(startTime)
    Q_UNUSED(endTime)
    QMap<QDate, QVector<ScheduleDataInfo>> searchScheduleInfo {};
    searchScheduleInfo[QDate::currentDate()] = TestDataInfo::getScheduleItemDInfo();
    return searchScheduleInfo;
}

//
TEST_F(test_schedulesearchview, getPixmap)
{
    Stub stub;

    stub.set((QMap<QDate, QVector<ScheduleDataInfo>>(CScheduleTask::*)(const QString &, const QDateTime &, const QDateTime &))ADDR(CScheduleTask, getSearchScheduleInfo), stub_getSearchScheduleInfo);

    mScheduleSearchView->slotsetSearch("xjrc");
    mScheduleSearchView->setFixedSize(300, 800);
    QPixmap pixmap(mScheduleListWidget->size());
    pixmap = mScheduleSearchView->grab();
}
