// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <sys/types.h>

class Line
{
public:
    Line(const char *name, const char *cmdline, double n_recv_value,
         double n_sent_value, pid_t pid, uid_t uid, const char *n_devicename);

    double sent_value;
    double recv_value;
    const char *devicename;
    const char *m_name;
    const char *m_cmdline;
    pid_t m_pid;
    uid_t m_uid;
};
