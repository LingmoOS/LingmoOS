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

#define GS_TYPE_DETAILS_PAGE (gs_details_page_get_type ())

G_DECLARE_FINAL_TYPE (GsDetailsPage, gs_details_page, GS, DETAILS_PAGE, GsPage)

GsDetailsPage	*gs_details_page_new		(void);
void		 gs_details_page_set_app	(GsDetailsPage		*self,
						 GsApp			*app);
void		 gs_details_page_set_local_file(GsDetailsPage		*self,
						 GFile			*file);
void		 gs_details_page_set_url	(GsDetailsPage		*self,
						 const gchar		*url);
GsApp		*gs_details_page_get_app	(GsDetailsPage		*self);

GsOdrsProvider	*gs_details_page_get_odrs_provider	(GsDetailsPage	*self);
void		 gs_details_page_set_odrs_provider	(GsDetailsPage	*self,
							 GsOdrsProvider	*odrs_provider);

gboolean	 gs_details_page_get_is_narrow	(GsDetailsPage	*self);
void		 gs_details_page_set_is_narrow	(GsDetailsPage	*self,
						 gboolean	 is_narrow);
void		 gs_details_page_set_metainfo	(GsDetailsPage *self,
						 GFile *file);
gdouble		 gs_details_page_get_vscroll_position
						(GsDetailsPage *self);
void		 gs_details_page_set_vscroll_position
						(GsDetailsPage *self,
						 gdouble value);

G_END_DECLS
