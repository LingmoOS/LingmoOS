// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_yearscheduleview.h"
#include "../third-party_stub/stub.h"
#include "constants.h"
#include <QDebug>

//test_yearscheduleview::test_yearscheduleview()
//{
//}

//test_yearscheduleview::~test_yearscheduleview()
//{
//}

//void test_yearscheduleview::SetUp()
//{
//    cYearScheduleView = new CYearScheduleView();
//    zYearScheduleOutView = new CYearScheduleOutView();
//}

//void test_yearscheduleview::TearDown()
//{
//    delete cYearScheduleView;
//    cYearScheduleView = nullptr;
//    delete zYearScheduleOutView;
//    zYearScheduleOutView = nullptr;
//}

//QVector<ScheduleDataInfo> test_yearscheduleview::getScheduleDataInfo()
//{
//    QVector<ScheduleDataInfo> scheduleDate {};
//    ScheduleDataInfo schedule1, schedule2, schedule3, schedule4, schedule5, scheduleFes;

//    schedule1.setID(1);
//    schedule1.setBeginDateTime(QDateTime(QDate(2021, 1, 1), QTime(0, 0, 0)));
//    schedule1.setEndDateTime(QDateTime(QDate(2021, 1, 2), QTime(23, 59, 59)));
//    schedule1.setTitleName("schedule test one");
//    schedule1.setAllDay(true);
//    schedule1.setType(1);
//    schedule1.setRecurID(0);

//    schedule2.setID(2);
//    schedule2.setBeginDateTime(QDateTime(QDate(2021, 1, 3), QTime(0, 0, 0)));
//    schedule2.setEndDateTime(QDateTime(QDate(2021, 1, 5), QTime(23, 59, 59)));
//    schedule2.setTitleName("schedule test two");
//    schedule2.setAllDay(true);
//    schedule2.setType(2);
//    schedule2.setRecurID(0);

//    schedule3.setID(3);
//    schedule3.setBeginDateTime(QDateTime(QDate(2021, 1, 6), QTime(5, 23, 40)));
//    schedule3.setEndDateTime(QDateTime(QDate(2021, 1, 9), QTime(6, 23, 40)));
//    schedule3.setTitleName("schedule test three");
//    schedule3.setAllDay(false);
//    schedule3.setType(3);
//    schedule3.setRecurID(0);

//    schedule4.setID(4);
//    schedule4.setBeginDateTime(QDateTime(QDate(2021, 1, 10), QTime(7, 23, 40)));
//    schedule4.setEndDateTime(QDateTime(QDate(2021, 1, 14), QTime(8, 23, 40)));
//    schedule4.setTitleName("schedule test four");
//    schedule4.setAllDay(false);
//    schedule4.setType(1);
//    schedule4.setRecurID(0);

//    schedule5.setID(5);
//    schedule5.setBeginDateTime(QDateTime(QDate(2021, 1, 15), QTime(9, 23, 40)));
//    schedule5.setEndDateTime(QDateTime(QDate(2021, 1, 20), QTime(10, 23, 40)));
//    schedule5.setTitleName("schedule test five");
//    schedule5.setAllDay(false);
//    schedule5.setType(2);
//    schedule5.setRecurID(0);

//    scheduleFes.setID(6);
//    scheduleFes.setBeginDateTime(QDateTime(QDate(2021, 1, 21), QTime(0, 0, 0)));
//    scheduleFes.setEndDateTime(QDateTime(QDate(2021, 1, 21), QTime(23, 59, 59)));
//    scheduleFes.setTitleName("schedule test festival");
//    scheduleFes.setAllDay(true);
//    scheduleFes.setType(4);
//    scheduleFes.setRecurID(0);

//    scheduleDate.append(schedule1);
//    scheduleDate.append(schedule2);
//    scheduleDate.append(schedule3);
//    scheduleDate.append(schedule4);
//    scheduleDate.append(schedule5);
//    scheduleDate.append(scheduleFes);
//    return scheduleDate;
//}

