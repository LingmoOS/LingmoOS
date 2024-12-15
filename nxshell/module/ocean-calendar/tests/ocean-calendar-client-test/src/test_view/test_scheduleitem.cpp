// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_scheduleitem.h"

#include "../third-party_stub/stub.h"

#include <QPainter>

//test_scheduleitem::test_scheduleitem()
//{
//    QRectF rectf;
//    mScheduleItem = new CScheduleItem(rectf);
//}

//test_scheduleitem::~test_scheduleitem()
//{
//    delete mScheduleItem;
//    mScheduleItem = nullptr;
//}

//QVector<ScheduleDataInfo> getScheduleItemDInfo()
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

////void CScheduleItem::setData(const ScheduleDataInfo &info, QDate date, int totalNum)
//TEST_F(test_scheduleitem, setData)
//{
//    ScheduleDataInfo scheduleinof = getScheduleItemDInfo().first();
//    mScheduleItem->setData(scheduleinof, QDate::currentDate(), 4);
//}

////bool CScheduleItem::hasSelectSchedule(const ScheduleDataInfo &info)
//TEST_F(test_scheduleitem, hasSelectSchedule)
//{
//    ScheduleDataInfo scheduleinfo1 = getScheduleItemDInfo().first();
//    ScheduleDataInfo scheduleinfo2 = getScheduleItemDInfo().at(1);
//    mScheduleItem->setData(scheduleinfo1, QDate::currentDate(), 4);
//    bool res = mScheduleItem->hasSelectSchedule(scheduleinfo1);
//    EXPECT_TRUE(res);
//    res = mScheduleItem->hasSelectSchedule(scheduleinfo2);
//    EXPECT_FALSE(res);
//}

////void CScheduleItem::splitText(QFont font, int w, int h, QString str, QStringList &liststr, QFontMetrics &fontm)
//TEST_F(test_scheduleitem, splitText)
//{
//    QFont font;
//    QString str = "helo";
//    QStringList strlist("hello,word!");
//    QFontMetrics fontmetrics(font);
//    mScheduleItem->splitText(font, 10, 12, str, strlist, fontmetrics);
//    mScheduleItem->splitText(font, 40, 40, str, strlist, fontmetrics);
//}

//bool getItemFocus_Stub()
//{
//    return true;
//}

////paintBackground
//TEST_F(test_scheduleitem, paintBackground)
//{
//    QRectF rectf(0, 0, 100, 100);
//    mScheduleItem->setRect(rectf);
//    mScheduleItem->setData(getScheduleItemDInfo().first(), QDate::currentDate(), 4);
//    QPixmap pixmap(rectf.toRect().size());
//    pixmap.fill(Qt::transparent);
//    QPainter painter(&pixmap);
//    mScheduleItem->m_vHighflag = true;
//    mScheduleItem->m_vSelectflag = false;
//    mScheduleItem->paintBackground(&painter, pixmap.rect(), true);
//    Stub stub;
//    stub.set(ADDR(CFocusItem, getItemFocus), getItemFocus_Stub);
//    mScheduleItem->m_vSelectflag = true;
//    mScheduleItem->paintBackground(&painter, pixmap.rect(), true);
//}
