/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright © 2024 Joshua Lee <lee.son.wai@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <glib/gi18n.h>

#include "gs-enums.h"
#include "gs-updates-paused-banner.h"

struct _GsUpdatesPausedBanner
{
	AdwBin		 	 	 parent_instance;

	GtkWidget			*banner;

	GsUpdatesPausedBannerFlags	 updates_paused_flags;
};

G_DEFINE_TYPE (GsUpdatesPausedBanner, gs_updates_paused_banner, ADW_TYPE_BIN)

typedef enum {
	PROP_UPDATES_PAUSED_FLAGS = 1,
} GsUpdatesPausedBannerProperty;

static GParamSpec *obj_props[PROP_UPDATES_PAUSED_FLAGS + 1] = { NULL, };

static void
details_button_clicked_cb (GsUpdatesPausedBanner *self)
{
	GtkRoot *root;
	GtkWidget *dialog;
	g_autoptr(GString) body = NULL;
	GtkWidget *label;

	g_assert (GS_IS_UPDATES_PAUSED_BANNER (self));

	root = gtk_widget_get_root (GTK_WIDGET (self));
	dialog = adw_message_dialog_new (GTK_WINDOW (root),
					 _("Appstore Updates Paused"),
					 NULL);

	body = g_string_new (_("Automatic software updates have been paused for the following reasons:\n"));
	if (self->updates_paused_flags & GS_UPDATES_PAUSED_BANNER_FLAGS_METERED)
		body = g_string_append (body, _("\n• The current network connection is metered"));
	if (self->updates_paused_flags & GS_UPDATES_PAUSED_BANNER_FLAGS_NO_LARGE_DOWNLOADS)
		body = g_string_append (body, _("\n• The current network connection prohibits large downloads"));
	if (self->updates_paused_flags & GS_UPDATES_PAUSED_BANNER_FLAGS_POWER_SAVER)
		body = g_string_append (body, _("\n• Power saver mode is active"));
	if (self->updates_paused_flags & GS_UPDATES_PAUSED_BANNER_FLAGS_GAME_MODE)
		body = g_string_append (body, _("\n• Game mode is active"));

	label = gtk_label_new (body->str);
	gtk_label_set_wrap (GTK_LABEL (label), TRUE);
	gtk_label_set_wrap_mode (GTK_LABEL (label), PANGO_WRAP_WORD_CHAR);
	gtk_label_set_max_width_chars (GTK_LABEL (label), 40);

	adw_message_dialog_set_extra_child (ADW_MESSAGE_DIALOG (dialog), label);
	adw_message_dialog_add_response (ADW_MESSAGE_DIALOG (dialog),
					 "close", _("_Close"));

	gtk_window_present (GTK_WINDOW (dialog));
}

static void
update_banner_title (GsUpdatesPausedBanner *self)
{
	char *title;

	g_assert (GS_IS_UPDATES_PAUSED_BANNER (self));

	if ((self->updates_paused_flags & (self->updates_paused_flags - 1)) != 0) {
		adw_banner_set_button_label (ADW_BANNER (self->banner), _("Details"));

		title = _("Appstore updates paused");
	} else {
		adw_banner_set_button_label (ADW_BANNER (self->banner), NULL);

		if (self->updates_paused_flags & GS_UPDATES_PAUSED_BANNER_FLAGS_METERED)
			title = _("Network connection is metered - software updates paused");
		else if (self->updates_paused_flags & GS_UPDATES_PAUSED_BANNER_FLAGS_NO_LARGE_DOWNLOADS)
			title = _("Network connection prohibits large downloads - software updates paused");
		else if (self->updates_paused_flags & GS_UPDATES_PAUSED_BANNER_FLAGS_POWER_SAVER)
			title = _("Power saver mode is active - software updates paused");
		else if (self->updates_paused_flags & GS_UPDATES_PAUSED_BANNER_FLAGS_GAME_MODE)
			title = _("Game mode is active - software updates paused");
		else
			g_assert_not_reached ();
	}

	adw_banner_set_title (ADW_BANNER (self->banner), title);
}

static void
gs_updates_paused_banner_dispose (GObject *object)
{
	GsUpdatesPausedBanner *self = GS_UPDATES_PAUSED_BANNER (object);

	g_clear_object (&self->banner);

	G_OBJECT_CLASS (gs_updates_paused_banner_parent_class)->dispose (object);
}

static void
gs_updates_paused_banner_get_property (GObject    *object,
                                       guint       prop_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
	GsUpdatesPausedBanner *self = GS_UPDATES_PAUSED_BANNER (object);

	switch ((GsUpdatesPausedBannerProperty) prop_id) {
	case PROP_UPDATES_PAUSED_FLAGS:
		g_value_set_flags (value, self->updates_paused_flags);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_updates_paused_banner_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
	GsUpdatesPausedBanner *self = GS_UPDATES_PAUSED_BANNER (object);

	switch ((GsUpdatesPausedBannerProperty) prop_id) {
	case PROP_UPDATES_PAUSED_FLAGS:
		gs_updates_paused_banner_set_flags (self, g_value_get_flags (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_updates_paused_banner_init (GsUpdatesPausedBanner *self)
{
	self->banner = g_object_ref_sink (adw_banner_new (""));
	adw_bin_set_child (ADW_BIN (self), self->banner);

	g_signal_connect_swapped (self->banner, "button-clicked",
				  G_CALLBACK (details_button_clicked_cb),
				  self);
}

static void
gs_updates_paused_banner_class_init (GsUpdatesPausedBannerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gs_updates_paused_banner_get_property;
	object_class->set_property = gs_updates_paused_banner_set_property;
	object_class->dispose = gs_updates_paused_banner_dispose;

	/**
	 * GsUpdatesPausedBanner:updates-paused-flags:
	 *
	 * The flags specifying the reason(s) automatic updates are paused.
	 *
	 * Since: 46
	 */
	obj_props[PROP_UPDATES_PAUSED_FLAGS] =
		g_param_spec_flags ("updates-paused-flags", NULL, NULL,
				    GS_TYPE_UPDATES_PAUSED_BANNER_FLAGS, GS_UPDATES_PAUSED_BANNER_FLAGS_NONE,
				    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);
}

void
gs_updates_paused_banner_set_flags (GsUpdatesPausedBanner      *self,
                                    GsUpdatesPausedBannerFlags  flags)
{
	g_return_if_fail (GS_IS_UPDATES_PAUSED_BANNER (self));

	if (self->updates_paused_flags == flags)
		return;

	self->updates_paused_flags = flags;

	if (self->updates_paused_flags != GS_UPDATES_PAUSED_BANNER_FLAGS_NONE) {
		update_banner_title (self);
		adw_banner_set_revealed (ADW_BANNER (self->banner), TRUE);
	} else {
		adw_banner_set_revealed (ADW_BANNER (self->banner), FALSE);
	}
}
