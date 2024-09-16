// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_daymonthview.h"

//test_daymonthview::test_daymonthview()
//{
//    mDayMonthView = new CDayMonthView();
//}

//test_daymonthview::~test_daymonthview()
//{
//    delete mDayMonthView;
//    mDayMonthView = nullptr;
//}

//QVector<QDate> dayMonthviewGetDayList()
//{
//    QVector<QDate> dateList {};
//    QDate currentDate = QDate::currentDate();
//    for (int i = 0; i < 42; i++) {
//        dateList.append(currentDate.addDays(i));
//    }

//    return dateList;
//}

//QMap<QDate, CaHuangLiDayInfo> dayMonthViewGetHuangLiDayInfo()
//{
//    QMap<QDate, CaHuangLiDayInfo> huangLiDayInfo {};
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

//    huangLiDayInfo.insert(QDate::currentDate(), huangLiInfo1);
//    return huangLiDayInfo;
//}

//QVector<bool> dayMonthViewGetLineFlag()
//{
//    QVector<bool> flags {};
//    for (int i = 0; i < 42; i++) {
//        flags.append(true);
//    }
//    return flags;
//}

////void CDayMonthView::setShowDate(const QVector<QDate> &showDate, const QDate &selectDate, const QDate &currentDate)
//TEST_F(test_daymonthview, setShowDate)
//{
//    mDayMonthView->setShowDate(dayMonthviewGetDayList(), QDate::currentDate(), QDate::currentDate());
//}

////void CDayMonthView::setLunarVisible(bool visible)
//TEST_F(test_daymonthview, setLunarVisible)
//{
//    mDayMonthView->setLunarVisible(true);
//}

////void CDayMonthView::setTheMe(int type)
//TEST_F(test_daymonthview, setTheMe)
//{
//    mDayMonthView->setTheMe(1);
//    mDayMonthView->setTheMe(2);
//}

////void CDayMonthView::setSearchFlag(bool flag)
//TEST_F(test_daymonthview, setSearchFlag)
//{
//    mDayMonthView->setSearchFlag(true);
//}

////void CDayMonthView::setHuangLiInfo(const CaHuangLiDayInfo &huangLiInfo)
//TEST_F(test_daymonthview, setHuangLiInfo)
//{
//    mDayMonthView->setHuangLiInfo(dayMonthViewGetHuangLiDayInfo().value(QDate::currentDate()));
//}

////void CDayMonthView::setHasScheduleFlag(const QVector<bool> &hasScheduleFlag)
//TEST_F(test_daymonthview, setHasScheduleFlag)
//{
//    mDayMonthView->setHasScheduleFlag(dayMonthViewGetLineFlag());
//}

////void CDayMonthView::updateDateLunarDay()
//TEST_F(test_daymonthview, updateDateLunarDay)
//{
//    mDayMonthView->updateDateLunarDay();
//}

////void CDayMonthView::changeSelectDate(const QDate &date)
//TEST_F(test_daymonthview, changeSelectDate)
//{
//    mDayMonthView->changeSelectDate(QDate::currentDate());
//}

////void CDayMonthView::slotprev()
//TEST_F(test_daymonthview, slotprev)
//{
//    mDayMonthView->setShowDate(dayMonthviewGetDayList(), QDate::currentDate().addDays(1), QDate::currentDate().addDays(1));
//    mDayMonthView->slotprev();
//}

////void CDayMonthView::slotnext()
//TEST_F(test_daymonthview, slotnext)
//{
//    mDayMonthView->setShowDate(dayMonthviewGetDayList(), QDate::currentDate().addDays(1), QDate::currentDate().addDays(1));
//    mDayMonthView->slotnext();
//}

////void CDayMonthView::slottoday()
//TEST_F(test_daymonthview, slottoday)
//{
//    mDayMonthView->setShowDate(dayMonthviewGetDayList(), QDate::currentDate().addDays(1), QDate::currentDate().addDays(1));
//    mDayMonthView->slottoday();
//}
