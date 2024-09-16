// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DPROCESSDEVICE_H
#define DPROCESSDEVICE_H

#include "dtkdevice_global.h"

#include <QObject>

DDEVICE_BEGIN_NAMESPACE

class DProcessDevicePrivate;
class LIBDTKDEVICESHARED_EXPORT DProcessDevice : public QObject
{
public:
    struct DProcessBaseInfo {
        QString          name;
        int              type;
        QString          cmdline;       //bool Process::readCmdline()
        QString          usrerName;
        quint32          mask;                 // mask
        pid_t            pid;                  // pid
        QByteArray       icon;
    };

    struct DProcessMemoryInfo {  ///bool Process::readStatm()
        quint64          vmsize;               // vm size in kB
        quint64          rss;                  // resident set size in kB
        quint64          shm;                  // resident shared size in kB
    };

    struct DProcessIOInfo {  ///void Process::readIO()
        quint64          readBytes;            // disk read bytes
        quint64          writeBytes;           // disk write bytes
        quint64          cancelledWriteBytes;  // cancelled write bytes
    };

    struct DProcessNetworkInfo {
        quint64 recvBytes;
        quint64 sentBytes;
    };

    struct DProcessStatus { ///bool Process::readStat()
        char             state;                // process state
        pid_t            ppid;                 // parent process
        gid_t            pgid;                 // process group id
        uid_t            uid;                  // real uid
        gid_t            gid;                  // real gid
        uid_t            euid;                 // effective uid
        gid_t            egid;                 // effective gid
        uid_t            suid;                 // saved uid
        gid_t            sgid;                 // saved gid
        uid_t            fuid;                 // filesystem uid
        gid_t            fgid;                 // filesystem gid

        quint32          processor;            // cpu number
        quint32          rtPrio;               // real time priority
        quint32          policy;               // scheduling policy
        quint32          nthreads;             // number of threads
        int              nice;                 // process nice

        quint64          utime;                // user time
        quint64          stime;                // kernel time
        qint64           cutime;               // user time on waiting children
        qint64           cstime;               // kernel time on waiting children
        quint64          startTime;           // start time since system boot in clock ticks
        quint64          guestTime;            // guest time (virtual cpu time for guest os)
        qint64           cguestTime;           // children guest time in clock ticks
        quint64          wtime;                // time spent waiting on a runqueue
    };

    explicit DProcessDevice(QObject *parent = nullptr);
    ~DProcessDevice();

    QList<pid_t> allPids();
    QHash<QString, QString> environment(pid_t pid);
    QString groupName(const gid_t &gid);
    DProcessBaseInfo baseInfo(pid_t pid);
    DProcessMemoryInfo memoryInfo(pid_t pid);
    DProcessIOInfo ioInfo(pid_t pid);
    DProcessNetworkInfo networkInfo(pid_t pid);
    DProcessStatus status(pid_t pid);

private:
    QScopedPointer<DProcessDevicePrivate> d_ptr;
    Q_DECLARE_PRIVATE(DProcessDevice)
};

DDEVICE_END_NAMESPACE

#endif // DPROCESSDEVICE_H
