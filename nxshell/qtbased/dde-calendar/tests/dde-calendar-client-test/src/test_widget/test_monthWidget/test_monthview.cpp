// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_monthview.h"

//test_monthview::test_monthview()
//{
//    mMonthView = new CMonthView();
//}

//test_monthview::~test_monthview()
//{
//    delete mMonthView;
//    mMonthView = nullptr;
//}

//QVector<ScheduleDataInfo> getMonthViewScheduleDInfo()
//{
//    QVector<ScheduleDataInfo> scheduleDate {};
//    ScheduleDataInfo schedule1, schedule2, schedule3, schedule4, schedule5, scheduleFes;
//    QDateTime currentDateTime = QDateTime::currentDateTime();

//    schedule1.setID(1);
//    schedule1.setBeginDateTime(currentDateTime);
//    schedule1.setEndDateTime(currentDateTime.addDays(1));
//    schedule1.setTitleName("scheduleOne");
//    schedule1.setAllDay(true);
//    schedule1.setType(1);
//    schedule1.setRecurID(0);

//    schedule2.setID(2);
//    schedule2.setBeginDateTime(currentDateTime.addDays(1));
//    schedule2.setEndDateTime(currentDateTime.addDays(1).addSecs(60 * 60));
//    schedule2.setTitleName("scheduleTwo");
//    schedule2.setAllDay(true);
//    schedule2.setType(2);
//    schedule2.setRecurID(0);

//    schedule3.setID(3);
//    schedule3.setBeginDateTime(currentDateTime.addDays(2));
//    schedule3.setEndDateTime(currentDateTime.addDays(2).addSecs(60 * 60));
//    schedule3.setTitleName("scheduleThree");
//    schedule3.setAllDay(false);
//    schedule3.setType(3);
//    schedule3.setRecurID(0);

//    schedule4.setID(4);
//    schedule4.setBeginDateTime(currentDateTime.addDays(3));
//    schedule4.setEndDateTime(currentDateTime.addDays(3).addSecs(60 * 60));
//    schedule4.setTitleName("scheduleFour");
//    schedule4.setAllDay(false);
//    schedule4.setType(1);
//    schedule4.setRecurID(0);

//    schedule5.setID(5);
//    schedule5.setBeginDateTime(currentDateTime.addDays(4));
//    schedule5.setEndDateTime(currentDateTime.addDays(4).addSecs(60 * 60));
//    schedule5.setTitleName("scheduleFive");
//    schedule5.setAllDay(false);
//    schedule5.setType(2);
//    schedule5.setRecurID(0);

//    scheduleFes.setID(6);
//    scheduleFes.setBeginDateTime(currentDateTime.addDays(5));
//    scheduleFes.setEndDateTime(currentDateTime.addDays(5).addSecs(60 * 60));
//    scheduleFes.setTitleName("scheduleFestival");
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

//QVector<QDate> MonthviewGetDayList()
//{
//    QVector<QDate> dateList {};
//    QDate currentDate = QDate::currentDate();
//    for (int i = 0; i < 42; i++) {
//        dateList.append(currentDate.addDays(i));
//    }

//    return dateList;
//}

//QMap<QDate, CaHuangLiDayInfo> MonthViewGetHuangLiDayInfo()
//{
//    QMap<QDate, CaHuangLiDayInfo> huangLiDayinfo {};
//    CaHuangLiDayInfo huangLiInfo1 {};
//    huangLiInfo1.mGanZhiYear = "辛丑牛年"; //年干支
//    huangLiInfo1.mGanZhiMonth = "庚寅月"; //月干支
//    huangLiInfo1.mGanZhiDay = "辛卯日"; //日干支
//    huangLiInfo1.mLunarMonthName = "正月"; //农历月名称
//    huangLiInfo1.mLunarDayName = "初一"; //农历日名称
//    huangLiInfo1.mLunarLeapMonth = 4; //闰月
//    huangLiInfo1.mZodiac = "牛"; //生肖
//    huangLiInfo1.mTerm = ""; //农历节气
//    huangLiInfo1.mSolarFestival = "国庆节"; //阳历节日
//    huangLiInfo1.mLunarFestival = "除夕"; //农历节日
//    huangLiInfo1.mSuit = "嫁娶"; //黄历宜
//    huangLiInfo1.mAvoid = "入土"; //黄历忌

