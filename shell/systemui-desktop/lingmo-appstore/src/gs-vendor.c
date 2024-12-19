/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2008 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2015 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include "gs-vendor.h"

struct _GsVendor
{
	GObject				  parent_instance;

	GKeyFile			 *file;
};

G_DEFINE_TYPE (GsVendor, gs_vendor, G_TYPE_OBJECT)

#ifdef HAVE_PACKAGEKIT
static const gchar *
gs_vendor_type_to_string (GsVendorUrlType type)
{
	if (type == GS_VENDOR_URL_TYPE_CODEC)
		return "CodecUrl";
	if (type == GS_VENDOR_URL_TYPE_FONT)
		return "FontUrl";
	if (type == GS_VENDOR_URL_TYPE_MIME)
		return "MimeUrl";
	if (type == GS_VENDOR_URL_TYPE_HARDWARE)
		return "HardwareUrl";
	return "DefaultUrl";
}
#endif

gchar *
gs_vendor_get_not_found_url (GsVendor *vendor, GsVendorUrlType type)
{
#ifdef HAVE_PACKAGEKIT
	const gchar *key;
	gchar *url = NULL;

	/* get data */
	key = gs_vendor_type_to_string (type);
	url = g_key_file_get_string (vendor->file, "PackagesNotFound", key, NULL);

	/* none is a special value */
	if (g_strcmp0 (url, "none") == 0) {
		g_free (url);
		url = NULL;
	}

	/* got a valid URL */
	if (url != NULL)
		goto out;

	/* default has no fallback */
	if (type == GS_VENDOR_URL_TYPE_DEFAULT)
		goto out;

	/* get fallback data */
	g_debug ("using fallback");
	key = gs_vendor_type_to_string (GS_VENDOR_URL_TYPE_DEFAULT);
	url = g_key_file_get_string (vendor->file, "PackagesNotFound", key, NULL);

	/* none is a special value */
	if (g_strcmp0 (url, "none") == 0) {
		g_free (url);
		url = NULL;
	}
out:
	g_debug ("url=%s", url);
	return url;
#else
	return NULL;
#endif
}

static void
gs_vendor_init (GsVendor *vendor)
{
#ifdef HAVE_PACKAGEKIT
	g_autoptr(GError) local_error = NULL;
	const gchar *fn = "/etc/PackageKit/Vendor.conf";
	gboolean ret;

	vendor->file = g_key_file_new ();
	ret = g_key_file_load_from_file (vendor->file, fn, G_KEY_FILE_NONE, &local_error);
	if (!ret && local_error && !g_error_matches (local_error, G_FILE_ERROR, G_FILE_ERROR_NOENT))
		g_warning ("Failed to read '%s': %s", fn, local_error->message);
#endif
}

static void
gs_vendor_finalize (GObject *object)
{
	GsVendor *vendor = GS_VENDOR (object);

	if (vendor->file != NULL)
		g_key_file_free (vendor->file);

	G_OBJECT_CLASS (gs_vendor_parent_class)->finalize (object);
}

static void
gs_vendor_class_init (GsVendorClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = gs_vendor_finalize;
}

/**
 * gs_vendor_new:
 *
 * Return value: a new GsVendor object.
 **/
GsVendor *
gs_vendor_new (void)
{
	GsVendor *vendor;
	vendor = g_object_new (GS_TYPE_VENDOR, NULL);
	return GS_VENDOR (vendor);
}

