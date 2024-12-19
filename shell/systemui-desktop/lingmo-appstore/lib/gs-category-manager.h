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

#include "gs-category.h"

G_BEGIN_DECLS

#define GS_TYPE_CATEGORY_MANAGER (gs_category_manager_get_type ())

G_DECLARE_FINAL_TYPE (GsCategoryManager, gs_category_manager, GS, CATEGORY_MANAGER, GObject)

GsCategoryManager	*gs_category_manager_new	(void);

GsCategory		*gs_category_manager_lookup	(GsCategoryManager	*self,
							 const gchar		*id);

GsCategory * const	*gs_category_manager_get_categories (GsCategoryManager	*self,
							      gsize		*out_n_categories);

G_END_DECLS
