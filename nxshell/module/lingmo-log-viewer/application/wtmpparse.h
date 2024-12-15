// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WTMPPARSE_H
#define WTMPPARSE_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <utmp.h>
#include <QDebug>
#define NRECS 16
#define NULLUT ((struct utmp *)NULL)
#define UTSIZE (sizeof(struct utmp))

static char utmpbuf[NRECS * UTSIZE];
static int num_recs;
static int cur_rec;
static int fdWtmp = -1;
static int bpos;
static __off_t fpos;

struct utmp_list {
    struct utmp value;
    struct utmp_list *next;
};
int wtmp_open_back(char *filename);
int wtmp_open(char *filename);
int seek_end(void);
int wtmp_reload(void);
int wtmp_reload_back(void);
struct utmp *wtmp_next(void);
struct utmp *wtmp_back(void);
void wtmp_close(void);

struct utmp_list *st_list_init(void);

struct utmp *st_utmp_init(void);

utmp_list * list_delete(struct utmp_list *list);

void list_insert(QList<utmp *> &plist, struct utmp *value);

utmp list_get_ele_and_del(QList<utmp > &list, char *value, int &rs);

char *show_end_time(time_t timeval);

char *show_start_time(time_t timeval);

void show_base_info(struct utmp *uBuf);

#endif  // WTMPPARSE_H
