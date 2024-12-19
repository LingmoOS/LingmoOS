/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Endless OS Foundation, Inc
 *
 * Author: Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-icon
 * @short_description: Utilities for handling #GIcons
 *
 * This file provides several utilities for creating and handling #GIcon
 * instances. #GIcon is used for representing icon sources throughout
 * lingmo-appstore, as it has low memory overheads, and allows the most
 * appropriate icon data to be loaded when it’s needed to be used in a UI.
 *
 * lingmo-appstore uses various classes which implement #GIcon, mostly the
 * built-in ones provided by GIO, but also #GsRemoteIcon. All of them are tagged
 * with `width` and `height` metadata (when that data was available at
 * construction time). See gs_icon_get_width().
 *
 * Since: 40
 */

#include "config.h"

#include <appstream.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gio/gio.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "gs-icon.h"
#include "gs-remote-icon.h"

/**
 * gs_icon_get_width:
 * @icon: a #GIcon
 *
 * Get the width of an icon, if it was attached as metadata when the #GIcon was
 * created from an #AsIcon.
 *
 * Returns: width of the icon (in device pixels), or `0` if unknown
 * Since: 40
 */
guint
gs_icon_get_width (GIcon *icon)
{
	g_return_val_if_fail (G_IS_ICON (icon), 0);

	return GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (icon), "width"));
}

/**
 * gs_icon_set_width:
 * @icon: a #GIcon
 * @width: width of the icon, in device pixels
 *
 * Set the width of an icon. See gs_icon_get_width().
 *
 * Since: 40
 */
void
gs_icon_set_width (GIcon *icon,
                   guint  width)
{
	g_return_if_fail (G_IS_ICON (icon));

	g_object_set_data (G_OBJECT (icon), "width", GUINT_TO_POINTER (width));
}

/**
 * gs_icon_get_height:
 * @icon: a #GIcon
 *
 * Get the height of an icon, if it was attached as metadata when the #GIcon was
 * created from an #AsIcon.
 *
 * Returns: height of the icon (in device pixels), or `0` if unknown
 * Since: 40
 */
guint
gs_icon_get_height (GIcon *icon)
{
	g_return_val_if_fail (G_IS_ICON (icon), 0);

	return GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (icon), "height"));
}

/**
 * gs_icon_set_height:
 * @icon: a #GIcon
 * @height: height of the icon, in device pixels
 *
 * Set the height of an icon. See gs_icon_get_height().
 *
 * Since: 40
 */
void
gs_icon_set_height (GIcon *icon,
                    guint  height)
{
	g_return_if_fail (G_IS_ICON (icon));

	g_object_set_data (G_OBJECT (icon), "height", GUINT_TO_POINTER (height));
}

/**
 * gs_icon_get_scale:
 * @icon: a #GIcon
 *
 * Get the scale of an icon, if it was attached as metadata when the #GIcon was
 * created from an #AsIcon.
 *
 * See gtk_widget_get_scale_factor() for more information about scales.
 *
 * Returns: scale of the icon, or `1` if unknown; guaranteed to always be
 *     greater than or equal to 1
 * Since: 40
 */
guint
gs_icon_get_scale (GIcon *icon)
{
	g_return_val_if_fail (G_IS_ICON (icon), 0);

	return MAX (1, GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (icon), "scale")));
}

/**
 * gs_icon_set_scale:
 * @icon: a #GIcon
 * @scale: scale of the icon, which must be greater than or equal to 1
 *
 * Set the scale of an icon. See gs_icon_get_scale().
 *
 * Since: 40
 */
void
gs_icon_set_scale (GIcon *icon,
                   guint  scale)
{
	g_return_if_fail (G_IS_ICON (icon));
	g_return_if_fail (scale >= 1);

	g_object_set_data (G_OBJECT (icon), "scale", GUINT_TO_POINTER (scale));
}

static GIcon *
gs_icon_load_local (AsIcon *icon)
{
	const gchar *filename = as_icon_get_filename (icon);
	g_autoptr(GFile) file = NULL;

	if (filename == NULL)
		return NULL;

	file = g_file_new_for_path (filename);
	return g_file_icon_new (file);
}

static GIcon *
gs_icon_load_stock (AsIcon *icon)
{
	const gchar *name = as_icon_get_name (icon);

	if (name == NULL)
		return NULL;

	return g_themed_icon_new (name);
}

static GIcon *
gs_icon_load_remote (AsIcon *icon)
{
	const gchar *url = as_icon_get_url (icon);

	if (url == NULL)
		return NULL;

	/* Load local files directly. */
	if (g_str_has_prefix (url, "file:")) {
		g_autoptr(GFile) file = g_file_new_for_path (url + strlen ("file:"));
		return g_file_icon_new (file);
	}

	/* Only HTTP and HTTPS are supported. */
	if (!g_str_has_prefix (url, "http:") &&
	    !g_str_has_prefix (url, "https:"))
		return NULL;

	return gs_remote_icon_new (url);
}

