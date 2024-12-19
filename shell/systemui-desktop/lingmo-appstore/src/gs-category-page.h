/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2015-2017 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "gs-page.h"

G_BEGIN_DECLS

#define GS_TYPE_CATEGORY_PAGE (gs_category_page_get_type ())

G_DECLARE_FINAL_TYPE (GsCategoryPage, gs_category_page, GS, CATEGORY_PAGE, GsPage)

GsCategoryPage	*gs_category_page_new		(void);
void		 gs_category_page_set_category	(GsCategoryPage		*self,
						 GsCategory		*category);
GsCategory	*gs_category_page_get_category	(GsCategoryPage		*self);

G_END_DECLS
