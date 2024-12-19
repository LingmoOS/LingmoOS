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

#define GS_TYPE_SEARCH_PAGE (gs_search_page_get_type ())

G_DECLARE_FINAL_TYPE (GsSearchPage, gs_search_page, GS, SEARCH_PAGE, GsPage)

GsSearchPage	*gs_search_page_new			(void);
void		 gs_search_page_set_appid_to_show	(GsSearchPage		*self,
							 const gchar		*appid);
const gchar	*gs_search_page_get_text		(GsSearchPage		*self);
void		 gs_search_page_set_text		(GsSearchPage		*self,
							 const gchar		*value);

G_END_DECLS
