/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Matthew Leeds <mwleeds@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <glib/gi18n.h>

#include "gs-app-version-history-row.h"
#include "gs-description-box.h"

#include "gs-common.h"

struct _GsAppVersionHistoryRow
{
	GtkListBoxRow	 parent_instance;

	GtkWidget	*version_number_label;
	GtkWidget	*version_date_label;
	GtkWidget	*version_description_box;
};

G_DEFINE_TYPE (GsAppVersionHistoryRow, gs_app_version_history_row, GTK_TYPE_LIST_BOX_ROW)

typedef enum {
	PROP_ALWAYS_EXPANDED = 1,
} GsAppVersionHistoryRowProperty;

static GParamSpec *obj_props[PROP_ALWAYS_EXPANDED + 1] = { NULL, };

static void
gs_app_version_history_row_get_property (GObject    *object,
					 guint       prop_id,
					 GValue     *value,
					 GParamSpec *pspec)
{
	GsAppVersionHistoryRow *self = GS_APP_VERSION_HISTORY_ROW (object);

	switch ((GsAppVersionHistoryRowProperty) prop_id) {
	case PROP_ALWAYS_EXPANDED:
		g_value_set_boolean (value, gs_app_version_history_row_get_always_expanded (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_app_version_history_row_set_property (GObject      *object,
					 guint         prop_id,
					 const GValue *value,
					 GParamSpec   *pspec)
{
	GsAppVersionHistoryRow *self = GS_APP_VERSION_HISTORY_ROW (object);

	switch ((GsAppVersionHistoryRowProperty) prop_id) {
	case PROP_ALWAYS_EXPANDED:
		gs_app_version_history_row_set_always_expanded (self, g_value_get_boolean (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_app_version_history_row_class_init (GsAppVersionHistoryRowClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = gs_app_version_history_row_get_property;
	object_class->set_property = gs_app_version_history_row_set_property;

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-app-version-history-row.ui");

	gtk_widget_class_bind_template_child (widget_class, GsAppVersionHistoryRow, version_number_label);
	gtk_widget_class_bind_template_child (widget_class, GsAppVersionHistoryRow, version_date_label);
	gtk_widget_class_bind_template_child (widget_class, GsAppVersionHistoryRow, version_description_box);

	/**
	 * GsAppVersionHistoryRow:always-expanded:
	 *
	 * A proxy property for internal GsDescriptionBox:always-expanded.
	 *
	 * Since: 44
	 */
	obj_props[PROP_ALWAYS_EXPANDED] =
		g_param_spec_boolean ("always-expanded", NULL, NULL,
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);
}

static void
gs_app_version_history_row_init (GsAppVersionHistoryRow *row)
{
	g_type_ensure (GS_TYPE_DESCRIPTION_BOX);

	gtk_widget_init_template (GTK_WIDGET (row));
}

/**
 * gs_app_version_history_row_set_info:
 * @row: a #GsAppVersionHistoryRow
 * @version_number: (nullable): version number of the release, or %NULL if unknown
 * @version_date: release date of the version, as seconds since the Unix epoch,
 *   or `0` if unknown
 * @version_description: (nullable): Pango Markup for the full human readable
 *   description of the release, or %NULL if unknown
 *
 * Set information about the release represented by this version history row.
 */
void
gs_app_version_history_row_set_info (GsAppVersionHistoryRow *row,
				     const char *version_number,
				     guint64     version_date,
				     const char *version_description)
{
	g_autofree char *version_date_string = NULL;
	g_autofree char *version_date_string_tooltip = NULL;

	if (version_number == NULL || *version_number == '\0')
		return;

	if (version_description != NULL && *version_description != '\0') {
		g_autofree char *version_tmp = NULL;
		version_tmp = g_strdup_printf (_("New in Version %s"), version_number);
		gtk_label_set_label (GTK_LABEL (row->version_number_label), version_tmp);
		gs_description_box_set_text (GS_DESCRIPTION_BOX (row->version_description_box), version_description);
		gtk_widget_remove_css_class (row->version_description_box, "dim-label");
	} else {
		g_autofree char *version_tmp = NULL;
		const gchar *version_description_fallback;
		version_tmp = g_strdup_printf (_("Version %s"), version_number);
		gtk_label_set_label (GTK_LABEL (row->version_number_label), version_tmp);
		version_description_fallback = _("No details for this release");
		gs_description_box_set_text (GS_DESCRIPTION_BOX (row->version_description_box), version_description_fallback);
		gtk_widget_add_css_class (row->version_description_box, "dim-label");
	}

	if (version_date != 0) {
		g_autoptr(GDateTime) date_time = NULL;
		const gchar *format_string;

		/* this is the date in the form of "x weeks ago" or "y months ago" */
		version_date_string = gs_utils_time_to_string ((gint64) version_date);

		/* TRANSLATORS: This is the date string with: day number, month name, year.
		   i.e. "25 May 2012" */
		format_string = _("%e %B %Y");
		date_time = g_date_time_new_from_unix_local (version_date);
		version_date_string_tooltip = g_date_time_format (date_time, format_string);
	}

	if (version_date_string == NULL)
		gtk_widget_set_visible (row->version_date_label, FALSE);
	else {
		gtk_label_set_label (GTK_LABEL (row->version_date_label), version_date_string);
		gtk_widget_set_visible (row->version_date_label, TRUE);
	}

	if (version_date_string_tooltip != NULL)
		gtk_widget_set_tooltip_text (row->version_date_label, version_date_string_tooltip);
}

GtkWidget *
gs_app_version_history_row_new (void)
{
	GsAppVersionHistoryRow *row;

	row = g_object_new (GS_TYPE_APP_VERSION_HISTORY_ROW, NULL);
	return GTK_WIDGET (row);
}

gboolean
gs_app_version_history_row_get_always_expanded (GsAppVersionHistoryRow *self)
{
	g_return_val_if_fail (GS_IS_APP_VERSION_HISTORY_ROW (self), FALSE);

	return gs_description_box_get_always_expanded (GS_DESCRIPTION_BOX (self->version_description_box));
}

void
gs_app_version_history_row_set_always_expanded (GsAppVersionHistoryRow *self,
						gboolean always_expanded)
{
	g_return_if_fail (GS_IS_APP_VERSION_HISTORY_ROW (self));

	gs_description_box_set_always_expanded (GS_DESCRIPTION_BOX (self->version_description_box), always_expanded);
}
