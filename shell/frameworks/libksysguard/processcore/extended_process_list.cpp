/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "extended_process_list.h"

#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginMetaData>
#include <KUser>

#include "process.h"
#include "process_attribute.h"
#include "process_data_provider.h"
#include "processcore_debug.h"

using namespace KSysGuard;

class Q_DECL_HIDDEN ExtendedProcesses::Private
{
public:
    Private(ExtendedProcesses *q);
    void loadPlugins();

    ExtendedProcesses *q;
    QList<ProcessAttribute *> m_coreAttributes;
    QList<ProcessDataProvider *> m_providers;
    QHash<K_UID, KUser> m_userCache;
};

enum GroupPolicy { Accumulate, Average, ForwardFirstEntry };

template<class T>
class ProcessSensor : public KSysGuard::ProcessAttribute
{
public:
    ProcessSensor(ExtendedProcesses *parent,
                  const QString &id,
                  const QString &name,
                  std::function<T(KSysGuard::Process *)> extractFunc,
                  KSysGuard::Process::Change changeFlag = KSysGuard::Process::Nothing,
                  GroupPolicy groupPolicy = Accumulate)
        : KSysGuard::ProcessAttribute(id, name, parent)
        , m_extractFunc(extractFunc)
        , m_changeFlag(changeFlag)
        , m_groupPolicy(groupPolicy)
    {
        if (m_changeFlag != 0) {
            connect(parent, &ExtendedProcesses::processChanged, this, [this](KSysGuard::Process *process) {
                if (!process->changes().testFlag(m_changeFlag)) {
                    return;
                }
                Q_EMIT dataChanged(process);
            });
        }
    }

    QVariant data(KSysGuard::Process *process) const override
    {
        return QVariant::fromValue(m_extractFunc(process));
    }

    QVariant cgroupData(KSysGuard::CGroup *cgroup, const QList<KSysGuard::Process *> &groupProcesses) const override
    {
        switch (m_groupPolicy) {
        case Accumulate:
            return ProcessAttribute::cgroupData(cgroup, groupProcesses);
        case Average:
            return ProcessAttribute::cgroupData(cgroup, groupProcesses).toDouble() / groupProcesses.size();
        case ForwardFirstEntry:
            return groupProcesses.isEmpty() ? QVariant() : data(groupProcesses.first());
        }
        Q_UNREACHABLE();
    }

private:
    std::function<T(KSysGuard::Process *)> m_extractFunc;
    KSysGuard::Process::Change m_changeFlag;
    GroupPolicy m_groupPolicy = Accumulate;
};

ExtendedProcesses::Private::Private(ExtendedProcesses *_q)
    : q(_q)
{
}

