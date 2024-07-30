/*
    SPDX-FileCopyrightText: 2007 John Tapsell <tapsell@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PROCESSES_REMOTE_P_H_
#define PROCESSES_REMOTE_P_H_

#include "processes_base_p.h"
#include <QSet>
class Process;
namespace KSysGuard
{
/**
 * This is used to connect to a remote host
 */
class ProcessesRemote : public AbstractProcesses
{
    Q_OBJECT
public:
    ProcessesRemote(const QString &hostname);
    ~ProcessesRemote() override;
    QSet<long> getAllPids() override;
    long getParentPid(long pid) override;
    bool updateProcessInfo(long pid, Process *process) override;
    Processes::Error sendSignal(long pid, int sig) override;
    Processes::Error setNiceness(long pid, int priority) override;
    Processes::Error setScheduler(long pid, int priorityClass, int priority) override;
    long long totalPhysicalMemory() override;
    Processes::Error setIoNiceness(long pid, int priorityClass, int priority) override;
    bool supportsIoNiceness() override;
    long numberProcessorCores() override;
    void updateAllProcesses(Processes::UpdateFlags updateFlags) override;

Q_SIGNALS:
    /** For a remote machine, we rely on being able to communicate with ksysguardd.
     *  This must be dealt with by the program including this widget.  It must listen to our
     *  'runCommand' signal, and run the given command, with the given id. */
    void runCommand(const QString &command, int id);

public Q_SLOTS:
    /** For a remote machine, we rely on being able to communicate with ksysguardd.
     *  The programming using this must call this slot when an answer is received from ksysguardd,
     *  in response to a runCommand request.  The id identifies the answer */
    void answerReceived(int id, const QList<QByteArray> &answer);
    /** Called soon after */
    void setup();

protected:
    enum { PsInfo, Ps, UsedMemory, FreeMemory, Kill, Renice, Ionice };

private:
    /**
     * You can use this for whatever data you want.  Be careful about preserving state in between getParentPid and updateProcessInfo calls
     * if you chose to do that. getParentPid may be called several times for different pids before the relevant updateProcessInfo calls are made.
     * This is because the tree structure has to be sorted out first.
     */
    class Private;
    Private *d;
};
}
#endif
