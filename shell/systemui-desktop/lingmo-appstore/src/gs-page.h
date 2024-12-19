/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2015-2016 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "gs-shell.h"

G_BEGIN_DECLS

#define GS_TYPE_PAGE (gs_page_get_type ())

G_DECLARE_DERIVABLE_TYPE (GsPage, gs_page, GS, PAGE, GtkWidget)

struct _GsPageClass
{
	GtkWidgetClass	 parent_class;

	void		(*app_installed)	(GsPage		 *page,
						 GsApp		 *app);
	void		(*app_removed)		(GsPage		 *page,
						 GsApp		 *app);
	void		(*switch_to)		(GsPage		 *page);
	void		(*switch_from)		(GsPage		 *page);
	void		(*reload)		(GsPage		 *page);
	gboolean	(*setup)		(GsPage		 *page,
						 GsShell	*shell,
						 GsPluginLoader	*plugin_loader,
						 GCancellable	*cancellable,
						 GError		**error);
};

GsPage		*gs_page_new				(void);
GsShell		*gs_page_get_shell			(GsPage		*page);
GtkWidget	*gs_page_get_header_start_widget	(GsPage		*page);
void		 gs_page_set_header_start_widget	(GsPage		*page,
							 GtkWidget	*widget);
GtkWidget	*gs_page_get_header_end_widget		(GsPage		*page);
void		 gs_page_set_header_end_widget		(GsPage		*page,
							 GtkWidget	*widget);
void		 gs_page_install_app			(GsPage			*page,
							 GsApp			*app,
							 GsShellInteraction	interaction,
							 GCancellable		*cancellable);
void		 gs_page_remove_app			(GsPage		*page,
							 GsApp		*app,
							 GCancellable	*cancellable);
void		 gs_page_update_app			(GsPage		*page,
							 GsApp		*app,
							 GCancellable	*cancellable);
void		 gs_page_launch_app			(GsPage		*page,
							 GsApp		*app,
							 GCancellable	*cancellable);
void		 gs_page_switch_to			(GsPage		*page);
void		 gs_page_switch_from			(GsPage		*page);
void		 gs_page_scroll_up			(GsPage		*page);
void		 gs_page_reload				(GsPage		*page);
gboolean	 gs_page_setup				(GsPage		*page,
							 GsShell	*shell,
							 GsPluginLoader	*plugin_loader,
							 GCancellable	*cancellable,
							 GError		**error);
gboolean	 gs_page_is_active			(GsPage		*page);
gboolean	 gs_page_is_active_and_focused		(GsPage		*self);

const gchar	*gs_page_get_title			(GsPage		*page);
guint		 gs_page_get_counter			(GsPage		*page);
GtkAdjustment	*gs_page_get_vadjustment		(GsPage		*page);
GsAppQueryLicenseType gs_page_get_query_license_type	(GsPage		*self);
GsAppQueryDeveloperVerifiedType
		 gs_page_get_query_developer_verified_type
							(GsPage		*self);

G_END_DECLS
