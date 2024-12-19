/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2023 Endless OS Foundation LLC
 *
 * Author: Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib.h>
#include <glib-object.h>

#include "gs-app-list.h"

G_BEGIN_DECLS

void		 gs_rewrite_resources_async		(GsAppList		 *list,
							 GCancellable		 *cancellable,
							 GAsyncReadyCallback	  callback,
							 gpointer		  user_data);
gboolean	 gs_rewrite_resources_finish		(GAsyncResult		 *result,
							 GError			**error);

G_END_DECLS
