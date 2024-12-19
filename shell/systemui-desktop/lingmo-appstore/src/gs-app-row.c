/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2012-2016 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
 * Copyright (C) 2014-2018 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <glib/gi18n.h>

#include "gs-app-row.h"
#include "gs-star-widget.h"
#include "gs-progress-button.h"
#include "gs-common.h"

typedef struct
{
	GsApp		*app;
	GtkWidget	*image;
	GtkWidget	*name_box;
	GtkWidget	*name_label;
	GtkWidget	*version_box;
	GtkWidget	*version_current_label;
	GtkWidget	*version_arrow_label;
	GtkWidget	*version_update_label;
	GtkWidget	*system_updates_label; /* Only for "System Updates" app */
	GtkWidget	*update_critical_image;
	GtkWidget	*star;
	GtkWidget	*description_label;
	GtkWidget	*button_box;
	GtkWidget	*button_revealer;
	GtkWidget	*button;
	GtkWidget	*spinner;
	GtkWidget	*label;
	GtkWidget	*box_tag;
	GtkWidget	*label_warning;
	GtkWidget	*label_origin;
	GtkWidget	*label_installed_box;
	GtkWidget	*label_installed;
	GtkWidget	*label_app_size;
	gboolean	 colorful;
	gboolean	 show_buttons;
	gboolean	 show_rating;
	gboolean	 show_description;
	gboolean	 show_source;
	gboolean	 show_update;
	gboolean	 show_installed_size;
	gboolean	 show_installed;
	guint		 pending_refresh_id;
	guint		 unreveal_in_idle_id;
	gboolean	 is_narrow;
} GsAppRowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GsAppRow, gs_app_row, GTK_TYPE_LIST_BOX_ROW)

enum {
	SIGNAL_BUTTON_CLICKED,
	SIGNAL_UNREVEALED,
	SIGNAL_LAST
};

static guint signals [SIGNAL_LAST] = { 0 };

typedef enum {
	PROP_APP = 1,
	PROP_COLORFUL,
	PROP_SHOW_DESCRIPTION,
	PROP_SHOW_SOURCE,
	PROP_SHOW_BUTTONS,
	PROP_SHOW_RATING,
	PROP_SHOW_UPDATE,
	PROP_SHOW_INSTALLED_SIZE,
	PROP_SHOW_INSTALLED,
	PROP_IS_NARROW,
} GsAppRowProperty;

static GParamSpec *obj_props[PROP_IS_NARROW + 1] = { NULL, };

/*
 * gs_app_row_get_description:
 *
 * Return value: PangoMarkup or text
 */
static GString *
gs_app_row_get_description (GsAppRow *app_row,
			    gboolean *out_is_markup)
{
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);
	const gchar *tmp = NULL;

	*out_is_markup = FALSE;

	/* convert the markdown update description into PangoMarkup */
	if (priv->show_update) {
		tmp = gs_app_get_update_details_markup (priv->app);
		if (tmp != NULL && tmp[0] != '\0') {
			*out_is_markup = TRUE;
			return g_string_new (tmp);
		}
	}

	/* if missing summary is set, return it without escaping in order to
	 * correctly show hyperlinks */
	if (gs_app_get_state (priv->app) == GS_APP_STATE_UNAVAILABLE) {
		tmp = gs_app_get_summary_missing (priv->app);
		if (tmp != NULL && tmp[0] != '\0')
			return g_string_new (tmp);
	}

	/* try all these things in order */
	if (tmp == NULL || (tmp != NULL && tmp[0] == '\0'))
		tmp = gs_app_get_summary (priv->app);
	if (tmp == NULL || (tmp != NULL && tmp[0] == '\0'))
		tmp = gs_app_get_description (priv->app);
	if (tmp == NULL || (tmp != NULL && tmp[0] == '\0'))
		tmp = gs_app_get_name (priv->app);
	if (tmp == NULL)
		return NULL;
	return g_string_new (tmp);
}

static void
gs_app_row_update_button_reveal (GsAppRow *app_row)
{
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);
	gboolean sensitive = gtk_widget_get_sensitive (priv->button);

	gtk_widget_set_visible (priv->button_revealer, sensitive || !priv->is_narrow);
}

