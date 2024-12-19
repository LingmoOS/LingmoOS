/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <adwaita.h>

#include "lingmo-appstore-private.h"
#include "gs-debug.h"

#define GS_APPLICATION_TYPE (gs_application_get_type ())

G_DECLARE_FINAL_TYPE (GsApplication, gs_application, GS, APPLICATION, AdwApplication)

GsApplication	*gs_application_new			(GsDebug *debug);
gboolean	 gs_application_has_active_window	(GsApplication *application);
void		 gs_application_emit_install_resources_done
							(GsApplication *application,
							 const gchar *ident,
							 const GError *op_error);
void		 gs_application_send_notification	(GsApplication *self,
							 const gchar *notification_id,
							 GNotification *notification,
							 guint timeout_minutes);
void		 gs_application_withdraw_notification	(GsApplication *self,
							 const gchar *notification_id);
