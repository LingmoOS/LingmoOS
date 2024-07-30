/*
    SPDX-FileCopyrightText: 2007 John Tapsell <tapsell@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QDebug>
#include <QProcess>
#include <QtCore>
#include <QtTestGui>

#include <limits>

#include "processcore/process.h"
#include "processcore/processes.h"
#include "processcore/processes_base_p.h"
#include "processcore_debug.h"

#include "processtest.h"

Q_DECLARE_METATYPE(KSysGuard::Process::Scheduler);
Q_DECLARE_METATYPE(KSysGuard::Process::IoPriorityClass);

void testProcess::testSetScheduler_data()
{
    QTest::addColumn<int>("priority");
    QTest::addColumn<KSysGuard::Process::Scheduler>("scheduler");
    QTest::addColumn<bool>("niceness");

    QTest::addRow("normal, 0") << 0 << KSysGuard::Process::Scheduler::Other << true;
    QTest::addRow("normal, 5") << 5 << KSysGuard::Process::Scheduler::Other << true;

    QTest::addRow("batch, 0") << 0 << KSysGuard::Process::Scheduler::Batch << true;
    QTest::addRow("batch, 5") << 5 << KSysGuard::Process::Scheduler::Batch << true;

    QTest::addRow("idle, 0") << 0 << KSysGuard::Process::Scheduler::SchedulerIdle << false;

    QTest::addRow("rr, 5") << 5 << KSysGuard::Process::Scheduler::RoundRobin << false;

    QTest::addRow("fifo, 5") << 5 << KSysGuard::Process::Scheduler::Fifo << false;
}

void testProcess::testSetScheduler()
{
    QFETCH(int, priority);
    QFETCH(KSysGuard::Process::Scheduler, scheduler);
    QFETCH(bool, niceness);

    KSysGuard::Processes *processController = new KSysGuard::Processes();

    QProcess proc;
    proc.start(QStringLiteral("sleep"), {QStringLiteral("100")});
    QVERIFY(proc.waitForStarted());

    int pid=proc.processId();
    QVERIFY(pid);

    if (!processController->setScheduler(pid, scheduler, priority))
        QSKIP("skipping verfifcation because setScheduler failed");

    if (niceness && !processController->setNiceness(pid, priority))
        QSKIP("skipping verfifcation because setNiceness failed");

    processController->updateAllProcesses();
    KSysGuard::Process* process = processController->getProcess(pid);

    QVERIFY(process);
    QCOMPARE(process->scheduler(), scheduler);
    QCOMPARE(process->niceLevel(), priority);
}

void testProcess::testSetIoScheduler_data()
{
    QTest::addColumn<int>("priority");
    QTest::addColumn<KSysGuard::Process::IoPriorityClass>("prioClass");

    QTest::addRow("idle, 0") << 0 << KSysGuard::Process::Idle;
    QTest::addRow("idle, 5") << 5 << KSysGuard::Process::Idle;

    QTest::addRow("best-effort, 0") << 0 << KSysGuard::Process::BestEffort;
    QTest::addRow("best-effort, 5") << 5 << KSysGuard::Process::BestEffort;

    QTest::addRow("rt, 0") << 0 << KSysGuard::Process::RealTime;
    QTest::addRow("rt, 5") << 5 << KSysGuard::Process::RealTime;
}

void testProcess::testSetIoScheduler()
{
    QFETCH(int, priority);
    QFETCH(KSysGuard::Process::IoPriorityClass, prioClass);

    KSysGuard::Processes *processController = new KSysGuard::Processes();

    QProcess proc;
    proc.start(QStringLiteral("sleep"), {QStringLiteral("100")});
    QVERIFY(proc.waitForStarted());

    int pid=proc.processId();
    QVERIFY(pid);

    if (!processController->setIoNiceness(pid, prioClass, priority))
        QSKIP("skipping verfifcation because setNiceness failed");

    processController->updateAllProcesses();
    KSysGuard::Process* process = processController->getProcess(pid);

    QVERIFY(process);
    QCOMPARE(process->ioPriorityClass(), prioClass);
    QCOMPARE(process->ioniceLevel(), priority);
}

void testProcess::testProcesses()
{
    KSysGuard::Processes *processController = new KSysGuard::Processes();
    processController->updateAllProcesses();
    const QList<KSysGuard::Process *> processes = processController->getAllProcesses();
    QSet<long> pids;
    for (KSysGuard::Process *process : processes) {
        if (process->pid() == 0)
            continue;
        QVERIFY(process->pid() > 0);
        QVERIFY(!process->name().isEmpty());

        // test all the pids are unique
        QVERIFY(!pids.contains(process->pid()));
        pids.insert(process->pid());
    }
    processController->updateAllProcesses();
    const QList<KSysGuard::Process *> processes2 = processController->getAllProcesses();
    for (KSysGuard::Process *process : processes2) {
        if (process->pid() == 0)
            continue;
        QVERIFY(process->pid() > 0);
        QVERIFY(!process->name().isEmpty());

        // test all the pids are unique
        if (!pids.contains(process->pid())) {
            qCDebug(LIBKSYSGUARD_PROCESSCORE) << process->pid() << " not found. " << process->name();
        }
        pids.remove(process->pid());
    }

    QVERIFY(processes2.size() == processes.size());
    QCOMPARE(processes,
             processes2); // Make sure calling it twice gives the same results.  The difference in time is so small that it really shouldn't have changed
    delete processController;
}

unsigned long testProcess::countNumChildren(KSysGuard::Process *p)
{
    unsigned long total = p->children().size();
    for (int i = 0; i < p->children().size(); i++) {
        total += countNumChildren(p->children()[i]);
    }
    return total;
}

void testProcess::testProcessesTreeStructure()
{
    KSysGuard::Processes *processController = new KSysGuard::Processes();
    processController->updateAllProcesses();

    auto verify_counts = [this](const auto processes) {
        for (KSysGuard::Process *process : processes) {
            QCOMPARE(countNumChildren(process), process->numChildren());

            for(int i = 0; i < process->children().size(); i++) {
                QVERIFY(process->children()[i]->parent());
                QCOMPARE(process->children()[i]->parent(), process);
            }
        }
    };

    verify_counts(processController->getAllProcesses());

    // this should test if the children accounting isn't off on updates
    QProcess proc;
    proc.start(QStringLiteral("/bin/sh"), {QStringLiteral("-c"), QStringLiteral("sleep 100& (sleep 50; sleep 50) & while true; do :; done")});
    QVERIFY(proc.waitForStarted());
    QTest::qSleep(2000);

    processController->updateAllProcesses();
    verify_counts(processController->getAllProcesses());

    proc.terminate();

    QVERIFY(proc.waitForFinished());
    processController->updateAllProcesses();
    verify_counts(processController->getAllProcesses());

    delete processController;
}

void testProcess::testProcessesModification()
{
    // We will modify the tree, then re-call getProcesses and make sure that it fixed everything we modified
    KSysGuard::Processes *processController = new KSysGuard::Processes();
    processController->updateAllProcesses();
    KSysGuard::Process *initProcess = processController->getProcess(1);

    if (!initProcess || initProcess->numChildren() < 3) {
        delete processController;
        return;
    }

    QVERIFY(initProcess);
    QVERIFY(initProcess->children()[0]);
    QVERIFY(initProcess->children()[1]);
    qCDebug(LIBKSYSGUARD_PROCESSCORE) << initProcess->numChildren();
    initProcess->children()[0]->setParent(initProcess->children()[1]);
    initProcess->children()[1]->children().append(initProcess->children()[0]);
    initProcess->children()[1]->numChildren()++;
    initProcess->numChildren()--;
    initProcess->children().removeAt(0);
    delete processController;
}

void testProcess::testTimeToUpdateAllProcesses()
{
    // See how long it takes to get process information
    KSysGuard::Processes *processController = new KSysGuard::Processes();
    QBENCHMARK {
        processController->updateAllProcesses();
    }
    delete processController;
}

void testProcess::testHistories()
{
    KSysGuard::Processes *processController = new KSysGuard::Processes();
    QBENCHMARK_ONCE {
        if (!processController->isHistoryAvailable()) {
            qWarning("History was not available");
            delete processController;
            return;
        }
    }
    QCOMPARE(processController->historyFileName(), QStringLiteral("/var/log/atop.log"));
    QList<QPair<QDateTime, uint>> history = processController->historiesAvailable();
    bool success = processController->setViewingTime(history[0].first);
    QVERIFY(success);
    QVERIFY(processController->viewingTime() == history[0].first);
    success = processController->setViewingTime(history[0].first.addSecs(-1));
    QVERIFY(success);
    QVERIFY(processController->viewingTime() == history[0].first);
    success = processController->setViewingTime(history[0].first.addSecs(-history[0].second - 1));
    QVERIFY(!success);
    QVERIFY(processController->viewingTime() == history[0].first);
    QCOMPARE(processController->historyFileName(), QStringLiteral("/var/log/atop.log"));

    // Test the tree structure
    processController->updateAllProcesses();
    const QList<KSysGuard::Process *> processes = processController->getAllProcesses();

    for (KSysGuard::Process *process : processes) {
        QCOMPARE(countNumChildren(process), process->numChildren());

        for (int i = 0; i < process->children().size(); i++) {
            QVERIFY(process->children()[i]->parent());
            QCOMPARE(process->children()[i]->parent(), process);
        }
    }

    // test all the pids are unique
    QSet<long> pids;
    for (KSysGuard::Process *process : processes) {
        if (process->pid() == 0)
            continue;
        QVERIFY(process->pid() > 0);
        QVERIFY(!process->name().isEmpty());

        QVERIFY(!pids.contains(process->pid()));
        pids.insert(process->pid());
    }
    delete processController;
}

void testProcess::testUpdateOrAddProcess()
{
    KSysGuard::Processes *processController = new KSysGuard::Processes();
    processController->updateAllProcesses();
    KSysGuard::Process *process;
    // Make sure that this doesn't crash at least
    processController->getProcess(0);
    process = processController->getProcess(1);
    if (process)
        QCOMPARE(process->pid(), 1l);

    // Make sure that this doesn't crash at least
    processController->updateOrAddProcess(1);
    processController->updateOrAddProcess(0);
    processController->updateOrAddProcess(-1);

    processController->updateOrAddProcess(std::numeric_limits<long>::max()-1);
    QVERIFY(processController->getProcess(std::numeric_limits<long>::max()-1));
    processController->updateAllProcesses();
    QVERIFY(processController->getProcess(std::numeric_limits<long>::max()-1));
    QCOMPARE(processController->getProcess(std::numeric_limits<long>::max()-1)->status(), KSysGuard::Process::Ended);
    processController->updateAllProcesses();
    QVERIFY(!processController->getProcess(std::numeric_limits<long>::max()-1));
}

QTEST_MAIN(testProcess)