static void
gs_app_row_refresh_button (GsAppRow *app_row, gboolean missing_search_result)
{
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);

	/* disabled */
	if (!priv->show_buttons) {
		gs_app_row_update_button_reveal (app_row);
		gtk_widget_set_visible (priv->button, FALSE);
		return;
	}

	gtk_widget_set_sensitive (priv->button, TRUE);

	/* label */
	switch (gs_app_get_state (priv->app)) {
	case GS_APP_STATE_UNAVAILABLE:
		gtk_widget_set_visible (priv->button, TRUE);
		if (missing_search_result) {
			/* TRANSLATORS: this is a button next to the search results that
			 * allows the app to be easily installed */
			gs_progress_button_set_label (GS_PROGRESS_BUTTON (priv->button), _("Visit Website"));
			gs_progress_button_set_icon_name (GS_PROGRESS_BUTTON (priv->button), NULL);
		} else {
			/* TRANSLATORS: this is a button next to the search results that
			 * allows the app to be easily installed.
			 * The ellipsis indicates that further steps are required */
			gs_progress_button_set_label (GS_PROGRESS_BUTTON (priv->button), _("Install…"));
			gs_progress_button_set_icon_name (GS_PROGRESS_BUTTON (priv->button), NULL);
		}
		break;
	case GS_APP_STATE_QUEUED_FOR_INSTALL:
		gtk_widget_set_visible (priv->button, TRUE);
		/* TRANSLATORS: this is a button next to the search results that
		 * allows to cancel a queued install of the app */
		gs_progress_button_set_label (GS_PROGRESS_BUTTON (priv->button), _("Cancel"));
		gs_progress_button_set_icon_name (GS_PROGRESS_BUTTON (priv->button), "edit-delete-symbolic");
		break;
	case GS_APP_STATE_AVAILABLE:
	case GS_APP_STATE_AVAILABLE_LOCAL:
		gtk_widget_set_visible (priv->button, TRUE);
		/* TRANSLATORS: this is a button next to the search results that
		 * allows the app to be easily installed */
		gs_progress_button_set_label (GS_PROGRESS_BUTTON (priv->button), _("Install"));
		gs_progress_button_set_icon_name (GS_PROGRESS_BUTTON (priv->button), "list-add-symbolic");
		break;
	case GS_APP_STATE_UPDATABLE_LIVE:
		gtk_widget_set_visible (priv->button, TRUE);
		if (priv->show_update) {
			if (gs_app_has_quirk (priv->app, GS_APP_QUIRK_NEEDS_REBOOT) &&
			    !gs_app_is_downloaded (priv->app)) {
				/* TRANSLATORS: this is a button in the updates panel */
				gs_progress_button_set_label (GS_PROGRESS_BUTTON (priv->button), _("Download"));
			} else {
				/* TRANSLATORS: this is a button in the updates panel
				 * that allows the app to be easily updated live */
				gs_progress_button_set_label (GS_PROGRESS_BUTTON (priv->button), _("Update"));
			}
			gs_progress_button_set_icon_name (GS_PROGRESS_BUTTON (priv->button), "software-update-available-symbolic");
			gtk_widget_set_sensitive (priv->button, !gs_app_has_quirk (priv->app, GS_APP_QUIRK_NEEDS_USER_ACTION));
		} else {
			/* TRANSLATORS: this is a button next to the search results that
			 * allows the app to be easily removed */
			gs_progress_button_set_label (GS_PROGRESS_BUTTON (priv->button), _("Uninstall…"));
			gs_progress_button_set_icon_name (GS_PROGRESS_BUTTON (priv->button), "app-remove-symbolic");
		}
		break;
	case GS_APP_STATE_UPDATABLE:
	case GS_APP_STATE_INSTALLED:
		if (!gs_app_has_quirk (priv->app, GS_APP_QUIRK_COMPULSORY))
			gtk_widget_set_visible (priv->button, TRUE);
		/* TRANSLATORS: this is a button next to the search results that
		 * allows the app to be easily removed */
		gs_progress_button_set_label (GS_PROGRESS_BUTTON (priv->button), _("Uninstall…"));
		gs_progress_button_set_icon_name (GS_PROGRESS_BUTTON (priv->button), "app-remove-symbolic");
		break;
	case GS_APP_STATE_INSTALLING:
		gtk_widget_set_visible (priv->button, TRUE);
		/* TRANSLATORS: this is a button next to the search results that
		 * shows the status of an app being installed */
		gs_progress_button_set_label (GS_PROGRESS_BUTTON (priv->button), _("Installing"));
		gs_progress_button_set_icon_name (GS_PROGRESS_BUTTON (priv->button), NULL);
		break;
	case GS_APP_STATE_REMOVING:
		gtk_widget_set_visible (priv->button, TRUE);
		/* TRANSLATORS: this is a button next to the search results that
		 * shows the status of an app being erased */
		gs_progress_button_set_label (GS_PROGRESS_BUTTON (priv->button), _("Uninstalling"));
		gs_progress_button_set_icon_name (GS_PROGRESS_BUTTON (priv->button), NULL);
		break;
	case GS_APP_STATE_DOWNLOADING:
		gtk_widget_set_visible (priv->button, TRUE);
		/* TRANSLATORS: this is a button next to the search results that
		 * shows the status of an app being downloaded */
		gs_progress_button_set_label (GS_PROGRESS_BUTTON (priv->button), _("Downloading"));
		gs_progress_button_set_icon_name (GS_PROGRESS_BUTTON (priv->button), NULL);
		break;
	default:
		break;
	}

	/* visible */
	switch (gs_app_get_state (priv->app)) {
	case GS_APP_STATE_UNAVAILABLE:
	case GS_APP_STATE_QUEUED_FOR_INSTALL:
	case GS_APP_STATE_AVAILABLE:
	case GS_APP_STATE_AVAILABLE_LOCAL:
	case GS_APP_STATE_UPDATABLE_LIVE:
	case GS_APP_STATE_INSTALLING:
	case GS_APP_STATE_REMOVING:
	case GS_APP_STATE_DOWNLOADING:
		gtk_widget_set_visible (priv->button, TRUE);
		break;
	case GS_APP_STATE_UPDATABLE:
	case GS_APP_STATE_INSTALLED:
		gtk_widget_set_visible (priv->button,
					!gs_app_has_quirk (priv->app,
							   GS_APP_QUIRK_COMPULSORY));
		break;
	default:
		gtk_widget_set_visible (priv->button, FALSE);
		break;
	}

	/* colorful */
	if (!priv->colorful) {
		gtk_widget_remove_css_class (priv->button, "destructive-action");
	} else {
		switch (gs_app_get_state (priv->app)) {
		case GS_APP_STATE_UPDATABLE:
		case GS_APP_STATE_INSTALLED:
			gtk_widget_add_css_class (priv->button, "destructive-action");
			break;
		case GS_APP_STATE_UPDATABLE_LIVE:
			if (priv->show_update)
				gtk_widget_remove_css_class (priv->button, "destructive-action");
			else
				gtk_widget_add_css_class (priv->button, "destructive-action");
			break;
		default:
			gtk_widget_remove_css_class (priv->button, "destructive-action");
			break;
		}
	}

	/* always insensitive when in selection mode */
	switch (gs_app_get_state (priv->app)) {
	case GS_APP_STATE_INSTALLING:
	case GS_APP_STATE_REMOVING:
	case GS_APP_STATE_DOWNLOADING:
		gtk_widget_set_sensitive (priv->button, FALSE);
		break;
	default:
		gtk_widget_set_sensitive (priv->button, TRUE);
		break;
	}

	gs_app_row_update_button_reveal (app_row);
}

static void
append_to_name_if_meaningful (GPtrArray *name_parts, GtkLabel *label)
{
	const gchar *text = gtk_label_get_text (label);
	if (gtk_widget_is_visible (GTK_WIDGET (label)) && text != NULL && text[0] != '\0') {
		g_ptr_array_add (name_parts, (gpointer) text);
	}
}

