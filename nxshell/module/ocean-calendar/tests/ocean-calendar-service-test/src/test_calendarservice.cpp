// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_calendarservice.h"

test_calendarservice::test_calendarservice()
{

}

TEST_F(test_calendarservice, GetFestivalMonth_01)
{
    mService->GetFestivalMonth(2022, 4);
}

TEST_F(test_calendarservice, GetHuangLiDay_01)
{
    EXPECT_FALSE(mService->GetHuangLiDay(2022, 4, 22).isEmpty());
}

TEST_F(test_calendarservice, GetHuangLiMonth_01)
{
    EXPECT_FALSE(mService->GetHuangLiMonth(2022, 4, true).isEmpty());
}

TEST_F(test_calendarservice, GetLunarInfoBySolar_01)
{
    EXPECT_FALSE(mService->GetLunarInfoBySolar(2022, 4, true).mLunarDayName.isEmpty());
}

TEST_F(test_calendarservice, GetLunarMonthCalendar_01)
{
    EXPECT_TRUE(mService->GetLunarMonthCalendar(2022, 4, true).mDays);
}

TEST_F(test_calendarservice, CreateJob_01)
{
    QString str("{\"AllDay\":true,\"Description\":\"\",\"End\":\"2022-04-07T23:59:00+08:00\",\"ID\":1,\"Ignore\":[],\"IsLunar\":false,\"RRule\":\"\",\"RecurID\":0,\"Remind\":\"1;09:00\",\"Start\":\"2022-04-06T00:00:00+08:00\",\"Title\":\"新建日程\",\"Type\":1}");
    mService->CreateJob(str);
}

TEST_F(test_calendarservice, CreateType_01)
{
    QString str("[{\"Authority\":7,\"ColorHex\":\"#5bdd80\",\"ColorTypeNo\":4,\"JobTypeName\":\"123\",\"JobTypeNo\":12}]");
    mService->CreateType(str);
}

TEST_F(test_calendarservice, DeleteJob_01)
{
    mService->DeleteJob(1);
}

TEST_F(test_calendarservice, GetJob_01)
{
    mService->GetJob(1);
}

TEST_F(test_calendarservice, GetJobs_01)
{
    mService->GetJobs(2022, 2, 1, 2022, 4, 1);
}

TEST_F(test_calendarservice, UpdateJob_01)
{
    QString str("{\"AllDay\":true,\"Description\":\"\",\"End\":\"2022-04-07T23:59:00+08:00\",\"ID\":1,\"Ignore\":[],\"IsLunar\":false,\"RRule\":\"\",\"RecurID\":0,\"Remind\":\"1;09:00\",\"Start\":\"2022-04-06T00:00:00+08:00\",\"Title\":\"新建日程\",\"Type\":1}");
    mService->UpdateJob(str);
}

TEST_F(test_calendarservice, QueryJobs_01)
{
    QString str("{\"End\":\"2022-10-22T14:52:29+08:00\",\"Key\":\"sd\",\"Start\":\"2021-10-22T14:52:29+08:00\"}");
    EXPECT_FALSE(mService->QueryJobs(str).isEmpty());
}

TEST_F(test_calendarservice, QueryJobsWithLimit_01)
{
    QString str("{\"End\":\"2022-10-22T14:52:29+08:00\",\"Key\":\"sd\",\"Start\":\"2021-10-22T14:52:29+08:00\"}");
    EXPECT_FALSE(mService->QueryJobsWithLimit(str, 5).isEmpty());
}

TEST_F(test_calendarservice, QueryJobsWithRule_01)
{
    QString str("{\"End\":\"2022-10-22T14:52:29+08:00\",\"Key\":\"sd\",\"Start\":\"2021-10-22T14:52:29+08:00\"}");
    EXPECT_FALSE(mService->QueryJobsWithRule(str, "").isEmpty());
}

TEST_F(test_calendarservice, remindJob_01)
{
    mService->remindJob(1, 2);
}

TEST_F(test_calendarservice, updateRemindJob_01)
{
    mService->updateRemindJob(true);
}

TEST_F(test_calendarservice, CreateJobType_01)
{
    QString str("[{\"Authority\":7,\"ColorHex\":\"#5bdd80\",\"ColorTypeNo\":4,\"JobTypeName\":\"123\",\"JobTypeNo\":8}]");
    EXPECT_FALSE(mService->CreateJobType(str));
}

TEST_F(test_calendarservice, DeleteJobType_01)
{
    mService->DeleteJobType(7);
}

TEST_F(test_calendarservice, UpdateJobType_01)
{
    QString str("[{\"Authority\":7,\"ColorHex\":\"#5bdd80\",\"ColorTypeNo\":4,\"JobTypeName\":\"123\",\"JobTypeNo\":8}]");
    mService->UpdateJobType(str);
}

TEST_F(test_calendarservice, GetJobTypeList_01)
{
    mService->GetJobTypeList().isEmpty();
}

TEST_F(test_calendarservice, isJobTypeUsed_01)
{
    mService->isJobTypeUsed(0);
}

TEST_F(test_calendarservice, GetColorTypeList_01)
{
    mService->GetColorTypeList().isEmpty();
}

TEST_F(test_calendarservice, notifyMsgHanding_01)
{
    mService->notifyMsgHanding(1, 1, 1);
}

TEST_F(test_calendarservice, initConnections_01)
{
    mService->initConnections();
}
