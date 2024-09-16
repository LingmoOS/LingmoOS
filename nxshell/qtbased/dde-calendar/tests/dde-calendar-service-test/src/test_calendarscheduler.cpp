// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_calendarscheduler.h"
#include "../third-party_stub/stub.h"
#include "service_stub.h"

test_calendarscheduler::test_calendarscheduler()
{

}

TEST_F(test_calendarscheduler, DeleteJob_01)
{
    mCalendar->DeleteJob(0);
}

TEST_F(test_calendarscheduler, GetJob_01)
{
    EXPECT_TRUE(mCalendar->GetJob(0).isEmpty());
}

TEST_F(test_calendarscheduler, CreateJob_01)
{
    mCalendar->CreateJob("");
}

TEST_F(test_calendarscheduler, UpdateJob_01)
{
    mCalendar->UpdateJob("");
}

TEST_F(test_calendarscheduler, GetJobs_01)
{
    QDateTime starTime;
    QDateTime endTime = starTime.addDays(1);
    EXPECT_FALSE(mCalendar->GetJobs(starTime, endTime).isEmpty());
}

TEST_F(test_calendarscheduler, QueryJobs_01)
{
    QString str("{\"End\":\"2022-10-22T14:52:29+08:00\",\"Key\":\"sd\",\"Start\":\"2021-10-22T14:52:29+08:00\"}");
    EXPECT_FALSE(mCalendar->QueryJobs(str).isEmpty());
}

TEST_F(test_calendarscheduler, QueryJobsWithLimit_01)
{
    QString str("{\"End\":\"2022-10-22T14:52:29+08:00\",\"Key\":\"sd\",\"Start\":\"2021-10-22T14:52:29+08:00\"}");
    EXPECT_FALSE(mCalendar->QueryJobsWithLimit(str, 5).isEmpty());
}

TEST_F(test_calendarscheduler, QueryJobsWithRule_01)
{
    QString str("{\"End\":\"2022-10-22T14:52:29+08:00\",\"Key\":\"sd\",\"Start\":\"2021-10-22T14:52:29+08:00\"}");
    EXPECT_FALSE(mCalendar->QueryJobsWithRule(str, "").isEmpty());
}

TEST_F(test_calendarscheduler, CreateJobType_01)
{
    QString str("[{\"Authority\":7,\"ColorHex\":\"#5bdd80\",\"ColorTypeNo\":4,\"JobTypeName\":\"123\",\"JobTypeNo\":8}]");
    EXPECT_FALSE(mCalendar->CreateJobType(str));
}

TEST_F(test_calendarscheduler, DeleteJobType_01)
{
    mCalendar->DeleteJobType(7);
}

TEST_F(test_calendarscheduler, UpdateJobType_01)
{
    QString str("[{\"Authority\":7,\"ColorHex\":\"#5bdd80\",\"ColorTypeNo\":4,\"JobTypeName\":\"123\",\"JobTypeNo\":8}]");
    mCalendar->UpdateJobType(str);
}

TEST_F(test_calendarscheduler, GetJobTypeList_01)
{
    mCalendar->GetJobTypeList().isEmpty();
}

TEST_F(test_calendarscheduler, isJobTypeUsed_01)
{
    mCalendar->isJobTypeUsed(0);
}

TEST_F(test_calendarscheduler, GetColorTypeList_01)
{
    mCalendar->GetColorTypeList().isEmpty();
}

TEST_F(test_calendarscheduler, UpdateRemindTimeout_01)
{
     mCalendar->UpdateRemindTimeout(true);
     mCalendar->UpdateRemindTimeout(false);
}

TEST_F(test_calendarscheduler, notifyMsgHanding_01)
{
    for (int i = 0; i < 25; i++) {
        mCalendar->notifyMsgHanding(1, 1, i);
    }
}

TEST_F(test_calendarscheduler, initConnections_01)
{
    mCalendar->initConnections();
}

TEST_F(test_calendarscheduler, GetFestivalId_01)
{
    mCalendar->GetFestivalId("123");
}

TEST_F(test_calendarscheduler, IsFestivalJobEnabled_01)
{
    mCalendar->IsFestivalJobEnabled();
}

