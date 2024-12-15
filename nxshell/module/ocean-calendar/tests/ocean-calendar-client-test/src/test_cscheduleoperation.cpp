// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_cscheduleoperation.h"

//#include "cscheduledbus.h"
//#include "dialog/dcalendarddialog.h"
//#include "schedulectrldlg.h"
//#include "dialog_stub.h"
//#include "cscheduledbusstub.h"

//test_cscheduleoperation::test_cscheduleoperation()
//{
//}

//test_cscheduleoperation::~test_cscheduleoperation()
//{
//}

//namespace ScheduleTestBtnNum {
//static int button_num = 0;
//}
//int clickButton_stub(void *obj)
//{
//    Q_UNUSED(obj)
//    return ScheduleTestBtnNum::button_num;
//}

//void test_cscheduleoperation::SetUp()
//{
//    cscheduleDbusStub(stub);
//}

//void test_cscheduleoperation::TearDown()
//{
//}

//TEST_F(test_cscheduleoperation, createSchedule)
//{
//    ScheduleDataInfo info;
//    operation.createSchedule(info);
//}

//TEST_F(test_cscheduleoperation, changeSchedule)
//{
//    calendarDDialogExecStub(stub);
//    stub.set(ADDR(CScheduleCtrlDlg, clickButton), clickButton_stub);
//    ScheduleDataInfo info;
//    QDate current = QDate::currentDate();
//    info.setBeginDateTime(QDateTime(current, QTime(0, 0, 0)));
//    info.setEndDateTime(QDateTime(current, QTime(23, 59, 59)));
//    info.setTitleName(tr("New Event"));
//    info.setAllDay(true);
//    info.setRemindData(RemindData(1, QTime(9, 0)));
//    info.setID(0);
//    info.setRecurID(0);

//    ScheduleDataInfo newinfo = info;
//    newinfo.setTitleName(tr("test"));

//    newinfo.setAllDay(false);
//    info.getRepetitionRule().setRuleId(RepetitionRule::RRule_NONE);
//    ScheduleTestBtnNum::button_num = 0;
//    operation.changeSchedule(newinfo, info);
//    ScheduleTestBtnNum::button_num = 1;
//    operation.changeSchedule(newinfo, info);

//    newinfo.setAllDay(true);
//    operation.changeSchedule(newinfo, info);
//    ScheduleTestBtnNum::button_num = 0;
//    newinfo.getRepetitionRule().setRuleId(RepetitionRule::RRule_EVEDAY);
//    operation.changeSchedule(newinfo, info);
//    ScheduleTestBtnNum::button_num = 1;
//    operation.changeSchedule(newinfo, info);
//    info.getRepetitionRule().setRuleId(RepetitionRule::RRule_EVEDAY);
//    operation.changeSchedule(newinfo, info);
//    ScheduleTestBtnNum::button_num = 0;
//    operation.changeSchedule(newinfo, info);
//    ScheduleTestBtnNum::button_num = 2;
//    operation.changeSchedule(newinfo, info);

//    newinfo.setRecurID(2);
//    ScheduleTestBtnNum::button_num = 0;
//    operation.changeSchedule(newinfo, info);
//    ScheduleTestBtnNum::button_num = 1;
//    operation.changeSchedule(newinfo, info);
//    ScheduleTestBtnNum::button_num = 2;
//    operation.changeSchedule(newinfo, info);
//}

//TEST_F(test_cscheduleoperation, changeSchedule_01)
//{
//    calendarDDialogExecStub(stub);
//    ScheduleDataInfo oldInfo;
//    ScheduleDataInfo newInfo;
//    oldInfo.setAllDay(true);
//    oldInfo.m_ScheduleRRule.setRuleId(RepetitionRule::RRule_NONE);
//    newInfo.setAllDay(false);
//    newInfo.m_ScheduleRRule.setRuleId(RepetitionRule::RRule_EVEDAY);
//    operation.changeSchedule(oldInfo, newInfo);
//}

