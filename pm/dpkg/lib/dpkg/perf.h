/*
 * libdpkg - Debian packaging suite library routines
 * perf.h - performance testing support
 *
 * Copyright © 2009-2019 Guillem Jover <guillem@debian.org>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef LIBDPKG_PERF_H
#define LIBDPKG_PERF_H

#include <config.h>
#include <compat.h>

#include <time.h>
#include <stdio.h>

#define TEST_OMIT_VARIABLES
#include <dpkg/test.h>

DPKG_BEGIN_DECLS

struct perf_slot {
	struct timespec t_ini, t_end;
};

static inline void
perf_ts_sub(struct timespec *a, struct timespec *b, struct timespec *res)
{
	res->tv_sec = a->tv_sec - b->tv_sec;
	res->tv_nsec = a->tv_nsec - b->tv_nsec;
	if (res->tv_nsec < 0) {
		res->tv_sec--;
		res->tv_nsec += 1000000000;
	}
}

static inline void
perf_ts_mark_print(const char *str)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	printf("%lu.%.9lu: %s\n", ts.tv_sec, ts.tv_nsec, str);
}

static inline void
perf_ts_slot_print(struct perf_slot *ps, const char *str)
{
	struct timespec t_res;

	perf_ts_sub(&ps->t_end, &ps->t_ini, &t_res);

	printf("%lu.%.9lu: %s (%lu.%.9lu sec)\n",
	       ps->t_end.tv_sec, ps->t_end.tv_nsec,
	       str, t_res.tv_sec, t_res.tv_nsec);
}

#define perf_ts_slot_start(ps) clock_gettime(CLOCK_MONOTONIC, &((ps)->t_ini))
#define perf_ts_slot_stop(ps) clock_gettime(CLOCK_MONOTONIC, &((ps)->t_end))

DPKG_END_DECLS

#endif
