// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYS_INFO_H
#define SYS_INFO_H

// local
#include "private/sys_info_p.h"
#include "packet.h"
// qt
#include <QtGlobal>
#include <QSharedDataPointer>
#include <QDebug>

// c++11
#include <memory>

// posix
#include <pwd.h>
#include <grp.h>

namespace core {
namespace process {
class ProcessSet;
}
} // namespace core
using namespace core::process;

namespace core {
namespace system {

struct load_avg_t {
    float lavg_1m {0};
    float lavg_5m {0};
    float lavg_15m {0};
};
using LoadAvg = std::shared_ptr<struct load_avg_t>;

inline QString &operator<<(QString &buffer, const struct load_avg_t &rhs);

class CPUSet;

class SysInfo
{
public:
    SysInfo();
    SysInfo(const SysInfo &other);
    SysInfo &operator=(const SysInfo &rhs);
    virtual ~SysInfo();

    static SysInfo *instance();

    quint32 nprocesses() const;
    quint32 nthreads() const;
    quint32 nfds() const;
    QString hostname() const;
    QString arch() const;
    QString version() const;
    LoadAvg loadAvg() const;
    struct timeval btime() const;
    struct timeval uptime() const;

    uid_t uid() const;
    gid_t gid() const;
    uid_t euid() const;
    gid_t egid() const;

    QByteArray realUserName() const;
    QByteArray realGroupName() const;
    QByteArray effectiveUserName() const;
    QByteArray effectiveGroupName() const;

    static QByteArray userName(uid_t uid);
    static QByteArray groupName(gid_t gid);

    void readSysInfo();
    void readSysInfoStatic();
    static bool readSockStat(SockStatMap &statMap);

private:
    quint32 read_file_nr();
    quint32 read_threads();
    quint32 read_processes();
    QString read_hostname();
    QString read_arch();
    QString read_version();
    void read_uptime(struct timeval &uptime);
    void read_btime(struct timeval &btime);
    void read_loadavg(LoadAvg &loadAvg);

    inline void set_nprocesses(quint32 nprocs);
    inline void set_nthreads(quint32 nthrs);
    inline void set_btime(struct timeval &btime);

private:
    QSharedDataPointer<SysInfoPrivate> d;

    friend class CPUSet;
    friend class ProcessSet;
};

inline quint32 SysInfo::nprocesses() const
{
    return d->nprocs;
}

inline quint32 SysInfo::nthreads() const
{
    return d->nthrs;
}

inline quint32 SysInfo::nfds() const
{
    return d->nfds;
}

inline QString SysInfo::hostname() const
{
    return d->hostname;
}

inline QString SysInfo::arch() const
{
    return d->arch;
}

inline QString SysInfo::version() const
{
    return d->version;
}

inline LoadAvg SysInfo::loadAvg() const
{
    return d->loadAvg;
}

inline timeval SysInfo::btime() const
{
    return d->btime;
}

inline timeval SysInfo::uptime() const
{
    return d->uptime;
}

inline uid_t SysInfo::uid() const
{
    return d->uid;
}

inline gid_t SysInfo::gid() const
{
    return d->gid;
}

inline uid_t SysInfo::euid() const
{
    return d->euid;
}

inline gid_t SysInfo::egid() const
{
    return d->egid;
}

inline QByteArray SysInfo::realUserName() const
{
    return d->user_name;
}

inline QByteArray SysInfo::realGroupName() const
{
    return d->group_name;
}

inline QByteArray SysInfo::effectiveUserName() const
{
    return d->effective_user_name;
}

inline QByteArray SysInfo::effectiveGroupName() const
{
    return d->effective_group_name;
}

inline QByteArray SysInfo::userName(uid_t uid)
{
    auto *pwd = getpwuid(uid);
    if (pwd)
        return {pwd->pw_name};
    else
        return {};
}

inline QByteArray SysInfo::groupName(gid_t gid)
{
    auto *grp = getgrgid(gid);
    if (grp)
        return {grp->gr_name};
    else
        return {};
}

inline void SysInfo::set_nprocesses(quint32 nprocs)
{
    d->nprocs = nprocs;
}

inline void SysInfo::set_nthreads(quint32 nthrs)
{
    d->nthrs = nthrs;
}

inline void SysInfo::set_btime(timeval &btime)
{
    d->btime = btime;
}

inline QString &operator<<(QString &buffer, const load_avg_t &rhs)
{
    return buffer.append(QString("%1 %2 %3")
                         .arg(rhs.lavg_1m , 0, 'f', 2, QLatin1Char(' '))
                         .arg(rhs.lavg_5m , 0, 'f', 2, QLatin1Char(' '))
                         .arg(rhs.lavg_15m , 0, 'f', 2, QLatin1Char(' ')));
}

} // namespace system
} // namespace core

Q_DECLARE_TYPEINFO(core::system::SysInfo, Q_MOVABLE_TYPE);

#endif // SYS_INFO_H
