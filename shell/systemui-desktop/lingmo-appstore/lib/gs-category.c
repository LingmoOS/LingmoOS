/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2016 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
 * Copyright (C) 2015 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-category
 * @short_description: An category that contains applications
 *
 * This object provides functionality that allows a plugin to create
 * a tree structure of categories that each contain #GsApp's.
 */

#include "config.h"

#include <glib/gi18n.h>

#include "gs-category-private.h"
#include "gs-desktop-data.h"

struct _GsCategory
{
	GObject		 parent_instance;

	const GsDesktopData	*desktop_data;  /* NULL for subcategories */
	const GsDesktopMap	*desktop_map;  /* NULL for parent categories */

	GPtrArray	*desktop_groups;  /* potentially NULL if empty */
	GsCategory	*parent;
	guint		 size;  /* (atomic) */
	GPtrArray	*children;  /* potentially NULL if empty */
};

G_DEFINE_TYPE (GsCategory, gs_category, G_TYPE_OBJECT)

typedef enum {
	PROP_ID = 1,
	PROP_NAME,
	PROP_ICON_NAME,
	PROP_SCORE,
	PROP_PARENT,
	PROP_SIZE,
} GsCategoryProperty;

static GParamSpec *obj_props[PROP_SIZE + 1] = { NULL, };

/**
 * gs_category_to_string:
 * @category: a #GsCategory
 *
 * Returns a string representation of the category
 *
 * Returns: a string
 *
 * Since: 3.22
 **/
gchar *
gs_category_to_string (GsCategory *category)
{
	guint i;
	GString *str = g_string_new (NULL);
	g_string_append_printf (str, "GsCategory[%p]:\n", category);
	g_string_append_printf (str, "  id: %s\n",
				gs_category_get_id (category));
	if (gs_category_get_name (category) != NULL) {
		g_string_append_printf (str, "  name: %s\n",
					gs_category_get_name (category));
	}
	if (gs_category_get_icon_name (category) != NULL) {
		g_string_append_printf (str, "  icon-name: %s\n",
					gs_category_get_icon_name (category));
	}
	g_string_append_printf (str, "  size: %u\n",
				gs_category_get_size (category));
	g_string_append_printf (str, "  desktop-groups: %u\n",
				(category->desktop_groups != NULL) ? category->desktop_groups->len : 0);
	if (category->parent != NULL) {
		g_string_append_printf (str, "  parent: %s\n",
					gs_category_get_id (category->parent));
	}
	g_string_append_printf (str, "  score: %i\n", gs_category_get_score (category));
	if (category->children == NULL || category->children->len == 0) {
		g_string_append_printf (str, "  children: %u\n", 0u);
	} else {
		g_string_append_printf (str, "  children: %u\n", category->children->len);
		for (i = 0; i < category->children->len; i++) {
			GsCategory *child = g_ptr_array_index (category->children, i);
			g_string_append_printf (str, "  - %s\n",
						gs_category_get_id (child));
		}
	}
	return g_string_free (str, FALSE);
}

/**
 * gs_category_get_size:
 * @category: a #GsCategory
 *
 * Returns how many applications the category could contain.
 *
 * NOTE: This may over-estimate the number if duplicate applications are
 * filtered or core applications are not shown.
 *
 * Returns: the number of apps in the category
 *
 * Since: 3.22
 **/
guint
gs_category_get_size (GsCategory *category)
{
	g_return_val_if_fail (GS_IS_CATEGORY (category), 0);

	/* The ‘all’ subcategory is a bit special. */
	if (category->parent != NULL && g_str_equal (gs_category_get_id (category), "all"))
		return gs_category_get_size (category->parent);

	return g_atomic_int_get (&category->size);
}

/**
 * gs_category_set_size:
 * @category: a #GsCategory
 * @size: the number of applications
 *
 * Sets the number of applications in the category.
 * Most plugins do not need to call this function.
 *
 * Since: 3.22
 **/
