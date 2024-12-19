/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2020 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <adwaita.h>

#include "lingmo-appstore-private.h"

G_BEGIN_DECLS

typedef void (*GsBasicAuthCallback) (const gchar *user, const gchar *password, gpointer callback_data);

#define GS_TYPE_BASIC_AUTH_DIALOG (gs_basic_auth_dialog_get_type ())

G_DECLARE_FINAL_TYPE (GsBasicAuthDialog, gs_basic_auth_dialog, GS, BASIC_AUTH_DIALOG, AdwWindow)

GtkWidget	*gs_basic_auth_dialog_new		(GtkWindow		*parent,
							 const gchar		*remote,
							 const gchar		*realm,
							 GsBasicAuthCallback	 callback,
							 gpointer		 callback_data);

G_END_DECLS
