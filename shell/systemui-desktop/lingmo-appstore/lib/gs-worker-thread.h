/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Endless OS Foundation LLC
 *
 * Author: Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define GS_TYPE_WORKER_THREAD (gs_worker_thread_get_type ())

G_DECLARE_FINAL_TYPE (GsWorkerThread, gs_worker_thread, GS, WORKER_THREAD, GObject)

GsWorkerThread	*gs_worker_thread_new			(const gchar *name);

void		 gs_worker_thread_queue			(GsWorkerThread  *self,
							 gint             priority,
							 GTaskThreadFunc  work_func,
							 GTask           *task);

gboolean	 gs_worker_thread_is_in_worker_context	(GsWorkerThread *self);

void		 gs_worker_thread_shutdown_async	(GsWorkerThread      *self,
							 GCancellable        *cancellable,
							 GAsyncReadyCallback  callback,
							 gpointer             user_data);

gboolean	 gs_worker_thread_shutdown_finish	(GsWorkerThread  *self,
							 GAsyncResult    *result,
							 GError         **error);

G_END_DECLS
