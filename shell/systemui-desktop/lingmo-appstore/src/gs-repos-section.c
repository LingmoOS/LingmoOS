/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Red Hat <www.redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <gio/gio.h>

#include "gs-repo-row.h"
#include "gs-repos-section.h"

struct _GsReposSection
{
	AdwPreferencesGroup	 parent_instance;
	GtkWidget		*title;
	GtkListBox		*list;
	gchar			*sort_key;
	gboolean		 always_allow_enable_disable;
	gboolean		 related_loaded;
};

G_DEFINE_TYPE (GsReposSection, gs_repos_section, ADW_TYPE_PREFERENCES_GROUP)

typedef enum {
	PROP_RELATED_LOADED = 1,
} GsReposSectionProperty;

enum {
	SIGNAL_REMOVE_CLICKED,
	SIGNAL_SWITCH_CLICKED,
	SIGNAL_LAST
};

static GParamSpec *obj_props[PROP_RELATED_LOADED + 1] = { NULL, };
static guint signals [SIGNAL_LAST] = { 0 };

static void
repo_remove_clicked_cb (GsRepoRow *row,
			GsReposSection *section)
{
	g_signal_emit (section, signals[SIGNAL_REMOVE_CLICKED], 0, row);
}

static void
repo_switch_clicked_cb (GsRepoRow *row,
			GsReposSection *section)
{
	g_signal_emit (section, signals[SIGNAL_SWITCH_CLICKED], 0, row);
}

static void
gs_repos_section_row_activated_cb (GtkListBox *box,
				   GtkListBoxRow *row,
				   gpointer user_data)
{
	GsReposSection *section = user_data;
	g_return_if_fail (GS_IS_REPOS_SECTION (section));
	gs_repo_row_emit_switch_clicked (GS_REPO_ROW (row));
}

static gchar *
_get_app_sort_key (GsApp *app)
{
	if (gs_app_get_name (app) != NULL)
		return gs_utils_sort_key (gs_app_get_name (app));

	return NULL;
}

static gint
_list_sort_func (GtkListBoxRow *a, GtkListBoxRow *b, gpointer user_data)
{
	GsApp *a1 = gs_repo_row_get_repo (GS_REPO_ROW (a));
	GsApp *a2 = gs_repo_row_get_repo (GS_REPO_ROW (b));
	g_autofree gchar *key1 = _get_app_sort_key (a1);
	g_autofree gchar *key2 = _get_app_sort_key (a2);

	return g_strcmp0 (key1, key2);
}