static void
gs_app_row_update_accessible_name (GsAppRow *app_row)
{
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);
	/* typically not more than 3-4 of these widgets are actually visible, hence 5 below */
	g_autoptr(GPtrArray) parts = g_ptr_array_new_null_terminated (5, NULL, TRUE);
	g_autofree char *accessible_name = NULL;
	gboolean is_rtl = gtk_widget_get_direction (GTK_WIDGET (app_row)) == GTK_TEXT_DIR_RTL;

	/* As this is a complex widget, the screen reader doesn’t read it all
	 * out correctly by default, so we provide an override label. The label
	 * contains the textual versions of most of the widgets in the row, in
	 * the order they appear visually. This order differs in RTL
	 * environments, where each sub-row of the app row is reversed. In
	 * practice, that means only the name/critical and the version. */

	g_ptr_array_add (parts, (gpointer) gtk_label_get_text (GTK_LABEL (priv->name_label)));

	if (gtk_widget_get_visible (priv->update_critical_image))
		g_ptr_array_insert (parts, is_rtl ? 0 : -1, _("Critical update"));

	append_to_name_if_meaningful (parts, GTK_LABEL (priv->description_label));

	if (!is_rtl) {
		append_to_name_if_meaningful (parts, GTK_LABEL (priv->version_current_label));
		append_to_name_if_meaningful (parts, GTK_LABEL (priv->version_arrow_label));
		append_to_name_if_meaningful (parts, GTK_LABEL (priv->version_update_label));
	} else {
		append_to_name_if_meaningful (parts, GTK_LABEL (priv->version_update_label));
		append_to_name_if_meaningful (parts, GTK_LABEL (priv->version_arrow_label));
		append_to_name_if_meaningful (parts, GTK_LABEL (priv->version_current_label));
	}

	/* each of these are visually on a separate row, so don’t need RTL treatment */
	append_to_name_if_meaningful (parts, GTK_LABEL (priv->label_installed));
	append_to_name_if_meaningful (parts, GTK_LABEL (priv->label_app_size));
	append_to_name_if_meaningful (parts, GTK_LABEL (priv->label_origin));
	append_to_name_if_meaningful (parts, GTK_LABEL (priv->system_updates_label));
	append_to_name_if_meaningful (parts, GTK_LABEL (priv->label_warning));
	append_to_name_if_meaningful (parts, GTK_LABEL (priv->label));

	accessible_name = g_strjoinv (" ", (gchar **) parts->pdata);
	gtk_accessible_update_property (GTK_ACCESSIBLE (app_row),
					GTK_ACCESSIBLE_PROPERTY_LABEL, accessible_name,
					-1);
}

