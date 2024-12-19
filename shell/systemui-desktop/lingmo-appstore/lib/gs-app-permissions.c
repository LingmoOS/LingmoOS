/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2022 Red Hat <www.redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-app-permissions
 * @short_description: A representation of the permissions requested by an app
 *
 * #GsAppPermissions is an object to represent the permissions requested by an app.
 *
 * While some common permissions are handled with the #GsAppPermissionsFlags,
 * the object allows more detailed permissions to be represented, such as
 * specific file system path access.
 *
 * Since: 43
 */

#include "config.h"

#include <stdlib.h>

#include <glib.h>
#include <glib-object.h>

#include "gs-app-permissions.h"

#define DOES_NOT_CONTAIN ((guint) ~0)

struct _GsAppPermissions
{
	GObject parent;

	gboolean is_sealed;
	GsAppPermissionsFlags flags;
	GPtrArray *filesystem_read; /* (owner) (nullable) (element-type utf-8) */
	GPtrArray *filesystem_full; /* (owner) (nullable) (element-type utf-8) */
};

G_DEFINE_TYPE (GsAppPermissions, gs_app_permissions, G_TYPE_OBJECT)

static gint
cmp_filename_qsort (gconstpointer item1,
		    gconstpointer item2)
{
	const gchar * const *pitem1 = item1;
	const gchar * const *pitem2 = item2;
	return strcmp (*pitem1, *pitem2);
}

static gint
cmp_filename_bsearch (gconstpointer item1,
		      gconstpointer item2)
{
	return strcmp (item1, item2);
}

static void
gs_app_permissions_finalize (GObject *object)
{
	GsAppPermissions *self = GS_APP_PERMISSIONS (object);

	g_clear_pointer (&self->filesystem_read, g_ptr_array_unref);
	g_clear_pointer (&self->filesystem_full, g_ptr_array_unref);

	G_OBJECT_CLASS (gs_app_permissions_parent_class)->finalize (object);
}

static void
gs_app_permissions_class_init (GsAppPermissionsClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gs_app_permissions_finalize;
}

static void
gs_app_permissions_init (GsAppPermissions *self)
{
}

/**
 * gs_app_permissions_new:
 *
 * Create a new #GsAppPermissions containing the app permissions.
 *
 * Returns: (transfer full): a new #GsAppPermissions
 * Since: 43
 */
GsAppPermissions *
gs_app_permissions_new (void)
{
	return g_object_new (GS_TYPE_APP_PERMISSIONS, NULL);
}

/**
 * gs_app_permissions_seal:
 * @self: a #GsAppPermissions
 *
 * Seal the @self. After being called, no modifications can be
 * done on the @self.
 *
 * Since: 43
 **/
void
gs_app_permissions_seal (GsAppPermissions *self)
{
	g_return_if_fail (GS_IS_APP_PERMISSIONS (self));

	if (self->is_sealed)
		return;

	self->is_sealed = TRUE;

	/* Sort the arrays, which will help with searching */
	if (self->filesystem_read)
		qsort (self->filesystem_read->pdata, self->filesystem_read->len, sizeof (gpointer), cmp_filename_qsort);

	if (self->filesystem_full)
		qsort (self->filesystem_full->pdata, self->filesystem_full->len, sizeof (gpointer), cmp_filename_qsort);
}

/**
 * gs_app_permissions_is_sealed:
 * @self: a #GsAppPermissions
 *
 * Checks whether the @self had been sealed. Once the @self is sealed,
 * no modifications can be made to it.
 *
 * Returns: whether the @self had been sealed
 *
 * Since: 43
 **/
gboolean
gs_app_permissions_is_sealed (GsAppPermissions *self)
{
	g_return_val_if_fail (GS_IS_APP_PERMISSIONS (self), TRUE);

	return self->is_sealed;
}

/**
 * gs_app_permissions_set_flags:
 * @self: a #GsAppPermissions
 * @flags: a #GsAppPermissionsFlags to set
 *
 * Set the permission flags, overwriting any previously set flags.
 * Compare to gs_app_permissions_add_flag() and
 * gs_app_permissions_remove_flag().
 *
 * Since: 43
 */
void
gs_app_permissions_set_flags (GsAppPermissions *self,
			      GsAppPermissionsFlags flags)
{
	g_return_if_fail (GS_IS_APP_PERMISSIONS (self));

	g_assert (!self->is_sealed);

	self->flags = flags;
}

