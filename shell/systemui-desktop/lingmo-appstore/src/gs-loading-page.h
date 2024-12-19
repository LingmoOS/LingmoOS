/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2017 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "gs-page.h"

G_BEGIN_DECLS

#define GS_TYPE_LOADING_PAGE (gs_loading_page_get_type ())

G_DECLARE_DERIVABLE_TYPE (GsLoadingPage, gs_loading_page, GS, LOADING_PAGE, GsPage)

struct _GsLoadingPageClass
{
	GsPageClass		 parent_class;

	void	(*refreshed)	(GsLoadingPage	*self);
};

GsLoadingPage	*gs_loading_page_new		(void);

G_END_DECLS
