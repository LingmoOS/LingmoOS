/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2015-2016 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2018 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <config.h>

#include <lingmo-appstore.h>

#include "gs-plugin-provenance.h"

/*
 * SECTION:
 * Sets the package provenance to TRUE if installed by an official
 * software source. Also sets compulsory quirk when a required repository.
 *
 * This plugin executes entirely in the main thread.
 */

struct _GsPluginProvenance {
	GsPlugin		 parent;

	GSettings		*settings;
	GHashTable		*repos; /* gchar *name ~> guint flags */
	GPtrArray		*provenance_wildcards; /* non-NULL, when have names with wildcards */
	GPtrArray		*compulsory_wildcards; /* non-NULL, when have names with wildcards */
};

G_DEFINE_TYPE (GsPluginProvenance, gs_plugin_provenance, GS_TYPE_PLUGIN)

static GHashTable *
gs_plugin_provenance_remove_by_flag (GHashTable *old_repos,
				     GsAppQuirk quirk)
{
	GHashTable *new_repos = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	GHashTableIter iter;
	gpointer key, value;
	g_hash_table_iter_init (&iter, old_repos);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		guint flags = GPOINTER_TO_UINT (value);
		flags = flags & (~quirk);
		if (flags != 0)
			g_hash_table_insert (new_repos, g_strdup (key), GUINT_TO_POINTER (flags));
	}
	return new_repos;
}

static void
gs_plugin_provenance_add_quirks (GsApp *app,
				 guint quirks)
{
	if ((quirks & GS_APP_QUIRK_PROVENANCE) != 0)
		gs_app_add_quirk (app, GS_APP_QUIRK_PROVENANCE);
	if ((quirks & GS_APP_QUIRK_COMPULSORY) != 0 &&
	    gs_app_get_kind (app) == AS_COMPONENT_KIND_REPOSITORY)
		gs_app_add_quirk (app, GS_APP_QUIRK_COMPULSORY);
}

static gchar **
gs_plugin_provenance_get_sources (GsPluginProvenance *self,
				  const gchar *key)
{
	const gchar *tmp;
	tmp = g_getenv ("GS_SELF_TEST_PROVENANCE_SOURCES");
	if (tmp != NULL) {
		if (g_strcmp0 (key, "required-repos") == 0)
			return NULL;
		g_debug ("using custom provenance sources of %s", tmp);
		return g_strsplit (tmp, ",", -1);
	}
	return g_settings_get_strv (self->settings, key);
}

static void
gs_plugin_provenance_settings_changed_cb (GSettings *settings,
					  const gchar *key,
					  gpointer user_data)
{
	GsPluginProvenance *self = GS_PLUGIN_PROVENANCE (user_data);
	GsAppQuirk quirk = GS_APP_QUIRK_NONE;
	GPtrArray **pwildcards = NULL;

	if (g_strcmp0 (key, "official-repos") == 0) {
		quirk = GS_APP_QUIRK_PROVENANCE;
		pwildcards = &self->provenance_wildcards;
	} else if (g_strcmp0 (key, "required-repos") == 0) {
		quirk = GS_APP_QUIRK_COMPULSORY;
		pwildcards = &self->compulsory_wildcards;
	}

	if (quirk != GS_APP_QUIRK_NONE) {
		/* The keys are stolen by the hash table, thus free only the array */
		g_autofree gchar **repos = NULL;
		g_autoptr(GHashTable) old_repos = self->repos;
		g_autoptr(GPtrArray) old_wildcards = *pwildcards;
		GHashTable *new_repos = gs_plugin_provenance_remove_by_flag (old_repos, quirk);
		GPtrArray *new_wildcards = NULL;
		repos = gs_plugin_provenance_get_sources (self, key);
		for (guint ii = 0; repos && repos[ii]; ii++) {
			gchar *repo = g_steal_pointer (&(repos[ii]));
			if (strchr (repo, '*') ||
			    strchr (repo, '?') ||
			    strchr (repo, '[')) {
				if (new_wildcards == NULL)
					new_wildcards = g_ptr_array_new_with_free_func (g_free);
				g_ptr_array_add (new_wildcards, repo);
			} else {
				g_hash_table_insert (new_repos, repo,
					GUINT_TO_POINTER (quirk |
					GPOINTER_TO_UINT (g_hash_table_lookup (new_repos, repo))));
			}
		}
		if (new_wildcards != NULL)
			g_ptr_array_add (new_wildcards, NULL);
		self->repos = new_repos;
		*pwildcards = new_wildcards;
	}
}

static void
gs_plugin_provenance_init (GsPluginProvenance *self)
{
	self->settings = g_settings_new ("org.lingmo.appstore");
	self->repos = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	g_signal_connect (self->settings, "changed",
			  G_CALLBACK (gs_plugin_provenance_settings_changed_cb), self);
	gs_plugin_provenance_settings_changed_cb (self->settings, "official-repos", self);
	gs_plugin_provenance_settings_changed_cb (self->settings, "required-repos", self);

	/* after the package source is set */
	gs_plugin_add_rule (GS_PLUGIN (self), GS_PLUGIN_RULE_RUN_AFTER, "dummy");
	gs_plugin_add_rule (GS_PLUGIN (self), GS_PLUGIN_RULE_RUN_AFTER, "packagekit");
	gs_plugin_add_rule (GS_PLUGIN (self), GS_PLUGIN_RULE_RUN_AFTER, "rpm-ostree");
}