/**
 * gs_app_permissions_get_flags:
 * @self: a #GsAppPermissions
 *
 * Get the permission flags.
 *
 * Returns: the permission flags
 * Since: 43
 */
GsAppPermissionsFlags
gs_app_permissions_get_flags (GsAppPermissions *self)
{
	g_return_val_if_fail (GS_IS_APP_PERMISSIONS (self), GS_APP_PERMISSIONS_FLAGS_NONE);

	return self->flags;
}

/**
 * gs_app_permissions_add_flag:
 * @self: a #GsAppPermissions
 * @flags: a #GsAppPermissionsFlags to add
 *
 * Add the @flags into the already set flags. The @flags cannot be
 * #GS_APP_PERMISSIONS_FLAGS_NONE.
 * To set that use gs_app_permissions_set_flags() instead.
 *
 * In case the current flags contain #GS_APP_PERMISSIONS_FLAGS_NONE, it's
 * automatically unset.
 *
 * Since: 43
 */
void
gs_app_permissions_add_flag (GsAppPermissions *self,
			     GsAppPermissionsFlags flags)
{
	g_return_if_fail (GS_IS_APP_PERMISSIONS (self));
	g_return_if_fail (flags != GS_APP_PERMISSIONS_FLAGS_NONE);

	g_assert (!self->is_sealed);

	self->flags = self->flags | flags;
}

/**
 * gs_app_permissions_remove_flag:
 * @self: a #GsAppPermissions
 * @flags: a #GsAppPermissionsFlags to remove
 *
 * Remove the @flags from the already set flags. The @flags cannot be
 * #GS_APP_PERMISSIONS_FLAGS_NONE.
 * To set this use gs_app_permissions_set_flags() instead.
 *
 * In case the result of the removal would lead to no flag set the #GS_APP_PERMISSIONS_FLAGS_NONE
 * is set automatically.
 *
 * Since: 43
 */
void
gs_app_permissions_remove_flag (GsAppPermissions *self,
				GsAppPermissionsFlags flags)
{
	g_return_if_fail (GS_IS_APP_PERMISSIONS (self));
	g_return_if_fail (flags != GS_APP_PERMISSIONS_FLAGS_NONE);

	g_assert (!self->is_sealed);

	self->flags = (self->flags & (~flags));
}

static guint
app_permissions_get_array_index (GPtrArray *array,
				 const gchar *filename)
{
	g_return_val_if_fail (filename != NULL, DOES_NOT_CONTAIN);

	if (array == NULL)
		return DOES_NOT_CONTAIN;

	for (guint i = 0; i < array->len; i++) {
		const gchar *item = g_ptr_array_index (array, i);
		if (g_strcmp0 (item, filename) == 0)
			return 0;
	}

	return DOES_NOT_CONTAIN;
}

/**
 * gs_app_permissions_add_filesystem_read:
 * @self: a #GsAppPermissions
 * @filename: a filename to access
 *
 * Add @filename as a file to access for read. The @filename
 * can be either a path or a localized pretty name of it, like "Documents".
 * The addition is ignored in case the same @filename is part of
 * the read or full access file names.
 *
 * Since: 43
 */
void
gs_app_permissions_add_filesystem_read (GsAppPermissions *self,
					const gchar *filename)
{
	g_return_if_fail (GS_IS_APP_PERMISSIONS (self));
	g_return_if_fail (filename != NULL);

	g_assert (!self->is_sealed);

	/* Already known */
	if (app_permissions_get_array_index (self->filesystem_read, filename) != DOES_NOT_CONTAIN ||
	    app_permissions_get_array_index (self->filesystem_full, filename) != DOES_NOT_CONTAIN)
		return;

	if (self->filesystem_read == NULL)
		self->filesystem_read = g_ptr_array_new_with_free_func (g_free);

	g_ptr_array_add (self->filesystem_read, g_strdup (filename));
}

/**
 * gs_app_permissions_get_filesystem_read:
 * @self: a #GsAppPermissions
 *
 * Get the list of filesystem file names requested for read access using
 * gs_app_permissions_add_filesystem_read().
 * The array is owned by the @self and should not be modified by any way.
 * It can be %NULL, when no file access was set.
 *
 * Returns: (nullable) (transfer none) (element-type utf-8): an array of
 *    file names requesting read access or %NULL, when none was set.
 *
 * Since: 43
 */
