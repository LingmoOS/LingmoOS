// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYS_INFO_P_H
#define SYS_INFO_P_H

#include <QSharedData>

#include <memory>

namespace core {
namespace system {

class SysInfo;
struct load_avg_t;
using LoadAvg = std::shared_ptr<struct load_avg_t>;

class SysInfoPrivate : public QSharedData
{
public:
    SysInfoPrivate()
        : QSharedData()
        , uid {0}
        , gid {0}
        , euid {0}
        , egid {0}
        , nprocs {0}
        , nthrs {0}
        , nfds {0}
        , hostname {}
        , arch {}
        , version {}
        , uptime {timeval {0, 0}}
        , btime {timeval {0, 0}}
        , loadAvg {}
        , user_name {}
        , group_name {}
        , effective_user_name {}
        , effective_group_name {}
    {
    }
    SysInfoPrivate(const SysInfoPrivate &other)
        : QSharedData(other)
        , uid(other.uid)
        , gid(other.gid)
        , euid(other.euid)
        , egid(other.egid)
        , nprocs(other.nprocs)
        , nthrs(other.nthrs)
        , nfds(other.nfds)
        , hostname(other.hostname)
        , arch(other.arch)
        , version(other.version)
        , uptime(other.uptime)
        , btime(other.btime)
        , loadAvg(other.loadAvg)
        , user_name(other.user_name)
        , group_name(other.group_name)
        , effective_user_name(other.effective_user_name)
        , effective_group_name(other.effective_group_name)
    {
    }
    ~SysInfoPrivate() {}

private:
    uid_t uid; // real user name
    gid_t gid; // real group name
    uid_t euid; // effective user name
    gid_t egid; // effective group name

    quint32 nprocs; // number of processes
    quint32 nthrs; // number of threads
    quint32 nfds; // number of file descriptors
    QString hostname; // hostname
    QString arch; // os type
    QString version; // os version

    struct timeval uptime; // up time
    struct timeval btime; // boot time
    LoadAvg loadAvg; // load avg.

    QByteArray user_name; // real user name
    QByteArray group_name; // real group name
    QByteArray effective_user_name; // effective user name
    QByteArray effective_group_name; // effective group name

    friend class SysInfo;
};

} // namespace system
} // namespace core

#endif // SYS_INFO_P_H