static void
gs_repos_section_get_property (GObject *object,
			       guint prop_id,
			       GValue *value,
			       GParamSpec *pspec)
{
	GsReposSection *self = GS_REPOS_SECTION (object);

	switch ((GsReposSectionProperty) prop_id) {
	case PROP_RELATED_LOADED:
		g_value_set_boolean (value, gs_repos_section_get_related_loaded (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_repos_section_set_property (GObject *object,
			       guint prop_id,
			       const GValue *value,
			       GParamSpec *pspec)
{
	GsReposSection *self = GS_REPOS_SECTION (object);

	switch ((GsReposSectionProperty) prop_id) {
	case PROP_RELATED_LOADED:
		gs_repos_section_set_related_loaded (self, g_value_get_boolean (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_repos_section_finalize (GObject *object)
{
	GsReposSection *self = GS_REPOS_SECTION (object);

	g_free (self->sort_key);

	G_OBJECT_CLASS (gs_repos_section_parent_class)->finalize (object);
}

static void
gs_repos_section_class_init (GsReposSectionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gs_repos_section_get_property;
	object_class->set_property = gs_repos_section_set_property;
	object_class->finalize = gs_repos_section_finalize;

	/**
	 * GsReposSection:related-loaded:
	 *
	 * Whether the related apps for this repo section have been
	 * successfully loaded. If so, the number of apps/installed
	 * apps is shown in each row.
	 *
	 * Since: 45
	 */
	obj_props[PROP_RELATED_LOADED] =
		g_param_spec_boolean ("related-loaded", NULL, NULL,
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);

	signals [SIGNAL_REMOVE_CLICKED] =
		g_signal_new ("remove-clicked",
		              G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
		              0,
		              NULL, NULL, g_cclosure_marshal_VOID__OBJECT,
		              G_TYPE_NONE, 1, GS_TYPE_REPO_ROW);

	signals [SIGNAL_SWITCH_CLICKED] =
		g_signal_new ("switch-clicked",
		              G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
		              0,
		              NULL, NULL, g_cclosure_marshal_VOID__OBJECT,
		              G_TYPE_NONE, 1, GS_TYPE_REPO_ROW);
}

static void
gs_repos_section_init (GsReposSection *self)
{
	self->list = GTK_LIST_BOX (gtk_list_box_new ());
	g_object_set (G_OBJECT (self->list),
		      "visible", TRUE,
		      "selection-mode", GTK_SELECTION_NONE,
		      NULL);
	gtk_list_box_set_sort_func (self->list, _list_sort_func, self, NULL);

	gtk_widget_add_css_class (GTK_WIDGET (self->list), "boxed-list");

	adw_preferences_group_add (ADW_PREFERENCES_GROUP (self), GTK_WIDGET (self->list));

	g_signal_connect (self->list, "row-activated",
			  G_CALLBACK (gs_repos_section_row_activated_cb), self);
}

/*
 * gs_repos_section_new:
 * @always_allow_enable_disable: always allow enable/disable of the repos in this section
 *
 * Creates a new #GsReposSection. @always_allow_enable_disable is passed to each
 * #GsRepoRow.
 *
 * The @always_allow_enable_disable, when %TRUE, means that every repo in this section
 * can be enabled/disabled by the user, if supported by the related plugin, regardless
 * of the other heuristics, which can avoid the repo enable/disable.
 *
 * Returns: (transfer full): a newly created #GsReposSection
 */
GtkWidget *
gs_repos_section_new (gboolean always_allow_enable_disable)
{
	GsReposSection *self;

	self = g_object_new (GS_TYPE_REPOS_SECTION, NULL);

	self->always_allow_enable_disable = always_allow_enable_disable;

	return GTK_WIDGET (self);
}

void
gs_repos_section_add_repo (GsReposSection *self,
			   GsApp *repo)
{
	GtkWidget *row;

	g_return_if_fail (GS_IS_REPOS_SECTION (self));
	g_return_if_fail (GS_IS_APP (repo));

	/* Derive the sort key from the repository. All repositories of the same kind
	   should have set the same sort key. It's because there's no other way to provide
	   the section sort key by the plugin without breaking the abstraction. */
	if (!self->sort_key)
		self->sort_key = g_strdup (gs_app_get_metadata_item (repo, "LingmoAppstore::SortKey"));

	row = gs_repo_row_new (repo, self->always_allow_enable_disable);
	g_object_bind_property (self, "related-loaded",
				row, "related-loaded",
				G_BINDING_SYNC_CREATE);
	g_signal_connect (row, "remove-clicked",
	                  G_CALLBACK (repo_remove_clicked_cb), self);
	g_signal_connect (row, "switch-clicked",
	                  G_CALLBACK (repo_switch_clicked_cb), self);

	gtk_list_box_prepend (self->list, row);
	gtk_widget_set_visible (row, TRUE);
}

const gchar *
gs_repos_section_get_sort_key (GsReposSection *self)
{
	g_return_val_if_fail (GS_IS_REPOS_SECTION (self), NULL);

	return self->sort_key;
}

void
gs_repos_section_set_sort_key (GsReposSection *self,
			       const gchar *sort_key)
{
	g_return_if_fail (GS_IS_REPOS_SECTION (self));

	if (g_strcmp0 (sort_key, self->sort_key) != 0) {
		g_free (self->sort_key);
		self->sort_key = g_strdup (sort_key);
	}
}

gboolean
gs_repos_section_get_related_loaded (GsReposSection *self)
{
	g_return_val_if_fail (GS_IS_REPOS_SECTION (self), FALSE);

	return self->related_loaded;
}

void
gs_repos_section_set_related_loaded (GsReposSection *self,
				     gboolean value)
{
	g_return_if_fail (GS_IS_REPOS_SECTION (self));

	if (!self->related_loaded == !value)
		return;

	self->related_loaded = value;

	g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_RELATED_LOADED]);
}
