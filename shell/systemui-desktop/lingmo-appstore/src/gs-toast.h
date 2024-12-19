/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 204 Red Hat www.redhat.com
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

typedef enum {
	GS_TOAST_BUTTON_NONE,
	GS_TOAST_BUTTON_SOURCES,
	GS_TOAST_BUTTON_NO_SPACE,
	GS_TOAST_BUTTON_NETWORK_SETTINGS,
	GS_TOAST_BUTTON_RESTART_REQUIRED,
	GS_TOAST_BUTTON_DETAILS_URI,
	GS_TOAST_BUTTON_LAST
} GsToastButton;

/* Wrapper functions around AdwToast, because AdwToast is a final type, thus it cannot be derived from */

AdwToast	*gs_toast_new			(const gchar	*title,
						 GsToastButton	 button,
						 const gchar	*details_message,
						 const gchar	*details_text);
GsToastButton	 gs_toast_get_button		(AdwToast	*self);
const gchar	*gs_toast_get_details_message	(AdwToast	*self);
const gchar	*gs_toast_get_details_text	(AdwToast	*self);

G_END_DECLS
