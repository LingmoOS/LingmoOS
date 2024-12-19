/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2017 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

G_BEGIN_DECLS

#include <lingmo-appstore.h>

void		 gs_flatpak_error_convert		(GError		**perror);
GsApp		*gs_flatpak_app_new_from_remote		(GsPlugin	*plugin,
							 FlatpakRemote	*xremote,
							 gboolean	 is_user);
GsApp		*gs_flatpak_app_new_from_repo_file	(GFile		*file,
							 GCancellable	*cancellable,
							 GError		**error);
void		 gs_flatpak_app_set_packaging_info	(GsApp		*app);

G_END_DECLS
