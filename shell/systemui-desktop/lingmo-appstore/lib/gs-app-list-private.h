/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "gs-app-list.h"

G_BEGIN_DECLS

/**
 * GsAppListFlags:
 * @GS_APP_LIST_FLAG_NONE:			No flags set
 * @GS_APP_LIST_FLAG_IS_TRUNCATED:		List has been truncated
 * @GS_APP_LIST_FLAG_WATCH_APPS:		Applications will be monitored
 * @GS_APP_LIST_FLAG_WATCH_APPS_RELATED:	Applications related apps will be monitored
 * @GS_APP_LIST_FLAG_WATCH_APPS_ADDONS:		Applications addon apps will be monitored
 *
 * Flags used to describe the list.
 **/
typedef enum {
	GS_APP_LIST_FLAG_NONE			= 0,
	/* empty slot */
	GS_APP_LIST_FLAG_IS_TRUNCATED		= 1 << 1,
	GS_APP_LIST_FLAG_WATCH_APPS		= 1 << 2,
	GS_APP_LIST_FLAG_WATCH_APPS_RELATED	= 1 << 3,
	GS_APP_LIST_FLAG_WATCH_APPS_ADDONS	= 1 << 4,
	GS_APP_LIST_FLAG_LAST  /*< skip >*/
} GsAppListFlags;

guint		 gs_app_list_get_size_peak	(GsAppList	*list);
void		 gs_app_list_set_size_peak	(GsAppList	*list,
						 guint		 size_peak);
void		 gs_app_list_filter_duplicates	(GsAppList	*list,
						 GsAppListFilterFlags flags);
void		 gs_app_list_randomize		(GsAppList	*list);
void		 gs_app_list_truncate		(GsAppList	*list,
						 guint		 length);
gboolean	 gs_app_list_has_flag		(GsAppList	*list,
						 GsAppListFlags	 flag);
void		 gs_app_list_add_flag		(GsAppList	*list,
						 GsAppListFlags	 flag);
GsAppState	 gs_app_list_get_state		(GsAppList	*list);
guint		 gs_app_list_get_progress	(GsAppList	*list);

G_END_DECLS