ExtendedProcesses::ExtendedProcesses(QObject *parent)
    : Processes(QString(), parent)
    , d(new Private(this))
{
    d->loadPlugins();

    auto pidSensor =
        new ProcessSensor<qlonglong>(this, QStringLiteral("pid"), i18n("PID"), &KSysGuard::Process::pid, KSysGuard::Process::Status, ForwardFirstEntry);
    pidSensor->setDescription(i18n("The unique Process ID that identifies this process."));
    d->m_coreAttributes << pidSensor;

    auto parentPidSensor = new ProcessSensor<qlonglong>(this,
                                                        QStringLiteral("parentPid"),
                                                        i18n("Parent PID"),
                                                        &KSysGuard::Process::parentPid,
                                                        Process::Nothing,
                                                        ForwardFirstEntry);
    d->m_coreAttributes << parentPidSensor;

    auto loginSensor =
        new ProcessSensor<QString>(this, QStringLiteral("login"), i18n("Login"), &KSysGuard::Process::login, KSysGuard::Process::Login, ForwardFirstEntry);
    loginSensor->setDescription(i18n("The user who owns this process."));
    d->m_coreAttributes << loginSensor;

    auto uidSensor =
        new ProcessSensor<qlonglong>(this, QStringLiteral("uid"), i18n("UID"), &KSysGuard::Process::uid, KSysGuard::Process::Uids, ForwardFirstEntry);
    d->m_coreAttributes << uidSensor;

    auto userNameSensor = new ProcessSensor<QString>(
        this,
        QStringLiteral("username"),
        i18n("Username"),
        [this](KSysGuard::Process *p) {
            const K_UID uid = p->uid();
            auto userIt = d->m_userCache.find(uid);
            if (userIt == d->m_userCache.end()) {
                userIt = d->m_userCache.insert(uid, KUser(uid));
            }
            return userIt->loginName();
        },
        KSysGuard::Process::Uids,
        ForwardFirstEntry);
    d->m_coreAttributes << userNameSensor;

    auto canUserLoginSensor = new ProcessSensor<bool>(
        this,
        QStringLiteral("canUserLogin"),
        i18n("Can Login"),
        [this](KSysGuard::Process *p) {
            const K_UID uid = p->uid();
            if (uid == 65534) { // special value meaning nobody
                return false;
            }
            auto userIt = d->m_userCache.find(uid);
            if (userIt == d->m_userCache.end()) {
                userIt = d->m_userCache.insert(uid, KUser(uid));
            }

            if (!userIt->isValid()) {
                // For some reason the user isn't recognised.  This might happen under certain security situations.
                // Just return true to be safe
                return true;
            }
            const QString shell = userIt->shell();
            if (shell == QLatin1String("/bin/false")) { // FIXME - add in any other shells it could be for false
                return false;
            }
            return true;
        },
        KSysGuard::Process::Uids,
        ForwardFirstEntry);
    d->m_coreAttributes << canUserLoginSensor;

    auto euidSensor =
        new ProcessSensor<qlonglong>(this, QStringLiteral("euid"), i18n("EUID"), &KSysGuard::Process::euid, KSysGuard::Process::Uids, ForwardFirstEntry);
    d->m_coreAttributes << euidSensor;

    auto suidSensor =
        new ProcessSensor<qlonglong>(this, QStringLiteral("suid"), i18n("suid"), &KSysGuard::Process::suid, KSysGuard::Process::Uids, ForwardFirstEntry);
    d->m_coreAttributes << suidSensor;

    auto fsuidSensor =
        new ProcessSensor<qlonglong>(this, QStringLiteral("fsuid"), i18n("fsuid"), &KSysGuard::Process::fsuid, KSysGuard::Process::Uids, ForwardFirstEntry);
    d->m_coreAttributes << fsuidSensor;

    auto gidSensor =
        new ProcessSensor<qlonglong>(this, QStringLiteral("gid"), i18n("gid"), &KSysGuard::Process::gid, KSysGuard::Process::Gids, ForwardFirstEntry);
    d->m_coreAttributes << gidSensor;

    auto egidSensor =
        new ProcessSensor<qlonglong>(this, QStringLiteral("egid"), i18n("egid"), &KSysGuard::Process::egid, KSysGuard::Process::Gids, ForwardFirstEntry);
    d->m_coreAttributes << egidSensor;

    auto sgidSensor =
        new ProcessSensor<qlonglong>(this, QStringLiteral("sgid"), i18n("sgid"), &KSysGuard::Process::sgid, KSysGuard::Process::Gids, ForwardFirstEntry);
    d->m_coreAttributes << sgidSensor;

    auto fsgidSensor =
        new ProcessSensor<qlonglong>(this, QStringLiteral("fsgid"), i18n("fsgid"), &KSysGuard::Process::fsgid, KSysGuard::Process::Gids, ForwardFirstEntry);
    d->m_coreAttributes << fsgidSensor;

    auto tracerpidSensor = new ProcessSensor<qlonglong>(this,
                                                        QStringLiteral("tracerpid"),
                                                        i18n("Tracer Pid"),
                                                        &KSysGuard::Process::tracerpid,
                                                        Process::Nothing,
                                                        ForwardFirstEntry);
    d->m_coreAttributes << tracerpidSensor;

    auto ttySensor =
        new ProcessSensor<QByteArray>(this, QStringLiteral("tty"), i18n("tty"), &KSysGuard::Process::tty, KSysGuard::Process::Tty, ForwardFirstEntry);
    ttySensor->setDescription(i18n("The controlling terminal on which this process is running."));
    d->m_coreAttributes << ttySensor;

    auto userTimeSensor = new ProcessSensor<qlonglong>(this, QStringLiteral("userTime"), i18n("User Time"), &KSysGuard::Process::userTime);
    userTimeSensor->setUnit(KSysGuard::UnitTicks);
    d->m_coreAttributes << userTimeSensor;

    auto sysTimeSensor = new ProcessSensor<qlonglong>(this, QStringLiteral("sysTime"), i18n("System Time"), &KSysGuard::Process::sysTime);
    sysTimeSensor->setUnit(KSysGuard::UnitTicks);
    d->m_coreAttributes << sysTimeSensor;

    auto timeSensor = new ProcessSensor<qlonglong>(this, QStringLiteral("totalTime"), i18n("Total Time"), [](KSysGuard::Process *p) {
        return p->userTime() + p->sysTime();
    });
    timeSensor->setShortName(i18n("Time"));
    timeSensor->setUnit(KSysGuard::UnitTicks);
    timeSensor->setDescription(i18n("The total user and system time that this process has been running for"));
    d->m_coreAttributes << timeSensor;

    auto startTimeSensor = new ProcessSensor<qlonglong>(this,
                                                        QStringLiteral("startTime"),
                                                        i18n("Start Time"),
                                                        &KSysGuard::Process::startTime,
                                                        Process::Nothing,
                                                        ForwardFirstEntry); // Is this correct for apps?
    startTimeSensor->setUnit(KSysGuard::UnitBootTimestamp);
    d->m_coreAttributes << startTimeSensor;

    const int maximumCpuPercent = 100 * numberProcessorCores();

    auto userUsageSensor =
        new ProcessSensor<int>(this, QStringLiteral("userUsage"), i18n("User CPU Usage"), &KSysGuard::Process::userUsage, KSysGuard::Process::Usage);
    userUsageSensor->setShortName(i18n("User CPU"));
    userUsageSensor->setMin(0);
    userUsageSensor->setMax(maximumCpuPercent);
    userUsageSensor->setUnit(KSysGuard::UnitPercent);
    d->m_coreAttributes << userUsageSensor;

    auto sysUsageSensor =
        new ProcessSensor<int>(this, QStringLiteral("sysUsage"), i18n("System CPU Usage"), &KSysGuard::Process::sysUsage, KSysGuard::Process::Usage);
    sysUsageSensor->setShortName(i18n("System CPU"));
    sysUsageSensor->setMin(0);
    sysUsageSensor->setMax(maximumCpuPercent);
    sysUsageSensor->setUnit(KSysGuard::UnitPercent);
    d->m_coreAttributes << sysUsageSensor;

    auto usageSensor = new ProcessSensor<int>(
        this,
        QStringLiteral("usage"),
        i18n("Total CPU Usage"),
        [](KSysGuard::Process *p) {
            return p->userUsage() + p->sysUsage();
        },
        KSysGuard::Process::Usage,
        Accumulate);
    usageSensor->setShortName(i18n("CPU"));
    usageSensor->setMin(0);
    usageSensor->setMax(maximumCpuPercent);
    usageSensor->setUnit(KSysGuard::UnitPercent);
    usageSensor->setDescription(i18n("The current total CPU usage of the process."));
    d->m_coreAttributes << usageSensor;

    auto totalUserUsageSensor = new ProcessSensor<int>(this,
                                                       QStringLiteral("totalUserUsage"),
                                                       i18n("Group User CPU Usage"),
                                                       &KSysGuard::Process::totalUserUsage,
                                                       KSysGuard::Process::TotalUsage,
                                                       Average);
    totalUserUsageSensor->setDescription(i18n("The amount of userspace CPU used by this process and all its children."));
    totalUserUsageSensor->setMin(0);
    totalUserUsageSensor->setMax(maximumCpuPercent);
    totalUserUsageSensor->setUnit(KSysGuard::UnitPercent);
    d->m_coreAttributes << totalUserUsageSensor;

    auto totalSysUsageSensor = new ProcessSensor<int>(this,
                                                      QStringLiteral("totalSysUsage"),
                                                      i18n("Group System CPU Usage"),
                                                      &KSysGuard::Process::totalSysUsage,
                                                      KSysGuard::Process::TotalUsage,
                                                      Average);
    totalUserUsageSensor->setDescription(i18n("The amount of system CPU used by this process and all its children."));
    totalSysUsageSensor->setMin(0);
    totalSysUsageSensor->setMax(maximumCpuPercent);
    totalSysUsageSensor->setUnit(KSysGuard::UnitPercent);
    d->m_coreAttributes << totalSysUsageSensor;

    auto totalUsageSensor = new ProcessSensor<int>(
        this,
        QStringLiteral("totalUsage"),
        i18n("Group Total CPU Usage"),
        [](KSysGuard::Process *p) {
            return p->totalUserUsage() + p->totalSysUsage();
        },
        KSysGuard::Process::TotalUsage,
        Average);
    totalUsageSensor->setShortName(i18n("Group CPU"));
    totalUserUsageSensor->setDescription(i18n("The total amount of CPU used by this process and all its children."));
    totalUsageSensor->setMin(0);
    totalUsageSensor->setMax(maximumCpuPercent);
    totalUsageSensor->setUnit(KSysGuard::UnitPercent);
    d->m_coreAttributes << totalUsageSensor;

    auto niceLevelSensor =
        new ProcessSensor<int>(this, QStringLiteral("niceLevel"), i18n("Nice Level"), &KSysGuard::Process::niceLevel, KSysGuard::Process::NiceLevels);
    niceLevelSensor->setDescription(i18n(
        "The priority with which this process is being run. For the normal scheduler, this ranges from 19 (very nice, least priority) to -19 (top priority)."));
    d->m_coreAttributes << niceLevelSensor;

    auto schedulerSensor =
        new ProcessSensor<uint>(this, QStringLiteral("scheduler"), i18n("Scheduler"), &KSysGuard::Process::scheduler, KSysGuard::Process::NiceLevels);
    d->m_coreAttributes << schedulerSensor;

    auto ioPriorityClassSensor = new ProcessSensor<uint>(this,
                                                         QStringLiteral("ioPriorityClass"),
                                                         i18n("IO Priority Class"),
                                                         &KSysGuard::Process::ioPriorityClass,
                                                         KSysGuard::Process::NiceLevels);
    d->m_coreAttributes << ioPriorityClassSensor;

    auto ioniceLevelSensor =
        new ProcessSensor<int>(this, QStringLiteral("ioniceLevel"), i18n("IO Nice Level"), &KSysGuard::Process::ioniceLevel, KSysGuard::Process::NiceLevels);
    ioniceLevelSensor->setUnit(KSysGuard::UnitNone);
    d->m_coreAttributes << ioniceLevelSensor;

    auto vmSizeSensor = new ProcessSensor<qlonglong>(this, QStringLiteral("vmSize"), i18n("VM Size"), &KSysGuard::Process::vmSize, KSysGuard::Process::VmSize);
    vmSizeSensor->setUnit(KSysGuard::UnitKiloByte);
    vmSizeSensor->setMin(0);
    vmSizeSensor->setMax(totalPhysicalMemory());
    vmSizeSensor->setDescription(
        i18n("This is the amount of virtual memory space that the process is using, included shared libraries, graphics memory, files on disk, and so on. This "
             "number is almost meaningless."));
    d->m_coreAttributes << vmSizeSensor;

    auto vmRSSSensor =
        new ProcessSensor<qlonglong>(this, QStringLiteral("vmRSS"), i18n("Resident Memory Usage"), &KSysGuard::Process::vmRSS, KSysGuard::Process::VmRSS);
    vmRSSSensor->setUnit(KSysGuard::UnitKiloByte);
    vmRSSSensor->setShortName(i18n("Resident"));
    vmRSSSensor->setMin(0);
    vmRSSSensor->setMax(totalPhysicalMemory());
    vmRSSSensor->setDescription(
        i18n("This is the amount of physical memory that this process is using and includes the amount of memory used by shared libraries."));
    d->m_coreAttributes << vmRSSSensor;

    auto vmURSSSensor =
        new ProcessSensor<qlonglong>(this, QStringLiteral("vmURSS"), i18n("Private Memory Usage"), &KSysGuard::Process::vmURSS, KSysGuard::Process::VmURSS);
    vmURSSSensor->setUnit(KSysGuard::UnitKiloByte);
    vmURSSSensor->setShortName(i18n("Private"));
    vmURSSSensor->setMin(0);
    vmURSSSensor->setMax(totalPhysicalMemory());
    vmURSSSensor->setDescription(
        i18n("This is the amount of physical memory that this process is using by itself, and approximates the Private memory usage of the process. It does "
             "not include any swapped out memory, nor the code size of its shared libraries."));
    d->m_coreAttributes << vmURSSSensor;

    auto sharedMemorySensor = new ProcessSensor<qlonglong>(
        this,
        QStringLiteral("vmShared"),
        i18n("Shared Memory Usage"),
        [](KSysGuard::Process *p) -> qlonglong {
            if (p->vmRSS() - p->vmURSS() < 0 || p->vmURSS() == -1) {
                return 0;
            }
            return (qlonglong)(p->vmRSS() - p->vmURSS());
        },
        KSysGuard::Process::VmRSS);
    d->m_coreAttributes << sharedMemorySensor;
    sharedMemorySensor->setShortName(i18n("Shared"));
    sharedMemorySensor->setDescription(
        i18n("This is approximately the amount of real physical memory that this process's shared libraries are using. This memory is shared among all "
             "processes that use this library."));
    sharedMemorySensor->setUnit(KSysGuard::UnitKiloByte);
    sharedMemorySensor->setMin(0);
    sharedMemorySensor->setMax(totalPhysicalMemory());

    auto vmPSSSensor = new ProcessSensor<qlonglong>(this, QStringLiteral("vmPSS"), i18n("Memory Usage"), &KSysGuard::Process::vmPSS, KSysGuard::Process::VmPSS);
    vmPSSSensor->setShortName(i18n("Memory"));
    vmPSSSensor->setUnit(KSysGuard::UnitKiloByte);
    vmPSSSensor->setMin(0);
    vmPSSSensor->setMax(totalPhysicalMemory());
    vmPSSSensor->setRequiredUpdateFlags(Processes::Smaps);
    vmPSSSensor->setDescription(
        i18n("This is an approximation of the real amount of physical memory that this process is using. It is calculated by dividing the process' shared "
             "memory usage by the amount of processes sharing that memory, then adding the process' private memory."));
    d->m_coreAttributes << vmPSSSensor;

    auto nameSensor =
        new ProcessSensor<QString>(this, QStringLiteral("name"), i18n("Name"), &KSysGuard::Process::name, KSysGuard::Process::Name, ForwardFirstEntry);
    nameSensor->setDescription(i18n("The process name."));
    d->m_coreAttributes << nameSensor;

    auto commandSensor = new ProcessSensor<QString>(this,
                                                    QStringLiteral("command"),
                                                    i18n("Command"),
                                                    &KSysGuard::Process::command,
                                                    KSysGuard::Process::Command,
                                                    ForwardFirstEntry);
    commandSensor->setDescription(i18n("The command with which this process was launched."));
    d->m_coreAttributes << commandSensor;

    auto statusSensor =
        new ProcessSensor<QString>(this, QStringLiteral("status"), i18n("Status"), &KSysGuard::Process::translatedStatus, KSysGuard::Process::Status);
    d->m_coreAttributes << statusSensor;

    auto ioCharactersReadSensor = new ProcessSensor<qlonglong>(this,
                                                               QStringLiteral("ioCharactersRead"),
                                                               i18n("IO Characters Read"),
                                                               &KSysGuard::Process::ioCharactersRead,
                                                               KSysGuard::Process::IO);
    ioCharactersReadSensor->setUnit(KSysGuard::UnitByte);
    ioCharactersReadSensor->setRequiredUpdateFlags(Processes::IOStatistics);
    d->m_coreAttributes << ioCharactersReadSensor;

    auto ioCharactersWrittenSensor = new ProcessSensor<qlonglong>(this,
                                                                  QStringLiteral("ioCharactersWritten"),
                                                                  i18n("IO Characters Written"),
                                                                  &KSysGuard::Process::ioCharactersWritten,
                                                                  KSysGuard::Process::IO);
    ioCharactersWrittenSensor->setUnit(KSysGuard::UnitByte);
    ioCharactersWrittenSensor->setRequiredUpdateFlags(Processes::IOStatistics);
    d->m_coreAttributes << ioCharactersWrittenSensor;

    auto ioReadSyscallsSensor = new ProcessSensor<qlonglong>(this,
                                                             QStringLiteral("ioReadSyscalls"),
                                                             i18n("IO Read Syscalls"),
                                                             &KSysGuard::Process::ioReadSyscalls,
                                                             KSysGuard::Process::IO);
    ioReadSyscallsSensor->setUnit(KSysGuard::UnitRate);
    ioReadSyscallsSensor->setRequiredUpdateFlags(Processes::IOStatistics);
    d->m_coreAttributes << ioReadSyscallsSensor;

    auto ioReadSyscallsRateSensor = new ProcessSensor<qlonglong>(this,
                                                                 QStringLiteral("ioReadSyscallsRate"),
                                                                 i18n("IO Read Syscalls Rate"),
                                                                 &KSysGuard::Process::ioReadSyscallsRate,
                                                                 KSysGuard::Process::IO);
    ioReadSyscallsRateSensor->setUnit(KSysGuard::UnitRate);
    ioReadSyscallsRateSensor->setRequiredUpdateFlags(Processes::IOStatistics);
    d->m_coreAttributes << ioReadSyscallsSensor;

    auto ioWriteSyscallsSensor = new ProcessSensor<qlonglong>(this,
                                                              QStringLiteral("ioWriteSyscalls"),
                                                              i18n("IO Write Syscalls"),
                                                              &KSysGuard::Process::ioWriteSyscalls,
                                                              KSysGuard::Process::IO);
    ioWriteSyscallsSensor->setUnit(KSysGuard::UnitRate);
    ioWriteSyscallsSensor->setRequiredUpdateFlags(Processes::IOStatistics);
    d->m_coreAttributes << ioWriteSyscallsSensor;

    auto ioWriteSyscallsRateSensor = new ProcessSensor<qlonglong>(this,
                                                                  QStringLiteral("ioReadSyscallsRate"),
                                                                  i18n("IO Write Syscalls Rate"),
                                                                  &KSysGuard::Process::ioWriteSyscallsRate,
                                                                  KSysGuard::Process::IO);
    ioWriteSyscallsRateSensor->setUnit(KSysGuard::UnitRate);
    ioWriteSyscallsRateSensor->setRequiredUpdateFlags(Processes::IOStatistics);
    d->m_coreAttributes << ioWriteSyscallsRateSensor;

    auto ioCharactersActuallyReadSensor = new ProcessSensor<qlonglong>(this,
                                                                       QStringLiteral("ioCharactersActuallyRead"),
                                                                       i18n("IO Characters Actually Read"),
                                                                       &KSysGuard::Process::ioCharactersActuallyRead,
                                                                       KSysGuard::Process::IO);
    ioCharactersActuallyReadSensor->setUnit(KSysGuard::UnitByte);
    ioCharactersActuallyReadSensor->setRequiredUpdateFlags(Processes::IOStatistics);
    d->m_coreAttributes << ioCharactersActuallyReadSensor;

    auto ioCharactersReadRateSensor = new ProcessSensor<qlonglong>(this,
                                                                   QStringLiteral("ioCharactersReadRate"),
                                                                   i18n("IO Characters Read Rate"),
                                                                   &KSysGuard::Process::ioCharactersReadRate,
                                                                   KSysGuard::Process::IO);
    ioCharactersReadRateSensor->setDescription(i18n("The read rate for all of a process' IO, including disk cache and other nonphysical IO."));
    ioCharactersReadRateSensor->setUnit(KSysGuard::UnitByteRate);
    ioCharactersReadRateSensor->setRequiredUpdateFlags(Processes::IOStatistics);
    d->m_coreAttributes << ioCharactersReadRateSensor;

    auto ioCharactersWrittenRateSensor = new ProcessSensor<qlonglong>(this,
                                                                      QStringLiteral("ioCharactersWrittenRate"),
                                                                      i18n("IO Characters Written Rate"),
                                                                      &KSysGuard::Process::ioCharactersWrittenRate,
                                                                      KSysGuard::Process::IO);
    ioCharactersWrittenRateSensor->setDescription(i18n("The write rate for all of a process' IO, including disk cache and other nonphysical IO."));
    ioCharactersWrittenRateSensor->setUnit(KSysGuard::UnitByteRate);
    ioCharactersWrittenRateSensor->setRequiredUpdateFlags(Processes::IOStatistics);
    d->m_coreAttributes << ioCharactersWrittenRateSensor;

    auto ioCharactersActuallyReadRateSensor = new ProcessSensor<qlonglong>(this,
                                                                           QStringLiteral("ioCharactersActuallyReadRate"),
                                                                           i18n("Disk Read Rate"),
                                                                           &KSysGuard::Process::ioCharactersActuallyReadRate,
                                                                           KSysGuard::Process::IO);
    ioCharactersActuallyReadRateSensor->setUnit(KSysGuard::UnitByteRate);
    ioCharactersActuallyReadRateSensor->setShortName(i18n("Read"));
    ioCharactersActuallyReadRateSensor->setDescription(i18n("The rate of data being read from disk."));
    ioCharactersActuallyReadRateSensor->setRequiredUpdateFlags(Processes::IOStatistics);
    d->m_coreAttributes << ioCharactersActuallyReadRateSensor;

    auto ioCharactersActuallyWrittenRateSensor = new ProcessSensor<qlonglong>(this,
                                                                              QStringLiteral("ioCharactersActuallyWrittenRate"),
                                                                              i18n("Disk Write Rate"),
                                                                              &KSysGuard::Process::ioCharactersActuallyWrittenRate,
                                                                              KSysGuard::Process::IO);
    ioCharactersActuallyWrittenRateSensor->setUnit(KSysGuard::UnitByteRate);
    ioCharactersActuallyWrittenRateSensor->setShortName(i18n("Write"));
    ioCharactersActuallyWrittenRateSensor->setDescription(i18n("The rate of data being written to the disk."));
    ioCharactersActuallyWrittenRateSensor->setRequiredUpdateFlags(Processes::IOStatistics);
    d->m_coreAttributes << ioCharactersActuallyWrittenRateSensor;

    auto numThreadsSensor = new ProcessSensor<int>(this,
                                                   QStringLiteral("numThreads"),
                                                   i18n("Threads"),
                                                   &KSysGuard::Process::numThreads,
                                                   KSysGuard::Process::NumThreads,
                                                   ForwardFirstEntry);
    d->m_coreAttributes << numThreadsSensor;

    connect(this, &KSysGuard::Processes::beginRemoveProcess, this, [this](KSysGuard::Process *process) {
        const auto attrs = attributes();
        for (auto a : attrs) {
            a->clearData(process);
        }
    });

    connect(this, &KSysGuard::Processes::updated, this, [this]() {
        for (auto p : std::as_const(d->m_providers)) {
            if (p->enabled()) {
                p->update();
            }
        }
    });
}

