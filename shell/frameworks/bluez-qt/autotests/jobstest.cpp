/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "jobstest.h"
#include "device.h"
#include "initmanagerjob.h"
#include "initobexmanagerjob.h"
#include "manager.h"
#include "obexmanager.h"

#include <QSignalSpy>
#include <QTest>

using namespace BluezQt;

void JobsTest::deleteManagerTest()
{
    Manager *manager = new Manager();
    InitManagerJob *job = manager->init();
    QSignalSpy jobSpy(job, SIGNAL(result(InitManagerJob *)));

    job->start();
    delete manager;

    // Deleting manager while init job is in progress should emit error
    QTRY_COMPARE(jobSpy.count(), 1);
}

void JobsTest::deleteInitManagerJobTest()
{
    Manager *manager = new Manager();
    InitManagerJob *job = manager->init();
    QSignalSpy jobSpy(job, SIGNAL(result(InitManagerJob *)));

    job->start();
    delete job;

    // Deleting the job should emit error
    QTRY_COMPARE(jobSpy.count(), 1);

    delete manager;

    manager = new Manager();
    job = manager->init();
    QSignalSpy jobSpy2(job, SIGNAL(result(InitManagerJob *)));

    job->start();
    job->kill();

    // Killing the job should NOT emit error
    QTRY_COMPARE(jobSpy2.count(), 0);
}

void JobsTest::deleteObexManagerTest()
{
    ObexManager *manager = new ObexManager();
    InitObexManagerJob *job = manager->init();
    QSignalSpy jobSpy(job, SIGNAL(result(InitObexManagerJob *)));

    job->start();
    delete manager;

    // Deleting manager while init job is in progress should emit error
    QTRY_COMPARE(jobSpy.count(), 1);
}

void JobsTest::deleteInitObexManagerJobTest()
{
    ObexManager *manager = new ObexManager();
    InitObexManagerJob *job = manager->init();
    QSignalSpy jobSpy(job, SIGNAL(result(InitObexManagerJob *)));

    job->start();
    delete job;

    // Deleting the job should emit error
    QTRY_COMPARE(jobSpy.count(), 1);

    delete manager;

    manager = new ObexManager();
    job = manager->init();
    QSignalSpy jobSpy2(job, SIGNAL(result(InitObexManagerJob *)));

    job->start();
    job->kill();

    // Killing the job should NOT emit error
    QTRY_COMPARE(jobSpy2.count(), 0);
}

QTEST_MAIN(JobsTest)

#include "moc_jobstest.cpp"
