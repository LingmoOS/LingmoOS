/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * gs-shell-search-provider.c - Implementation of a LINGMO Shell
 *   search provider
 *
 * Copyright (C) 2013 Matthias Clasen
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <config.h>

#include <gio/gio.h>
#include <glib/gi18n.h>
#include <string.h>

#include "gs-shell-search-provider-generated.h"
#include "gs-shell-search-provider.h"
#include "gs-common.h"

#define GS_SHELL_SEARCH_PROVIDER_MAX_RESULTS	20

typedef struct {
	GsShellSearchProvider *provider;
	GDBusMethodInvocation *invocation;
} PendingSearch;

struct _GsShellSearchProvider {
	GObject parent;

	GsShellSearchProvider2 *skeleton;
	GsPluginLoader *plugin_loader;
	GCancellable *cancellable;

	GHashTable *metas_cache;
	GsAppList *search_results;
};

G_DEFINE_TYPE (GsShellSearchProvider, gs_shell_search_provider, G_TYPE_OBJECT)

static void
pending_search_free (PendingSearch *search)
{
	g_object_unref (search->invocation);
	g_slice_free (PendingSearch, search);
}

static gint
search_sort_by_kudo_cb (GsApp *app1, GsApp *app2, gpointer user_data)
{
	guint pa, pb;
	pa = gs_app_get_kudos_percentage (app1);
	pb = gs_app_get_kudos_percentage (app2);
	if (pa < pb)
		return 1;
	else if (pa > pb)
		return -1;
	return 0;
}

static void
search_done_cb (GObject *source,
		GAsyncResult *res,
		gpointer user_data)
{
	PendingSearch *search = user_data;
	GsShellSearchProvider *self = search->provider;
	guint i;
	GVariantBuilder builder;
	g_autoptr(GsAppList) list = NULL;

	/* cache no longer valid */
	gs_app_list_remove_all (self->search_results);

	list = gs_plugin_loader_job_process_finish (self->plugin_loader, res, NULL);
	if (list == NULL) {
		g_dbus_method_invocation_return_value (search->invocation, g_variant_new ("(as)", NULL));
		pending_search_free (search);
		g_application_release (g_application_get_default ());
		return;	
	}

	/* sort by kudos, as there is no ratings data by default */
	gs_app_list_sort (list, search_sort_by_kudo_cb, NULL);

	g_variant_builder_init (&builder, G_VARIANT_TYPE ("as"));
	for (i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app = gs_app_list_index (list, i);
		g_variant_builder_add (&builder, "s", gs_app_get_unique_id (app));

		/* cache this in case we need the app in GetResultMetas */
		gs_app_list_add (self->search_results, app);
	}
	g_dbus_method_invocation_return_value (search->invocation, g_variant_new ("(as)", &builder));

	pending_search_free (search);
	g_application_release (g_application_get_default ());
}

static gchar *
gs_shell_search_provider_get_app_sort_key (GsApp *app)
{
	GString *key = g_string_sized_new (64);

	/* sort available apps before installed ones */
	switch (gs_app_get_state (app)) {
	case GS_APP_STATE_AVAILABLE:
		g_string_append (key, "9:");
		break;
	default:
		g_string_append (key, "1:");
		break;
	}

	/* sort apps before runtimes and extensions */
	switch (gs_app_get_kind (app)) {
	case AS_COMPONENT_KIND_DESKTOP_APP:
		g_string_append (key, "9:");
		break;
	default:
		g_string_append (key, "1:");
		break;
	}

	/* sort by the search key */
	g_string_append_printf (key, "%05x:", gs_app_get_match_value (app));

	/* tie-break with id */
	g_string_append (key, gs_app_get_unique_id (app));

	return g_string_free (key, FALSE);
}

static gint
gs_shell_search_provider_sort_cb (GsApp *app1, GsApp *app2, gpointer user_data)
{
	g_autofree gchar *key1 = NULL;
	g_autofree gchar *key2 = NULL;
	key1 = gs_shell_search_provider_get_app_sort_key (app1);
	key2 = gs_shell_search_provider_get_app_sort_key (app2);
	return g_strcmp0 (key2, key1);
}