const GPtrArray *
gs_app_permissions_get_filesystem_read (GsAppPermissions *self)
{
	g_return_val_if_fail (GS_IS_APP_PERMISSIONS (self), NULL);

	return self->filesystem_read;
}

static gboolean
array_contains_filename (GPtrArray *array,
			 const gchar *filename)
{
	if (array == NULL)
		return FALSE;

	return bsearch (filename, array->pdata, array->len, sizeof (gpointer), cmp_filename_bsearch) != NULL;
}

/**
 * gs_app_permissions_contains_filesystem_read:
 * @self: a #GsAppPermissions
 * @filename: a file name to search for
 *
 * Checks whether the @filename is included in the filesystem read permissions.
 * This can be called only after the @self is sealed.
 *
 * Returns: whether the @filename is part of the filesystem read permissions
 *
 * Since: 43
 **/
gboolean
gs_app_permissions_contains_filesystem_read (GsAppPermissions *self,
					     const gchar *filename)
{
	g_return_val_if_fail (GS_IS_APP_PERMISSIONS (self), FALSE);
	g_return_val_if_fail (filename != NULL, FALSE);
	g_return_val_if_fail (self->is_sealed, FALSE);

	return array_contains_filename (self->filesystem_read, filename);
}

/**
 * gs_app_permissions_add_filesystem_full:
 * @self: a #GsAppPermissions
 * @filename: a filename to access
 *
 * Add @filename as a file to access for read and write. The @filename
 * can be either a path or a localized pretty name of it, like "Documents".
 * The addition is ignored in case the same @filename is include in the list
 * already. The @filename is removed from the read list, if it's part of it.
 *
 * Since: 43
 */
void
gs_app_permissions_add_filesystem_full (GsAppPermissions *self,
					const gchar *filename)
{
	guint read_index;

	g_return_if_fail (GS_IS_APP_PERMISSIONS (self));
	g_return_if_fail (filename != NULL);

	g_assert (!self->is_sealed);

	/* Already known */
	if (app_permissions_get_array_index (self->filesystem_full, filename) != DOES_NOT_CONTAIN)
		return;

	if (self->filesystem_full == NULL)
		self->filesystem_full = g_ptr_array_new_with_free_func (g_free);

	g_ptr_array_add (self->filesystem_full, g_strdup (filename));

	/* Remove from the read list and free the read list if becomes empty */
	read_index = app_permissions_get_array_index (self->filesystem_read, filename);
	if (read_index != DOES_NOT_CONTAIN) {
		g_ptr_array_remove_index (self->filesystem_read, read_index);
		if (self->filesystem_read->len == 0)
			g_clear_pointer (&self->filesystem_read, g_ptr_array_unref);
	}
}

/**
 * gs_app_permissions_get_filesystem_full:
 * @self: a #GsAppPermissions
 *
 * Get the list of filesystem file names requested for read and write access using
 * gs_app_permissions_add_filesystem_full().
 * The array is owned by the @self and should not be modified by any way.
 * It can be %NULL, when no file access was set.
 *
 * Returns: (nullable) (transfer none) (element-type utf-8): an array of
 *    file names requesting read and write access or %NULL, when none was set.
 *
 * Since: 43
 */
const GPtrArray *
gs_app_permissions_get_filesystem_full (GsAppPermissions *self)
{
	g_return_val_if_fail (GS_IS_APP_PERMISSIONS (self), NULL);

	return self->filesystem_full;
}

/**
 * gs_app_permissions_contains_filesystem_full:
 * @self: a #GsAppPermissions
 * @filename: a file name to search for
 *
 * Checks whether the @filename is included in the filesystem full permissions.
 * This can be called only after the @self is sealed.
 *
 * Returns: whether the @filename is part of the filesystem full permissions
 *
 * Since: 43
 **/
gboolean
gs_app_permissions_contains_filesystem_full (GsAppPermissions *self,
					     const gchar *filename)
{
	g_return_val_if_fail (GS_IS_APP_PERMISSIONS (self), FALSE);
	g_return_val_if_fail (filename != NULL, FALSE);
	g_return_val_if_fail (self->is_sealed, FALSE);

	return array_contains_filename (self->filesystem_full, filename);
}