//QVector<ScheduleDataInfo> test_yearscheduleview::getScheduleDateAndTitle()
//{
//    QVector<ScheduleDataInfo> scheduleDate {};
//    ScheduleDataInfo s11, s12;
//    s11.setBeginDateTime(QDateTime(QDate(2020, 12, 31), QTime(16, 23, 40)));
//    s11.setEndDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 23, 40)));
//    s12.setBeginDateTime(QDateTime(QDate(2020, 12, 31), QTime(16, 23, 40)));
//    s12.setEndDateTime(QDateTime(QDate(2020, 12, 31), QTime(16, 23, 40)));
//    ScheduleDataInfo s21, s22;
//    s21.setBeginDateTime(QDateTime(QDate(2020, 12, 31), QTime(16, 23, 40)));
//    s21.setEndDateTime(QDateTime(QDate(2020, 12, 31), QTime(16, 23, 40)));
//    s22.setBeginDateTime(QDateTime(QDate(2020, 12, 31), QTime(16, 23, 40)));
//    s22.setEndDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 23, 40)));
//    ScheduleDataInfo s31, s32;
//    s31.setBeginDateTime(QDateTime(QDate(2020, 12, 30), QTime(16, 23, 40)));
//    s31.setEndDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 23, 40)));
//    s32.setBeginDateTime(QDateTime(QDate(2020, 12, 31), QTime(16, 23, 40)));
//    s32.setEndDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 23, 40)));
//    ScheduleDataInfo s41, s42;
//    s41.setBeginDateTime(QDateTime(QDate(2020, 12, 31), QTime(16, 23, 40)));
//    s41.setEndDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 23, 40)));
//    s42.setBeginDateTime(QDateTime(QDate(2020, 12, 30), QTime(16, 23, 40)));
//    s42.setEndDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 23, 40)));
//    ScheduleDataInfo s51, s52;
//    s51.setBeginDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 23, 40)));
//    s51.setEndDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 23, 40)));
//    s51.setTitleName("ab");
//    s52.setBeginDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 23, 40)));
//    s52.setEndDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 23, 40)));
//    s52.setTitleName("ba");
//    ScheduleDataInfo s61, s62;
//    s61.setBeginDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 23, 40)));
//    s61.setEndDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 23, 40)));
//    s61.setTitleName("ba");
//    s62.setBeginDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 23, 40)));
//    s62.setEndDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 23, 40)));
//    s62.setTitleName("ab");
//    ScheduleDataInfo s71, s72;
//    s71.setBeginDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 2, 40)));
//    s71.setEndDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 23, 40)));
//    s72.setBeginDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 23, 40)));
//    s72.setEndDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 23, 40)));
//    ScheduleDataInfo s81, s82;
//    s81.setBeginDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 23, 40)));
//    s81.setEndDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 23, 40)));
//    s82.setBeginDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 2, 40)));
//    s82.setEndDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 23, 40)));
//    ScheduleDataInfo s91, s92;
//    s91.setBeginDateTime(QDateTime(QDate(2020, 12, 31), QTime(16, 23, 40)));
//    s91.setEndDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 23, 40)));
//    s92.setBeginDateTime(QDateTime(QDate(2020, 12, 31), QTime(16, 23, 40)));
//    s92.setEndDateTime(QDateTime(QDate(2020, 12, 31), QTime(16, 23, 40)));
//    ScheduleDataInfo sa1, sa2;
//    sa1.setBeginDateTime(QDateTime(QDate(2020, 12, 31), QTime(16, 23, 40)));
//    sa1.setEndDateTime(QDateTime(QDate(2021, 1, 1), QTime(16, 23, 40)));
//    sa2.setBeginDateTime(QDateTime(QDate(2020, 12, 31), QTime(16, 23, 40)));
//    sa2.setEndDateTime(QDateTime(QDate(2021, 12, 31), QTime(16, 23, 40)));

//    scheduleDate << s11 << s12 << s21 << s22 << s31 << s32 << s41 << s42
//                 << s51 << s52 << s61 << s62 << s71 << s72 << s81 << s82
//                 << s91 << s92 << sa1 << sa2;
//    return scheduleDate;
//}

////bool YScheduleDateThan(const ScheduleDataInfo &s1, const ScheduleDataInfo &s2)
//bool YScheduleDateThan(const ScheduleDataInfo &s1, const ScheduleDataInfo &s2);
//TEST_F(test_yearscheduleview, yschesuledatethan)
//{
//    //bdate1 != edate1 && bdate2 == edate2
//    EXPECT_TRUE(YScheduleDateThan(getScheduleDateAndTitle().at(0), getScheduleDateAndTitle().at(1)));
//    //bdate1 == edate1 && bdate2 != edate2
//    EXPECT_FALSE(YScheduleDateThan(getScheduleDateAndTitle().at(2), getScheduleDateAndTitle().at(3)));
//    //bdate1 != edate1 && bdate2 != edate2
//    //bdate1 < bdate2
//    EXPECT_TRUE(YScheduleDateThan(getScheduleDateAndTitle().at(4), getScheduleDateAndTitle().at(5)));
//    //bdate1 != edate1 && bdate2 != edate2
//    //bdate1 < bdate2
//    EXPECT_FALSE(YScheduleDateThan(getScheduleDateAndTitle().at(6), getScheduleDateAndTitle().at(7)));
//    //bdate1 = edate1 && bdate2 = edate2
//    //s1.getTitleName() < s2.getTitleName()
//    EXPECT_TRUE(YScheduleDateThan(getScheduleDateAndTitle().at(8), getScheduleDateAndTitle().at(9)));
//    //bdate1 = edate1 && bdate2 = edate2
//    //s1.getBeginDateTime() == s2.getBeginDateTime()
//    //s1.getTitleName() < s2.getTitleName()
//    EXPECT_FALSE(YScheduleDateThan(getScheduleDateAndTitle().at(10), getScheduleDateAndTitle().at(11)));
//    //bdate1 = edate1 && bdate2 = edate2
//    //s1.getBeginDateTime() != s2.getBeginDateTime()
//    //s1.getBeginDateTime() < s2.getBeginDateTime()
//    EXPECT_TRUE(YScheduleDateThan(getScheduleDateAndTitle().at(12), getScheduleDateAndTitle().at(13)));
//    //bdate1 = edate1 && bdate2 = edate2
//    //s1.getBeginDateTime() != s2.getBeginDateTime()
//    //s1.getBeginDateTime() < s2.getBeginDateTime()
//    EXPECT_FALSE(YScheduleDateThan(getScheduleDateAndTitle().at(14), getScheduleDateAndTitle().at(15)));
//}