static void
execute_search (GsShellSearchProvider  *self,
		GDBusMethodInvocation  *invocation,
		gchar		 **terms)
{
	PendingSearch *pending_search;
	g_autoptr(GsPluginJob) plugin_job = NULL;
	g_autoptr(GsAppQuery) query = NULL;
	g_autoptr(GSettings) settings = NULL;

	g_cancellable_cancel (self->cancellable);
	g_clear_object (&self->cancellable);

	/* don't attempt searches for a single character */
	if (g_strv_length (terms) == 1 &&
	    g_utf8_strlen (terms[0], -1) == 1) {
		g_dbus_method_invocation_return_value (invocation, g_variant_new ("(as)", NULL));
		return;
	}

	pending_search = g_slice_new (PendingSearch);
	pending_search->provider = self;
	pending_search->invocation = g_object_ref (invocation);

	g_application_hold (g_application_get_default ());
	self->cancellable = g_cancellable_new ();

	settings = g_settings_new ("org.lingmo.appstore");

	query = gs_app_query_new ("keywords", terms,
				  "refine-flags", GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON |
						  GS_PLUGIN_REFINE_FLAGS_REQUIRE_ORIGIN_HOSTNAME,
				  "dedupe-flags", GS_APP_LIST_FILTER_FLAG_PREFER_INSTALLED |
						  GS_APP_LIST_FILTER_FLAG_KEY_ID_PROVIDES,
				  "max-results", GS_SHELL_SEARCH_PROVIDER_MAX_RESULTS,
				  "sort-func", gs_shell_search_provider_sort_cb,
				  "sort-user-data", self,
				  "license-type", g_settings_get_boolean (settings, "show-only-free-apps") ? GS_APP_QUERY_LICENSE_FOSS : GS_APP_QUERY_LICENSE_ANY,
				  "developer-verified-type", g_settings_get_boolean (settings, "show-only-verified-apps") ?
							     GS_APP_QUERY_DEVELOPER_VERIFIED_ONLY : GS_APP_QUERY_DEVELOPER_VERIFIED_ANY,
				  NULL);
	plugin_job = gs_plugin_job_list_apps_new (query, GS_PLUGIN_LIST_APPS_FLAGS_NONE);

	gs_plugin_loader_job_process_async (self->plugin_loader, plugin_job,
					    self->cancellable,
					    search_done_cb,
					    pending_search);
}

static gboolean
handle_get_initial_result_set (GsShellSearchProvider2	*skeleton,
			       GDBusMethodInvocation	 *invocation,
			       gchar			**terms,
			       gpointer		       user_data)
{
	GsShellSearchProvider *self = user_data;

	g_debug ("****** GetInitialResultSet");
	execute_search (self, invocation, terms);
	return TRUE;
}

static gboolean
handle_get_subsearch_result_set (GsShellSearchProvider2	*skeleton,
				 GDBusMethodInvocation	 *invocation,
				 gchar			**previous_results,
				 gchar			**terms,
				 gpointer		       user_data)
{
	GsShellSearchProvider *self = user_data;

	g_debug ("****** GetSubSearchResultSet");
	execute_search (self, invocation, terms);
	return TRUE;
}

static gboolean
handle_get_result_metas (GsShellSearchProvider2	*skeleton,
			 GDBusMethodInvocation	 *invocation,
			 gchar			**results,
			 gpointer		       user_data)
{
	GsShellSearchProvider *self = user_data;
	GVariantBuilder meta;
	GVariant *meta_variant;
	gint i;
	GVariantBuilder builder;

	g_debug ("****** GetResultMetas");

	for (i = 0; results[i]; i++) {
		GsApp *app;
		g_autoptr(GIcon) icon = NULL;
		g_autofree gchar *description = NULL;

		/* already built */
		if (g_hash_table_lookup (self->metas_cache, results[i]) != NULL)
			continue;

		/* get previously found app */
		app = gs_app_list_lookup (self->search_results, results[i]);
		if (app == NULL) {
			g_warning ("failed to refine find app %s in cache", results[i]);
			continue;
		}

		g_variant_builder_init (&meta, G_VARIANT_TYPE ("a{sv}"));
		g_variant_builder_add (&meta, "{sv}", "id", g_variant_new_string (gs_app_get_unique_id (app)));
		g_variant_builder_add (&meta, "{sv}", "name", g_variant_new_string (gs_app_get_name (app)));

		/* ICON_SIZE is defined as 24px in js/ui/search.js in lingmo-shell */
		icon = gs_app_get_icon_for_size (app, 24, 1, NULL);
		if (icon != NULL) {
			g_autofree gchar *icon_str = g_icon_to_string (icon);
			if (icon_str != NULL) {
				g_variant_builder_add (&meta, "{sv}", "gicon", g_variant_new_string (icon_str));
			} else {
				g_autoptr(GVariant) icon_serialized = g_icon_serialize (icon);
				g_variant_builder_add (&meta, "{sv}", "icon", icon_serialized);
			}
		}

		if (gs_utils_list_has_component_fuzzy (self->search_results, app) &&
		    gs_app_get_origin_hostname (app) != NULL) {
			/* TRANSLATORS: this refers to where the app came from */
			g_autofree gchar *source_text = g_strdup_printf (_("Source: %s"),
			                                                 gs_app_get_origin_hostname (app));
			description = g_strdup_printf ("%s     %s",
			                               gs_app_get_summary (app),
			                               source_text);
		} else {
			description = g_strdup (gs_app_get_summary (app));
		}
		g_variant_builder_add (&meta, "{sv}", "description", g_variant_new_string (description));

		meta_variant = g_variant_builder_end (&meta);
		g_hash_table_insert (self->metas_cache,
				     g_strdup (gs_app_get_unique_id (app)),
				     g_variant_ref_sink (meta_variant));

	}

	g_variant_builder_init (&builder, G_VARIANT_TYPE ("aa{sv}"));
	for (i = 0; results[i]; i++) {
		meta_variant = (GVariant*)g_hash_table_lookup (self->metas_cache, results[i]);
		if (meta_variant == NULL)
			continue;
		g_variant_builder_add_value (&builder, meta_variant);
	}

	g_dbus_method_invocation_return_value (invocation, g_variant_new ("(aa{sv})", &builder));

	return TRUE;
}

