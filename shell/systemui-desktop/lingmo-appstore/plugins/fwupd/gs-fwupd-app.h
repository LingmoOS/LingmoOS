/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2017 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <lingmo-appstore.h>
#include <fwupd.h>

G_BEGIN_DECLS

const gchar		*gs_fwupd_app_get_device_id		(GsApp		*app);
const gchar		*gs_fwupd_app_get_update_uri		(GsApp		*app);
gboolean		 gs_fwupd_app_get_is_locked		(GsApp		*app);

void			 gs_fwupd_app_set_device_id		(GsApp		*app,
								 const gchar	*device_id);
void			 gs_fwupd_app_set_update_uri		(GsApp		*app,
								 const gchar	*update_uri);
void			 gs_fwupd_app_set_is_locked		(GsApp		*app,
								 gboolean	 is_locked);
void			 gs_fwupd_app_set_from_device		(GsApp		*app,
								 FwupdClient	*client,
								 FwupdDevice	*dev);
void			 gs_fwupd_app_set_from_release		(GsApp		*app,
								 FwupdRelease	*rel);

G_END_DECLS
