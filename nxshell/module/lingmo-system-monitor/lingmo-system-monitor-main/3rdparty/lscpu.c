/*
 * lscpu - CPU architecture information helper
 *
 * Copyright (C) 2008 Cai Qian <qcai@redhat.com>
 * Copyright (C) 2008 Karel Zak <kzak@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/personality.h>

#include <libsmartcols.h>

#include "closestream.h"
#include "optutils.h"

#include "lscpu.h"

struct polarization_modes {
	char *parsable;
	char *readable;
};


/*
 * IDs
 */
enum {
	COL_CPU_BOGOMIPS,
	COL_CPU_CPU,
	COL_CPU_CORE,
	COL_CPU_SOCKET,
	COL_CPU_CLUSTER,
	COL_CPU_NODE,
	COL_CPU_BOOK,
	COL_CPU_DRAWER,
	COL_CPU_CACHE,
	COL_CPU_POLARIZATION,
	COL_CPU_ADDRESS,
	COL_CPU_CONFIGURED,
	COL_CPU_ONLINE,
	COL_CPU_MHZ,
	COL_CPU_SCALMHZ,
	COL_CPU_MAXMHZ,
	COL_CPU_MINMHZ,
};

enum {
	COL_CACHE_ALLSIZE,
	COL_CACHE_LEVEL,
	COL_CACHE_NAME,
	COL_CACHE_ONESIZE,
	COL_CACHE_TYPE,
	COL_CACHE_WAYS,
	COL_CACHE_ALLOCPOL,
	COL_CACHE_WRITEPOL,
	COL_CACHE_PHYLINE,
	COL_CACHE_SETS,
	COL_CACHE_COHERENCYSIZE
};


/* column description
 */
struct lscpu_coldesc {
	const char *name;
	const char *help;

	int flags;
	unsigned int  is_abbr:1;	/* name is abbreviation */
	int json_type;
};

UL_DEBUG_DEFINE_MASK(lscpu);
