/*
    SPDX-FileCopyrightText: 2007 John Tapsell <tapsell@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PROCESSTEST_H
#define PROCESSTEST_H

#include <QObject>
namespace KSysGuard
{
class Process;
}
class testProcess : public QObject
{
    Q_OBJECT
private:
    unsigned long countNumChildren(KSysGuard::Process *p);
private slots:
    void testTimeToUpdateAllProcesses();
    void testProcesses();
    void testProcessesTreeStructure();
    void testProcessesModification();
    void testHistories();
    void testUpdateOrAddProcess();

    void testSetScheduler();
    void testSetScheduler_data();
    void testSetIoScheduler_data();
    void testSetIoScheduler();
};
#endif