static void
gs_app_row_actually_refresh (GsAppRow *app_row)
{
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);
	GString *str = NULL;
	const gchar *tmp;
	gboolean missing_search_result;
	gboolean is_markup = FALSE;
	guint64 size_installed_bytes = 0;
	GsSizeType size_installed_type = GS_SIZE_TYPE_UNKNOWN;
	g_autoptr(GIcon) icon = NULL;

	if (priv->app == NULL)
		return;

	/* is this a missing search result from the extras page? */
	missing_search_result = (gs_app_get_state (priv->app) == GS_APP_STATE_UNAVAILABLE &&
	                         gs_app_get_url_missing (priv->app) != NULL);

	/* do a fill bar for the current progress */
	switch (gs_app_get_state (priv->app)) {
	case GS_APP_STATE_INSTALLING:
	case GS_APP_STATE_DOWNLOADING:
		gs_progress_button_set_progress (GS_PROGRESS_BUTTON (priv->button),
		                                 gs_app_get_progress (priv->app));
		gs_progress_button_set_show_progress (GS_PROGRESS_BUTTON (priv->button), TRUE);
		break;
	default:
		gs_progress_button_set_show_progress (GS_PROGRESS_BUTTON (priv->button), FALSE);
		break;
	}

	/* join the description lines */
	str = gs_app_row_get_description (app_row, &is_markup);
	if (str != NULL) {
		gs_utils_gstring_replace (str, "\n", " ");
		if (is_markup)
			gtk_label_set_markup (GTK_LABEL (priv->description_label), str->str);
		else
			gtk_label_set_label (GTK_LABEL (priv->description_label), str->str);
		g_string_free (str, TRUE);
	} else {
		gtk_label_set_text (GTK_LABEL (priv->description_label), NULL);
	}

	/* add warning */
	if (gs_app_has_quirk (priv->app, GS_APP_QUIRK_UNUSABLE_DURING_UPDATE)) {
		gtk_label_set_text (GTK_LABEL (priv->label_warning),
				    /* TRANSLATORS: during the update the device
				     * will restart into a special update-only mode */
				    _("Device cannot be used during update."));
		gtk_widget_set_visible (priv->label_warning, TRUE);
	}

	/* where did this app come from */
	if (priv->show_source) {
		tmp = gs_app_get_origin_hostname (priv->app);
		if (tmp != NULL) {
			g_autofree gchar *origin_tmp = NULL;
			/* TRANSLATORS: this refers to where the app came from */
			origin_tmp = g_strdup_printf (_("Source: %s"), tmp);
			gtk_label_set_label (GTK_LABEL (priv->label_origin), origin_tmp);
		}
		gtk_widget_set_visible (priv->label_origin, tmp != NULL);
	} else {
		gtk_widget_set_visible (priv->label_origin, FALSE);
	}

	/* installed tag */
	if (!priv->show_buttons) {
		switch (gs_app_get_state (priv->app)) {
		case GS_APP_STATE_UPDATABLE:
		case GS_APP_STATE_UPDATABLE_LIVE:
		case GS_APP_STATE_INSTALLED:
			gtk_widget_set_visible (priv->label_installed_box, priv->show_installed);
			break;
		default:
			gtk_widget_set_visible (priv->label_installed_box, FALSE);
			break;
		}
	} else {
		gtk_widget_set_visible (priv->label_installed_box, FALSE);
	}

	/* name */
	gtk_label_set_label (GTK_LABEL (priv->name_label),
	                     gs_app_get_name (priv->app));

	if (priv->show_update) {
		const gchar *version_current = NULL;
		const gchar *version_update = NULL;

		/* current version */
		tmp = gs_app_get_version_ui (priv->app);
		if (tmp != NULL && tmp[0] != '\0') {
			version_current = tmp;
			gtk_label_set_label (GTK_LABEL (priv->version_current_label),
			                     version_current);
			gtk_widget_set_visible (priv->version_current_label, TRUE);
		} else {
			gtk_widget_set_visible (priv->version_current_label, FALSE);
		}

		/* update version */
		tmp = gs_app_get_update_version_ui (priv->app);
		if (tmp != NULL && tmp[0] != '\0' &&
		    g_strcmp0 (tmp, version_current) != 0) {
			version_update = tmp;
			gtk_label_set_label (GTK_LABEL (priv->version_update_label),
			                     version_update);
			gtk_widget_set_visible (priv->version_update_label, TRUE);
		} else {
			gtk_widget_set_visible (priv->version_update_label, FALSE);
		}

		/* have both: show arrow */
		gtk_widget_set_visible (priv->version_arrow_label,
					(version_current != NULL &&
					 version_update != NULL &&
					 g_strcmp0 (version_current, version_update) != 0));

		/* ensure the arrow is the right way round for the text direction,
		 * as arrows are not bidi-mirrored automatically
		 * See section 2 of http://www.unicode.org/L2/L2017/17438-bidi-math-fdbk.html */
		switch (gtk_widget_get_direction (priv->version_box)) {
		case GTK_TEXT_DIR_RTL:
			gtk_label_set_label (GTK_LABEL (priv->version_arrow_label), "←");
			break;
		case GTK_TEXT_DIR_NONE:
		case GTK_TEXT_DIR_LTR:
		default:
			gtk_label_set_label (GTK_LABEL (priv->version_arrow_label), "→");
			break;
		}

		/* show the box if we have either of the versions */
		gtk_widget_set_visible (priv->version_box,
					(version_current != NULL || version_update != NULL));

		gtk_widget_set_visible (priv->star, FALSE);
	} else {
		gtk_widget_set_visible (priv->version_box, FALSE);
		if (missing_search_result || gs_app_get_rating (priv->app) <= 0 || !priv->show_rating) {
			gtk_widget_set_visible (priv->star, FALSE);
		} else {
			gtk_widget_set_visible (priv->star, TRUE);
			gtk_widget_set_sensitive (priv->star, FALSE);
			gs_star_widget_set_rating (GS_STAR_WIDGET (priv->star),
						   gs_app_get_rating (priv->app));
		}
	}

	if (priv->show_update && gs_app_get_special_kind (priv->app) == GS_APP_SPECIAL_KIND_OS_UPDATE) {
		gtk_label_set_label (GTK_LABEL (priv->system_updates_label), gs_app_get_summary (priv->app));
		gtk_widget_set_visible (priv->system_updates_label, TRUE);
	} else {
		gtk_widget_set_visible (priv->system_updates_label, FALSE);
	}

	gtk_widget_set_visible (priv->update_critical_image, priv->show_update && gs_app_get_update_urgency (priv->app) >= AS_URGENCY_KIND_CRITICAL);

	/* pixbuf */
	icon = gs_app_get_icon_for_size (priv->app,
					 gtk_image_get_pixel_size (GTK_IMAGE (priv->image)),
					 gtk_widget_get_scale_factor (priv->image),
					 "application-x-executable");
	gtk_image_set_from_gicon (GTK_IMAGE (priv->image), icon);

	if (missing_search_result)
		gtk_widget_add_css_class (priv->image, "dimmer-label");
	else
		gtk_widget_remove_css_class (priv->image, "dimmer-label");

	/* pending label */
	switch (gs_app_get_state (priv->app)) {
	case GS_APP_STATE_QUEUED_FOR_INSTALL:
		gtk_widget_set_visible (priv->label, TRUE);
		gtk_label_set_label (GTK_LABEL (priv->label), _("Pending"));
		break;
	case GS_APP_STATE_PENDING_INSTALL:
		gtk_widget_set_visible (priv->label, TRUE);
		gtk_label_set_label (GTK_LABEL (priv->label), _("Pending install"));
		break;
	case GS_APP_STATE_PENDING_REMOVE:
		gtk_widget_set_visible (priv->label, TRUE);
		gtk_label_set_label (GTK_LABEL (priv->label), _("Pending remove"));
		break;
	default:
		gtk_widget_set_visible (priv->label, FALSE);
		break;
	}

	/* spinner */
	switch (gs_app_get_state (priv->app)) {
	case GS_APP_STATE_REMOVING:
		gtk_spinner_start (GTK_SPINNER (priv->spinner));
		gtk_widget_set_visible (priv->spinner, TRUE);
		break;
	default:
		gtk_widget_set_visible (priv->spinner, FALSE);
		break;
	}

	/* button */
	gs_app_row_refresh_button (app_row, missing_search_result);

	/* hide buttons in the update list, unless the app is live updatable */
	switch (gs_app_get_state (priv->app)) {
	case GS_APP_STATE_UPDATABLE_LIVE:
		gtk_widget_set_visible (priv->button_box,
					!priv->show_update ||
					!gs_app_has_quirk (priv->app, GS_APP_QUIRK_NEEDS_USER_ACTION));
		break;
	case GS_APP_STATE_INSTALLING:
	case GS_APP_STATE_DOWNLOADING:
		gtk_widget_set_visible (priv->button_box, TRUE);
		break;
	default:
		gtk_widget_set_visible (priv->button_box, !priv->show_update);
		break;
	}

	/* show the right size */
	if (priv->show_installed_size) {
		size_installed_type = gs_app_get_size_installed (priv->app, &size_installed_bytes);
	}
	if (size_installed_type == GS_SIZE_TYPE_VALID && size_installed_bytes > 0) {
		g_autofree gchar *sizestr = NULL;
		sizestr = g_format_size (size_installed_bytes);
		gtk_label_set_label (GTK_LABEL (priv->label_app_size), sizestr);
		gtk_widget_set_visible (priv->label_app_size, TRUE);
	} else {
		gtk_widget_set_visible (priv->label_app_size, FALSE);
	}

	/* add warning */
	if (priv->show_update) {
		g_autoptr(GString) warning = g_string_new (NULL);
		const gchar *renamed_from;

		if (gs_app_has_quirk (priv->app, GS_APP_QUIRK_NEW_PERMISSIONS))
			g_string_append (warning, _("Requires additional permissions"));

		renamed_from = gs_app_get_renamed_from (priv->app);
		if (renamed_from && g_strcmp0 (renamed_from, gs_app_get_name (priv->app)) != 0) {
			if (warning->len > 0)
				g_string_append (warning, "\n");
			/* Translators: A message to indicate that an app has been renamed. The placeholder is the old human-readable name. */
			g_string_append_printf (warning, _("Renamed from %s"), renamed_from);
		}

		if (gs_app_has_quirk (priv->app, GS_APP_QUIRK_NEEDS_USER_ACTION)) {
			const gchar *problems;
			problems = gs_app_get_metadata_item (priv->app, "LingmoAppstore::problems");
			if (problems != NULL && *problems != '\0') {
				if (warning->len > 0)
					g_string_append_c (warning, '\n');
				g_string_append (warning, problems);
			}
		}

		if (warning->len > 0) {
			gtk_label_set_text (GTK_LABEL (priv->label_warning), warning->str);
			gtk_widget_set_tooltip_text (priv->label_warning, warning->str);
			gtk_widget_set_visible (priv->label_warning, TRUE);
		}
	} else if (priv->show_installed) {
		g_autofree gchar *warning_tmp = NULL;
		const gchar *problems, *eol_reason;
		problems = gs_app_get_metadata_item (priv->app, "LingmoAppstore::problems");
		if (problems == NULL || *problems == '\0') {
			/* Show runtime problems on the apps which use them, unless they have their own problems */
			GsApp *runtime = gs_app_get_runtime (priv->app);
			if (runtime != NULL)
				problems = gs_app_get_metadata_item (runtime, "LingmoAppstore::problems");
		}
		eol_reason = gs_app_get_metadata_item (priv->app, "LingmoAppstore::EolReason");
		if (eol_reason == NULL || *eol_reason == '\0') {
			/* Show runtime EOL on the apps which use them, unless they have their own EOL */
			GsApp *runtime = gs_app_get_runtime (priv->app);
			if (runtime != NULL)
				eol_reason = gs_app_get_metadata_item (runtime, "LingmoAppstore::EolReason");
		}
		if (eol_reason != NULL && *eol_reason != '\0') {
			/* Replace user-provided non-localized string with a localized text */
			eol_reason = _("Stopped Receiving Updates");
			if (problems != NULL && *problems != '\0') {
				warning_tmp = g_strconcat (problems, "\n", eol_reason, NULL);
				problems = warning_tmp;
			} else {
				problems = eol_reason;
			}
		}
		if (problems != NULL && *problems != '\0') {
			gtk_label_set_text (GTK_LABEL (priv->label_warning), problems);
			gtk_widget_set_tooltip_text (priv->label_warning, problems);
			gtk_widget_set_visible (priv->label_warning, TRUE);
		}
	}

	gtk_widget_set_visible (priv->box_tag,
				gtk_widget_get_visible (priv->label_origin) ||
				gtk_widget_get_visible (priv->label_installed_box) ||
				gtk_widget_get_visible (priv->label_warning));

	gtk_label_set_max_width_chars (GTK_LABEL (priv->name_label),
				       gtk_widget_get_visible (priv->description_label) ? 20 : -1);

	gs_app_row_update_accessible_name (app_row);
}

