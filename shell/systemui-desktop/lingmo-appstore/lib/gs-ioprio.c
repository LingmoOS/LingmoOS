/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2005, Novell, Inc.
 * Copyright (C) 2006, Jamie McCracken <jamiemcc@lingmo.org>
 * Copyright (C) 2006, Anders Aagaard
 *
 * Based mostly on code by Robert Love <rml@novell.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Appstore"),
 * to deal in the Appstore without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Appstore, and to permit persons to whom the
 * Appstore is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Appstore.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

#ifdef __linux__

#include <stdio.h>
#include <errno.h>

#ifdef HAVE_LINUX_UNISTD_H
#include <linux/unistd.h>
#endif

#include <sys/syscall.h>
#include <unistd.h>

#include <glib/gstdio.h>

#endif /* __linux__ */

#include "gs-ioprio.h"

/* We assume ALL linux architectures have the syscalls defined here */
#ifdef __linux__

/* Make sure the system call is supported */
#ifndef __NR_ioprio_set

#if defined(__i386__)
#define __NR_ioprio_set                 289
#define __NR_ioprio_get                 290
#elif defined(__powerpc__) || defined(__powerpc64__)
#define __NR_ioprio_set                 273
#define __NR_ioprio_get                 274
#elif defined(__x86_64__)
#define __NR_ioprio_set                 251
#define __NR_ioprio_get                 252
#elif defined(__ia64__)
#define __NR_ioprio_set                 1274
#define __NR_ioprio_get                 1275
#elif defined(__alpha__)
#define __NR_ioprio_set                 442
#define __NR_ioprio_get                 443
#elif defined(__s390x__) || defined(__s390__)
#define __NR_ioprio_set                 282
#define __NR_ioprio_get                 283
#elif defined(__SH4__)
#define __NR_ioprio_set                 288
#define __NR_ioprio_get                 289
#elif defined(__SH5__)
#define __NR_ioprio_set                 316
#define __NR_ioprio_get                 317
#elif defined(__sparc__) || defined(__sparc64__)
#define __NR_ioprio_set                 196
#define __NR_ioprio_get                 218
#elif defined(__arm__)
#define __NR_ioprio_set                 314
#define __NR_ioprio_get                 315
#else
#error "Unsupported architecture!"
#endif

#endif /* __NR_ioprio_set */

enum {
	IOPRIO_CLASS_NONE,
	IOPRIO_CLASS_RT,
	IOPRIO_CLASS_BE,
	IOPRIO_CLASS_IDLE,
};

enum {
	IOPRIO_WHO_PROCESS = 1,
	IOPRIO_WHO_PGRP,
	IOPRIO_WHO_USER,
};

#define IOPRIO_CLASS_SHIFT 13

static inline int
ioprio_set (int which, int who, int ioprio_val)
{
	return syscall (__NR_ioprio_set, which, who, ioprio_val);
}

static int
set_io_priority (int ioprio,
		 int ioclass)
{
	return ioprio_set (IOPRIO_WHO_PROCESS, 0, ioprio | (ioclass << IOPRIO_CLASS_SHIFT));
}

static const gchar *
ioclass_to_string (int ioclass)
{
	switch (ioclass) {
	case IOPRIO_CLASS_IDLE:
		return "IDLE";
	case IOPRIO_CLASS_BE:
		return "BE";
	default:
		return "unknown";
	}
}

/**
 * gs_ioprio_set:
 * @priority: I/O priority, with higher numeric values indicating lower priority;
 *   use %G_PRIORITY_DEFAULT as the default
 *
 * Set the I/O priority of the current thread using the `ioprio_set()` syscall.
 *
 * The @priority is quantised before being passed to the kernel.
 *
 * This function may fail if the process doesn’t have permission to change its
 * I/O priority to the given value. If so, a warning will be printed, as the
 * quantised priority values are chosen so they shouldn’t typically require
 * permissions to set.
 */
void
gs_ioprio_set (gint priority)
{
	int ioprio, ioclass;

	/* If the priority is lower than default, use an idle I/O priority. The
	 * condition looks wrong because higher integers indicate lower priority
	 * in GLib.
	 *
	 * Otherwise use a default best-effort priority, which is the same as
	 * what all new threads get (in the absence of an I/O context with
	 * `CLONE_IO`). */
	if (priority > G_PRIORITY_DEFAULT) {
		ioprio = 7;
		ioclass = IOPRIO_CLASS_IDLE;
	} else if (priority == G_PRIORITY_DEFAULT) {
		ioprio = 4;  /* this is the default priority in the BE class */
		ioclass = IOPRIO_CLASS_BE;
	} else {
		ioprio = 0;  /* this is the highest priority in the BE class */
		ioclass = IOPRIO_CLASS_BE;
	}

	g_debug ("Setting I/O priority of thread %p to %s, %d",
		 g_thread_self (), ioclass_to_string (ioclass), ioprio);

	if (set_io_priority (ioprio, ioclass) == -1) {
		g_warning ("Could not set I/O priority to %s, %d",
			   ioclass_to_string (ioclass), ioprio);

		/* If we were trying to set to idle priority, try again with the
		 * lowest-possible best-effort priority. This is because kernels
		 * older than 2.6.25 required `CAP_SYS_ADMIN` to set
		 * `IOPRIO_CLASS_IDLE`. Newer kernels do not. */
		if (ioclass == IOPRIO_CLASS_IDLE) {
			ioprio = 7;  /* this is the lowest priority in the BE class */
			ioclass = IOPRIO_CLASS_BE;

			if (set_io_priority (ioprio, ioclass) == -1)
				g_warning ("Could not set best effort IO priority either, giving up");
		}
	}
}

#else  /* __linux__ */

void
gs_ioprio_set (gint priority)
{
}

#endif /* __linux__ */
