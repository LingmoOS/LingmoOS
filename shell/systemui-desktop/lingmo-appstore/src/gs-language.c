/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2008 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2015-2016 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <string.h>

#include "gs-language.h"

struct _GsLanguage
{
	GObject			 parent_instance;

	GHashTable		*hash;
};

G_DEFINE_TYPE (GsLanguage, gs_language, G_TYPE_OBJECT)

static void
gs_language_parser_start_element (GMarkupParseContext *context, const gchar *element_name,
                                  const gchar **attribute_names, const gchar **attribute_values,
                                  gpointer user_data, GError **error)
{
	guint i, len;
	const gchar *code1 = NULL;
	const gchar *code2b = NULL;
	const gchar *name = NULL;
	GsLanguage *language = user_data;

	if (strcmp (element_name, "iso_639_entry") != 0)
		return;

	/* find data */
	len = g_strv_length ((gchar**)attribute_names);
	for (i=0; i<len; i++) {
		if (strcmp (attribute_names[i], "iso_639_1_code") == 0)
			code1 = attribute_values[i];
		if (strcmp (attribute_names[i], "iso_639_2B_code") == 0)
			code2b = attribute_values[i];
		if (strcmp (attribute_names[i], "name") == 0)
			name = attribute_values[i];
	}

	/* not valid entry */
	if (name == NULL)
		return;

	/* add both to hash */
	if (code1 != NULL)
		g_hash_table_insert (language->hash, g_strdup (code1), g_strdup (name));
	if (code2b != NULL)
		g_hash_table_insert (language->hash, g_strdup (code2b), g_strdup (name));
}

/* trivial parser */
static const GMarkupParser gs_language_markup_parser =
{
	gs_language_parser_start_element,
	NULL, /* end_element */
	NULL, /* characters */
	NULL, /* passthrough */
	NULL /* error */
};

/**
 * gs_language_populate:
 *
 * <iso_639_entry iso_639_2B_code="hun" iso_639_2T_code="hun" iso_639_1_code="hu" name="Hungarian" />
 **/
gboolean
gs_language_populate (GsLanguage *language, GError **error)
{
	gsize size;
	g_autofree gchar *contents = NULL;
	g_autofree gchar *filename = NULL;
	g_autoptr(GMarkupParseContext) context = NULL;

	/* find filename */
	filename = g_build_filename (DATADIR, "xml", "iso-codes", "iso_639.xml", NULL);
	if (!g_file_test (filename, G_FILE_TEST_EXISTS)) {
		g_free (filename);
		filename = g_build_filename ("/usr", "share", "xml", "iso-codes", "iso_639.xml", NULL);
	}
	/* FreeBSD and OpenBSD ports */
	if (!g_file_test (filename, G_FILE_TEST_EXISTS)) {
		g_free (filename);
		filename = g_build_filename ("/usr", "local", "share", "xml", "iso-codes", "iso_639.xml", NULL);
	}
	/* NetBSD pkgsrc */
	if (!g_file_test (filename, G_FILE_TEST_EXISTS)) {
		g_free (filename);
		filename = g_build_filename ("/usr", "pkg", "share", "xml", "iso-codes", "iso_639.xml", NULL);
	}
	if (!g_file_test (filename, G_FILE_TEST_EXISTS)) {
		g_set_error (error, 1, 0, "cannot find source file : '%s'", filename);
		return FALSE;
	}

	/* get contents */
	if (!g_file_get_contents (filename, &contents, &size, error))
		return FALSE;

	/* create parser */
	context = g_markup_parse_context_new (&gs_language_markup_parser, G_MARKUP_PREFIX_ERROR_POSITION, language, NULL);

	/* parse data */
	if (!g_markup_parse_context_parse (context, contents, (gssize) size, error))
		return FALSE;

	return TRUE;;
}

gchar *
gs_language_iso639_to_language (GsLanguage *language, const gchar *iso639)
{
	return g_strdup (g_hash_table_lookup (language->hash, iso639));
}

static void
gs_language_finalize (GObject *object)
{
	GsLanguage *language;

	g_return_if_fail (GS_IS_LANGUAGE (object));

	language = GS_LANGUAGE (object);

	g_hash_table_unref (language->hash);

	G_OBJECT_CLASS (gs_language_parent_class)->finalize (object);
}

static void
gs_language_init (GsLanguage *language)
{
	language->hash = g_hash_table_new_full (g_str_hash, g_str_equal, (GDestroyNotify) g_free, (GDestroyNotify) g_free);
}

static void
gs_language_class_init (GsLanguageClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = gs_language_finalize;
}

/**
 * gs_language_new:
 *
 * Return value: a new GsLanguage object.
 **/
GsLanguage *
gs_language_new (void)
{
	GsLanguage *language;
	language = g_object_new (GS_TYPE_LANGUAGE, NULL);
	return GS_LANGUAGE (language);
}
