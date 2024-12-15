// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wtmpparse.h"
#include <QDebug>

int wtmp_open(char *filename)
{
    fdWtmp = open(filename, O_RDONLY);
    cur_rec = num_recs = 0;
    return fdWtmp;
}


int wtmp_open_back(char *filename)
{
    fdWtmp = open(filename, O_RDONLY);
    bpos = cur_rec = num_recs = 0;
    fpos = lseek(fdWtmp, 0, SEEK_END);
    return fdWtmp;
}

int seek_end(void)
{
    return 0;
}

int wtmp_reload(void)
{
    int amt_read;
    amt_read = read(fdWtmp, utmpbuf, NRECS * UTSIZE);
    num_recs = amt_read / UTSIZE;

    cur_rec = 0;

    return num_recs;
}


struct utmp *wtmp_next(void)
{
    struct utmp *recp;

    if (fdWtmp == -1)
        return NULLUT;

    if (cur_rec == num_recs && wtmp_reload() == 0)
        return NULLUT;

    recp = (struct utmp *)&utmpbuf[cur_rec * UTSIZE];
    cur_rec++;

    return recp;
}


int wtmp_reload_back(void)
{
    int amt_read;

    off_t o;
    o = ((fpos - 1) / (NRECS * UTSIZE)) * (NRECS * UTSIZE);
    bpos = (int)(fpos - o);
    fpos -= NRECS * UTSIZE;
    amt_read = pread(fdWtmp, utmpbuf, NRECS * UTSIZE, fpos);
    num_recs = amt_read / UTSIZE;

    cur_rec = 0;

    return num_recs;
}
struct utmp *wtmp_back(void)
{
    struct utmp *recp;

    if (fdWtmp == -1)
        return NULLUT;

    if (cur_rec == 0 && wtmp_reload_back() == 0)
        return NULLUT;

    recp = (struct utmp *)&utmpbuf[cur_rec * UTSIZE];
    cur_rec--;

    return recp;
}

void wtmp_close(void)
{
    if (fdWtmp != -1)
        close(fdWtmp);
}

struct utmp_list *st_list_init(void)
{
    struct utmp_list *list = static_cast<struct utmp_list *>(malloc(sizeof(struct utmp_list)));
    if (!list) {
        printf("struct utmp_list malloc failed\n");
        return NULL;
    }

    memset(list, 0, sizeof(struct utmp_list));

    return list;
}

struct utmp *st_utmp_init(void)
{
    struct utmp *stUTMP = static_cast<struct utmp *>(malloc(sizeof(struct utmp)));
    if (!stUTMP) {
        printf("struct utmp malloc failed\n");
        return NULL;
    }

    memset(stUTMP, 0, sizeof(struct utmp));

    return stUTMP;
}

utmp_list * list_delete(struct utmp_list *list)
{
    while (list->next) {
        struct utmp_list *p = list;
        list = p->next;
        free(p);
    }
    return list;
}

void list_insert(QList<utmp *> &plist, struct utmp *value)
{
    utmp v = *value;
    utmp *value_ = &(v);
    plist.append(value_);

}

utmp list_get_ele_and_del(QList<utmp > &list, char *value, int &rs)
{
    utmp temp = {};
    for (int i = 0; i < list.length(); ++i) {
        utmp itemValue = list.value(i);
        QString a(itemValue.ut_line);
        QString b(value);
        int result = QString::compare(a, b);
        if (result == 0) {
            list.removeAt(i);
            rs = 0;
            return itemValue;
        }
    }
    rs = -1;
    return temp;
}

char *show_end_time(time_t timeval)
{
    struct tm *t;
    char tt[256] = {0};

    t = localtime(&timeval);
    strftime(tt, sizeof(tt) - 1, "%R", t);
    //printf("%s", tt);
    return asctime(t);
}

char *show_start_time(time_t timeval)
{
    struct tm *t;
    char tt[256] = {0};

    t = localtime(&timeval);
    strftime(tt, sizeof(tt) - 1, "%a %b %e %R", t);
    //    printf("%s", tt);
    //    printf(" - ");

    return asctime(t);
}
void show_base_info(struct utmp *uBuf)
{
    printf("%-9.8s", uBuf->ut_name);

    if (uBuf->ut_type == BOOT_TIME)
        printf("%-13s", "system boot");
    else
        printf("%-13.8s", uBuf->ut_line);

    printf("%-17.16s", uBuf->ut_host);
}