static void
finish_unreveal (GsAppRow *app_row)
{
	gtk_widget_set_visible (GTK_WIDGET (app_row), FALSE);

	g_signal_emit (app_row, signals[SIGNAL_UNREVEALED], 0);
}

static void
child_unrevealed (GObject *revealer, GParamSpec *pspec, gpointer user_data)
{
	GsAppRow *app_row = user_data;
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);

	/* return immediately if we are in destruction (this doesn't, however,
	 * catch the case where we are being removed from a container without
	 * having been destroyed first.)
	 */
	if (priv->app == NULL || !gtk_widget_get_mapped (GTK_WIDGET (app_row)))
		return;

	finish_unreveal (app_row);
}

static gboolean
child_unrevealed_unmapped_cb (gpointer user_data)
{
	GsAppRow *app_row = user_data;
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);

	priv->unreveal_in_idle_id = 0;

	finish_unreveal (app_row);

	return G_SOURCE_REMOVE;
}

/**
 * gs_app_row_unreveal:
 * @app_row: a #GsAppRow
 *
 * Hide the row with an animation. Once the animation is done
 * the GsAppRow:unrevealed signal is emitted. This handles
 * the case when the widget is not mapped as well, in which case
 * the GsAppRow:unrevealed signal is emitted from an idle
 * callback, to ensure async nature of the function call and
 * the signal emission.
 *
 * Calling the function multiple times has no effect.
 **/
void
gs_app_row_unreveal (GsAppRow *app_row)
{
	GtkWidget *child;
	GtkWidget *revealer;

	g_return_if_fail (GS_IS_APP_ROW (app_row));

	child = gtk_list_box_row_get_child (GTK_LIST_BOX_ROW (app_row));

	/* This means the row is already hiding */
	if (GTK_IS_REVEALER (child))
		return;

	gtk_widget_set_sensitive (child, FALSE);

	/* Revealer does not animate when the widget is not mapped */
	if (!gtk_widget_get_mapped (GTK_WIDGET (app_row))) {
		GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);
		if (priv->unreveal_in_idle_id == 0)
			priv->unreveal_in_idle_id = g_idle_add_full (G_PRIORITY_HIGH, child_unrevealed_unmapped_cb, app_row, NULL);
		return;
	}

	revealer = gtk_revealer_new ();
	gtk_revealer_set_reveal_child (GTK_REVEALER (revealer), TRUE);
	gtk_widget_set_visible (revealer, TRUE);

	g_object_ref (child);
	gtk_list_box_row_set_child (GTK_LIST_BOX_ROW (app_row), revealer);
	gtk_revealer_set_child (GTK_REVEALER (revealer), child);
	g_object_unref (child);

	g_signal_connect (revealer, "notify::child-revealed",
			  G_CALLBACK (child_unrevealed), app_row);
	gtk_revealer_set_reveal_child (GTK_REVEALER (revealer), FALSE);
}

GsApp *
gs_app_row_get_app (GsAppRow *app_row)
{
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);
	g_return_val_if_fail (GS_IS_APP_ROW (app_row), NULL);
	return priv->app;
}

