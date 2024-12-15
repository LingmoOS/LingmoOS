// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESS_P_H
#define PROCESS_P_H

#include "common/sample.h"
#include "process/process_icon.h"
#include "process/process_name.h"

#include <QSharedData>

#include <memory>

namespace core {
namespace process {

inline struct timeval default_interval()
{
    return {2, 0};
}

using CPUTimeSample = Sample<qulonglong>;
using CPUTimeSampleFrame = SampleFrame<qulonglong>;
using CPUUsageSample = Sample<qreal>;
using CPUUsageSampleFrame = SampleFrame<qreal>;

class Process;

/**
 * @brief The proc_info_t struct
 */
class ProcessPrivate : public QSharedData
{
public:
    ProcessPrivate()
        : QSharedData()
        , valid {false}
        , state {'\0'}
        , apptype {2}
        , pid {0}
        , ppid {0}
        , pgid {0}
        , mask {0}
        , uid {}
        , gid {}
        , euid {}
        , egid {}
        , suid {}
        , sgid {}
        , fuid {}
        , fgid {}
        , processor {0}
        , rt_prio {0}
        , policy {0}
        , nthreads {0}
        , nice {0}
        , utime {0}
        , stime {0}
        , cutime {0}
        , cstime {0}
        , start_time {0}
        , vmsize {0}
        , rss {0}
        , shm {0}
        , guest_time {0}
        , cguest_time {0}
        , wtime {0}
        , read_bytes {0}
        , write_bytes {0}
        , cancelled_write_bytes {0}
        , usrerName {}
        , name {}
        , proc_name{}
        , proc_icon{}
        , cmdline {}
        , environ {}
        , uptime {timeval {0, 0}}
        , sockInodes {}
        , cpuTimeSample(new CPUTimeSample(TimePeriod(TimePeriod::kNoPeriod, default_interval())))
        , cpuUsageSample(new CPUUsageSample(TimePeriod(TimePeriod::kNoPeriod, default_interval())))
        , networkIOSample(new IOSample(TimePeriod(TimePeriod::kNoPeriod, default_interval())))
        , networkBandwidthSample(new IOPSSample(TimePeriod(TimePeriod::kNoPeriod, default_interval())))
        , diskIOSample(new DISKIOSample(TimePeriod(TimePeriod::kNoPeriod, default_interval())))
        , diskIOSpeedSample(new IOPSSample(TimePeriod(TimePeriod::kNoPeriod, default_interval())))
    {
    }
    ProcessPrivate(const ProcessPrivate &other)
        : QSharedData(other)
        , valid(other.valid)
        , state(other.state)
        , apptype(other.apptype)
        , pid(other.pid)
        , ppid(other.ppid)
        , pgid(other.pgid)
        , mask(other.mask)
        , uid(other.uid)
        , gid(other.gid)
        , euid(other.euid)
        , egid(other.egid)
        , suid(other.suid)
        , sgid(other.sgid)
        , fuid(other.fuid)
        , fgid(other.fgid)
        , processor(other.processor)
        , rt_prio(other.rt_prio)
        , policy(other.policy)
        , nthreads(other.nthreads)
        , nice(other.nice)
        , utime(other.utime)
        , stime(other.stime)
        , cutime(other.cutime)
        , cstime(other.cstime)
        , start_time(other.start_time)
        , vmsize(other.vmsize)
        , rss(other.rss)
        , shm(other.shm)
        , guest_time(other.guest_time)
        , cguest_time(other.cguest_time)
        , wtime(other.wtime)
        , read_bytes(other.read_bytes)
        , write_bytes(other.write_bytes)
        , cancelled_write_bytes(other.cancelled_write_bytes)
        , usrerName(other.name)
        , name(other.name)
        , proc_name(other.proc_name)
        , proc_icon(other.proc_icon)
        , cmdline(other.cmdline)
        , environ(other.environ)
        , uptime {other.uptime}
        , sockInodes(other.sockInodes)
        , cpuTimeSample(std::unique_ptr<CPUTimeSample>(new CPUTimeSample(*(other.cpuTimeSample))))
        , cpuUsageSample(std::unique_ptr<CPUUsageSample>(new CPUUsageSample(*(other.cpuUsageSample))))
        , networkIOSample(std::unique_ptr<IOSample>(new IOSample(*(other.networkIOSample))))
        , networkBandwidthSample(std::unique_ptr<IOPSSample>(new IOPSSample(*(other.networkBandwidthSample))))
        , diskIOSample(std::unique_ptr<DISKIOSample>(new DISKIOSample(*(other.diskIOSample))))
        , diskIOSpeedSample(std::unique_ptr<IOPSSample>(new IOPSSample(*(other.diskIOSpeedSample))))
    {
    }
    ~ProcessPrivate() {}

    inline bool isValid() const
    {
        return valid;
    }

private:
    bool valid;
    char state; // process state

    int apptype;
    pid_t pid; // pid
    pid_t ppid; // parent process
    pid_t pgid; // process group id
    unsigned int mask; // mask
    uid_t uid; // real uid
    gid_t gid; // real gid
    uid_t euid; // effective uid
    gid_t egid; // effective gid
    uid_t suid; // saved uid
    gid_t sgid; // saved gid
    uid_t fuid; // filesystem uid
    gid_t fgid; // filesystem gid
    unsigned int processor; // cpu number
    unsigned int rt_prio; // real time priority
    unsigned int policy; // scheduling policy
    unsigned int nthreads; // number of threads
    int nice; // process nice

    unsigned long long utime; // user time
    unsigned long long stime; // kernel time
    long long cutime; // user time on waiting children
    long long cstime; // kernel time on waiting children
    unsigned long long start_time; // start time since system boot in clock ticks
    unsigned long long vmsize; // vm size in kB
    unsigned long long rss; // resident set size in kB
    unsigned long long shm; // resident shared size in kB
    unsigned long long guest_time; // guest time (virtual cpu time for guest os)
    long long cguest_time; // children guest time in clock ticks

    unsigned long long wtime; // time spent waiting on a runqueue

    // blockdev io
    unsigned long long read_bytes; // disk read bytes
    unsigned long long write_bytes; // disk write bytes
    unsigned long long cancelled_write_bytes; // cancelled write bytes

    QString usrerName;

    QString name; // raw name
    ProcessName proc_name; // process name object
    ProcessIcon proc_icon; // process icon object
    QByteArrayList cmdline; // process cmdline
    QHash<QString, QString> environ; // environment cache

    struct timeval uptime;

    QList<ino_t> sockInodes; // socket inodes opened by this process

    // only 2 samples are kept here for each process, to avoid too much memory
    // consumption if there're too many processes
    std::unique_ptr<CPUTimeSample> cpuTimeSample;
    std::unique_ptr<CPUUsageSample> cpuUsageSample;
    std::unique_ptr<IOSample> networkIOSample;
    std::unique_ptr<IOPSSample> networkBandwidthSample;
    std::unique_ptr<DISKIOSample> diskIOSample;
    std::unique_ptr<IOPSSample> diskIOSpeedSample;

    friend class Process;
};

} // namespace process
} // namespace core

#endif // PROCESS_P_H
