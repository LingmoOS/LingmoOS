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
 * SECTION:gs-category-manager
 * @short_description: A container to store #GsCategory instances in
 *
 * #GsCategoryManager is a container object which stores #GsCategory instances,
 * so that they can be consistently reused by other code, without creating
 * multiple #GsCategory instances for the same category ID.
 *
 * It is intended to be used as a singleton, and typically accessed by calling
 * gs_plugin_loader_get_category_manager().
 *
 * Since: 40
 */

#include "config.h"

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>

#include "gs-category-manager.h"
#include "gs-desktop-data.h"

struct _GsCategoryManager
{
	GObject		 parent;

	/* Array of #GsCategory instances corresponding to the entries in gs_desktop_get_data()
	 * The +1 is for a NULL terminator */
	GsCategory	*categories[GS_DESKTOP_DATA_N_ENTRIES + 1];
};

G_DEFINE_TYPE (GsCategoryManager, gs_category_manager, G_TYPE_OBJECT)

static void
gs_category_manager_dispose (GObject *object)
{
	GsCategoryManager *self = GS_CATEGORY_MANAGER (object);

	for (gsize i = 0; i < G_N_ELEMENTS (self->categories); i++)
		g_clear_object (&self->categories[i]);

	G_OBJECT_CLASS (gs_category_manager_parent_class)->dispose (object);
}

static void
gs_category_manager_class_init (GsCategoryManagerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = gs_category_manager_dispose;
}

static void
gs_category_manager_init (GsCategoryManager *self)
{
	const GsDesktopData *msdata;

	/* Set up the category data, and check our expectations about the length
	 * of gs_desktop_get_data() match reality. */
	msdata = gs_desktop_get_data ();
	for (gsize i = 0; msdata[i].id != NULL; i++) {
		g_assert (i < G_N_ELEMENTS (self->categories) - 1);
		self->categories[i] = gs_category_new_for_desktop_data (&msdata[i]);
	}

	g_assert (self->categories[G_N_ELEMENTS (self->categories) - 2] != NULL);
	g_assert (self->categories[G_N_ELEMENTS (self->categories) - 1] == NULL);
}

/**
 * gs_category_manager_new:
 *
 * Create a new #GsCategoryManager. It will contain all the categories, but
 * their sizes will not be set until gs_category_increment_size() is called
 * on them.
 *
 * Returns: (transfer full): a new #GsCategoryManager
 * Since: 40
 */
GsCategoryManager *
gs_category_manager_new (void)
{
	return g_object_new (GS_TYPE_CATEGORY_MANAGER, NULL);
}

/**
 * gs_category_manager_lookup:
 * @self: a #GsCategoryManager
 * @id: ID of the category to look up
 *
 * Look up a category by its ID. If the category is not found, %NULL is
 * returned.
 *
 * Returns: (transfer full) (nullable): the #GsCategory, or %NULL
 * Since: 40
 */
GsCategory *
gs_category_manager_lookup (GsCategoryManager *self,
                            const gchar       *id)
{
	g_return_val_if_fail (GS_IS_CATEGORY_MANAGER (self), NULL);
	g_return_val_if_fail (id != NULL && *id != '\0', NULL);

	/* There are only on the order of 10 categories, so this is quick */
	for (gsize i = 0; i < G_N_ELEMENTS (self->categories) - 1; i++) {
		if (g_str_equal (gs_category_get_id (self->categories[i]), id))
			return g_object_ref (self->categories[i]);
	}

	return NULL;
}

/**
 * gs_category_manager_get_categories:
 * @self: a #GsCategoryManager
 * @out_n_categories: (optional) (out caller-allocates): return location for
 *     the number of categories in the return value, or %NULL to ignore
 *
 * Get the full list of categories from the category manager. The returned array
 * is %NULL terminated and guaranteed to be non-%NULL (although it may be
 * empty).
 *
 * If @out_n_categories is provided, it will be set to the number of #GsCategory
 * objects in the return value, not including the %NULL terminator.
 *
 * Returns: (array length=out_n_categories) (transfer none) (not nullable): the
 *     categories; do not free this memory
 * Since: 40
 */
GsCategory * const *
gs_category_manager_get_categories (GsCategoryManager *self,
                                    gsize             *out_n_categories)
{
	g_return_val_if_fail (GS_IS_CATEGORY_MANAGER (self), NULL);

	if (out_n_categories != NULL)
		*out_n_categories = G_N_ELEMENTS (self->categories) - 1;

	return self->categories;
}
