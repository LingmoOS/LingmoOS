// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_jobremindmanager.h"
#include "../third-party_stub/stub.h"
#include "service_stub.h"

#include <QDBusAbstractInterface>
#include <QDebug>

void stub_callWithArgumentList(QDBus::CallMode mode,
                               const QString &method,
                               const QList<QVariant> &args)
{
    Q_UNUSED(mode);
    Q_UNUSED(method);
    Q_UNUSED(args);
}

test_jobremindmanager::test_jobremindmanager()
{
    Stub stub;
    stub.set(ADDR(QDBusAbstractInterface, callWithArgumentList), stub_callWithArgumentList);
    jobRemindManager = new JobRemindManager();
}

test_jobremindmanager::~test_jobremindmanager()
{
    delete jobRemindManager;
}

QList<Job> createJobs()
{
    Job job1;
    job1.Start = QDateTime::fromString("2020-12-21T00:00:00+08:00", Qt::ISODate);
    job1.End = QDateTime::fromString("2020-12-21T23:59:00+08:00", Qt::ISODate);
    job1.AllDay = true;
    job1.Type = 1;
    job1.Description = "";
    job1.ID = 1;
    job1.Ignore = "[]";
    job1.Title = "UT测试A";
    job1.RRule = "FREQ=DAILY;BYDAY=MO,TU,WE,TH,FR;COUNT=3";
    job1.RecurID = 0;
    job1.Remind = "1;09:00";

    Job job2;
    job2.Start = QDateTime::fromString("2020-12-22T00:00:00+08:00", Qt::ISODate);
    job2.End = QDateTime::fromString("2020-12-22T23:59:00+08:00", Qt::ISODate);
    job2.AllDay = true;
    job2.Type = 1;
    job2.Description = "";
    job2.ID = 2;
    job2.Ignore = "[\"2020-12-11T00:00:00+08:00\"]";
    job2.Title = "UT测试B";
    job2.RRule = "FREQ=DAILY;BYDAY=MO,TU,WE,TH,FR;COUNT=2";
    job2.RecurID = 0;
    job2.Remind = "1;09:00";

    Job job3;
    job3.Start = QDateTime::fromString("2020-12-24T00:00:00+08:00", Qt::ISODate);
    job3.End = QDateTime::fromString("2020-12-24T23:59:00+08:00", Qt::ISODate);
    job3.AllDay = true;
    job3.Type = 1;
    job3.Description = "";
    job3.ID = 3;
    job3.Ignore = "[]";
    job3.Title = "UT测试C";
    job3.RRule = "FREQ=DAILY;BYDAY=MO,TU,WE,TH,FR;COUNT=4";
    job3.RecurID = 0;
    job3.Remind = "1;09:00";

    QList<Job> jobs;
    jobs << job1 << job2 << job3;
    return jobs;
}

//void JobRemindManager::CallUiOpenSchedule(const Job &job)
TEST_F(test_jobremindmanager, CallUiOpenSchedule)
{
    Stub stub;
    qDBusAbstractInterface_callWithArgumentList_stub(stub);
    jobRemindManager->CallUiOpenSchedule(createJobs().at(0));
}

//void JobRemindManager::RemindJob(const Job &job)
TEST_F(test_jobremindmanager, RemindJob)
{
    Stub stub;
    qDBusAbstractInterface_callWithArgumentList_stub(stub);
    jobRemindManager->RemindJob(createJobs().at(0));
}

//int JobRemindManager::GetRemindAdvanceDays(const QString &remind)
TEST_F(test_jobremindmanager, GetRemindAdvanceDays)
{
    const QString remind = "1,09:00";
    jobRemindManager->GetRemindAdvanceDays(remind);
}

//void JobRemindManager::RemindJobLater(const Job &job)
TEST_F(test_jobremindmanager, RemindJobLater)
{
    jobRemindManager->RemindJobLater(createJobs().at(0), 1);
}

//void JobRemindManager::UpdateRemindJobs(const QList<Job> &jobs)
TEST_F(test_jobremindmanager, UpdateRemindJobs)
{
    jobRemindManager->UpdateRemindJobs(createJobs());
}
