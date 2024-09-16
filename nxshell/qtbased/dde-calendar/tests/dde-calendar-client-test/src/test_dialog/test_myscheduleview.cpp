// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

//#include "test_myscheduleview.h"

//#include "../dialog_stub.h"

//#include <QEvent>
//#include <QApplication>

//QVector<ScheduleDataInfo> getScheduleViewData()
//{
//    ScheduleDataInfo schedule1, schedule2;

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

//    QVector<ScheduleDataInfo> scheduleList{};
//    scheduleList.append(schedule1);
//    scheduleList.append(schedule2);
//    return scheduleList;
//}

//test_myscheduleview::test_myscheduleview()
//{
//    ScheduleDataInfo scheduleinfo = getScheduleViewData().first();
//    mScheduleView = new CMyScheduleView(scheduleinfo);
//}

//test_myscheduleview::~test_myscheduleview()
//{
//    delete mScheduleView;
//    mScheduleView = nullptr;
//}

////void CMyScheduleView::setLabelTextColor(const int type)
//TEST_F(test_myscheduleview, setLabelTextColor)
//{
//    mScheduleView->setLabelTextColor(1);
//    mScheduleView->setLabelTextColor(2);
//}

////
//TEST_F(test_myscheduleview, CMyScheduleView)
//{
//    ScheduleDataInfo scheduleinfo;
//    QDateTime currentDateTime = QDateTime::currentDateTime();
//    scheduleinfo.setID(1);
//    scheduleinfo.setBeginDateTime(currentDateTime);
//    scheduleinfo.setEndDateTime(currentDateTime.addDays(1));
//    scheduleinfo.setTitleName("测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试"
//                              "测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试"
//                              "测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试"
//                              "测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试"
//                              "测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试"
//                              "测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试测试"
//                              "测试测试测试测试测试测试测试测试测试测试测试测试测试测试");
//    scheduleinfo.setAllDay(true);
//    scheduleinfo.setType(4);
//    scheduleinfo.setRecurID(0);
//    CMyScheduleView scheduleView(scheduleinfo);
//}

////FontChange
//TEST_F(test_myscheduleview, FontChange)
//{
//    QEvent event(QEvent::FontChange);
//    QApplication::sendEvent(mScheduleView, &event);
//}

////slotBtClick
//TEST_F(test_myscheduleview, slotBtClick)
//{
//    Stub stub;
//    calendarDDialogExecStub(stub);
//    mScheduleView->slotBtClick(1, "tt");
//}