static void
gs_plugin_provenance_dispose (GObject *object)
{
	GsPluginProvenance *self = GS_PLUGIN_PROVENANCE (object);

	g_clear_pointer (&self->repos, g_hash_table_unref);
	g_clear_pointer (&self->provenance_wildcards, g_ptr_array_unref);
	g_clear_pointer (&self->compulsory_wildcards, g_ptr_array_unref);
	g_clear_object (&self->settings);

	G_OBJECT_CLASS (gs_plugin_provenance_parent_class)->dispose (object);
}

static gboolean
gs_plugin_provenance_find_repo_flags (GHashTable *repos,
				      GPtrArray *provenance_wildcards,
				      GPtrArray *compulsory_wildcards,
				      const gchar *repo,
				      guint *out_flags)
{
	if (repo == NULL || *repo == '\0')
		return FALSE;
	*out_flags = GPOINTER_TO_UINT (g_hash_table_lookup (repos, repo));
	if (provenance_wildcards != NULL &&
	    gs_utils_strv_fnmatch ((gchar **) provenance_wildcards->pdata, repo))
		*out_flags |= GS_APP_QUIRK_PROVENANCE;
	if (compulsory_wildcards != NULL &&
	    gs_utils_strv_fnmatch ((gchar **) compulsory_wildcards->pdata, repo))
		*out_flags |= GS_APP_QUIRK_COMPULSORY;
	return *out_flags != 0;
}

static gboolean
refine_app (GsPlugin             *plugin,
	    GsApp                *app,
	    GsPluginRefineFlags   flags,
	    GHashTable		 *repos,
	    GPtrArray		 *provenance_wildcards,
	    GPtrArray		 *compulsory_wildcards,
	    GCancellable         *cancellable,
	    GError              **error)
{
	const gchar *origin;
	guint quirks;

	/* not required */
	if ((flags & GS_PLUGIN_REFINE_FLAGS_REQUIRE_PROVENANCE) == 0)
		return TRUE;
	if (gs_app_has_quirk (app, GS_APP_QUIRK_PROVENANCE))
		return TRUE;

	/* Appstore sources/repositories are represented as #GsApps too. Add the
	 * provenance quirk to the system-configured repositories (but not
	 * user-configured ones). */
	if (gs_app_get_kind (app) == AS_COMPONENT_KIND_REPOSITORY) {
		if (gs_plugin_provenance_find_repo_flags (repos, provenance_wildcards, compulsory_wildcards, gs_app_get_id (app), &quirks) &&
		    gs_app_get_scope (app) != AS_COMPONENT_SCOPE_USER)
			gs_plugin_provenance_add_quirks (app, quirks);
		return TRUE;
	}

	/* simple case */
	origin = gs_app_get_origin (app);
	if (gs_plugin_provenance_find_repo_flags (repos, provenance_wildcards, compulsory_wildcards, origin, &quirks)) {
		gs_plugin_provenance_add_quirks (app, quirks);
		return TRUE;
	}

	/* this only works for packages */
	origin = gs_app_get_source_id_default (app);
	if (origin == NULL)
		return TRUE;
	origin = g_strrstr (origin, ";");
	if (origin == NULL)
		return TRUE;
	if (g_str_has_prefix (origin + 1, "installed:"))
		origin += 10;
	if (gs_plugin_provenance_find_repo_flags (repos, provenance_wildcards, compulsory_wildcards, origin + 1, &quirks))
		gs_plugin_provenance_add_quirks (app, quirks);

	return TRUE;
}

static void
gs_plugin_provenance_refine_async (GsPlugin            *plugin,
                                   GsAppList           *list,
                                   GsPluginRefineFlags  flags,
                                   GCancellable        *cancellable,
                                   GAsyncReadyCallback  callback,
                                   gpointer             user_data)
{
	GsPluginProvenance *self = GS_PLUGIN_PROVENANCE (plugin);
	g_autoptr(GTask) task = NULL;
	g_autoptr(GError) local_error = NULL;
	g_autoptr(GHashTable) repos = NULL;
	g_autoptr(GPtrArray) provenance_wildcards = NULL;
	g_autoptr(GPtrArray) compulsory_wildcards = NULL;

	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_provenance_refine_async);

	/* nothing to do here */
	if ((flags & GS_PLUGIN_REFINE_FLAGS_REQUIRE_PROVENANCE) == 0) {
		g_task_return_boolean (task, TRUE);
		return;
	}

	repos = g_hash_table_ref (self->repos);
	provenance_wildcards = self->provenance_wildcards != NULL ? g_ptr_array_ref (self->provenance_wildcards) : NULL;
	compulsory_wildcards = self->compulsory_wildcards != NULL ? g_ptr_array_ref (self->compulsory_wildcards) : NULL;

	/* nothing to search */
	if (g_hash_table_size (repos) == 0 && provenance_wildcards == NULL && compulsory_wildcards == NULL) {
		g_task_return_boolean (task, TRUE);
		return;
	}

	for (guint i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app = gs_app_list_index (list, i);
		if (!refine_app (plugin, app, flags, repos, provenance_wildcards, compulsory_wildcards, cancellable, &local_error)) {
			g_task_return_error (task, g_steal_pointer (&local_error));
			return;
		}
	}

	g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_provenance_refine_finish (GsPlugin      *plugin,
                                    GAsyncResult  *result,
                                    GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gs_plugin_provenance_class_init (GsPluginProvenanceClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPluginClass *plugin_class = GS_PLUGIN_CLASS (klass);

	object_class->dispose = gs_plugin_provenance_dispose;

	plugin_class->refine_async = gs_plugin_provenance_refine_async;
	plugin_class->refine_finish = gs_plugin_provenance_refine_finish;
}

GType
gs_plugin_query_type (void)
{
	return GS_TYPE_PLUGIN_PROVENANCE;
}