ExtendedProcesses::~ExtendedProcesses()
{
}

QList<ProcessAttribute *> ExtendedProcesses::attributes() const
{
    return d->m_coreAttributes + extendedAttributes();
}

QList<ProcessAttribute *> ExtendedProcesses::extendedAttributes() const
{
    QList<ProcessAttribute *> rc;
    for (auto p : std::as_const(d->m_providers)) {
        rc << p->attributes();
    }
    return rc;
}

void ExtendedProcesses::Private::loadPlugins()
{
    const QList<KPluginMetaData> listMetaData = KPluginMetaData::findPlugins(QStringLiteral("ksysguard/process"));
    // instantiate all plugins
    for (const auto &pluginMetaData : listMetaData) {
        qCDebug(LIBKSYSGUARD_PROCESSCORE) << "loading plugin" << pluginMetaData.name();
        auto provider = KPluginFactory::instantiatePlugin<ProcessDataProvider>(pluginMetaData, q);
        if (!provider.plugin) {
            qCCritical(LIBKSYSGUARD_PROCESSCORE) << "failed to instantiate ProcessDataProvider" << pluginMetaData.name();
            continue;
        }
        m_providers << provider.plugin;
    }
}

QSharedPointer<ExtendedProcesses> ExtendedProcesses::instance()
{
    static QWeakPointer<ExtendedProcesses> instance;
    auto processes = instance.lock();
    if (!processes) {
        processes = QSharedPointer<ExtendedProcesses>(new ExtendedProcesses, [](ExtendedProcesses *p) {
            delete p;
        });
        instance = processes;
    }
    return processes;
}
