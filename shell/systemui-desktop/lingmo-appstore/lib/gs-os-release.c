/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016 Kalev Lember <klember@redhat.com>
 * Copyright (C) 2016 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/**
 * SECTION:gs-os-release
 * @title: GsOsRelease
 * @include: lingmo-appstore.h
 * @stability: Unstable
 * @short_description: Data from os-release
 *
 * This object allows plugins to parse /etc/os-release for distribution
 * metadata information.
 */

#include "config.h"

#include <glib.h>

#include "gs-os-release.h"

struct _GsOsRelease
{
	GObject			 parent_instance;
	gchar			*name;
	gchar			*version;
	gchar			*id;
	gchar			**id_like;
	gchar			*version_id;
	gchar			*pretty_name;
	gchar			*cpe_name;
	gchar			*distro_codename;
	gchar			*home_url;
	gchar			*logo;
	gchar			*vendor_name;
};

static void gs_os_release_initable_iface_init (GInitableIface *iface);

G_DEFINE_TYPE_WITH_CODE (GsOsRelease, gs_os_release, G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE, gs_os_release_initable_iface_init))

static void
gs_os_release_set_string_nonempty (gchar **inout_string,
				   const gchar *value)
{
	if (*inout_string == value)
		return;

	g_clear_pointer (inout_string, g_free);
	if (value != NULL && *value != '\0')
		*inout_string = g_strdup (value);
}