static GIcon *
gs_icon_load_cached (AsIcon *icon)
{
	const gchar *filename = as_icon_get_filename (icon);
	const gchar *name = as_icon_get_name (icon);
	g_autofree gchar *name_allocated = NULL;
	g_autofree gchar *full_filename = NULL;
	g_autoptr(GFile) file = NULL;

	if (filename == NULL || name == NULL)
		return NULL;

	/* FIXME: Work around https://github.com/hughsie/appstream-glib/pull/390
	 * where appstream files generated with appstream-builder from
	 * appstream-glib, with its hidpi option enabled, will contain an
	 * unnecessary size subdirectory in the icon name. */
	if (g_str_has_prefix (name, "64x64/"))
		name = name_allocated = g_strdup (name + strlen ("64x64/"));
	else if (g_str_has_prefix (name, "128x128/"))
		name = name_allocated = g_strdup (name + strlen ("128x128/"));

	if (!g_str_has_suffix (filename, name)) {
		/* Spec: https://www.freedesktop.org/software/appstream/docs/sect-AppStream-IconCache.html#spec-iconcache-location */
		if (as_icon_get_scale (icon) <= 1) {
			full_filename = g_strdup_printf ("%s/%ux%u/%s",
							 filename,
							 as_icon_get_width (icon),
							 as_icon_get_height (icon),
							 name);
		} else {
			full_filename = g_strdup_printf ("%s/%ux%u@%u/%s",
							 filename,
							 as_icon_get_width (icon),
							 as_icon_get_height (icon),
							 as_icon_get_scale (icon),
							 name);
		}

		filename = full_filename;
	}

	file = g_file_new_for_path (filename);
	return g_file_icon_new (file);
}

/**
 * gs_icon_new_for_appstream_icon:
 * @appstream_icon: an #AsIcon
 *
 * Create a new #GIcon representing the given #AsIcon. The actual type of the
 * returned icon will vary depending on the #AsIconKind of @appstream_icon.
 *
 * If the width or height of the icon are set on the #AsIcon, they are stored
 * as the `width` and `height` data associated with the returned object, using
 * g_object_set_data().
 *
 * This can fail (and return %NULL) if the @appstream_icon has invalid or
 * missing properties.
 *
 * Returns: (transfer full) (nullable): the #GIcon, or %NULL
 * Since: 40
 */
GIcon *
gs_icon_new_for_appstream_icon (AsIcon *appstream_icon)
{
	g_autoptr(GIcon) icon = NULL;

	g_return_val_if_fail (AS_IS_ICON (appstream_icon), NULL);

	switch (as_icon_get_kind (appstream_icon)) {
	case AS_ICON_KIND_LOCAL:
		icon = gs_icon_load_local (appstream_icon);
		break;
	case AS_ICON_KIND_STOCK:
		icon = gs_icon_load_stock (appstream_icon);
		break;
	case AS_ICON_KIND_REMOTE:
		icon = gs_icon_load_remote (appstream_icon);
		break;
	case AS_ICON_KIND_CACHED:
		icon = gs_icon_load_cached (appstream_icon);
		break;
	default:
		g_assert_not_reached ();
	}

	if (icon == NULL) {
		g_debug ("Error creating GIcon for AsIcon of kind %s",
			 as_icon_kind_to_string (as_icon_get_kind (appstream_icon)));
		return NULL;
	}

	/* Store the width, height and scale as associated metadata (if
	 * available) so that #GsApp can sort icons by size and return the most
	 * appropriately sized one in gs_app_get_icon_by_size().
	 *
	 * FIXME: Ideally we’d store these as properties on the objects, but
	 * GIO currently doesn’t allow subclassing of its #GIcon classes. If we
	 * were to implement a #GLoadableIcon with these as properties, all the
	 * fast paths in GTK for loading icon data (particularly named icons)
	 * would be ignored.
	 *
	 * Storing the width and height as associated metadata means GObject
	 * creates a hash table for each GIcon object. This is a waste of memory
	 * (compared to using properties), but seems like the least-worst
	 * option.
	 *
	 * See https://github.com/LingmoOS/glib/-/issues/2345
	 */
	if (as_icon_get_width (appstream_icon) != 0 || as_icon_get_height (appstream_icon) != 0) {
		gs_icon_set_width (icon, as_icon_get_width (appstream_icon));
		gs_icon_set_height (icon, as_icon_get_height (appstream_icon));
	}
	if (as_icon_get_scale (appstream_icon) != 0)
		gs_icon_set_scale (icon, as_icon_get_scale (appstream_icon));

	return g_steal_pointer (&icon);
}
