/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Endless OS Foundation LLC
 *
 * Author: Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-app-translation-dialog
 * @short_description: A dialog showing translation information about an app
 *
 * #GsAppTranslationDialog is a dialog which shows a message about the
 * translation status of an app, and provides information and a link for how
 * to contribute more translations to the app.
 *
 * It is intended to be shown if the app is not sufficiently translated to the
 * current locale.
 *
 * The widget has no special appearance if the app is unset, so callers will
 * typically want to hide the dialog in that case.
 *
 * Since: 41
 */

#include "config.h"

#include <adwaita.h>
#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <locale.h>

#include "gs-app.h"
#include "gs-app-translation-dialog.h"
#include "gs-common.h"
#include "gs-lozenge.h"

struct _GsAppTranslationDialog
{
	GsInfoWindow		 parent_instance;

	GsApp			*app;  /* (not nullable) (owned) */
	gulong			 app_notify_name_handler;

	GtkLabel		*title;
	GtkLabel		*description;
};

G_DEFINE_TYPE (GsAppTranslationDialog, gs_app_translation_dialog, GS_TYPE_INFO_WINDOW)

typedef enum {
	PROP_APP = 1,
} GsAppTranslationDialogProperty;

static GParamSpec *obj_props[PROP_APP + 1] = { NULL, };

static void
update_labels (GsAppTranslationDialog *self)
{
	g_autofree gchar *title = NULL;
	g_autofree gchar *description = NULL;

	/* Translators: The placeholder is an app name */
	title = g_strdup_printf (_("Help Translate %s"), gs_app_get_name (self->app));

	/* Translators: The placeholder is an app name */
	description = g_strdup_printf (_("%s is designed, developed, and translated by an "
					 "international community of contributors."
					 "\n\n"
					 "This means that while it’s not yet available in "
					 "your language, you can get involved and help "
					 "translate it yourself."), gs_app_get_name (self->app));

	gtk_label_set_text (self->title, title);
	gtk_label_set_text (self->description, description);
}

static void
app_notify_cb (GObject    *obj,
               GParamSpec *pspec,
               gpointer    user_data)
{
	GsAppTranslationDialog *self = GS_APP_TRANSLATION_DIALOG (user_data);

	update_labels (self);
}

static const gchar *
get_url_for_app (GsApp *app)
{
	const gchar *url;

	/* Try the translate URL, or a fallback */
	url = gs_app_get_url (app, AS_URL_KIND_TRANSLATE);
#if AS_CHECK_VERSION(0, 15, 3)
	if (url == NULL)
		url = gs_app_get_url (app, AS_URL_KIND_CONTRIBUTE);
#endif
	if (url == NULL)
		url = gs_app_get_url (app, AS_URL_KIND_BUGTRACKER);

	return url;
}

static void
button_clicked_cb (GtkButton *button,
                   gpointer   user_data)
{
	GsAppTranslationDialog *self = GS_APP_TRANSLATION_DIALOG (user_data);
	const gchar *url = get_url_for_app (self->app);

	gs_show_uri (GTK_WINDOW (self), url);
}

static void
gs_app_translation_dialog_init (GsAppTranslationDialog *self)
{
	g_type_ensure (GS_TYPE_LOZENGE);

	gtk_widget_init_template (GTK_WIDGET (self));
}

static void
gs_app_translation_dialog_get_property (GObject    *object,
                                        guint       prop_id,
                                        GValue     *value,
                                        GParamSpec *pspec)
{
	GsAppTranslationDialog *self = GS_APP_TRANSLATION_DIALOG (object);

	switch ((GsAppTranslationDialogProperty) prop_id) {
	case PROP_APP:
		g_value_set_object (value, gs_app_translation_dialog_get_app (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_app_translation_dialog_set_property (GObject      *object,
                                        guint         prop_id,
                                        const GValue *value,
                                        GParamSpec   *pspec)
{
	GsAppTranslationDialog *self = GS_APP_TRANSLATION_DIALOG (object);

	switch ((GsAppTranslationDialogProperty) prop_id) {
	case PROP_APP:
		/* Construct only */
		g_assert (self->app == NULL);
		g_assert (self->app_notify_name_handler == 0);

		self->app = g_value_dup_object (value);
		self->app_notify_name_handler = g_signal_connect (self->app, "notify::name", G_CALLBACK (app_notify_cb), self);

		/* Update the UI. */
		update_labels (self);

		g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_APP]);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_app_translation_dialog_dispose (GObject *object)
{
	GsAppTranslationDialog *self = GS_APP_TRANSLATION_DIALOG (object);

	g_clear_signal_handler (&self->app_notify_name_handler, self->app);
	g_clear_object (&self->app);

	G_OBJECT_CLASS (gs_app_translation_dialog_parent_class)->dispose (object);
}

static void
gs_app_translation_dialog_class_init (GsAppTranslationDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->get_property = gs_app_translation_dialog_get_property;
	object_class->set_property = gs_app_translation_dialog_set_property;
	object_class->dispose = gs_app_translation_dialog_dispose;

	/**
	 * GsAppTranslationDialog:app: (not nullable)
	 *
	 * The app to display the translation details for.
	 *
	 * This must not be %NULL.
	 *
	 * Since: 41
	 */
	obj_props[PROP_APP] =
		g_param_spec_object ("app", NULL, NULL,
				     GS_TYPE_APP,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-app-translation-dialog.ui");

	gtk_widget_class_bind_template_child (widget_class, GsAppTranslationDialog, title);
	gtk_widget_class_bind_template_child (widget_class, GsAppTranslationDialog, description);

	gtk_widget_class_bind_template_callback (widget_class, button_clicked_cb);
}

/**
 * gs_app_translation_dialog_new:
 * @app: (not nullable): the app to display translation information for
 *
 * Create a new #GsAppTranslationDialog and set its initial app to @app.
 *
 * Returns: (transfer full): a new #GsAppTranslationDialog
 * Since: 41
 */
GsAppTranslationDialog *
gs_app_translation_dialog_new (GsApp *app)
{
	g_return_val_if_fail (GS_IS_APP (app), NULL);

	return g_object_new (GS_TYPE_APP_TRANSLATION_DIALOG,
			     "app", app,
			     NULL);
}

/**
 * gs_app_translation_dialog_get_app:
 * @self: a #GsAppTranslationDialog
 *
 * Gets the value of #GsAppTranslationDialog:app.
 *
 * Returns: (not nullable) (transfer none): app whose translation information is
 *     being displayed
 * Since: 41
 */
GsApp *
gs_app_translation_dialog_get_app (GsAppTranslationDialog *self)
{
	g_return_val_if_fail (GS_IS_APP_TRANSLATION_DIALOG (self), NULL);

	return self->app;
}

/**
 * gs_app_translation_dialog_app_has_url:
 * @app: a #GsApp
 *
 * Check @app to see if it has appropriate URLs set on it to allow the user
 * to be linked to a page relevant to translating the app.
 *
 * Generally this should be used to work out whether to show a
 * #GsAppTranslationDialog dialog for a given @app.
 *
 * Returns: %TRUE if an URL exists, %FALSE otherwise
 * Since: 41
 */
gboolean
gs_app_translation_dialog_app_has_url (GsApp *app)
{
	g_return_val_if_fail (GS_IS_APP (app), FALSE);

	return (get_url_for_app (app) != NULL);
}