static gboolean
gs_os_release_initable_init (GInitable *initable,
			     GCancellable *cancellable,
			     GError **error)
{
	GsOsRelease *os_release = GS_OS_RELEASE (initable);
	const gchar *filename;
	g_autofree gchar *data = NULL;
	g_auto(GStrv) lines = NULL;
	guint i;

	g_return_val_if_fail (GS_IS_OS_RELEASE (os_release), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	/* get contents */
	filename = g_getenv ("GS_SELF_TEST_OS_RELEASE_FILENAME");
	if (filename == NULL) {
		filename = "/etc/os-release";
		if (!g_file_test (filename, G_FILE_TEST_EXISTS))
			filename = "/usr/lib/os-release";
	}
	if (!g_file_get_contents (filename, &data, NULL, error))
		return FALSE;

	/* parse */
	lines = g_strsplit (data, "\n", -1);
	for (i = 0; lines[i] != NULL; i++) {
		gchar *tmp;

		/* split the line up into two halves */
		tmp = g_strstr_len (lines[i], -1, "=");
		if (tmp == NULL)
			continue;
		*tmp = '\0';
		tmp++;

		/* ignore trailing quote */
		if (tmp[0] == '\"')
			tmp++;

		/* ignore trailing quote */
		g_strdelimit (tmp, "\"", '\0');

		/* match fields we're interested in */
		if (g_strcmp0 (lines[i], "NAME") == 0) {
			gs_os_release_set_string_nonempty (&os_release->name, tmp);
			continue;
		}
		if (g_strcmp0 (lines[i], "VERSION") == 0) {
			gs_os_release_set_string_nonempty (&os_release->version, tmp);
			continue;
		}
		if (g_strcmp0 (lines[i], "ID") == 0) {
			gs_os_release_set_string_nonempty (&os_release->id, tmp);
			continue;
		}
		if (g_strcmp0 (lines[i], "ID_LIKE") == 0) {
			if (os_release->id_like != NULL)
				g_strfreev (os_release->id_like);
			os_release->id_like = g_strsplit (tmp, " ", 0);
			continue;
		}
		if (g_strcmp0 (lines[i], "VERSION_ID") == 0) {
			gs_os_release_set_string_nonempty (&os_release->version_id, tmp);
			continue;
		}
		if (g_strcmp0 (lines[i], "PRETTY_NAME") == 0) {
			gs_os_release_set_string_nonempty (&os_release->pretty_name, tmp);
			continue;
		}
		if (g_strcmp0 (lines[i], "CPE_NAME") == 0) {
			gs_os_release_set_string_nonempty (&os_release->cpe_name, tmp);
			continue;
		}
		if (g_strcmp0 (lines[i], "UBUNTU_CODENAME") == 0) {
			gs_os_release_set_string_nonempty (&os_release->distro_codename, tmp);
			continue;
		}
		if (g_strcmp0 (lines[i], "HOME_URL") == 0) {
			gs_os_release_set_string_nonempty (&os_release->home_url, tmp);
			continue;
		}
		if (g_strcmp0 (lines[i], "LOGO") == 0) {
			gs_os_release_set_string_nonempty (&os_release->logo, tmp);
			continue;
		}
		if (g_strcmp0 (lines[i], "VENDOR_NAME") == 0) {
			gs_os_release_set_string_nonempty (&os_release->vendor_name, tmp);
			continue;
		}
	}
	return TRUE;
}

/**
 * gs_os_release_get_name:
 * @os_release: A #GsOsRelease
 *
 * Gets the name from the os-release parser.
 *
 * Returns: a string, or %NULL
 *
 * Since: 3.22
 **/
const gchar *
gs_os_release_get_name (GsOsRelease *os_release)
{
	g_return_val_if_fail (GS_IS_OS_RELEASE (os_release), NULL);
	return os_release->name;
}

/**
 * gs_os_release_get_version:
 * @os_release: A #GsOsRelease
 *
 * Gets the version from the os-release parser.
 *
 * Returns: a string, or %NULL
 *
 * Since: 3.22
 **/
const gchar *
gs_os_release_get_version (GsOsRelease *os_release)
{
	g_return_val_if_fail (GS_IS_OS_RELEASE (os_release), NULL);
	return os_release->version;
}

/**
 * gs_os_release_get_id:
 * @os_release: A #GsOsRelease
 *
 * Gets the ID from the os-release parser.
 *
 * Returns: a string, or %NULL
 *
 * Since: 3.22
 **/
const gchar *
gs_os_release_get_id (GsOsRelease *os_release)
{
	g_return_val_if_fail (GS_IS_OS_RELEASE (os_release), NULL);
	return os_release->id;
}

/**
 * gs_os_release_get_id_like:
 * @os_release: A #GsOsRelease
 *
 * Gets the ID_LIKE from the os-release parser. This is a list of operating
 * systems that are "closely related" to the local operating system, possibly
 * by being a derivative distribution.
 *
 * Returns: a %NULL terminated list
 *
 * Since: 3.26.2
 **/
const gchar * const *
gs_os_release_get_id_like (GsOsRelease *os_release)
{
	g_return_val_if_fail (GS_IS_OS_RELEASE (os_release), NULL);
	return (const gchar * const *) os_release->id_like;
}

/**
 * gs_os_release_get_version_id:
 * @os_release: A #GsOsRelease
 *
 * Gets the version ID from the os-release parser.
 *
 * Returns: a string, or %NULL
 *
 * Since: 3.22
 **/
const gchar *
gs_os_release_get_version_id (GsOsRelease *os_release)
{
	g_return_val_if_fail (GS_IS_OS_RELEASE (os_release), NULL);
	return os_release->version_id;
}

/**
 * gs_os_release_get_pretty_name:
 * @os_release: A #GsOsRelease
 *
 * Gets the pretty name from the os-release parser.
 *
 * Returns: a string, or %NULL
 *
 * Since: 3.22
 **/
const gchar *
gs_os_release_get_pretty_name (GsOsRelease *os_release)
{
	g_return_val_if_fail (GS_IS_OS_RELEASE (os_release), NULL);
	return os_release->pretty_name;
}

/**
 * gs_os_release_get_cpe_name:
 * @os_release: A #GsOsRelease
 *
 * Gets the pretty name from the os-release parser.
 *
 * Returns: a string, or %NULL
 *
 * Since: 3.22
 **/
const gchar *
gs_os_release_get_cpe_name (GsOsRelease *os_release)
{
	g_return_val_if_fail (GS_IS_OS_RELEASE (os_release), NULL);
	return os_release->cpe_name;
}

/**
 * gs_os_release_get_distro_codename:
 * @os_release: A #GsOsRelease
 *
 * Gets the distro codename from the os-release parser.
 *
 * Returns: a string, or %NULL
 *
 * Since: 3.22
 **/
const gchar *
gs_os_release_get_distro_codename (GsOsRelease *os_release)
{
	g_return_val_if_fail (GS_IS_OS_RELEASE (os_release), NULL);
	return os_release->distro_codename;
}

/**
 * gs_os_release_get_home_url:
 * @os_release: A #GsOsRelease
 *
 * Gets the home URL from the os-release parser.
 *
 * Returns: a string, or %NULL
 *
 * Since: 3.22
 **/
const gchar *
gs_os_release_get_home_url (GsOsRelease *os_release)
{
	g_return_val_if_fail (GS_IS_OS_RELEASE (os_release), NULL);
	return os_release->home_url;
}

/**
 * gs_os_release_get_logo:
 * @os_release: A #GsOsRelease
 *
 * Gets the logo icon name from the os-release parser.
 *
 * Returns: a string, or %NULL
 *
 * Since: 44
 **/
const gchar *
gs_os_release_get_logo (GsOsRelease *os_release)
{
	g_return_val_if_fail (GS_IS_OS_RELEASE (os_release), NULL);
	return os_release->logo;
}

/**
 * gs_os_release_get_vendor_name:
 * @os_release: A #GsOsRelease
 *
 * Gets the vendor name from the os-release parser.
 *
 * Returns: (nullable): a string, or %NULL
 *
 * Since: 46.3
 **/
const gchar *
gs_os_release_get_vendor_name (GsOsRelease *os_release)
{
	g_return_val_if_fail (GS_IS_OS_RELEASE (os_release), NULL);
	return os_release->vendor_name;
}

static void
gs_os_release_finalize (GObject *object)
{
	GsOsRelease *os_release = GS_OS_RELEASE (object);
	g_free (os_release->name);
	g_free (os_release->version);
	g_free (os_release->id);
	g_strfreev (os_release->id_like);
	g_free (os_release->version_id);
	g_free (os_release->pretty_name);
	g_free (os_release->cpe_name);
	g_free (os_release->distro_codename);
	g_free (os_release->home_url);
	g_free (os_release->logo);
	g_free (os_release->vendor_name);

	G_OBJECT_CLASS (gs_os_release_parent_class)->finalize (object);
}

static void
gs_os_release_class_init (GsOsReleaseClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = gs_os_release_finalize;
}

static void
gs_os_release_initable_iface_init (GInitableIface *iface)
{
	iface->init = gs_os_release_initable_init;
}

static void
gs_os_release_init (GsOsRelease *os_release)
{
}

/**
 * gs_os_release_new:
 * @error: a #GError, or %NULL
 *
 * Returns a new reference to a #GsOsRelease. The information may be cached.
 *
 * Returns: (transfer full): A new reference to a #GsOsRelease, or %NULL for error
 *
 * Since: 3.22
 **/
GsOsRelease *
gs_os_release_new (GError **error)
{
	static gsize initialised = 0;
	static GsOsRelease *os_release = NULL;
	static GError *os_release_error = NULL;

	if (g_once_init_enter (&initialised)) {
		os_release = g_initable_new (GS_TYPE_OS_RELEASE, NULL, &os_release_error, NULL);
		g_once_init_leave (&initialised, 1);
	}

	if (os_release != NULL) {
		return g_object_ref (os_release);
	} else {
		g_propagate_error (error, g_error_copy (os_release_error));
		return NULL;
	}
}
