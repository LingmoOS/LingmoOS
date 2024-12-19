/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2009-2016 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Appstore Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Appstore
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <errno.h>
#include <locale.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <xmlb.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include "gs-external-appstream-utils.h"

static gboolean
gs_install_appstream_move_file (GFile *file, GError **error)
{
	g_autofree gchar *basename = g_file_get_basename (file);
	g_autofree gchar *legacy_cachefn = gs_external_appstream_utils_get_legacy_file_cache_path (basename);
	g_autofree gchar *cachefn = gs_external_appstream_utils_get_file_cache_path (basename);
	g_autoptr(GFile) cachefn_file = g_file_new_for_path (cachefn);
	g_autoptr(GFile) cachedir_file = g_file_get_parent (cachefn_file);
	GStatBuf stat_buf = { 0 };

	/* Try to cleanup the old cache directory, but do not panic, when it fails */
	if (g_unlink (legacy_cachefn) == -1) {
		int errn = errno;
		if (errn != ENOENT)
			g_debug ("Failed to unlink '%s': %s", legacy_cachefn, g_strerror (errn));
	}

	/* make sure the parent directory exists, but if not then create with
	 * the ownership and permissions of the current process */
	if (!g_file_query_exists (cachedir_file, NULL)) {
		if (!g_file_make_directory_with_parents (cachedir_file, NULL, error))
			return FALSE;
	}

	/* do the move, overwriting existing files and setting the permissions
	 * of the current process (so that should be -rw-r--r--) */
	if (!g_file_move (file, cachefn_file,
			  G_FILE_COPY_OVERWRITE |
			  G_FILE_COPY_NOFOLLOW_SYMLINKS |
			  G_FILE_COPY_TARGET_DEFAULT_PERMS,
			  NULL, NULL, NULL, error))
		return FALSE;

	/* verify it is "-rw-r--r--" and the root owns the file */
	if (g_stat (cachefn, &stat_buf)  == 0) {
		struct passwd *pwd;
		mode_t expected_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		if ((stat_buf.st_mode & expected_mode) != expected_mode &&
		     g_chmod (cachefn, expected_mode) == -1) {
			int errn = errno;
			g_debug ("Failed to chmod '%s': %s", cachefn, g_strerror (errn));
		}

		/* the file should be owned by the root */
		pwd = getpwnam ("root");
		if (pwd != NULL) {
			if (chown (cachefn, pwd->pw_uid, pwd->pw_gid) == -1) {
				int errn = errno;
				g_debug ("Failed to chown on '%s': %s", cachefn, g_strerror (errn));
			}
		} else {
			int errn = errno;
			g_debug ("Failed to get root info: %s", g_strerror (errn));
		}
	} else {
		int errn = errno;
		g_debug ("Failed to stat '%s': %s", cachefn, g_strerror (errn));
	}

	return TRUE;
}

static gboolean
gs_install_appstream_check_content_type (GFile *file, GError **error)
{
	const gchar *type;
	g_autoptr(GError) error_local = NULL;
	g_autoptr(GFileInfo) info = NULL;
	g_autoptr(GPtrArray) components = NULL;
	g_autoptr(XbBuilder) builder = xb_builder_new ();
	g_autoptr(XbBuilderSource) source = xb_builder_source_new ();
	g_autoptr(XbSilo) silo = NULL;

	/* check is correct type */
	info = g_file_query_info (file,
				  G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
				  G_FILE_QUERY_INFO_NONE,
				  NULL, error);
	if (info == NULL)
		return FALSE;
	type = g_file_info_get_attribute_string (info, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE);
	if (g_strcmp0 (type, "application/gzip") != 0 &&
	    g_strcmp0 (type, "application/xml") != 0) {
		g_set_error (error,
			     G_IO_ERROR,
			     G_IO_ERROR_INVALID_DATA,
			     "Invalid type %s: ", type);
		return FALSE;
	}

	/* check is an AppStream file */
	if (!xb_builder_source_load_file (source, file,
					  XB_BUILDER_SOURCE_FLAG_NONE,
					  NULL, &error_local)) {
		g_set_error (error,
			     G_IO_ERROR,
			     G_IO_ERROR_INVALID_DATA,
			     "Failed to import XML: %s", error_local->message);
		return FALSE;
	}
	xb_builder_import_source (builder, source);
	/* No need to change the thread-default main context because the silo
	 * doesn’t live beyond this function. */
	silo = xb_builder_compile (builder,
				   XB_BUILDER_COMPILE_FLAG_NONE,
				   NULL, &error_local);
	if (silo == NULL) {
		g_set_error (error,
			     G_IO_ERROR,
			     G_IO_ERROR_INVALID_DATA,
			     "Failed to parse XML: %s", error_local->message);
		return FALSE;
	}
	components = xb_silo_query (silo, "components/component", 0, &error_local);
	if (components == NULL) {
		if (g_error_matches (error_local, G_IO_ERROR, G_IO_ERROR_NOT_FOUND)) {
			g_set_error_literal (error,
					     G_IO_ERROR,
					     G_IO_ERROR_INVALID_DATA,
					     "No apps found in the AppStream XML");
			return FALSE;
		}
		g_set_error (error,
			     G_IO_ERROR,
			     G_IO_ERROR_INVALID_DATA,
			     "Failed to query XML: %s", error_local->message);
		return FALSE;
	}

	return TRUE;
}

int
main (int argc, char *argv[])
{
	g_autoptr(GError) error = NULL;
	g_autoptr(GFile) file = NULL;
	g_autoptr(GOptionContext) context = NULL;

	/* setup translations */
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	context = g_option_context_new (NULL);
	/* TRANSLATORS: tool that is used when moving profiles system-wide */
	g_option_context_set_summary (context, _("LINGMO Appstore AppStream system-wide installer"));
	if (!g_option_context_parse (context, &argc, &argv, &error)) {
		g_print ("%s\n", _("Failed to parse command line arguments"));
		return EXIT_FAILURE;
	}

	/* check input */
	if (g_strv_length (argv) != 2) {
		/* TRANSLATORS: user did not specify a valid filename */
		g_print ("%s\n", _("You need to specify exactly one filename"));
		return EXIT_FAILURE;
	}

	/* check calling process */
	if (getuid () != 0 || geteuid () != 0) {
		/* TRANSLATORS: only able to install files as root */
		g_print ("%s\n", _("This program can only be used by the root user"));
		return EXIT_FAILURE;
	}

	/* check content type for file */
	file = g_file_new_for_path (argv[1]);
	if (!gs_install_appstream_check_content_type (file, &error)) {
		/* TRANSLATORS: error details */
		g_print (_("Failed to validate content type: %s"), error->message);
		g_print ("\n");
		return EXIT_FAILURE;
	}

	/* Set the umask to ensure it is read-only to all users except root. */
	umask (022);

	/* do the move */
	if (!gs_install_appstream_move_file (file, &error)) {
		/* TRANSLATORS: error details */
		g_print (_("Failed to move: %s"), error->message);
		g_print ("\n");
		return EXIT_FAILURE;
	}

	/* success */
	return EXIT_SUCCESS;
}
