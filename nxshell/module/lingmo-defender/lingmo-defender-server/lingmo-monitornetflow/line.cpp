// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "line.h"
#include <assert.h>

#define PID_MAX 4194303

Line::Line(const char *name, const char *cmdline, double n_recv_value,
     double n_sent_value, pid_t pid, uid_t uid, const char *n_devicename)
{
    assert(pid >= 0);
    assert(pid <= PID_MAX);
    m_name = name;
    m_cmdline = cmdline;
    sent_value = n_sent_value;
    recv_value = n_recv_value;
    devicename = n_devicename;
    m_pid = pid;
    m_uid = uid;
    assert(m_pid >= 0);
}
