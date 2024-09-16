// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REMINDER_INFO_H
#define REMINDER_INFO_H

#include <utmpx.h>
#include <time.h>

#ifndef BTMPX_FILE
#define BTMPX_FILE "/var/log/btmp"
#endif

int count_utmpx(const char *file,
                const char *user,
                struct timeval *since,
                struct utmpx *current,
                struct utmpx *last);

#endif // !REMINDER_INFO_H