void
gs_category_set_size (GsCategory *category, guint size)
{
	g_return_if_fail (GS_IS_CATEGORY (category));

	g_atomic_int_set (&category->size, size);
	g_object_notify_by_pspec (G_OBJECT (category), obj_props[PROP_SIZE]);
}

/**
 * gs_category_increment_size:
 * @category: a #GsCategory
 * @value: how many to add
 *
 * Adds @value to the size count.
 *
 * Since: 3.22
 **/
void
gs_category_increment_size (GsCategory *category,
			    guint value)
{
	g_return_if_fail (GS_IS_CATEGORY (category));

	g_atomic_int_add (&category->size, value);
	if (value != 0)
		g_object_notify_by_pspec (G_OBJECT (category), obj_props[PROP_SIZE]);
}

/**
 * gs_category_get_id:
 * @category: a #GsCategory
 *
 * Gets the category ID.
 *
 * Returns: the string, e.g. "other"
 *
 * Since: 3.22
 **/
const gchar *
gs_category_get_id (GsCategory *category)
{
	g_return_val_if_fail (GS_IS_CATEGORY (category), NULL);

	if (category->desktop_data != NULL)
		return category->desktop_data->id;
	else if (category->desktop_map != NULL)
		return category->desktop_map->id;
	g_assert_not_reached ();
}

/**
 * gs_category_get_name:
 * @category: a #GsCategory
 *
 * Gets the category name.
 *
 * Returns: the string, or %NULL
 *
 * Since: 3.22
 **/
const gchar *
gs_category_get_name (GsCategory *category)
{
	const gchar *category_id;

	g_return_val_if_fail (GS_IS_CATEGORY (category), NULL);

	category_id = gs_category_get_id (category);

	/* special case, we don't want translations in the plugins */
	if (g_strcmp0 (category_id, "other") == 0) {
		/* TRANSLATORS: this is where all apps that don't
		 * fit in other groups are put */
		return _("Other");
	}
	if (g_strcmp0 (category_id, "all") == 0) {
		/* TRANSLATORS: this is a subcategory matching all the
		 * different apps in the parent category, e.g. "Games" */
		return C_("Category", "All");
	}
	if (g_strcmp0 (category_id, "featured") == 0) {
		/* TRANSLATORS: this is a subcategory of featured apps */
		return _("Featured");
	}

	/* normal case */
	if (category->desktop_data != NULL) {
		return gettext (category->desktop_data->name);
	} else if (category->desktop_map != NULL) {
		g_autofree gchar *msgctxt = g_strdup_printf ("Menu of %s", category->parent->desktop_data->name);
		return g_dpgettext2 (GETTEXT_PACKAGE, msgctxt, category->desktop_map->name);
	}

	g_assert_not_reached ();
}

/**
 * gs_category_get_icon_name:
 * @category: a #GsCategory
 *
 * Gets the category icon name.
 *
 * Returns: the string, or %NULL
 *
 * Since: 3.22
 **/
const gchar *
gs_category_get_icon_name (GsCategory *category)
{
	const gchar *category_id;

	g_return_val_if_fail (GS_IS_CATEGORY (category), NULL);

	category_id = gs_category_get_id (category);

	/* special case */
	if (g_strcmp0 (category_id, "other") == 0)
		return "emblem-system-symbolic";
	if (g_strcmp0 (category_id, "all") == 0)
		return "emblem-default-symbolic";
	if (g_strcmp0 (category_id, "featured") == 0)
		return "emblem-favorite-symbolic";

	if (category->desktop_data != NULL)
		return category->desktop_data->icon;
	else
		return NULL;
}

/**
 * gs_category_get_score:
 * @category: a #GsCategory
 *
 * Gets if the category score.
 * Important categories may be shown before other categories, or tagged in a
 * different way, for example with color or in a different section.
 *
 * Returns: the string, or %NULL
 *
 * Since: 3.22
 **/
gint
gs_category_get_score (GsCategory *category)
{
	g_return_val_if_fail (GS_IS_CATEGORY (category), FALSE);

	if (category->desktop_data != NULL)
		return category->desktop_data->score;
	else
		return 0;
}

