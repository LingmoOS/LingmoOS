/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2016 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "gs-app.h"
#include "gs-plugin-loader.h"

G_BEGIN_DECLS

void	 gs_test_init				(gint		*pargc,
						 gchar        ***pargv);
void	 gs_test_flush_main_context		(void);
gchar	*gs_test_get_filename			(const gchar	*testdatadir,
						 const gchar	*filename);
void	 gs_test_expose_icon_theme_paths	(void);

void	 gs_test_reinitialise_plugin_loader	(GsPluginLoader		*plugin_loader,
						 const gchar * const	*allowlist,
						 const gchar * const	*blocklist);

G_END_DECLS
