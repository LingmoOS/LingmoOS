// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_scheduledlg.h"

#include "../cscheduledbusstub.h"
#include "../dialog_stub.h"

#include <QTest>

test_scheduledlg::test_scheduledlg()
{
    mScheduleDlg = new CScheduleDlg(1);
}

test_scheduledlg::~test_scheduledlg()
{
    delete mScheduleDlg;
    mScheduleDlg = nullptr;
}

QVector<ScheduleDataInfo> getScheduleDlgData()
{
    ScheduleDataInfo schedule1, schedule2;

    QDateTime currentDateTime = QDateTime::currentDateTime();
    schedule1.setID(1);
    schedule1.setBeginDateTime(currentDateTime);
    schedule1.setEndDateTime(currentDateTime.addDays(1));
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

    QVector<ScheduleDataInfo> scheduleList{};
    scheduleList.append(schedule1);
    scheduleList.append(schedule2);
    return scheduleList;
}
//void CScheduleDlg::setData(const ScheduleDataInfo &info)
TEST_F(test_scheduledlg, setData)
{
    mScheduleDlg->setData(getScheduleDlgData().first());
}

//void CScheduleDlg::setDate(const QDateTime &date)
TEST_F(test_scheduledlg, setDate)
{
    QDateTime datetime1 = QDateTime(QDate::currentDate(), QTime(23, 54, 55));
    mScheduleDlg->setDate(datetime1);

    QDateTime datetime11 = QDateTime(QDate::currentDate(), QTime(13, 54, 55));
    mScheduleDlg->setDate(datetime1);

    QDateTime datetime2 = QDateTime(QDate::currentDate(), QTime(13, 40, 55));
    mScheduleDlg->setDate(datetime2);
}

//void CScheduleDlg::setAllDay(bool flag)
TEST_F(test_scheduledlg, setAllDay)
{
    mScheduleDlg->setAllDay(true);
}

//bool CScheduleDlg::clickOkBtn()
TEST_F(test_scheduledlg, clickOkBtn)
{
    Stub stub;
    cscheduleDbusStub(stub);
    mScheduleDlg->clickOkBtn();

    //begindatetime < enddatetime
    ScheduleDataInfo schedule;
    QDateTime currentDateTime = QDateTime::currentDateTime();
    schedule.setID(1);
    schedule.setBeginDateTime(currentDateTime);
    schedule.setEndDateTime(currentDateTime.addDays(-1));
    schedule.setTitleName("scheduleOne");
    schedule.setAllDay(true);
    schedule.setType(1);
    schedule.setRecurID(0);
    mScheduleDlg->setData(schedule);
    mScheduleDlg->clickOkBtn();

    //
    mScheduleDlg->m_type = 1;
    schedule.setEndDateTime(currentDateTime.addDays(1));
    mScheduleDlg->setData(schedule);
    mScheduleDlg->clickOkBtn();

    mScheduleDlg->m_rmindCombox->setCurrentIndex(3);
    mScheduleDlg->clickOkBtn();

    mScheduleDlg->m_rmindCombox->setCurrentIndex(4);
    mScheduleDlg->clickOkBtn();

    mScheduleDlg->m_rmindCombox->setCurrentIndex(5);
    mScheduleDlg->clickOkBtn();

    schedule.setAllDay(false);
    mScheduleDlg->setData(schedule);
    mScheduleDlg->clickOkBtn();

    mScheduleDlg->m_rmindCombox->setCurrentIndex(3);
    mScheduleDlg->clickOkBtn();

    mScheduleDlg->m_rmindCombox->setCurrentIndex(4);
    mScheduleDlg->clickOkBtn();

    mScheduleDlg->m_rmindCombox->setCurrentIndex(5);
    mScheduleDlg->clickOkBtn();

    mScheduleDlg->m_endrepeatCombox->setCurrentIndex(2);
    mScheduleDlg->clickOkBtn();

    mScheduleDlg->m_endrepeatCombox->setCurrentIndex(3);
    mScheduleDlg->clickOkBtn();
}

//void CScheduleDlg::slotBtClick(int buttonIndex, QString buttonName)
TEST_F(test_scheduledlg, slotBtClick)
{
    Stub stub;
    cscheduleDbusStub(stub);
    mScheduleDlg->slotBtClick(0, "ok");
    mScheduleDlg->slotBtClick(1, "ok");
}

//void CScheduleDlg::slotTextChange()
TEST_F(test_scheduledlg, slotTextChange)
{
    mScheduleDlg->slotTextChange();
}

//void CScheduleDlg::slotendrepeatTextchange()
TEST_F(test_scheduledlg, slotendrepeaTextchange)
{
    mScheduleDlg->slotendrepeatTextchange();
}

//void CScheduleDlg::slotBDateEidtInfo(const QDate &date)
TEST_F(test_scheduledlg, slotBDateEidtInfo)
{
    mScheduleDlg->slotBDateEidtInfo(QDate::currentDate());
}

//void CScheduleDlg::slotallDayStateChanged(int state)
TEST_F(test_scheduledlg, slotallDayStateCheanged)
{
    mScheduleDlg->slotallDayStateChanged(1);
    mScheduleDlg->slotallDayStateChanged(0);
}

//void CScheduleDlg::slotbRpeatactivated(int index)
TEST_F(test_scheduledlg, slotbRpearactivated)
{
    mScheduleDlg->slotbRpeatactivated(0);
    mScheduleDlg->slotbRpeatactivated(2);
}

//void CScheduleDlg::sloteRpeatactivated(int index)
TEST_F(test_scheduledlg, sloteRpearactivated)
{
    mScheduleDlg->sloteRpeatactivated(0);
    mScheduleDlg->sloteRpeatactivated(1);
    mScheduleDlg->sloteRpeatactivated(2);
}

//void CScheduleDlg::setTheMe(const int type)
TEST_F(test_scheduledlg, setTheMe)
{
    mScheduleDlg->setTheMe(1);
    mScheduleDlg->setTheMe(2);
}

//
TEST_F(test_scheduledlg, exec)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    mScheduleDlg->exec();
}

//setDateFormat
TEST_F(test_scheduledlg, setDateFormat)
{
    for (int i = 0; i < 10; ++i) {
        mScheduleDlg->setDateFormat(i);
    }
}

//setTimeFormat
TEST_F(test_scheduledlg, setTimeFormat)
{
    mScheduleDlg->setTimeFormat(0);
    mScheduleDlg->setTimeFormat(1);
}

//mouseMoveEvent
TEST_F(test_scheduledlg, mouseMoveEvent)
{
    QTest::mouseMove(mScheduleDlg);
}

//updateDateTimeFormat
TEST_F(test_scheduledlg, updateDateTimeFormat)
{
    mScheduleDlg->updateDateTimeFormat();
}

//changeEvent
TEST_F(test_scheduledlg, changeEvent)
{
    QEvent event(QEvent::FontChange);
    QApplication::sendEvent(mScheduleDlg, &event);
}