static gboolean
gs_app_row_refresh_idle_cb (gpointer user_data)
{
	GsAppRow *app_row = GS_APP_ROW (user_data);
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);
	priv->pending_refresh_id = 0;
	gs_app_row_actually_refresh (app_row);
	return G_SOURCE_REMOVE;
}

/* Schedule an idle call to gs_app_row_actually_refresh() unless one’s already pending. */
static void
gs_app_row_schedule_refresh (GsAppRow *app_row)
{
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);

	if (priv->pending_refresh_id > 0)
		return;
	priv->pending_refresh_id = g_idle_add (gs_app_row_refresh_idle_cb, app_row);
}

static void
gs_app_row_notify_props_changed_cb (GsApp *app,
				    GParamSpec *pspec,
				    GsAppRow *app_row)
{
	gs_app_row_schedule_refresh (app_row);
}

static void
gs_app_row_set_app (GsAppRow *app_row, GsApp *app)
{
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);

	priv->app = g_object_ref (app);

	g_signal_connect_object (priv->app, "notify::state",
				 G_CALLBACK (gs_app_row_notify_props_changed_cb),
				 app_row, 0);
	g_signal_connect_object (priv->app, "notify::rating",
				 G_CALLBACK (gs_app_row_notify_props_changed_cb),
				 app_row, 0);
	g_signal_connect_object (priv->app, "notify::progress",
				 G_CALLBACK (gs_app_row_notify_props_changed_cb),
				 app_row, 0);
	g_signal_connect_object (priv->app, "notify::allow-cancel",
				 G_CALLBACK (gs_app_row_notify_props_changed_cb),
				 app_row, 0);

	gs_app_row_schedule_refresh (app_row);
	g_object_notify_by_pspec (G_OBJECT (app_row), obj_props[PROP_APP]);
}