//TEST_F(test_cscheduleoperation, deleteSchedule)
//{
//    calendarDDialogExecStub(stub);
//    stub.set(ADDR(CScheduleCtrlDlg, clickButton), clickButton_stub);
//    ScheduleDataInfo info;
//    QDate current = QDate::currentDate();
//    info.setBeginDateTime(QDateTime(current, QTime(0, 0, 0)));
//    info.setEndDateTime(QDateTime(current, QTime(23, 59, 59)));
//    info.setTitleName(tr("New Event"));
//    info.setAllDay(true);
//    info.setRemindData(RemindData(1, QTime(9, 0)));
//    info.setID(0);
//    info.setRecurID(0);
//    ScheduleTestBtnNum::button_num = 0;
//    operation.deleteSchedule(info);
//    ScheduleTestBtnNum::button_num = 1;
//    operation.deleteSchedule(info);

//    //删除重复日程
//    info.getRepetitionRule().setRuleId(RepetitionRule::RRule_EVEDAY);
//    ScheduleTestBtnNum::button_num = 0;
//    operation.deleteSchedule(info);
//    ScheduleTestBtnNum::button_num = 1;
//    operation.deleteSchedule(info);
//    ScheduleTestBtnNum::button_num = 2;
//    operation.deleteSchedule(info);

//    info.setRecurID(2);
//    ScheduleTestBtnNum::button_num = 0;
//    operation.deleteSchedule(info);
//    ScheduleTestBtnNum::button_num = 1;
//    operation.deleteSchedule(info);
//    ScheduleTestBtnNum::button_num = 2;
//    operation.deleteSchedule(info);
//}

//TEST_F(test_cscheduleoperation, queryScheduleStr)
//{
//    QDateTime currenttime = QDateTime::currentDateTime();
//    operation.queryScheduleStr("", currenttime, currenttime);
//}

//TEST_F(test_cscheduleoperation, deleteOnlyInfo_01)
//{
//    ScheduleDataInfo info;
//    operation.deleteOnlyInfo(info);
//}

//TEST_F(test_cscheduleoperation, deleteOnlyInfo_02)
//{
//    ScheduleDataInfo info;
//    info.m_ScheduleRRule.setRuleId(RepetitionRule::RRule_EVEDAY);
//    operation.deleteOnlyInfo(info);
//}

//TEST_F(test_cscheduleoperation, deleteOnlyInfo_03)
//{
//    ScheduleDataInfo info;
//    info.setType(4);
//    operation.deleteOnlyInfo(info);
//}

//TEST_F(test_cscheduleoperation, queryScheduleInfo_01)
//{
//    ScheduleDataInfo info;
//    QDateTime date;
//    QMap<QDate, QVector<ScheduleDataInfo>> map;
//    operation.queryScheduleInfo("", date, date, map);
//}

//TEST_F(test_cscheduleoperation, updateJobType_01)
//{
//    calendarDDialogExecStub(stub);
//    JobTypeInfo oldInfo(0);
//    JobTypeInfo newInfo;
//    operation.updateJobType(oldInfo, newInfo);
//}

//TEST_F(test_cscheduleoperation, updateJobType_02)
//{
//    calendarDDialogExecStub(stub);
//    JobTypeInfo oldInfo(1);
//    JobTypeInfo newInfo(1);
//    EXPECT_TRUE(operation.updateJobType(oldInfo, newInfo));
//}

//TEST_F(test_cscheduleoperation, updateJobType_03)
//{
//    calendarDDialogExecStub(stub);
//    JobTypeInfo oldInfo(1, "", 10);
//    JobTypeInfo newInfo(0, "", 0);
//    operation.updateJobType(oldInfo, newInfo);
//    EXPECT_EQ(oldInfo.getColorTypeNo(), newInfo.getColorTypeNo());
//}

//TEST_F(test_cscheduleoperation, updateJobType_04)
//{
//    calendarDDialogExecStub(stub);
//    JobTypeInfo oldInfo(1, "", 5);
//    JobTypeInfo newInfo(0, "", 0);
//    operation.updateJobType(oldInfo, newInfo);
//    EXPECT_TRUE(newInfo.getColorTypeNo() != 5);
//}

//TEST_F(test_cscheduleoperation, updateJobType_05)
//{
//    calendarDDialogExecStub(stub);
//    JobTypeInfo oldInfo(1, "", 5);
//    operation.updateJobType(oldInfo);
//}