/**
 * gs_category_get_desktop_groups:
 * @category: a #GsCategory
 *
 * Gets the list of AppStream groups for the category.
 *
 * Returns: (element-type utf8) (transfer none): An array
 *
 * Since: 3.22
 **/
GPtrArray *
gs_category_get_desktop_groups (GsCategory *category)
{
	g_return_val_if_fail (GS_IS_CATEGORY (category), NULL);

	if (category->desktop_groups == NULL)
		category->desktop_groups = g_ptr_array_new_with_free_func (g_free);

	return category->desktop_groups;
}

/**
 * gs_category_has_desktop_group:
 * @category: a #GsCategory
 * @desktop_group: a group of categories found in AppStream, e.g. "AudioVisual::Player"
 *
 * Finds out if the category has the specific AppStream desktop group.
 *
 * Returns: %TRUE if found, %FALSE otherwise
 *
 * Since: 3.22
 **/
gboolean
gs_category_has_desktop_group (GsCategory *category, const gchar *desktop_group)
{
	guint i;

	g_return_val_if_fail (GS_IS_CATEGORY (category), FALSE);
	g_return_val_if_fail (desktop_group != NULL, FALSE);

	if (category->desktop_groups == NULL)
		return FALSE;

	for (i = 0; i < category->desktop_groups->len; i++) {
		const gchar *tmp = g_ptr_array_index (category->desktop_groups, i);
		if (g_strcmp0 (tmp, desktop_group) == 0)
			return TRUE;
	}
	return FALSE;
}

/*
 * gs_category_add_desktop_group:
 * @category: a #GsCategory
 * @desktop_group: a group of categories found in AppStream, e.g. "AudioVisual::Player"
 *
 * Adds a desktop group to the category.
 * A desktop group is a set of category strings that all must exist.
 *
 * Since: 3.22
 */
static void
gs_category_add_desktop_group (GsCategory *category, const gchar *desktop_group)
{
	g_return_if_fail (GS_IS_CATEGORY (category));
	g_return_if_fail (desktop_group != NULL);

	/* add if not already found, and lazily create the groups array
	 * (since it’s only needed in child categories) */
	if (gs_category_has_desktop_group (category, desktop_group))
		return;
	if (category->desktop_groups == NULL)
		category->desktop_groups = g_ptr_array_new_with_free_func (g_free);
	g_ptr_array_add (category->desktop_groups, g_strdup (desktop_group));
}

/**
 * gs_category_find_child:
 * @category: a #GsCategory
 * @id: a category ID, e.g. "other"
 *
 * Find a child category with a specific ID.
 *
 * Returns: (transfer none): the #GsCategory, or %NULL
 *
 * Since: 3.22
 **/
GsCategory *
gs_category_find_child (GsCategory *category, const gchar *id)
{
	GsCategory *tmp;
	guint i;

	if (category->children == NULL)
		return NULL;

	/* find the subcategory */
	for (i = 0; i < category->children->len; i++) {
		tmp = GS_CATEGORY (g_ptr_array_index (category->children, i));
		if (g_strcmp0 (id, gs_category_get_id (tmp)) == 0)
			return tmp;
	}
	return NULL;
}

/**
 * gs_category_get_parent:
 * @category: a #GsCategory
 *
 * Gets the parent category.
 *
 * Returns: the #GsCategory or %NULL
 *
 * Since: 3.22
 **/
GsCategory *
gs_category_get_parent (GsCategory *category)
{
	g_return_val_if_fail (GS_IS_CATEGORY (category), NULL);
	return category->parent;
}

/**
 * gs_category_get_children:
 * @category: a #GsCategory
 *
 * Gets the list if children for a category.
 *
 * Return value: (element-type GsApp) (transfer none): A list of children
 *
 * Since: 3.22
 **/
GPtrArray *
gs_category_get_children (GsCategory *category)
{
	g_return_val_if_fail (GS_IS_CATEGORY (category), NULL);

	if (category->children == NULL)
		category->children = g_ptr_array_new_with_free_func ((GDestroyNotify) g_object_unref);

	return category->children;
}