static void
gs_app_row_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	GsAppRow *app_row = GS_APP_ROW (object);
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);

	switch ((GsAppRowProperty) prop_id) {
	case PROP_APP:
		g_value_set_object (value, priv->app);
		break;
	case PROP_COLORFUL:
		g_value_set_boolean (value, priv->colorful);
		break;
	case PROP_SHOW_DESCRIPTION:
		g_value_set_boolean (value, gs_app_row_get_show_description (app_row));
		break;
	case PROP_SHOW_SOURCE:
		g_value_set_boolean (value, priv->show_source);
		break;
	case PROP_SHOW_BUTTONS:
		g_value_set_boolean (value, priv->show_buttons);
		break;
	case PROP_SHOW_RATING:
		g_value_set_boolean (value, priv->show_rating);
		break;
	case PROP_SHOW_UPDATE:
		g_value_set_boolean (value, priv->show_update);
		break;
	case PROP_SHOW_INSTALLED_SIZE:
		g_value_set_boolean (value, priv->show_installed_size);
		break;
	case PROP_SHOW_INSTALLED:
		g_value_set_boolean (value, priv->show_installed);
		break;
	case PROP_IS_NARROW:
		g_value_set_boolean (value, gs_app_row_get_is_narrow (app_row));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_app_row_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	GsAppRow *app_row = GS_APP_ROW (object);

	switch ((GsAppRowProperty) prop_id) {
	case PROP_APP:
		gs_app_row_set_app (app_row, g_value_get_object (value));
		break;
	case PROP_COLORFUL:
		gs_app_row_set_colorful (app_row, g_value_get_boolean (value));
		break;
	case PROP_SHOW_DESCRIPTION:
		gs_app_row_set_show_description (app_row, g_value_get_boolean (value));
		break;
	case PROP_SHOW_SOURCE:
		gs_app_row_set_show_source (app_row, g_value_get_boolean (value));
		break;
	case PROP_SHOW_BUTTONS:
		gs_app_row_set_show_buttons (app_row, g_value_get_boolean (value));
		break;
	case PROP_SHOW_RATING:
		gs_app_row_set_show_rating (app_row, g_value_get_boolean (value));
		break;
	case PROP_SHOW_UPDATE:
		gs_app_row_set_show_update (app_row, g_value_get_boolean (value));
		break;
	case PROP_SHOW_INSTALLED_SIZE:
		gs_app_row_set_show_installed_size (app_row, g_value_get_boolean (value));
		break;
	case PROP_SHOW_INSTALLED:
		gs_app_row_set_show_installed (app_row, g_value_get_boolean (value));
		break;
	case PROP_IS_NARROW:
		gs_app_row_set_is_narrow (app_row, g_value_get_boolean (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_app_row_dispose (GObject *object)
{
	GsAppRow *app_row = GS_APP_ROW (object);
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);

	if (priv->app)
		g_signal_handlers_disconnect_by_func (priv->app, gs_app_row_notify_props_changed_cb, app_row);

	g_clear_object (&priv->app);
	g_clear_handle_id (&priv->pending_refresh_id, g_source_remove);
	g_clear_handle_id (&priv->unreveal_in_idle_id, g_source_remove);

	G_OBJECT_CLASS (gs_app_row_parent_class)->dispose (object);
}

static void
gs_app_row_class_init (GsAppRowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->get_property = gs_app_row_get_property;
	object_class->set_property = gs_app_row_set_property;
	object_class->dispose = gs_app_row_dispose;

	/**
	 * GsAppRow:app:
	 *
	 * The #GsApp to show in this row.
	 *
	 * Since: 3.38
	 */
	obj_props[PROP_APP] =
		g_param_spec_object ("app", NULL, NULL,
				     GS_TYPE_APP,
				     G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * GsAppRow:colorful:
	 *
	 * Whether the buttons can be colorized in the row.
	 *
	 * Since: 42.1
	 */
	obj_props[PROP_COLORFUL] =
		g_param_spec_boolean ("colorful", NULL, NULL,
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppRow:show-description:
	 *
	 * Show the description of the app in the row.
	 *
	 * Since: 41
	 */
	obj_props[PROP_SHOW_DESCRIPTION] =
		g_param_spec_boolean ("show-description", NULL, NULL,
				      TRUE,
				      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppRow:show-source:
	 *
	 * Show the source of the app in the row.
	 *
	 * Since: 3.38
	 */
	obj_props[PROP_SHOW_SOURCE] =
		g_param_spec_boolean ("show-source", NULL, NULL,
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppRow:show-buttons:
	 *
	 * Show buttons (such as Install, Cancel or Update) in the app row.
	 *
	 * Since: 3.38
	 */
	obj_props[PROP_SHOW_BUTTONS] =
		g_param_spec_boolean ("show-buttons", NULL, NULL,
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppRow:show-rating:
	 *
	 * Show app rating in the app row.
	 *
	 * Since: 42.1
	 */
	obj_props[PROP_SHOW_RATING] =
		g_param_spec_boolean ("show-rating", NULL, NULL,
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppRow:show-update:
	 *
	 * Show update (version) information in the app row.
	 *
	 * Since: 42.1
	 */
	obj_props[PROP_SHOW_UPDATE] =
		g_param_spec_boolean ("show-update", NULL, NULL,
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppRow:show-installed:
	 *
	 * Show an "Installed" check in the app row, when the app is installed.
	 *
	 * Since: 42.1
	 */
	obj_props[PROP_SHOW_INSTALLED] =
		g_param_spec_boolean ("show-installed", NULL, NULL,
				      TRUE,
				      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppRow:show-installed-size:
	 *
	 * Show the installed size of the app in the row.
	 *
	 * Since: 3.38
	 */
	obj_props[PROP_SHOW_INSTALLED_SIZE] =
		g_param_spec_boolean ("show-installed-size", NULL, NULL,
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppRow:is-narrow:
	 *
	 * Whether the row is in narrow mode.
	 *
	 * In narrow mode, the row will take up less horizontal space, doing so
	 * by e.g. using icons rather than labels in buttons. This is needed to
	 * keep the UI useable on small form-factors like smartphones.
	 *
	 * Since: 41
	 */
	obj_props[PROP_IS_NARROW] =
		g_param_spec_boolean ("is-narrow", NULL, NULL,
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);

	signals [SIGNAL_BUTTON_CLICKED] =
		g_signal_new ("button-clicked",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GsAppRowClass, button_clicked),
			      NULL, NULL, g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);

	signals [SIGNAL_UNREVEALED] =
		g_signal_new ("unrevealed",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GsAppRowClass, unrevealed),
			      NULL, NULL, g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-app-row.ui");

	gtk_widget_class_bind_template_child_private (widget_class, GsAppRow, image);
	gtk_widget_class_bind_template_child_private (widget_class, GsAppRow, name_box);
	gtk_widget_class_bind_template_child_private (widget_class, GsAppRow, name_label);
	gtk_widget_class_bind_template_child_private (widget_class, GsAppRow, version_box);
	gtk_widget_class_bind_template_child_private (widget_class, GsAppRow, version_current_label);
	gtk_widget_class_bind_template_child_private (widget_class, GsAppRow, version_arrow_label);
	gtk_widget_class_bind_template_child_private (widget_class, GsAppRow, version_update_label);
	gtk_widget_class_bind_template_child_private (widget_class, GsAppRow, system_updates_label);
	gtk_widget_class_bind_template_child_private (widget_class, GsAppRow, update_critical_image);
	gtk_widget_class_bind_template_child_private (widget_class, GsAppRow, star);
	gtk_widget_class_bind_template_child_private (widget_class, GsAppRow, description_label);
	gtk_widget_class_bind_template_child_private (widget_class, GsAppRow, button_box);
	gtk_widget_class_bind_template_child_private (widget_class, GsAppRow, button_revealer);
	gtk_widget_class_bind_template_child_private (widget_class, GsAppRow, button);
	gtk_widget_class_bind_template_child_private (widget_class, GsAppRow, spinner);
	gtk_widget_class_bind_template_child_private (widget_class, GsAppRow, label);
	gtk_widget_class_bind_template_child_private (widget_class, GsAppRow, box_tag);
	gtk_widget_class_bind_template_child_private (widget_class, GsAppRow, label_warning);
	gtk_widget_class_bind_template_child_private (widget_class, GsAppRow, label_origin);
	gtk_widget_class_bind_template_child_private (widget_class, GsAppRow, label_installed_box);
	gtk_widget_class_bind_template_child_private (widget_class, GsAppRow, label_installed);
	gtk_widget_class_bind_template_child_private (widget_class, GsAppRow, label_app_size);
}

static void
button_clicked (GtkWidget *widget, GsAppRow *app_row)
{
	g_signal_emit (app_row, signals[SIGNAL_BUTTON_CLICKED], 0);
}

static void
gs_app_row_init (GsAppRow *app_row)
{
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);

	priv->show_description = TRUE;
	priv->show_installed = TRUE;

	g_type_ensure (GS_TYPE_PROGRESS_BUTTON);
	g_type_ensure (GS_TYPE_STAR_WIDGET);

	gtk_widget_init_template (GTK_WIDGET (app_row));

	g_signal_connect (priv->button, "clicked",
			  G_CALLBACK (button_clicked), app_row);

	/* A fix for this is included in 4.6.4, apply workaround, if not running with new-enough gtk. */
	if (gtk_get_major_version () < 4 ||
	   (gtk_get_major_version () == 4 && gtk_get_minor_version () < 6) ||
	   (gtk_get_major_version () == 4 && gtk_get_minor_version () == 6 && gtk_get_micro_version () < 4)) {
		g_object_set (G_OBJECT (priv->name_label),
			      "wrap", FALSE,
			      "lines", 1,
			      NULL);
		g_object_set (G_OBJECT (priv->description_label),
			      "wrap", FALSE,
			      "lines", 1,
			      NULL);
		g_object_set (G_OBJECT (priv->label_warning),
			      "wrap", FALSE,
			      "lines", 1,
			      NULL);
		g_object_set (G_OBJECT (priv->system_updates_label),
			      "wrap", FALSE,
			      "lines", 1,
			      NULL);
	}
}

void
gs_app_row_set_size_groups (GsAppRow *app_row,
			    GtkSizeGroup *name,
			    GtkSizeGroup *button_label,
			    GtkSizeGroup *button_image)
{
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);

	g_return_if_fail (GS_IS_APP_ROW (app_row));

	if (name != NULL)
		gtk_size_group_add_widget (name, priv->name_box);
	gs_progress_button_set_size_groups (GS_PROGRESS_BUTTON (priv->button), button_label, button_image);
}

void
gs_app_row_set_colorful (GsAppRow *app_row, gboolean colorful)
{
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);

	g_return_if_fail (GS_IS_APP_ROW (app_row));

	if ((!priv->colorful) == (!colorful))
		return;

	priv->colorful = colorful;
	gs_app_row_schedule_refresh (app_row);
	g_object_notify_by_pspec (G_OBJECT (app_row), obj_props[PROP_COLORFUL]);
}

void
gs_app_row_set_show_buttons (GsAppRow *app_row, gboolean show_buttons)
{
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);

	g_return_if_fail (GS_IS_APP_ROW (app_row));

	if ((!priv->show_buttons) == (!show_buttons))
		return;

	priv->show_buttons = show_buttons;
	gs_app_row_schedule_refresh (app_row);
	g_object_notify_by_pspec (G_OBJECT (app_row), obj_props[PROP_SHOW_BUTTONS]);
}

void
gs_app_row_set_show_rating (GsAppRow *app_row, gboolean show_rating)
{
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);

	g_return_if_fail (GS_IS_APP_ROW (app_row));

	if ((!priv->show_rating) == (!show_rating))
		return;

	priv->show_rating = show_rating;
	gs_app_row_schedule_refresh (app_row);
	g_object_notify_by_pspec (G_OBJECT (app_row), obj_props[PROP_SHOW_RATING]);
}

/**
 * gs_app_row_get_show_description:
 * @app_row: a #GsAppRow
 *
 * Get the value of #GsAppRow:show-description.
 *
 * Returns: %TRUE if the description is shown, %FALSE otherwise
 *
 * Since: 41
 */
gboolean
gs_app_row_get_show_description (GsAppRow *app_row)
{
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);

	g_return_val_if_fail (GS_IS_APP_ROW (app_row), FALSE);

	return priv->show_description;
}

/**
 * gs_app_row_set_show_description:
 * @app_row: a #GsAppRow
 * @show_description: %TRUE to show the description, %FALSE otherwise
 *
 * Set the value of #GsAppRow:show-description.
 *
 * Since: 41
 */
void
gs_app_row_set_show_description (GsAppRow *app_row, gboolean show_description)
{
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);

	g_return_if_fail (GS_IS_APP_ROW (app_row));

	show_description = !!show_description;

	if (priv->show_description == show_description)
		return;

	priv->show_description = show_description;
	gs_app_row_schedule_refresh (app_row);
	g_object_notify_by_pspec (G_OBJECT (app_row), obj_props[PROP_SHOW_DESCRIPTION]);
}

void
gs_app_row_set_show_source (GsAppRow *app_row, gboolean show_source)
{
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);

	g_return_if_fail (GS_IS_APP_ROW (app_row));

	if ((!priv->show_source) == (!show_source))
		return;

	priv->show_source = show_source;
	gs_app_row_schedule_refresh (app_row);
	g_object_notify_by_pspec (G_OBJECT (app_row), obj_props[PROP_SHOW_SOURCE]);
}

void
gs_app_row_set_show_installed_size (GsAppRow *app_row, gboolean show_size)
{
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);

	g_return_if_fail (GS_IS_APP_ROW (app_row));

	if ((!priv->show_installed_size) == (!show_size))
		return;

	priv->show_installed_size = show_size;
	gs_app_row_schedule_refresh (app_row);
	g_object_notify_by_pspec (G_OBJECT (app_row), obj_props[PROP_SHOW_INSTALLED_SIZE]);
}

/**
 * gs_app_row_get_is_narrow:
 * @app_row: a #GsAppRow
 *
 * Get the value of #GsAppRow:is-narrow.
 *
 * Retruns: %TRUE if the row is in narrow mode, %FALSE otherwise
 *
 * Since: 41
 */
gboolean
gs_app_row_get_is_narrow (GsAppRow *app_row)
{
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);

	g_return_val_if_fail (GS_IS_APP_ROW (app_row), FALSE);

	return priv->is_narrow;
}

/**
 * gs_app_row_set_is_narrow:
 * @app_row: a #GsAppRow
 * @is_narrow: %TRUE to set the row in narrow mode, %FALSE otherwise
 *
 * Set the value of #GsAppRow:is-narrow.
 *
 * Since: 41
 */
void
gs_app_row_set_is_narrow (GsAppRow *app_row, gboolean is_narrow)
{
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);

	g_return_if_fail (GS_IS_APP_ROW (app_row));

	is_narrow = !!is_narrow;

	if (priv->is_narrow == is_narrow)
		return;

	priv->is_narrow = is_narrow;
	gs_app_row_update_button_reveal (app_row);
	g_object_notify_by_pspec (G_OBJECT (app_row), obj_props[PROP_IS_NARROW]);
}

