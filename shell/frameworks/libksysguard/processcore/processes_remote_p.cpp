/*
    SPDX-FileCopyrightText: 2007 John Tapsell <tapsell@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "processes_remote_p.h"
#include "process.h"
#include "processcore_debug.h"

#include <QDebug>
#include <QString>
#include <QTimer>

namespace KSysGuard
{
class ProcessesRemote::Private
{
public:
    Private()
        : numColumns(0)
        , freeMemory(0)
    {
        havePsInfo = false;
        pidColumn = 1;
        ppidColumn = nameColumn = uidColumn = gidColumn = statusColumn = userColumn = systemColumn = niceColumn = vmSizeColumn = vmRSSColumn = loginColumn =
            commandColumn = tracerPidColumn = ttyColumn = ioprioClassColumn = ioprioColumn = vmURSSColumn = noNewPrivilegesColumn = cGroupColumn =
                macContextColumn = -1;
        usedMemory = freeMemory;
    }
    ~Private()
    {
    }
    QString host;
    QList<QByteArray> lastAnswer;
    QSet<long> pids;
    QHash<long, QList<QByteArray>> processByPid;

    bool havePsInfo;
    int pidColumn;
    int ppidColumn;
    int tracerPidColumn;
    int nameColumn;
    int uidColumn;
    int gidColumn;
    int statusColumn;
    int userColumn;
    int systemColumn;
    int niceColumn;
    int vmSizeColumn;
    int vmRSSColumn;
    int vmURSSColumn;
    int loginColumn;
    int commandColumn;
    int ioprioClassColumn;
    int ioprioColumn;
    int ttyColumn;
    int noNewPrivilegesColumn;
    int cGroupColumn;
    int macContextColumn;

    int numColumns;

    long freeMemory;
    long usedMemory;

    Processes::UpdateFlags updateFlags;
};
ProcessesRemote::ProcessesRemote(const QString &hostname)
    : d(new Private())
{
    d->host = hostname;
    QTimer::singleShot(0, this, &ProcessesRemote::setup);
}

void ProcessesRemote::setup()
{
    Q_EMIT runCommand(QStringLiteral("mem/physical/used"), (int)UsedMemory);
    Q_EMIT runCommand(QStringLiteral("mem/physical/free"), (int)FreeMemory);
    Q_EMIT runCommand(QStringLiteral("ps?"), (int)PsInfo);
    Q_EMIT runCommand(QStringLiteral("ps"), (int)Ps);
}

long ProcessesRemote::getParentPid(long pid)
{
    if (!d->processByPid.contains(pid)) {
        qCDebug(LIBKSYSGUARD_PROCESSCORE) << "Parent pid requested for pid that we do not have info on " << pid;
        return 0;
    }
    if (d->ppidColumn == -1) {
        qCDebug(LIBKSYSGUARD_PROCESSCORE) << "ppid column not known ";
        return 0;
    }
    return d->processByPid[pid].at(d->ppidColumn).toLong();
}
bool ProcessesRemote::updateProcessInfo(long pid, Process *process)
{
    Q_CHECK_PTR(process);
    if (!d->processByPid.contains(pid)) {
        qCDebug(LIBKSYSGUARD_PROCESSCORE) << "update request for pid that we do not have info on " << pid;
        return false;
    }
    QList<QByteArray> p = d->processByPid[pid];

    if (d->nameColumn != -1) {
        process->setName(QString::fromUtf8(p.at(d->nameColumn)));
    }
    if (d->uidColumn != -1) {
        process->setUid(p.at(d->uidColumn).toLong());
    }
    if (d->gidColumn != -1) {
        process->setGid(p.at(d->gidColumn).toLong());
    }
    if (d->statusColumn != -1) {
        switch (p.at(d->statusColumn)[0]) {
        case 's':
            process->setStatus(Process::Sleeping);
            break;
        case 'r':
            process->setStatus(Process::Running);
            break;
        }
    }
    if (d->userColumn != -1) {
        process->setUserTime(p.at(d->userColumn).toLong());
    }
    if (d->systemColumn != -1) {
        process->setSysTime(p.at(d->systemColumn).toLong());
    }
    if (d->niceColumn != -1) {
        process->setNiceLevel(p.at(d->niceColumn).toLong());
    }
    if (d->vmSizeColumn != -1) {
        process->setVmSize(p.at(d->vmSizeColumn).toLong());
    }
    if (d->vmRSSColumn != -1) {
        process->setVmRSS(p.at(d->vmRSSColumn).toLong());
    }
    if (d->vmURSSColumn != -1) {
        process->setVmURSS(p.at(d->vmURSSColumn).toLong());
    }
    if (d->loginColumn != -1) {
        process->setLogin(QString::fromUtf8(p.at(d->loginColumn).data()));
    }
    if (d->commandColumn != -1) {
        process->setCommand(QString::fromUtf8(p.at(d->commandColumn).data()));
    }
    if (d->tracerPidColumn != -1) {
        process->setTracerpid(p.at(d->tracerPidColumn).toLong());
    }
    if (d->vmURSSColumn != -1) {
        process->setVmURSS(p.at(d->vmURSSColumn).toLong());
    }
    if (d->ttyColumn != -1) {
        process->setTty(p.at(d->ttyColumn));
    }
    if (d->ioprioColumn != -1) {
        process->setIoniceLevel(p.at(d->ioprioColumn).toInt());
    }
    if (d->ioprioClassColumn != -1) {
        process->setIoPriorityClass((KSysGuard::Process::IoPriorityClass)(p.at(d->ioprioClassColumn).toInt()));
    }
    if (d->noNewPrivilegesColumn != -1) {
        process->setNoNewPrivileges(p.at(d->noNewPrivilegesColumn).toLong());
    }
    if (d->cGroupColumn != -1) {
        process->setCGroup(QString::fromUtf8(p.at(d->cGroupColumn)));
    }
    if (d->macContextColumn != -1) {
        process->setMACContext(QString::fromUtf8(p.at(d->macContextColumn)));
    }

    return true;
}

void ProcessesRemote::updateAllProcesses(Processes::UpdateFlags updateFlags)
{
    d->updateFlags = updateFlags;
    if (!d->havePsInfo) {
        Q_EMIT runCommand(QStringLiteral("ps?"), (int)PsInfo);
    }
    Q_EMIT runCommand(QStringLiteral("ps"), (int)Ps);
}
QSet<long> ProcessesRemote::getAllPids()
{
    d->pids.clear();
    d->processByPid.clear();
    Q_FOREACH (const QByteArray &process, d->lastAnswer) {
        QList<QByteArray> info = process.split('\t');
        if (info.size() == d->numColumns) {
            int pid = info.at(d->pidColumn).toLong();
            Q_ASSERT(!d->pids.contains(pid));
            d->pids << pid;
            d->processByPid[pid] = info;
        }
    }
    return d->pids;
}

Processes::Error ProcessesRemote::sendSignal(long pid, int sig)
{
    // TODO run the proper command for all these functions below
    Q_EMIT runCommand(QStringLiteral("kill ") + QString::number(pid) + QStringLiteral(" ") + QString::number(sig), (int)Kill);
    return Processes::NoError;
}
Processes::Error ProcessesRemote::setNiceness(long pid, int priority)
{
    Q_EMIT runCommand(QStringLiteral("setpriority ") + QString::number(pid) + QStringLiteral(" ") + QString::number(priority), (int)Renice);
    return Processes::NoError;
}

Processes::Error ProcessesRemote::setIoNiceness(long pid, int priorityClass, int priority)
{
    Q_EMIT runCommand(QStringLiteral("ionice ") + QString::number(pid) + QStringLiteral(" ") + QString::number(priorityClass) + QStringLiteral(" ")
                          + QString::number(priority),
                      (int)Ionice);
    return Processes::NoError;
}

Processes::Error ProcessesRemote::setScheduler(long pid, int priorityClass, int priority)
{
    Q_UNUSED(pid);
    Q_UNUSED(priorityClass);
    Q_UNUSED(priority);

    return Processes::NotSupported;
}

bool ProcessesRemote::supportsIoNiceness()
{
    return true;
}

long long ProcessesRemote::totalPhysicalMemory()
{
    return d->usedMemory + d->freeMemory;
}
long ProcessesRemote::numberProcessorCores()
{
    return 0;
}

void ProcessesRemote::answerReceived(int id, const QList<QByteArray> &answer)
{
    switch (id) {
    case PsInfo: {
        if (answer.isEmpty()) {
            return; // Invalid data
        }
        QList<QByteArray> info = answer.at(0).split('\t');
        d->numColumns = info.size();
        for (int i = 0; i < d->numColumns; i++) {
            if (info[i] == "Name") {
                d->nameColumn = i;
            } else if (info[i] == "PID") {
                d->pidColumn = i;
            } else if (info[i] == "PPID") {
                d->ppidColumn = i;
            } else if (info[i] == "UID") {
                d->uidColumn = i;
            } else if (info[i] == "GID") {
                d->gidColumn = i;
            } else if (info[i] == "TracerPID") {
                d->tracerPidColumn = i;
            } else if (info[i] == "Status") {
                d->statusColumn = i;
            } else if (info[i] == "User Time") {
                d->userColumn = i;
            } else if (info[i] == "System Time") {
                d->systemColumn = i;
            } else if (info[i] == "Nice") {
                d->niceColumn = i;
            } else if (info[i] == "VmSize") {
                d->vmSizeColumn = i;
            } else if (info[i] == "VmRss") {
                d->vmRSSColumn = i;
            } else if (info[i] == "VmURss") {
                d->vmURSSColumn = i;
            } else if (info[i] == "Login") {
                d->loginColumn = i;
            } else if (info[i] == "TTY") {
                d->ttyColumn = i;
            } else if (info[i] == "Command") {
                d->commandColumn = i;
            } else if (info[i] == "IO Priority Class") {
                d->ioprioClassColumn = i;
            } else if (info[i] == "IO Priority") {
                d->ioprioColumn = i;
            } else if (info[i] == "NNP") {
                d->noNewPrivilegesColumn = i;
            } else if (info[i] == "CGroup") {
                d->cGroupColumn = i;
            } else if (info[i] == "MAC Context") {
                d->macContextColumn = i;
            }
        }
        d->havePsInfo = true;
        break;
    }
    case Ps:
        d->lastAnswer = answer;
        if (!d->havePsInfo) {
            return; // Not setup yet.  Should never happen
        }
        Q_EMIT processesUpdated();
        break;
    case FreeMemory:
        if (answer.isEmpty()) {
            return; // Invalid data
        }
        d->freeMemory = answer[0].toLong();
        break;
    case UsedMemory:
        if (answer.isEmpty()) {
            return; // Invalid data
        }
        d->usedMemory = answer[0].toLong();
        break;
    }
}

ProcessesRemote::~ProcessesRemote()
{
    delete d;
}

}
