// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sys_info.h"

#include "common/time_period.h"
#include "common/sample.h"
#include "common/common.h"
#include "system/system_monitor.h"
#include "common/thread_manager.h"
#include "system/system_monitor_thread.h"
#include "packet.h"
#include <DSysInfo>

#include <QString>
#include <QtDBus>
#include <QFileInfo>
#include <QDir>

#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <errno.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <arpa/inet.h>
#include "common/hash.h"

#define PROC_PATH_SOCK_TCP  "/proc/net/tcp"
#define PROC_PATH_SOCK_TCP6 "/proc/net/tcp6"
#define PROC_PATH_SOCK_UDP  "/proc/net/udp"
#define PROC_PATH_SOCK_UDP6 "/proc/net/udp6"

using namespace common::alloc;
using namespace common::core;
using namespace common::error;
using namespace core::system;
DCORE_USE_NAMESPACE

#define PROC_PATH_FILE_NR "/proc/sys/fs/file-nr"
#define PROC_PATH_UPTIME "/proc/uptime"
#define PROC_PATH_STAT "/proc/stat"
#define PROC_PATH_LOADAVG "/proc/loadavg"

namespace core {
namespace system {

SysInfo::SysInfo()
    : d(new SysInfoPrivate())
{
    d->loadAvg = std::make_shared<struct load_avg_t>();
}
SysInfo::SysInfo(const SysInfo &other)
    : d(other.d)
{
}
SysInfo &SysInfo::operator=(const SysInfo &rhs)
{
    if (this == &rhs)
        return *this;

    d = rhs.d;
    return *this;
}
SysInfo::~SysInfo()
{

}

SysInfo *SysInfo::instance()
{
    auto *monitor = ThreadManager::instance()->thread<SystemMonitorThread>(BaseThread::kSystemMonitorThread)->systemMonitorInstance();
    return monitor->sysInfo();
}

bool SysInfo::readSockStat(SockStatMap &statMap)
{
    bool ok {true};

    auto parseSocks = [](int family, int proto, const char *proc, SockStatMap & statMap) -> bool {
        bool ok {true};
        FILE *fp {};
        const size_t BLEN = 4096;
        QByteArray buffer {BLEN, 0};
        int nr {};
        ino_t ino {};
        char s_addr[128] {}, d_addr[128] {};
        uint64_t hash {};
        QByteArray fmtbuf {};
        uint64_t cchash[2] {};
        QString patternA {}, patternB {};

        errno = 0;
        if (!(fp = fopen(proc, "r")))
        {
            return !ok;
        }

        while (fgets(buffer.data(), BLEN, fp))
        {
            auto stat = QSharedPointer<struct sock_stat_t>::create();

            //*****************************************************************
            nr = sscanf(buffer.data(), "%*s %64[0-9A-Fa-f]:%x %64[0-9A-Fa-f]:%x %*x %*s %*s %*s %u %*u %ld",
                        s_addr,
                        &stat->s_port,
                        d_addr,
                        &stat->d_port,
                        &stat->uid,
                        &ino);

            // ignore first line
            if (nr == 0)
                continue;

            // socket still in waiting state
            if (ino == 0) {
                continue;
            }

            stat->ino = ino;
            stat->sa_family = family;
            stat->proto = proto;

            // saddr & daddr
            if (family == AF_INET6) {
                sscanf(s_addr, "%08x%08x%08x%08x",
                       &stat->s_addr.in6.s6_addr32[0],
                       &stat->s_addr.in6.s6_addr32[1],
                       &stat->s_addr.in6.s6_addr32[2],
                       &stat->s_addr.in6.s6_addr32[3]);
                sscanf(d_addr, "%08x%08x%08x%08x",
                       &stat->d_addr.in6.s6_addr32[0],
                       &stat->d_addr.in6.s6_addr32[1],
                       &stat->d_addr.in6.s6_addr32[2],
                       &stat->d_addr.in6.s6_addr32[3]);
                // convert ipv4 mapped ipv6 address to ipv4
                if (stat->s_addr.in6.s6_addr32[0] == 0x0 &&
                        stat->s_addr.in6.s6_addr32[1] == 0x0 &&
                        stat->s_addr.in6.s6_addr32[2] == 0xffff0000) {
                    stat->sa_family = AF_INET;
                    stat->s_addr.in4.s_addr = stat->s_addr.in6.s6_addr32[3];
                    stat->d_addr.in4.s_addr = stat->d_addr.in6.s6_addr32[3];
                }
            } else {
                sscanf(s_addr, "%x", &stat->s_addr.in4.s_addr);
                sscanf(d_addr, "%x", &stat->d_addr.in4.s_addr);
            }

            if (stat->sa_family == AF_INET) {
                char saddr_str[INET_ADDRSTRLEN + 1] {}, daddr_str[INET_ADDRSTRLEN + 1] {};
                inet_ntop(AF_INET, &stat->s_addr.in4, saddr_str, INET_ADDRSTRLEN);
                inet_ntop(AF_INET, &stat->d_addr.in4, daddr_str, INET_ADDRSTRLEN);

                patternA = QString("%1:%2-%3:%4").arg(saddr_str).arg(stat->s_port).arg(daddr_str).arg(stat->d_port);
                if (proto == IPPROTO_TCP) {
                    patternB = QString("%1:%2-%3:%4").arg(daddr_str).arg(stat->d_port).arg(saddr_str).arg(stat->s_port);
                }

            } else if (stat->sa_family == AF_INET6) {
                char saddr6_str[INET6_ADDRSTRLEN + 1] {}, daddr6_str[INET6_ADDRSTRLEN + 1] {};
                inet_ntop(AF_INET6, &stat->s_addr.in6, saddr6_str, INET6_ADDRSTRLEN);
                inet_ntop(AF_INET6, &stat->d_addr.in6, daddr6_str, INET6_ADDRSTRLEN);

                patternA = QString("%1:%2-%3:%4").arg(saddr6_str).arg(stat->s_port).arg(daddr6_str).arg(stat->d_port);
                if (proto == IPPROTO_TCP) {
                    patternB = QString("%1:%2-%3:%4").arg(daddr6_str).arg(stat->d_port).arg(saddr6_str).arg(stat->s_port);
                }

            } else {
                // unexpected here
            }

            fmtbuf = patternA.toLocal8Bit();
            util::common::hash(fmtbuf.constData(), fmtbuf.length(), util::common::global_seed, cchash);
            hash = cchash[0];
            statMap.insert(hash, stat);

            // if it's TCP, we need add reverse mapping due to its bidirectional piping feature,
            // otherwise we wont be able to get the inode
            if (proto == IPPROTO_TCP) {
                fmtbuf = patternB.toLocal8Bit();
                util::common::hash(fmtbuf.constData(), fmtbuf.length(), util::common::global_seed, cchash);
                hash = cchash[0];
                statMap.insert(hash, stat);
            }
        }
        if (ferror(fp))
        {
            ok = !ok;
        }
        fclose(fp);

        return ok;
    };

    statMap.clear();

    ok = parseSocks(AF_INET, IPPROTO_TCP, PROC_PATH_SOCK_TCP, statMap);
    ok = parseSocks(AF_INET, IPPROTO_UDP, PROC_PATH_SOCK_UDP, statMap) && ok;
    ok = parseSocks(AF_INET6, IPPROTO_TCP, PROC_PATH_SOCK_TCP6, statMap) && ok;
    ok = parseSocks(AF_INET6, IPPROTO_UDP, PROC_PATH_SOCK_UDP6, statMap) && ok;

    return ok;
}

void SysInfo::readSysInfo()
{
    d->nfds = read_file_nr();
    d->nprocs = read_processes();
    d->nthrs = read_threads();
    
    read_uptime(d->uptime);
    read_btime(d->btime);
    read_loadavg(d->loadAvg);
}

void SysInfo::readSysInfoStatic()
{
    d->uid = getuid();
    d->euid = geteuid();
    d->gid = getgid();
    d->egid = getegid();

    d->user_name = userName(d->uid);
    d->effective_user_name = userName(d->euid);
    if (d->effective_user_name.isEmpty())
        d->effective_user_name = QByteArray(getenv("USER"));

    d->group_name = groupName(d->gid);
    d->effective_group_name = groupName(d->egid);
    if (d->effective_group_name.isEmpty())
        d->effective_group_name = QByteArray(getenv("GROUP"));
    d->hostname = read_hostname();
    d->arch = read_arch();
    d->version = read_version();
}

quint32 SysInfo::read_file_nr()
{
    FILE *fp;
    unsigned int file_nr = 0;

    errno = 0;
    if ((fp = fopen(PROC_PATH_FILE_NR, "r"))) {
        uFile fPtr;
        fPtr.reset(fp);

        char buf[128] {};
        if (fgets(buf, sizeof(buf), fp)) {
            int nm = sscanf(buf, "%u", &file_nr);
            if (nm == 1)
                return file_nr;

            print_errno(errno, QString("read %1 failed").arg(PROC_PATH_FILE_NR));
            return 0;
        } // ::if(fgets)
    } // ::if(fopen)

    print_errno(errno, QString("open %1 failed").arg(PROC_PATH_FILE_NR));
    return 0;
}

quint32 SysInfo::read_threads()
{
    QDir dir("/proc");
    QFileInfoList infoList = dir.entryInfoList();
    quint32 threads = 0;
    for (QFileInfo info : infoList) {
        if (info.isDir() && info.fileName().toInt() > 0) {
            QDir taskDir("/proc/" + info.fileName() + "/task");
            threads += taskDir.entryInfoList().count();
        }
    }

    return threads;
}

quint32 SysInfo::read_processes()
{
    QDir dir("/proc");
    QFileInfoList infoList = dir.entryInfoList();
    quint32 processes = 0;
    for (QFileInfo info : infoList) {
        if (info.isDir() && info.fileName().toInt() > 0) {
            processes++;
        }
    }

    return processes;
}

QString SysInfo::read_hostname()
{
    QDBusInterface busIf("org.freedesktop.hostname1",
                         "/org/freedesktop/hostname1",
                         "org.freedesktop.hostname1",
                         QDBusConnection::systemBus());
    auto name = busIf.property("Hostname");
    return name.toString();
}

QString SysInfo::read_arch()
{
    QString buffer {};

    struct utsname os {
    };
    auto rc = uname(&os);
    if (!rc)
        buffer = os.machine;

    return buffer;
}

QString SysInfo::read_version()
{
    auto ver = QString("%1 %2")
               //etc/os-version 下 MajorVersion
               .arg(DSysInfo::majorVersion())
               //etc/os-version 下 EditionName
               .arg(DSysInfo::uosEditionName());
    return ver;
}

void SysInfo::read_uptime(struct timeval &uptime)
{
    FILE *fp;
    errno = 0;

    if ((fp = fopen(PROC_PATH_UPTIME, "r"))) {
        uFile fPtr;
        fPtr.reset(fp);

        char buf[128] {};
        long up_sec, up_usec;
        if (fgets(buf, sizeof(buf), fp)) {
            int nm = sscanf(buf, "%ld.%ld", &up_sec, &up_usec);
            if (nm == 2) {
                uptime.tv_sec = up_sec;
                uptime.tv_usec = up_usec * 1000000;

                return;
            }
        } // ::if(gets)

        print_errno(errno, QString("read %1 failed").arg(PROC_PATH_UPTIME));
        return;
    } // ::if(fopen)

    print_errno(errno, QString("open %1 failed").arg(PROC_PATH_UPTIME));
}

void SysInfo::read_btime(struct timeval &btime)
{
    FILE *fp;
    errno = 0;
    if ((fp = fopen(PROC_PATH_STAT, "r"))) {
        uFile fPtr;
        fPtr.reset(fp);

        char line[4095] {};
        long nsec {0};
        while (fgets(line, sizeof(line), fp)) {
            if (!strncmp(line, "btime", 5)) {
                int nm = sscanf(line + 5, "%ld", &nsec);
                if (nm == 1) {
                    btime.tv_sec = nsec;
                    btime.tv_usec = 0;

                    return;
                }
            } // ::if(cmp)
        } // ::while

        print_errno(errno, QString("read %1 failed").arg(PROC_PATH_STAT));
        return;
    } // ::if(fopen)

    print_errno(errno, QString("open %1 failed").arg(PROC_PATH_STAT));
}

void SysInfo::read_loadavg(LoadAvg &loadAvg)
{
    QFile file("/proc/loadavg");
    if (file.exists() && file.open(QFile::ReadOnly)) {
        // 只需要读取第一行数据
        QByteArray lineData = file.readLine();
        file.close();
        /*样例数据:
            $ cat /proc/loadavg
            0.41 0.46 0.36 2/2646 20183
        */

        // 分割数据 取前3
        QStringList cpuStatus =  QString(lineData).split(" ", QString::SkipEmptyParts);

        if (cpuStatus.size() > 3) {
            loadAvg->lavg_1m = cpuStatus[0].toFloat();
            loadAvg->lavg_5m = cpuStatus[1].toFloat();
            loadAvg->lavg_15m = cpuStatus[2].toFloat();

            return ;
        }
    }

    print_errno(errno, QString("call sysinfo failed"));
}

} // namespace system
} // namespace core