/**
 * gs_app_row_set_show_update:
 *
 * Only really useful for the update panel to call
 **/
void
gs_app_row_set_show_update (GsAppRow *app_row, gboolean show_update)
{
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);

	if ((!priv->show_update) == (!show_update))
		return;

	priv->show_update = show_update;
	gs_app_row_schedule_refresh (app_row);
	g_object_notify_by_pspec (G_OBJECT (app_row), obj_props[PROP_SHOW_UPDATE]);
}

/**
 * gs_app_row_set_show_installed:
 * @app_row: a #GsAppRow
 * @show_installed: value to set
 *
 * Set whether to show "installed" label. Default is %TRUE. This has effect only
 * when not showing buttons (gs_app_row_set_show_buttons()).
 *
 * Since: 42.1
 **/
void
gs_app_row_set_show_installed (GsAppRow *app_row,
			       gboolean show_installed)
{
	GsAppRowPrivate *priv = gs_app_row_get_instance_private (app_row);

	g_return_if_fail (GS_IS_APP_ROW (app_row));

	if ((!show_installed) != (!priv->show_installed)) {
		priv->show_installed = show_installed;
		gs_app_row_schedule_refresh (app_row);
		g_object_notify_by_pspec (G_OBJECT (app_row), obj_props[PROP_SHOW_INSTALLED]);
	}
}

GtkWidget *
gs_app_row_new (GsApp *app)
{
	g_return_val_if_fail (GS_IS_APP (app), NULL);

	return g_object_new (GS_TYPE_APP_ROW,
			     "app", app,
			     NULL);
}
