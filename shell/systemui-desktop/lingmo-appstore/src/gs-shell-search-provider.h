/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * gs-shell-search-provider.h - Implementation of a LINGMO Shell
 *   search provider
 *
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "lingmo-appstore-private.h"

#define GS_TYPE_SHELL_SEARCH_PROVIDER gs_shell_search_provider_get_type()

G_DECLARE_FINAL_TYPE (GsShellSearchProvider, gs_shell_search_provider, GS, SHELL_SEARCH_PROVIDER, GObject)

gboolean		 gs_shell_search_provider_register	(GsShellSearchProvider	 *self,
								 GDBusConnection	 *connection,
								 GError			**error);
void			 gs_shell_search_provider_unregister	(GsShellSearchProvider	 *self);
GsShellSearchProvider	*gs_shell_search_provider_new		(void);
void			 gs_shell_search_provider_setup		(GsShellSearchProvider	 *provider,
								 GsPluginLoader		 *loader);
