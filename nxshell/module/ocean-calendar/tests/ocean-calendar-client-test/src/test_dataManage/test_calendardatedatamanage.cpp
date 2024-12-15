// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

//#include "test_calendardatedatamanage.h"
//#include <QDebug>

//test_calendardatedatamanage::test_calendardatedatamanage()
//{
//}

//void test_calendardatedatamanage::SetUp()
//{
//    calendarDateDataManager = new CalendarDateDataManager();
//}

//void test_calendardatedatamanage::TearDown()
//{
//    delete calendarDateDataManager;
//    calendarDateDataManager = nullptr;
//}

///**
// *本类是为获取当前日期的类,所拿到的数据会根据当前时间不断变化,因此暂时不做返回值判断
// */

////void CalendarDateDataManager::setSelectDate(const QDate &selectDate, const bool isSwitchYear)
//TEST_F(test_calendardatedatamanage, setSelectDate)
//{
//    QDate selectDate(2020, 12, 25);
//    bool isSwitchYear = true;
//    calendarDateDataManager->setSelectDate(selectDate, isSwitchYear);

//    isSwitchYear = false;
//    calendarDateDataManager->setSelectDate(selectDate, isSwitchYear);
//}

////QDate CalendarDateDataManager::getSelectDate() const
//TEST_F(test_calendardatedatamanage, getSelectDate)
//{
//    QDate selectDate;
//    selectDate = calendarDateDataManager->getSelectDate();
//    //qCInfo(CommonLogger) << selectDate;
//}

////void CalendarDateDataManager::setCurrentDateTime(const QDateTime &currentDateTime)
//TEST_F(test_calendardatedatamanage, setCurrentDateTime)
//{
//    QDate date(2020, 12, 25);
//    QDateTime currentDateTime(date);
//    calendarDateDataManager->setCurrentDateTime(currentDateTime);
//}

////QDateTime CalendarDateDataManager::getCurrentDate() const
//TEST_F(test_calendardatedatamanage, getCurrentDate)
//{
//    QDateTime currentDateTime;
//    currentDateTime = calendarDateDataManager->getCurrentDate();
//}

////QMap<int, QVector<QDate> > CalendarDateDataManager::getYearDate()
//TEST_F(test_calendardatedatamanage, getYearDate)
//{
//    QMap<int, QVector<QDate> > dateTime;
//    dateTime = calendarDateDataManager->getYearDate();
//}

////QVector<QDate> CalendarDateDataManager::getWeekDate(const QDate &date)
//TEST_F(test_calendardatedatamanage, getWeekDate)
//{
//    QDate selectDate(2020, 12, 25);
//    QVector<QDate> dateV = calendarDateDataManager->getWeekDate(selectDate);
//}

////void CalendarDateDataManager::setWeekFirstDay(const Qt::DayOfWeek &firstDay)
//TEST_F(test_calendardatedatamanage, setWeekFirstDay)
//{
//    Qt::DayOfWeek firstDay(Qt::Sunday);
//    calendarDateDataManager->setWeekFirstDay(firstDay);
//}

//// Qt::DayOfWeek CalendarDateDataManager::getWeekFirstDay()
//TEST_F(test_calendardatedatamanage, getWeekFirstDay)
//{
//    assert(Qt::Sunday == calendarDateDataManager->getWeekFirstDay());
//}

////void CalendarDateDataManager::setWeekDayFormatByID(const int &weekDayFormatID)
//TEST_F(test_calendardatedatamanage, setWeekDayFormatByID)
//{
//    int setWeekDayFormatByID = 0;
//    calendarDateDataManager->setWeekDayFormatByID(setWeekDayFormatByID);

//    setWeekDayFormatByID = 1;
//    calendarDateDataManager->setWeekDayFormatByID(setWeekDayFormatByID);
//}

////QString CalendarDateDataManager::getWeekDayFormat() const
//TEST_F(test_calendardatedatamanage, getWeekDayFormat)
//{
//    QString getWEKfomat;
//    calendarDateDataManager->getWeekDayFormat();
//}

////ShowDateRange CalendarDateDataManager::getShowDateRange() const
//TEST_F(test_calendardatedatamanage, getShowDateRange)
//{
//    ShowDateRange showDateR;
//    showDateR = calendarDateDataManager->getShowDateRange();
//}

////int CalendarDateDataManager::getWeekNumOfYear(const QDate &date)
//TEST_F(test_calendardatedatamanage, getWeekNumOfYear)
//{
//    //2020-12-25 为第52周
//    QDate date(2020, 12, 25);
//    assert(52 == calendarDateDataManager->getWeekNumOfYear(date));
//}

//TEST_F(test_calendardatedatamanage, setDateFormatChanged)
//{
//    for (int i = 0; i < 10; ++i) {
//        calendarDateDataManager->setDateFormatChanged(i);
//    }
//}

////setTimeFormatChanged
//TEST_F(test_calendardatedatamanage, setTimeFormatChanged)
//{
//    calendarDateDataManager->setTimeFormatChanged(0);
//    calendarDateDataManager->setTimeFormatChanged(1);
//}