/*
 * gs_category_add_child:
 * @category: a #GsCategory
 * @subcategory: a #GsCategory
 *
 * Adds a child category to a parent category.
 *
 * Since: 3.22
 */
static void
gs_category_add_child (GsCategory *category, GsCategory *subcategory)
{
	g_return_if_fail (GS_IS_CATEGORY (category));
	g_return_if_fail (GS_IS_CATEGORY (subcategory));

	/* lazily create the array to save memory in subcategories, which don’t
	 * recursively have children */
	if (category->children == NULL)
		category->children = g_ptr_array_new_with_free_func ((GDestroyNotify) g_object_unref);

	/* FIXME: do we need this? */
	subcategory->parent = category;
	g_object_add_weak_pointer (G_OBJECT (subcategory->parent),
				   (gpointer *) &subcategory->parent);

	g_ptr_array_add (category->children,
			 g_object_ref (subcategory));
}

static gchar *
gs_category_get_sort_key (GsCategory *category)
{
	guint sort_order = 5;
	if (g_strcmp0 (gs_category_get_id (category), "featured") == 0)
		sort_order = 0;
	else if (g_strcmp0 (gs_category_get_id (category), "all") == 0)
		sort_order = 2;
	else if (g_strcmp0 (gs_category_get_id (category), "other") == 0)
		sort_order = 9;
	return g_strdup_printf ("%u:%s",
				sort_order,
				gs_category_get_name (category));
}

static gint
gs_category_sort_children_cb (gconstpointer a, gconstpointer b)
{
	GsCategory *ca = GS_CATEGORY (*(GsCategory **) a);
	GsCategory *cb = GS_CATEGORY (*(GsCategory **) b);
	g_autofree gchar *id_a = gs_category_get_sort_key (ca);
	g_autofree gchar *id_b = gs_category_get_sort_key (cb);
	return g_strcmp0 (id_a, id_b);
}

/**
 * gs_category_sort_children:
 * @category: a #GsCategory
 *
 * Sorts the list of children.
 *
 * Since: 3.22
 **/
void
gs_category_sort_children (GsCategory *category)
{
	if (category->children == NULL)
		return;

	g_ptr_array_sort (category->children,
			  gs_category_sort_children_cb);
}

