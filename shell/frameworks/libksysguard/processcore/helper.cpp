/*
    KSysGuard, the KDE System Guard

    SPDX-FileCopyrightText: 2009 John Tapsell <john.tapsell@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

*/

#include "helper.h"
#include "processes_local_p.h"

using namespace KAuth;

KSysGuardProcessListHelper::KSysGuardProcessListHelper()
{
    qRegisterMetaType<QList<long long>>();
}

/* The functions here run as ROOT.  So be careful.  DO NOT TRUST THE INPUTS TO BE SANE. */
#define GET_PID(i)                                                                                                                                             \
    parameters.value(QStringLiteral("pid%1").arg(i), -1).toULongLong();                                                                                        \
    if (pid < 0) {                                                                                                                                             \
        return ActionReply(ActionReply::HelperErrorType);                                                                                                      \
    }

ActionReply KSysGuardProcessListHelper::sendsignal(const QVariantMap &parameters)
{
    ActionReply reply(ActionReply::HelperErrorType);
    if (!parameters.contains(QLatin1String("signal"))) {
        reply.setErrorDescription(QStringLiteral("Internal error - no signal parameter was passed to the helper"));
        reply.setErrorCode(static_cast<ActionReply::Error>(KSysGuard::Processes::InvalidPid));
        return reply;
    }
    if (!parameters.contains(QLatin1String("pidcount"))) {
        reply.setErrorDescription(QStringLiteral("Internal error - no pidcount parameter was passed to the helper"));
        reply.setErrorCode(static_cast<ActionReply::Error>(KSysGuard::Processes::InvalidParameter));
        return reply;
    }

    KSysGuard::ProcessesLocal processes;
    int signal = qvariant_cast<int>(parameters.value(QStringLiteral("signal")));
    bool success = true;
    int numProcesses = parameters.value(QStringLiteral("pidcount")).toInt();
    QStringList errorList;
    for (int i = 0; i < numProcesses; ++i) {
        qlonglong pid = GET_PID(i);
        KSysGuard::Processes::Error error = processes.sendSignal(pid, signal);
        if (error != KSysGuard::Processes::NoError) {
            errorList << QString::number(pid);
            success = false;
        }
    }
    if (success) {
        return ActionReply::SuccessReply();
    } else {
        reply.setErrorDescription(QStringLiteral("Could not send signal to: ") + errorList.join(QLatin1String(", ")));
        reply.setErrorCode(static_cast<ActionReply::Error>(KSysGuard::Processes::Unknown));
        return reply;
    }
}

ActionReply KSysGuardProcessListHelper::renice(const QVariantMap &parameters)
{
    if (!parameters.contains(QLatin1String("nicevalue")) || !parameters.contains(QLatin1String("pidcount"))) {
        return ActionReply(ActionReply::HelperErrorType);
    }

    KSysGuard::ProcessesLocal processes;
    int niceValue = qvariant_cast<int>(parameters.value(QStringLiteral("nicevalue")));
    bool success = true;
    int numProcesses = parameters.value(QStringLiteral("pidcount")).toInt();
    for (int i = 0; i < numProcesses; ++i) {
        qlonglong pid = GET_PID(i);
        success &= (processes.setNiceness(pid, niceValue) != KSysGuard::Processes::NoError);
    }
    if (success) {
        return ActionReply::SuccessReply();
    } else {
        return ActionReply(ActionReply::HelperErrorType);
    }
}

ActionReply KSysGuardProcessListHelper::changeioscheduler(const QVariantMap &parameters)
{
    if (!parameters.contains(QLatin1String("ioScheduler")) || !parameters.contains(QLatin1String("ioSchedulerPriority"))
        || !parameters.contains(QLatin1String("pidcount"))) {
        return ActionReply(ActionReply::HelperErrorType);
    }

    KSysGuard::ProcessesLocal processes;
    int ioScheduler = qvariant_cast<int>(parameters.value(QStringLiteral("ioScheduler")));
    int ioSchedulerPriority = qvariant_cast<int>(parameters.value(QStringLiteral("ioSchedulerPriority")));
    bool success = true;
    const int numProcesses = parameters.value(QStringLiteral("pidcount")).toInt();
    for (int i = 0; i < numProcesses; ++i) {
        qlonglong pid = GET_PID(i);
        success &= (processes.setIoNiceness(pid, ioScheduler, ioSchedulerPriority) != KSysGuard::Processes::NoError);
    }
    if (success) {
        return ActionReply::SuccessReply();
    } else {
        return ActionReply(ActionReply::HelperErrorType);
    }
}

ActionReply KSysGuardProcessListHelper::changecpuscheduler(const QVariantMap &parameters)
{
    if (!parameters.contains(QLatin1String("cpuScheduler")) || !parameters.contains(QLatin1String("cpuSchedulerPriority"))
        || !parameters.contains(QLatin1String("pidcount"))) {
        return ActionReply(ActionReply::HelperErrorType);
    }

    KSysGuard::ProcessesLocal processes;
    int cpuScheduler = qvariant_cast<int>(parameters.value(QStringLiteral("cpuScheduler")));
    int cpuSchedulerPriority = qvariant_cast<int>(parameters.value(QStringLiteral("cpuSchedulerPriority")));
    bool success = true;

    const int numProcesses = parameters.value(QStringLiteral("pidcount")).toInt();
    for (int i = 0; i < numProcesses; ++i) {
        qlonglong pid = GET_PID(i);
        success &= (processes.setScheduler(pid, cpuScheduler, cpuSchedulerPriority) != KSysGuard::Processes::NoError);
    }
    if (success) {
        return ActionReply::SuccessReply();
    } else {
        return ActionReply(ActionReply::HelperErrorType);
    }
}

KAUTH_HELPER_MAIN("org.kde.ksysguard.processlisthelper", KSysGuardProcessListHelper)
