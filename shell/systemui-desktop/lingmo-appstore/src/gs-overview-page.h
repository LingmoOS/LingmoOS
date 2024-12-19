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

#define GS_TYPE_OVERVIEW_PAGE (gs_overview_page_get_type ())

G_DECLARE_FINAL_TYPE (GsOverviewPage, gs_overview_page, GS, OVERVIEW_PAGE, GsPage)

GsOverviewPage	*gs_overview_page_new		(void);
void		 gs_overview_page_override_featured
						(GsOverviewPage	*self,
						 GsApp		*app);

G_END_DECLS
