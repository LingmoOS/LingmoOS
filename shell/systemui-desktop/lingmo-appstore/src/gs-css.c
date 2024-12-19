/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2017 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-css
 * @title: GsCss
 * @stability: Unstable
 * @short_description: Parse, validate and rewrite CSS resources
 */

#include "config.h"

#include <gtk/gtk.h>
#include <appstream.h>

#include "lib/gs-utils.h"
#include "gs-css.h"

struct _GsCss
{
	GObject			 parent_instance;
	GHashTable		*ids;
	GsCssRewriteFunc	 rewrite_func;
	gpointer		 rewrite_func_data;
};

G_DEFINE_TYPE (GsCss, gs_css, G_TYPE_OBJECT)

static void
_cleanup_string (GString *str)
{
	/* remove leading newlines */
	while (g_str_has_prefix (str->str, "\n") || g_str_has_prefix (str->str, " "))
		g_string_erase (str, 0, 1);

	/* remove trailing newlines */
	while (g_str_has_suffix (str->str, "\n") || g_str_has_suffix (str->str, " "))
		g_string_truncate (str, str->len - 1);
}

/**
 * gs_css_parse:
 * @self: a #GsCss
 * @markup: come CSS, or %NULL
 * @error: a #GError or %NULL
 *
 * Parses the CSS markup and does some basic validation checks on the input.
 *
 * Returns: %TRUE for success
 */
gboolean
gs_css_parse (GsCss *self, const gchar *markup, GError **error)
{
	g_auto(GStrv) parts = NULL;
	g_autoptr(GString) markup_str = NULL;

	/* no data */
	if (markup == NULL || markup[0] == '\0')
		return TRUE;

	/* old style, no IDs */
	markup_str = g_string_new (markup);
	gs_utils_gstring_replace (markup_str, "@datadir@", DATADIR);
	if (!g_str_has_prefix (markup_str->str, "#")) {
		g_hash_table_insert (self->ids,
				     g_strdup ("tile"),
				     g_string_free (g_steal_pointer (&markup_str), FALSE));
		return TRUE;
	}

	/* split up CSS into ID chunks, e.g.
	 *
	 *    #tile {border-radius: 0;}
	 *    #name {color: white;}
	 */
	parts = g_strsplit (markup_str->str + 1, "\n#", -1);
	for (guint i = 0; parts[i] != NULL; i++) {
		g_autoptr(GString) current_css = NULL;
		g_autoptr(GString) current_key = NULL;
		for (guint j = 1; parts[i][j] != '\0'; j++) {
			const gchar ch = parts[i][j];
			if (ch == '{') {
				if (current_key != NULL || current_css != NULL) {
					g_set_error_literal (error,
							     G_IO_ERROR,
							     G_IO_ERROR_INVALID_DATA,
							     "invalid '{'");
					return FALSE;
				}
				current_key = g_string_new_len (parts[i], j);
				current_css = g_string_new (NULL);
				_cleanup_string (current_key);

				/* already added */
				if (g_hash_table_lookup (self->ids, current_key->str) != NULL) {
					g_set_error (error,
						     G_IO_ERROR,
						     G_IO_ERROR_INVALID_DATA,
						     "duplicate ID '%s'",
						     current_key->str);
					return FALSE;
				}
				continue;
			}
			if (ch == '}') {
				if (current_key == NULL || current_css == NULL) {
					g_set_error_literal (error,
							     G_IO_ERROR,
							     G_IO_ERROR_INVALID_DATA,
							     "invalid '}'");
					return FALSE;
				}
				_cleanup_string (current_css);
				g_hash_table_insert (self->ids,
						     g_string_free (current_key, FALSE),
						     g_string_free (current_css, FALSE));
				current_key = NULL;
				current_css = NULL;
				continue;
			}
			if (current_css != NULL)
				g_string_append_c (current_css, ch);
		}
		if (current_key != NULL || current_css != NULL) {
			g_set_error_literal (error,
					     G_IO_ERROR,
					     G_IO_ERROR_INVALID_DATA,
					     "missing '}'");
			return FALSE;
		}
	}

	/* success */
	return TRUE;
}

/**
 * gs_css_get_markup_for_id:
 * @self: a #GsCss
 * @id: an ID, or %NULL for the default
 *
 * Gets the CSS markup for a specific ID.
 *
 * Returns: %TRUE for success
 */
const gchar *
gs_css_get_markup_for_id (GsCss *self, const gchar *id)
{
	if (id == NULL)
		id = "tile";
	return g_hash_table_lookup (self->ids, id);
}

