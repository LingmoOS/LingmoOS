/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2022, 2023 Endless OS Foundation LLC
 *
 * Author: Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "gs-app.h"
#include "gs-plugin-job.h"

G_BEGIN_DECLS

#define GS_TYPE_JOB_MANAGER (gs_job_manager_get_type ())

G_DECLARE_FINAL_TYPE (GsJobManager, gs_job_manager, GS, JOB_MANAGER, GObject)

GsJobManager	*gs_job_manager_new				(void);

gboolean	 gs_job_manager_add_job				(GsJobManager	*self,
								 GsPluginJob	*job);
gboolean	 gs_job_manager_remove_job			(GsJobManager	*self,
								 GsPluginJob	*job);

GPtrArray	*gs_job_manager_get_pending_jobs_for_app	(GsJobManager	*self,
								 GsApp		*app);
gboolean	 gs_job_manager_app_has_pending_job_type	(GsJobManager	*self,
								 GsApp		*app,
								 GType		 pending_job_type);
void		 gs_job_manager_shutdown_async			(GsJobManager	*self,
								 GCancellable	*cancellable,
								 GAsyncReadyCallback callback,
								 gpointer	 user_data);
gboolean	 gs_job_manager_shutdown_finish			(GsJobManager	*self,
								 GAsyncResult	*result,
								 GError		**error);

/**
 * GsJobManagerJobCallback:
 * @job_manager: a #GsJobManager
 * @job: (not nullable): a #GsPluginJob
 * @user_data: user data
 *
 * A callback related to a specific job.
 *
 * This is used by gs_job_manager_add_watch().
 *
 * Since: 44
 */
typedef void (*GsJobManagerJobCallback)	(GsJobManager	*job_manager,
					 GsPluginJob	*job,
					 gpointer	 user_data);

guint		 gs_job_manager_add_watch			(GsJobManager	*self,
								 GsApp		*match_app,
								 GType		 match_job_type,
								 GsJobManagerJobCallback added_handler,
								 GsJobManagerJobCallback removed_handler,
								 gpointer	 user_data,
								 GDestroyNotify	 user_data_free_func);
void		 gs_job_manager_remove_watch			(GsJobManager	*self,
								 guint		 watch_id);

G_END_DECLS