static void
gs_category_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	GsCategory *self = GS_CATEGORY (object);

	switch ((GsCategoryProperty) prop_id) {
	case PROP_ID:
		g_value_set_string (value, gs_category_get_id (self));
		break;
	case PROP_NAME:
		g_value_set_string (value, gs_category_get_name (self));
		break;
	case PROP_ICON_NAME:
		g_value_set_string (value, gs_category_get_icon_name (self));
		break;
	case PROP_SCORE:
		g_value_set_int (value, gs_category_get_score (self));
		break;
	case PROP_PARENT:
		g_value_set_object (value, self->parent);
		break;
	case PROP_SIZE:
		g_value_set_uint (value, gs_category_get_size (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_category_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	GsCategory *self = GS_CATEGORY (object);

	switch ((GsCategoryProperty) prop_id) {
	case PROP_ID:
	case PROP_NAME:
	case PROP_ICON_NAME:
	case PROP_SCORE:
	case PROP_PARENT:
		/* Read only */
		g_assert_not_reached ();
		break;
	case PROP_SIZE:
		gs_category_set_size (self, g_value_get_uint (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_category_finalize (GObject *object)
{
	GsCategory *category = GS_CATEGORY (object);

	if (category->parent != NULL)
		g_object_remove_weak_pointer (G_OBJECT (category->parent),
		                              (gpointer *) &category->parent);
	g_clear_pointer (&category->children, g_ptr_array_unref);
	g_clear_pointer (&category->desktop_groups, g_ptr_array_unref);

	G_OBJECT_CLASS (gs_category_parent_class)->finalize (object);
}

static void
gs_category_class_init (GsCategoryClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gs_category_get_property;
	object_class->set_property = gs_category_set_property;
	object_class->finalize = gs_category_finalize;

	/**
	 * GsCategory:id:
	 *
	 * A machine readable identifier for the category. Must be non-empty
	 * and in a valid format to be a
	 * [desktop category ID](https://specifications.freedesktop.org/menu-spec/latest/).
	 *
	 * Since: 40
	 */
	obj_props[PROP_ID] =
		g_param_spec_string ("id", NULL, NULL,
				     NULL,
				     G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * GsCategory:name:
	 *
	 * Human readable name for the category.
	 *
	 * Since: 40
	 */
	obj_props[PROP_NAME] =
		g_param_spec_string ("name", NULL, NULL,
				     NULL,
				     G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * GsCategory:icon-name: (nullable)
	 *
	 * Name of the icon to use for the category, or %NULL if none is set.
	 *
	 * Since: 40
	 */
	obj_props[PROP_ICON_NAME] =
		g_param_spec_string ("icon-name", NULL, NULL,
				     NULL,
				     G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * GsCategory:score:
	 *
	 * Score for sorting the category. Lower numeric values indicate more
	 * important categories.
	 *
	 * Since: 40
	 */
	obj_props[PROP_SCORE] =
		g_param_spec_int ("score", NULL, NULL,
				  G_MININT, G_MAXINT, 0,
				  G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * GsCategory:parent: (nullable)
	 *
	 * The parent #GsCategory, or %NULL if this category is at the top
	 * level.
	 *
	 * Since: 40
	 */
	obj_props[PROP_PARENT] =
		g_param_spec_object ("parent", NULL, NULL,
				     GS_TYPE_CATEGORY,
				     G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * GsCategory:size:
	 *
	 * Number of apps in this category, including apps in its subcategories.
	 *
	 * This has to be initialised externally to the #GsCategory by calling
	 * gs_category_increment_size().
	 *
	 * Since: 40
	 */
	obj_props[PROP_SIZE] =
		g_param_spec_uint ("size", NULL, NULL,
				   0, G_MAXUINT, 0,
				   G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);
}

static void
gs_category_init (GsCategory *category)
{
}

/**
 * gs_category_new_for_desktop_data:
 * @data: data for the category, which must be static and constant
 *
 * Create a new #GsCategory instance which wraps the desktop category
 * information in @data. Where possible, the static data will be reused, so
 * @data must be static and constant across the lifetime of the process.
 *
 * Returns: (transfer full): a new #GsCategory wrapping @data
 * Since: 40
 */
GsCategory *
gs_category_new_for_desktop_data (const GsDesktopData *data)
{
	g_autoptr(GsCategory) category = NULL;
	GsCategory *subcategory_all = NULL;

	/* parent category */
	category = g_object_new (GS_TYPE_CATEGORY, NULL);
	category->desktop_data = data;

	/* add subcategories */
	for (gsize j = 0; data->mapping[j].id != NULL; j++) {
		const GsDesktopMap *map = &data->mapping[j];
		g_autoptr(GsCategory) sub = g_object_new (GS_TYPE_CATEGORY, NULL);
		sub->desktop_map = map;
		for (gsize k = 0; map->fdo_cats[k] != NULL; k++)
			gs_category_add_desktop_group (sub, map->fdo_cats[k]);
		gs_category_add_child (category, sub);

		if (g_str_equal (map->id, "all"))
			subcategory_all = sub;
	}

	/* set up the ‘all’ subcategory specially, adding all the desktop groups
	 * from all other child categories to it */
	if (subcategory_all != NULL) {
		g_assert (category->children != NULL);

		for (guint i = 0; i < category->children->len; i++) {
			GPtrArray *desktop_groups;
			GsCategory *child;

			/* ignore the all category */
			child = g_ptr_array_index (category->children, i);
			if (child == subcategory_all)
				continue;

			/* add all desktop groups */
			desktop_groups = gs_category_get_desktop_groups (child);
			for (guint j = 0; j < desktop_groups->len; j++) {
				const gchar *tmp = g_ptr_array_index (desktop_groups, j);
				gs_category_add_desktop_group (subcategory_all, tmp);
			}
		}
	}

	return g_steal_pointer (&category);
}