//    huangLiDayinfo.insert(QDate::currentDate(), huangLiInfo1);
//    return huangLiDayinfo;
//}

////void CMonthView::setTheMe(int type)
//TEST_F(test_monthview, setTheMe)
//{
//    mMonthView->setTheMe(1);
//    mMonthView->setTheMe(2);
//}

////void CMonthView::setSelectSchedule(const ScheduleDataInfo &scheduleInfo)
//TEST_F(test_monthview, setSelectSchedule)
//{
//    ScheduleDataInfo scheduleinfo = getMonthViewScheduleDInfo().first();
//    mMonthView->setSelectSchedule(scheduleinfo);
//}

////void CMonthView::slotScheduleRemindWidget(const bool isShow, const ScheduleDataInfo &out)
//TEST_F(test_monthview, slotScheduleRemindWidget)
//{
//    ScheduleDataInfo scheduleinfo = getMonthViewScheduleDInfo().first();
//    mMonthView->slotScheduleRemindWidget(false, scheduleinfo);
//    mMonthView->slotScheduleRemindWidget(true, scheduleinfo);
//}

////void CMonthView::setFirstWeekday(Qt::DayOfWeek weekday)
//TEST_F(test_monthview, setFirstWeekday)
//{
//    mMonthView->setFirstWeekday(Qt::Sunday);
//}

////void CMonthView::setShowDate(const QVector<QDate> &showDate)
//TEST_F(test_monthview, setShowDate)
//{
//    mMonthView->setShowDate(MonthviewGetDayList());
//}

////void CMonthView::setHuangLiInfo(const QMap<QDate, CaHuangLiDayInfo> &huangLiInfo)
//TEST_F(test_monthview, setHuangLiInfo)
//{
//    mMonthView->setHuangLiInfo(MonthViewGetHuangLiDayInfo());
//}

////void CMonthView::setFestival(const QMap<QDate, int> &festivalInfo)
//TEST_F(test_monthview, setFestival)
//{
//    QMap<QDate, int> festivalInfo {};
//    festivalInfo.insert(QDate::currentDate(), 1);
//    festivalInfo.insert(QDate::currentDate().addDays(1), 2);
//    mMonthView->setFestival(festivalInfo);
//}

////void CMonthView::setScheduleInfo(const QMap<QDate, QVector<ScheduleDataInfo> > &scheduleInfo)
//TEST_F(test_monthview, setScheduleInfo)
//{
//    QMap<QDate, QVector<ScheduleDataInfo>> scheduleinfo {};
//    scheduleinfo.insert(QDate::currentDate(), getMonthViewScheduleDInfo());
//    mMonthView->setScheduleInfo(scheduleinfo);
//}

////void CMonthView::setSearchScheduleInfo(const QVector<ScheduleDataInfo> &searchScheduleInfo)
//TEST_F(test_monthview, setSearchScheduleInfo)
//{
//    mMonthView->setSearchScheduleInfo(getMonthViewScheduleDInfo());
//}

////void CMonthView::setCurrentDate(const QDate &currentDate)
//TEST_F(test_monthview, setCurrentDate)
//{
//    mMonthView->setCurrentDate(QDate::currentDate());
//}

////ScheduleDataInfo CMonthView::getScheduleInfo(const QDate &beginDate, const QDate &endDate)
//TEST_F(test_monthview, getScheduleInfo)
//{
//    QDate currentDate = QDate::currentDate();
//    QDate currentDate1 = QDate::currentDate().addDays(1);
//    mMonthView->getScheduleInfo(currentDate, currentDate1);
//    mMonthView->getScheduleInfo(currentDate1, currentDate);
//}

//TEST_F(test_monthview, isDragging)
//{
//    mMonthView->isDragging();
//}
