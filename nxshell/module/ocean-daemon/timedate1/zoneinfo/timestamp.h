// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __TIMESTAMP_H__
#define __TIMESTAMP_H__

typedef struct _DSTTime {
	long long enter;
	long long leave;
} DSTTime;

long long get_rawoffset_usec (const char *zone, long long t);
long get_offset_by_usec (const char *zone, long long t);
DSTTime get_dst_time(const char *zone, int year);

#endif
