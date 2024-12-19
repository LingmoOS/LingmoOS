/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2016 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "gs-category.h"

G_BEGIN_DECLS

void		 gs_category_sort_children	(GsCategory	*category);
void		 gs_category_set_size		(GsCategory	*category,
						 guint		 size);
gchar		*gs_category_to_string		(GsCategory	*category);

G_END_DECLS
