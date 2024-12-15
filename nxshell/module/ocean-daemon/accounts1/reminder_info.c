// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "reminder_info.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <shadow.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

int count_utmpx(const char *file,
                const char *user,
                struct timeval *since,
                struct utmpx *current,
                struct utmpx *last) {
    if (utmpxname(file) != 0) {
        fprintf(stderr, "failed to call utmpxname: %s\n", strerror(errno));
        return 0;
    }

    int cnt = 0;

    setutxent();

    struct utmpx current_u = {0};
    struct utmpx last_u = {0};
    struct utmpx *u = NULL;
    while ((u = getutxent())) {
        if (since != NULL) {
            struct timeval tc = {0};
            tc.tv_sec = u->ut_tv.tv_sec;
            tc.tv_usec = u->ut_tv.tv_usec;

            if (timercmp(since, &tc, >)) {
                continue;
            }
        }

        if (strcmp(user, u->ut_user) != 0) {
            continue;
        }

        last_u = current_u;
        current_u = *u;

        cnt++;
    }

    if (current_u.ut_type != EMPTY && current != NULL) {
        *current = current_u;
    }

    if (last_u.ut_type != EMPTY && last != NULL) {
        *last = last_u;
    }

    endutxent();

    return cnt;
}