static gboolean
handle_activate_result (GsShellSearchProvider2 	     *skeleton,
			GDBusMethodInvocation	*invocation,
			gchar			*result,
			gchar		       **terms,
			guint32		       timestamp,
			gpointer		      user_data)
{
	GApplication *app = g_application_get_default ();
	g_autofree gchar *string = NULL;

	string = g_strjoinv (" ", terms);

	g_action_group_activate_action (G_ACTION_GROUP (app), "details",
				  	g_variant_new ("(ss)", result, string));

	gs_shell_search_provider2_complete_activate_result (skeleton, invocation);
	return TRUE;
}

static gboolean
handle_launch_search (GsShellSearchProvider2 	   *skeleton,
		      GDBusMethodInvocation	*invocation,
		      gchar		       **terms,
		      guint32		       timestamp,
		      gpointer		      user_data)
{
	GApplication *app = g_application_get_default ();
	g_autofree gchar *string = g_strjoinv (" ", terms);

	g_action_group_activate_action (G_ACTION_GROUP (app), "search",
				  	g_variant_new ("s", string));

	gs_shell_search_provider2_complete_launch_search (skeleton, invocation);
	return TRUE;
}

gboolean
gs_shell_search_provider_register (GsShellSearchProvider *self,
                                   GDBusConnection       *connection,
                                   GError               **error)
{
	return g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (self->skeleton),
	                                         connection,
	                                         "/org/lingmo/Appstore/SearchProvider", error);
}

void
gs_shell_search_provider_unregister (GsShellSearchProvider *self)
{
	g_dbus_interface_skeleton_unexport (G_DBUS_INTERFACE_SKELETON (self->skeleton));
}

static void
search_provider_dispose (GObject *obj)
{
	GsShellSearchProvider *self = GS_SHELL_SEARCH_PROVIDER (obj);

	g_cancellable_cancel (self->cancellable);
	g_clear_object (&self->cancellable);

	if (self->metas_cache != NULL) {
		g_hash_table_destroy (self->metas_cache);
		self->metas_cache = NULL;
	}

	g_clear_object (&self->search_results);
	g_clear_object (&self->plugin_loader);
	g_clear_object (&self->skeleton);

	G_OBJECT_CLASS (gs_shell_search_provider_parent_class)->dispose (obj);
}

static void
gs_shell_search_provider_init (GsShellSearchProvider *self)
{
	self->metas_cache = g_hash_table_new_full ((GHashFunc) as_utils_data_id_hash,
						   (GEqualFunc) as_utils_data_id_equal,
						   g_free,
						   (GDestroyNotify) g_variant_unref);

	self->search_results = gs_app_list_new ();
	self->skeleton = gs_shell_search_provider2_skeleton_new ();

	g_signal_connect (self->skeleton, "handle-get-initial-result-set",
			G_CALLBACK (handle_get_initial_result_set), self);
	g_signal_connect (self->skeleton, "handle-get-subsearch-result-set",
			G_CALLBACK (handle_get_subsearch_result_set), self);
	g_signal_connect (self->skeleton, "handle-get-result-metas",
			G_CALLBACK (handle_get_result_metas), self);
	g_signal_connect (self->skeleton, "handle-activate-result",
			G_CALLBACK (handle_activate_result), self);
	g_signal_connect (self->skeleton, "handle-launch-search",
			G_CALLBACK (handle_launch_search), self);
}

static void
gs_shell_search_provider_class_init (GsShellSearchProviderClass *klass)
{
	GObjectClass *oclass = G_OBJECT_CLASS (klass);

	oclass->dispose = search_provider_dispose;
}

GsShellSearchProvider *
gs_shell_search_provider_new (void)
{
	return g_object_new (gs_shell_search_provider_get_type (), NULL);
}

void
gs_shell_search_provider_setup (GsShellSearchProvider *provider,
				GsPluginLoader *loader)
{
	provider->plugin_loader = g_object_ref (loader);
}
