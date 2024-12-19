/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Matthew Leeds <mwleeds@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <gtk/gtk.h>

#include "lingmo-appstore-private.h"

G_BEGIN_DECLS

#define GS_TYPE_APP_VERSION_HISTORY_ROW (gs_app_version_history_row_get_type ())

G_DECLARE_FINAL_TYPE (GsAppVersionHistoryRow, gs_app_version_history_row, GS, APP_VERSION_HISTORY_ROW, GtkListBoxRow)

GtkWidget	*gs_app_version_history_row_new		(void);
void		 gs_app_version_history_row_set_info	(GsAppVersionHistoryRow *row,
							 const char		*version_number,
							 guint64		 version_date,
							 const char		*version_description);
gboolean	 gs_app_version_history_row_get_always_expanded
							(GsAppVersionHistoryRow *self);
void		 gs_app_version_history_row_set_always_expanded
							(GsAppVersionHistoryRow *self,
							 gboolean always_expanded);

G_END_DECLS