TEST_F(test_calendarscheduler, GetJobTimesBetween_01)
{
    QString str("{\"AllDay\":true,\"Description\":\"\",\"End\":\"2022-04-07T23:59:00+08:00\",\"ID\":0,\"Ignore\":[],\"IsLunar\":false,\"RRule\":\"\",\"RecurID\":0,\"Remind\":\"1;09:00\",\"Start\":\"2022-04-06T00:00:00+08:00\",\"Title\":\"新建日程\",\"Type\":1}");
    Job job = mCalendar->josnStringToJob(str);
    QDateTime starTime;
    QDateTime endTime = starTime.addDays(-10);

    EXPECT_TRUE(mCalendar->GetJobTimesBetween(starTime, endTime, job).isEmpty());
}

TEST_F(test_calendarscheduler, ParseRRule_01)
{
    QString str = "BYDAY=MO,TU,WE,TH,FR;COUNT=2;UNTIL=3";
    QStringList sList;
    sList << ";FREQ=DAILY" << ";FREQ=WEEKLY" << ";FREQ=MONTHLY" << ";FREQ=YEARLY";
    for (QString s : sList) {
        mCalendar->ParseRRule(str + s);
    }
}

TEST_F(test_calendarscheduler, GetJobRemindTime_01)
{
    QString str("{\"AllDay\":true,\"Description\":\"\",\"End\":\"2022-04-07T23:59:00+08:00\",\"ID\":0,\"Ignore\":[],\"IsLunar\":false,\"RRule\":\"\",\"RecurID\":0,\"Remind\":\"1;09:00\",\"Start\":\"2022-04-06T00:00:00+08:00\",\"Title\":\"新建日程\",\"Type\":1}");
    Job job = mCalendar->josnStringToJob(str);
    mCalendar->GetJobRemindTime(job);
}

TEST_F(test_calendarscheduler, josnStringToJob_01)
{
    QString str("{\"AllDay\":true,\"Description\":\"\",\"End\":\"2022-04-07T23:59:00+08:00\",\"ID\":0,\"Ignore\":[],\"IsLunar\":false,\"RRule\":\"\",\"RecurID\":0,\"Remind\":\"1;09:00\",\"Start\":\"2022-04-06T00:00:00+08:00\",\"Title\":\"新建日程\",\"Type\":1}");
    Job job = mCalendar->josnStringToJob(str);
    EXPECT_TRUE(job.AllDay);
    EXPECT_TRUE(job.ID == 0);
}

TEST_F(test_calendarscheduler, getRemindTimeByCount_01)
{
    mCalendar->getRemindTimeByCount(0);
}

TEST_F(test_calendarscheduler, getRemindTimeByMesc_01)
{
    mCalendar->getRemindTimeByMesc(0);
}

TEST_F(test_calendarscheduler, closeNotification_01)
{
    mCalendar->closeNotification(0);
}

TEST_F(test_calendarscheduler, OnModifyJobRemind_01)
{
    QString str("{\"AllDay\":true,\"Description\":\"\",\"End\":\"2022-04-07T23:59:00+08:00\",\"ID\":0,\"Ignore\":[],\"IsLunar\":false,\"RRule\":\"\",\"RecurID\":0,\"Remind\":\"1;09:00\",\"Start\":\"2022-04-06T00:00:00+08:00\",\"Title\":\"新建日程\",\"Type\":1}");
    Job job = mCalendar->josnStringToJob(str);
    mCalendar->OnModifyJobRemind(job, "");
}

TEST_F(test_calendarscheduler, saveNotifyID_01)
{
    QString str("{\"AllDay\":true,\"Description\":\"\",\"End\":\"2022-04-07T23:59:00+08:00\",\"ID\":0,\"Ignore\":[],\"IsLunar\":false,\"RRule\":\"\",\"RecurID\":0,\"Remind\":\"1;09:00\",\"Start\":\"2022-04-06T00:00:00+08:00\",\"Title\":\"新建日程\",\"Type\":1}");
    Job job = mCalendar->josnStringToJob(str);
    mCalendar->saveNotifyID(job, 1);
}
