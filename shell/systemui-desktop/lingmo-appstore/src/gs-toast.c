/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 204 Red Hat www.redhat.com
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <adwaita.h>
#include <glib/gi18n.h>

#include "gs-enums.h"

#include "gs-toast.h"

/* Being able to derive from AdwToast (it's a final type), a descendant would be here instead */
#define GS_TOAST_DATA_KEY "GsToastData"

typedef struct _GsToastData
{
	GsToastButton	 button;
	gchar		*details_message;
	gchar		*details_text;
} GsToastData;

static void
gs_toast_data_free (gpointer ptr)
{
	GsToastData *data = (GsToastData *) ptr;

	if (data) {
		g_clear_pointer (&data->details_message, g_free);
		g_clear_pointer (&data->details_text, g_free);
		g_free (data);
	}
}

static GsToastData *
gs_toast_get_data (AdwToast *toast)
{
	return g_object_get_data (G_OBJECT (toast), GS_TOAST_DATA_KEY);
}

static void
gs_toast_setup (AdwToast *self)
{
	GsToastButton button = gs_toast_get_button (self);
	const gchar *details_text = gs_toast_get_details_text (self);

	if (button != GS_TOAST_BUTTON_NONE &&
	    button != GS_TOAST_BUTTON_DETAILS_URI &&
	    details_text != NULL)
		g_warning ("GsToast has set both button and details text, the Details button is being used");

	if (details_text != NULL) {
		adw_toast_set_button_label (self, _("_Details"));
	} else if (button != GS_TOAST_BUTTON_NONE) {
		switch (button) {
		case GS_TOAST_BUTTON_SOURCES:
			adw_toast_set_button_label (self, _("Appstore Repositories"));
			break;
		case GS_TOAST_BUTTON_NO_SPACE:
			adw_toast_set_button_label (self, _("Examine Disk"));
			break;
		case GS_TOAST_BUTTON_NETWORK_SETTINGS:
			adw_toast_set_button_label (self, _("Network Settings"));
			break;
		case GS_TOAST_BUTTON_RESTART_REQUIRED:
			adw_toast_set_button_label (self, _("_Restart Now…"));
			break;
		case GS_TOAST_BUTTON_DETAILS_URI:
			adw_toast_set_button_label (self, _("Details"));
			break;
		default:
			g_warn_if_reached ();
			break;
		}
	}
}

/**
 * gs_toast_new:
 * @title: a toast title
 * @button: what button to show, if any
 * @details_message: (optional): details message to use, or %NULL
 * @details_text: (optional): details text to use, or %NULL
 *
 * Creates a new #AdwToast with set properties from the argument.
 * The @details_message is ignored when @details_text is %NULL.
 *
 * Non-%NULL @details_text can be used only with %GS_TOAST_BUTTON_NONE @button,
 * because this adds button "Details", which will show the @details_text
 * as error details and either @details_message or AdwToast:title as
 * the dialog message.
 *
 * All @button variants expect the creator to listen to AdwToast::button-clicked
 * signal and respond to it accordingly.
 *
 * Returns: (transfer full): a new #AdwToast
 *
 * Since: 46
 **/
AdwToast *
gs_toast_new (const gchar *title,
	      GsToastButton button,
	      const gchar *details_message,
	      const gchar *details_text)
{
	AdwToast *toast;
	GsToastData *data;

	toast = adw_toast_new (title);

	data = g_new0 (GsToastData, 1);
	data->button = button;
	data->details_message = g_strdup (details_message);
	data->details_text = g_strdup (details_text);

	g_object_set_data_full (G_OBJECT (toast), GS_TOAST_DATA_KEY, data, gs_toast_data_free);

	gs_toast_setup (toast);

	return toast;
}

/**
 * gs_toast_get_button:
 * @self: an #AdwToast, previously created with gs_toast_new()
 *
 * Returns a #GsToastButton constant the @self was created with.
 *
 * Returns: a button constant the @self was created with.
 *
 * Since: 46
 **/
GsToastButton
gs_toast_get_button (AdwToast *self)
{
	GsToastData *data;

	g_return_val_if_fail (ADW_IS_TOAST (self), GS_TOAST_BUTTON_NONE);

	data = gs_toast_get_data (self);
	if (data == NULL)
		return GS_TOAST_BUTTON_NONE;

	return data->button;
}

/**
 * gs_toast_get_details_message:
 * @self: an #AdwToast, previously created with gs_toast_new()
 *
 * Returns a details message the @self was created with. It can be %NULL.
 *
 * Returns: (nullable): a details message the @self was created with.
 *
 * Since: 46
 **/
const gchar *
gs_toast_get_details_message (AdwToast *self)
{
	GsToastData *data;

	g_return_val_if_fail (ADW_IS_TOAST (self), NULL);

	data = gs_toast_get_data (self);
	if (data == NULL)
		return NULL;

	return data->details_message;
}

/**
 * gs_toast_get_details_text:
 * @self: an #AdwToast, previously created with gs_toast_new()
 *
 * Returns a details text the @self was created with. It can be %NULL.
 *
 * Returns: (nullable): a details text the @self was created with.
 *
 * Since: 46
 **/
const gchar *
gs_toast_get_details_text (AdwToast *self)
{
	GsToastData *data;

	g_return_val_if_fail (ADW_IS_TOAST (self), NULL);

	data = gs_toast_get_data (self);
	if (data == NULL)
		return NULL;

	return data->details_text;
}
