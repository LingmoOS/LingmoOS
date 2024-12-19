/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2023 Endless OS Foundation LLC
 *
 * Authors:
 *  - Georges Basile Stavracas Neto <georges@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "config.h"
#include <lingmo-appstore.h>

/**
 * SECTION:gs-profiler
 * @short_description: Macros for profiling
 *
 * LINGMO Appstore provides a simple profiling mechanism that both plugins
 * and LINGMO Appstore itself can make use. Use the GS_PROFILER_BEGIN_SCOPED()
 * macro to start profiling a specific code section; and pair it with
 * GS_PROFILER_END_SCOPED() to finish profiling the section:
 *
 * ```
 * GS_PROFILER_BEGIN_SCOPED(Flatpak, "list-installed-refs", "Description");
 * ... list all installed refs ...
 * GS_PROFILER_END_SCOPED(Flatpak);
 *
 *
 * GS_PROFILER_BEGIN_SCOPED(Foo, "parse-something", "Parse some data");
 * ... parse some data ...
 * GS_PROFILER_END_SCOPED(Foo);
 * ```
 *
 * The macros are scoped, so work correctly with early returns in the middle:
 *
 * ```
 * GS_PROFILER_BEGIN_SCOPED(Foo, "list-applications", NULL);
 *
 * if (!foo_is_correct (foo))
 * 	return FALSE;
 *
 * if (!long_async_op (foo, &error))
 * 	return FALSE;
 *
 * return TRUE;
 *
 * GS_PROFILER_END_SCOPED(Foo);
 * ```
 *
 * The description argument is nullable:
 *
 * ```
 * GS_PROFILER_BEGIN_SCOPED(Flatpak, "list-installed-refs", NULL);
 * ... list all installed refs ...
 * GS_PROFILER_END_SCOPED(Flatpak);
 *```
 *
 * A rather common case is to allocate new strings for the Sysprof name
 * and description. The convenience macro GS_PROFILER_BEGIN_SCOPED_TAKE() is
 * provided for that:
 *
 * ```
 * GS_PROFILER_BEGIN_SCOPED_TAKE(Foo, g_strdup_printf ("list-installed-refs:%s", name), NULL);
 * ... list all installed refs ...
 * GS_PROFILER_END_SCOPED(Foo);
 *```
 *
 * Asynchronous operations might need to track the start and end times in
 * separate functions. The convenience macros GS_PROFILER_ADD_MARK() and
 * GS_PROFILER_ADD_MARK_TAKE() allow for passing an independent begin time:
 *
 * ```
 * GS_PROFILER_ADD_MARK(Foo, task->begin_time, "do-something", NULL);
 *```
 *
 * Since: 44
 */

#ifdef HAVE_SYSPROF
#include <sysprof-capture.h>

typedef struct
{
	int64_t begin_time;
	gchar *name;
	gchar *description;
} GsProfilerHead;

static inline void
gs_profiler_tracing_end (GsProfilerHead *head)
{
	sysprof_collector_mark (head->begin_time,
				SYSPROF_CAPTURE_CURRENT_TIME - head->begin_time,
				"lingmo-appstore",
				head->name,
				head->description);

	g_clear_pointer (&head->name, g_free);
	g_clear_pointer (&head->description, g_free);
}
static inline void
gs_profiler_auto_trace_end_helper (GsProfilerHead **head)
{
	if (*head)
		gs_profiler_tracing_end (*head);
}

#define GS_PROFILER_BEGIN_SCOPED_TAKE(Name, sysprof_name, sysprof_description) \
	G_STMT_START { \
	GsProfilerHead GsProfiler##Name; \
	__attribute__((cleanup (gs_profiler_auto_trace_end_helper))) \
		GsProfilerHead *ScopedGsProfilerTraceHead##Name = &GsProfiler##Name; \
	GsProfiler##Name = (GsProfilerHead) { \
		.begin_time = SYSPROF_CAPTURE_CURRENT_TIME, \
		.name = sysprof_name, \
		.description = sysprof_description, \
	};

#define GS_PROFILER_BEGIN_SCOPED(Name, sysprof_name, sysprof_description) \
	GS_PROFILER_BEGIN_SCOPED_TAKE (Name, g_strdup (sysprof_name), g_strdup (sysprof_description))

#define GS_PROFILER_END_SCOPED(Name) \
	} G_STMT_END

#define GS_PROFILER_ADD_MARK_TAKE(Name, begin_time, sysprof_name, sysprof_description) \
	G_STMT_START { \
		g_autofree char *_owned_sysprof_name_##Name = sysprof_name; \
		g_autofree char *_owned_sysprof_description_##Name = sysprof_description; \
		sysprof_collector_mark (begin_time, \
					SYSPROF_CAPTURE_CURRENT_TIME - begin_time, \
					"lingmo-appstore", \
					_owned_sysprof_name_##Name, \
					_owned_sysprof_description_##Name); \
	} G_STMT_END

#define GS_PROFILER_ADD_MARK(Name, begin_time, sysprof_name, sysprof_description) \
	GS_PROFILER_ADD_MARK_TAKE (Name, begin_time, g_strdup (sysprof_name), g_strdup (sysprof_description))

#else

#define GS_PROFILER_BEGIN_SCOPED_TAKE(Name, sysprof_name, sysprof_description) \
	G_STMT_START {
#define GS_PROFILER_BEGIN_SCOPED(Name, sysprof_name, sysprof_description) \
	G_STMT_START {
#define GS_PROFILER_END_SCOPED(Name) \
	} G_STMT_END
#define GS_PROFILER_ADD_MARK_TAKE(Name, begin_time, sysprof_name, sysprof_description)
#define GS_PROFILER_ADD_MARK(Name, begin_time, sysprof_name, sysprof_description)

#endif