static void
_css_parsing_error_cb (GtkCssProvider *provider,
		       GtkCssSection *section,
		       GError *error,
		       gpointer user_data)
{
	GError **error_parse = (GError **) user_data;
	if (*error_parse != NULL) {
		const GtkCssLocation *start_location;

		start_location = gtk_css_section_get_start_location (section);
		g_warning ("ignoring parse error %" G_GSIZE_FORMAT ":%" G_GSIZE_FORMAT ": %s",
			   start_location->lines + 1,
			   start_location->line_chars,
			   error->message);
		return;
	}
	*error_parse = g_error_copy (error);
}

static gboolean
gs_css_validate_part (GsCss *self, const gchar *markup, GError **error)
{
	g_autofree gchar *markup_new = NULL;
	g_autoptr(GError) error_parse = NULL;
	g_autoptr(GString) str = NULL;
	g_autoptr(GtkCssProvider) provider = NULL;

	/* nothing set */
	if (markup == NULL)
		return TRUE;

	/* remove custom class if NULL */
	str = g_string_new (NULL);
	g_string_append (str, ".themed-widget {");
	if (self->rewrite_func != NULL) {
		markup_new = self->rewrite_func (self->rewrite_func_data,
						 markup,
						 error);
		if (markup_new == NULL)
			return FALSE;
	} else {
		markup_new = g_strdup (markup);
	}
	g_string_append (str, markup_new);
	g_string_append (str, "}");

	/* set up custom provider */
	provider = gtk_css_provider_new ();
	g_signal_connect (provider, "parsing-error",
			  G_CALLBACK (_css_parsing_error_cb), &error_parse);
	gtk_style_context_add_provider_for_display (gdk_display_get_default (),
						    GTK_STYLE_PROVIDER (provider),
						    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	#if GTK_CHECK_VERSION(4, 12, 0)
	gtk_css_provider_load_from_string (provider, str->str);
	#else
	gtk_css_provider_load_from_data (provider, str->str, -1);
	#endif
	if (error_parse != NULL) {
		if (error != NULL)
			*error = g_error_copy (error_parse);
		return FALSE;
	}
	return TRUE;
}

/**
 * gs_css_validate:
 * @self: a #GsCss
 * @error: a #GError or %NULL
 *
 * Validates each part of the CSS markup.
 *
 * Returns: %TRUE for success
 */
gboolean
gs_css_validate (GsCss *self, GError **error)
{
	g_autoptr(GList) keys = NULL;

	/* check each CSS ID */
	keys = g_hash_table_get_keys (self->ids);
	for (GList *l = keys; l != NULL; l = l->next) {
		const gchar *tmp;
		const gchar *id = l->data;
		if (g_strcmp0 (id, "tile") != 0 &&
		    g_strcmp0 (id, "name") != 0 &&
		    g_strcmp0 (id, "summary") != 0) {
			g_set_error (error,
				     G_IO_ERROR,
				     G_IO_ERROR_INVALID_DATA,
				     "Invalid CSS ID '%s'",
				     id);
			return FALSE;
		}
		tmp = g_hash_table_lookup (self->ids, id);
		if (!gs_css_validate_part (self, tmp, error))
			return FALSE;
	}

	/* success */
	return TRUE;
}

/**
 * gs_css_set_rewrite_func:
 * @self: a #GsCss
 * @func: a #GsCssRewriteFunc or %NULL
 * @user_data: user data to pass to @func
 *
 * Sets a function to be used when rewriting CSS before it is parsed.
 *
 * Returns: %TRUE for success
 */
void
gs_css_set_rewrite_func (GsCss *self, GsCssRewriteFunc func, gpointer user_data)
{
	self->rewrite_func = func;
	self->rewrite_func_data = user_data;
}

static void
gs_css_finalize (GObject *object)
{
	GsCss *self = GS_CSS (object);
	g_hash_table_unref (self->ids);
	G_OBJECT_CLASS (gs_css_parent_class)->finalize (object);
}

static void
gs_css_class_init (GsCssClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = gs_css_finalize;
}

static void
gs_css_init (GsCss *self)
{
	self->ids = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
}

/**
 * gs_css_new:
 *
 * Return value: a new #GsCss object.
 **/
GsCss *
gs_css_new (void)
{
	GsCss *self;
	self = g_object_new (GS_TYPE_CSS, NULL);
	return GS_CSS (self);
}
