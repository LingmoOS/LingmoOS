// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_weekheadview.h"
#include "../third-party_stub/stub.h"

//test_weekheadview::test_weekheadview()
//{
//    mWeekHeadView = new CWeekHeadView();
//}

//test_weekheadview::~test_weekheadview()
//{
//    delete mWeekHeadView;
//    mWeekHeadView = nullptr;
//}

//QMap<QDate, CaHuangLiDayInfo> getHuangLiDayInfo()
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

//QVector<QDate> getDayList()
//{
//    QVector<QDate> dateList {};
//    QDate currentDate = QDate::currentDate();
//    dateList.append(currentDate);
//    dateList.append(currentDate.addDays(1));
//    dateList.append(currentDate.addDays(2));
//    dateList.append(currentDate.addDays(3));
//    dateList.append(currentDate.addDays(4));
//    dateList.append(currentDate.addDays(5));
//    dateList.append(currentDate.addDays(6));

//    return dateList;
//}
////void CWeekHeadView::setTheMe(int type)
//TEST_F(test_weekheadview, setTheMe)
//{
//    mWeekHeadView->setTheMe(1);
//    mWeekHeadView->setTheMe(2);
//}

////void CWeekHeadView::setWeekDay(QVector<QDate> vDays, const QDate &selectDate)
//TEST_F(test_weekheadview, setWeekDay)
//{
//    QVector<QDate> dateList {};
//    dateList.append(getDayList().first());
//    mWeekHeadView->setWeekDay(dateList, QDate::currentDate());
//    mWeekHeadView->setWeekDay(getDayList(), QDate::currentDate());
//}

////void CWeekHeadView::setLunarVisible(bool visible)
//TEST_F(test_weekheadview, setLunarvisible)
//{
//    mWeekHeadView->setLunarVisible(false);
//    mWeekHeadView->setLunarVisible(true);
//}

////const QString CWeekHeadView::getCellDayNum(int pos)
//TEST_F(test_weekheadview, getCellDayNum)
//{
//    mWeekHeadView->setWeekDay(getDayList(), QDate::currentDate());
//    mWeekHeadView->getCellDayNum(4);
//}

////const QDate CWeekHeadView::getCellDate(int pos)
//TEST_F(test_weekheadview, getCellDate)
//{
//    mWeekHeadView->setWeekDay(getDayList(), QDate::currentDate());
//    mWeekHeadView->getCellDate(4);
//}

////const QString CWeekHeadView::getLunar(int pos)
//TEST_F(test_weekheadview, getLunar)
//{
//    mWeekHeadView->setWeekDay(getDayList(), QDate::currentDate());
//    mWeekHeadView->setHunagLiInfo(getHuangLiDayInfo());
//    mWeekHeadView->getLunar(1);
//}

//TEST_F(test_weekheadview, getPixmap)
//{
//    mWeekHeadView->setWeekDay(getDayList(), QDate::currentDate());
//    mWeekHeadView->setFixedSize(600, 80);
//    QPixmap pixmap(mWeekHeadView->size());
//    mWeekHeadView->render(&pixmap);
//}
