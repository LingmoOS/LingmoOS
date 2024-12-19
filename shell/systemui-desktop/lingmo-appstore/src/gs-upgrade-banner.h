/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <adwaita.h>

#include "lingmo-appstore-private.h"

G_BEGIN_DECLS

#define GS_TYPE_UPGRADE_BANNER (gs_upgrade_banner_get_type ())

G_DECLARE_DERIVABLE_TYPE (GsUpgradeBanner, gs_upgrade_banner, GS, UPGRADE_BANNER, AdwBin)

struct _GsUpgradeBannerClass
{
	AdwBinClass	 parent_class;

	void		(*download_clicked)	(GsUpgradeBanner	*self);
	void		(*install_clicked)	(GsUpgradeBanner	*self);
	void		(*cancel_clicked)	(GsUpgradeBanner	*self);
};

GtkWidget	*gs_upgrade_banner_new			(void);
void		 gs_upgrade_banner_set_app		(GsUpgradeBanner	*self,
							 GsApp			*app);
GsApp		*gs_upgrade_banner_get_app		(GsUpgradeBanner	*self);

G_END_DECLS