////bool YScheduleDaysThan(const ScheduleDataInfo &s1, const ScheduleDataInfo &s2)
//bool YScheduleDaysThan(const ScheduleDataInfo &s1, const ScheduleDataInfo &s2);
//TEST_F(test_yearscheduleview, yscheduleDaysThan)
//{
//    //s1.getBeginDateTime().date().daysTo(s1.getEndDateTime().date()) > s2.getBeginDateTime().date().daysTo(s2.getEndDateTime().date())
//    EXPECT_TRUE(YScheduleDaysThan(getScheduleDateAndTitle().at(16), getScheduleDateAndTitle().at(17)));
//    //s1.getBeginDateTime().date().daysTo(s1.getEndDateTime().date()) > s2.getBeginDateTime().date().daysTo(s2.getEndDateTime().date())
//    EXPECT_FALSE(YScheduleDaysThan(getScheduleDateAndTitle().at(18), getScheduleDateAndTitle().at(19)));
//}

////void CYearScheduleView::setData(QVector<ScheduleDataInfo> &vListData)
//TEST_F(test_yearscheduleview, setDate)
//{
//    QVector<ScheduleDataInfo> scheduleinfo = getScheduleDataInfo();
//    cYearScheduleView->setData(scheduleinfo);
//}

////void CYearScheduleView::clearData()
//TEST_F(test_yearscheduleview, clearDate)
//{
//    cYearScheduleView->clearData();
//}

////int CYearScheduleView::showWindow()
//TEST_F(test_yearscheduleview, showWindow)
//{
//    QVector<ScheduleDataInfo> vlistData = getScheduleDataInfo();
//    cYearScheduleView->showWindow();
//    cYearScheduleView->setData(vlistData);
//    cYearScheduleView->showWindow();
//}

////void CYearScheduleView::setTheMe(int type)
//TEST_F(test_yearscheduleview, setTheMe)
//{
//    int type = 1;
//    cYearScheduleView->setTheMe(type);

//    type = 2;
//    cYearScheduleView->setTheMe(type);
//}

////void CYearScheduleView::setCurrentDate(QDate cdate)
//TEST_F(test_yearscheduleview, setCurrentDate)
//{
//    cYearScheduleView->setCurrentDate(QDate(2021, 1, 6));
//}

////QDate CYearScheduleView::getCurrentDate()
//TEST_F(test_yearscheduleview, getCurrentDate)
//{
//    QDate currentDate = QDate::currentDate();
//    cYearScheduleView->setCurrentDate(currentDate);
//    bool iscurrent = currentDate == cYearScheduleView->getCurrentDate();
//    EXPECT_TRUE(iscurrent);

//    QDate earlyDate = QDate(1999, 1, 1);
//    cYearScheduleView->setCurrentDate(earlyDate);
//    iscurrent = currentDate == cYearScheduleView->getCurrentDate();
//    EXPECT_FALSE(iscurrent);
//}


////void CYearScheduleOutView::setData(QVector<ScheduleDataInfo> &vListData)
//TEST_F(test_yearscheduleview, setOutData)
//{
//    QVector<ScheduleDataInfo> scheduleInfo = getScheduleDataInfo();
//    zYearScheduleOutView->setData(scheduleInfo);
//}

////void CYearScheduleOutView::clearData()
//TEST_F(test_yearscheduleview, clearOutData)
//{
//    zYearScheduleOutView->clearData();
//}

////void CYearScheduleOutView::setTheMe(int type)
//TEST_F(test_yearscheduleview, setTheOutMe)
//{
//    zYearScheduleOutView->setTheMe(1);
//}

////void CYearScheduleOutView::setCurrentDate(QDate cdate)
//TEST_F(test_yearscheduleview, setCurrentOutDate)
//{
//    zYearScheduleOutView->setCurrentDate(QDate(2021, 1, 6));
//}

//TEST_F(test_yearscheduleview, paintEvent)
//{
//    QPixmap pixmap(cYearScheduleView->size());
//    QVector<ScheduleDataInfo> scheduleInfo = getScheduleDataInfo();
//    cYearScheduleView->setData(scheduleInfo);
//    cYearScheduleView->render(&pixmap);
//}

//TEST_F(test_yearscheduleview, mousePressEvent)
//{
//}
