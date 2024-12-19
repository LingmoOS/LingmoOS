/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2018-2019 Endless Mobile
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <config.h>

#include <glib/gi18n.h>
#include <lingmo-appstore.h>
#include <libmalcontent/malcontent.h>
#include <string.h>
#include <math.h>

#include "gs-plugin-malcontent.h"
#include "gs-plugin-private.h"

/*
 * SECTION:
 * Adds the %GS_APP_QUIRK_PARENTAL_FILTER and
 * %GS_APP_QUIRK_PARENTAL_NOT_LAUNCHABLE quirks to applications if they
 * contravene the effective user’s current parental controls policy.
 *
 * Specifically, %GS_APP_QUIRK_PARENTAL_FILTER will be added if an app’s OARS
 * rating is too extreme for the current parental controls OARS policy.
 * %GS_APP_QUIRK_PARENTAL_NOT_LAUNCHABLE will be added if the app is listed on
 * the current parental controls blocklist.
 *
 * Parental controls policy is loaded using libmalcontent.
 *
 * This plugin is ordered after flatpak and appstream as it uses OARS data from
 * them.
 *
 * Limiting access to applications by not allowing them to be launched by
 * lingmo-appstore is only one part of a wider approach to parental controls.
 * In order to guarantee users do not have access to applications they shouldn’t
 * have access to, an LSM (such as AppArmor) needs to be used. That complements,
 * rather than substitutes for, filtering in user visible UIs.
 */

struct _GsPluginMalcontent {
	GsPlugin	 parent;

	GMutex		 mutex;  /* protects @app_filter **/
	MctManager	*manager;  /* (owned) */
	gulong		 manager_app_filter_changed_id;
	MctAppFilter	*app_filter;  /* (mutex) (owned) (nullable) */
};

G_DEFINE_TYPE (GsPluginMalcontent, gs_plugin_malcontent, GS_TYPE_PLUGIN)

/* Convert an #MctAppFilterOarsValue to an #AsContentRatingValue. This is
 * actually a trivial cast, since the types are defined the same; but throw in
 * a static assertion to be sure. */
static AsContentRatingValue
convert_app_filter_oars_value (MctAppFilterOarsValue filter_value)
{
  G_STATIC_ASSERT (AS_CONTENT_RATING_VALUE_LAST == MCT_APP_FILTER_OARS_VALUE_INTENSE + 1);

  return (AsContentRatingValue) filter_value;
}

static gboolean
app_is_expected_to_have_content_rating (GsApp *app)
{
	if (gs_app_has_quirk (app, GS_APP_QUIRK_NOT_LAUNCHABLE))
		return FALSE;

	switch (gs_app_get_kind (app)) {
	case AS_COMPONENT_KIND_ADDON:
	case AS_COMPONENT_KIND_CODEC:
	case AS_COMPONENT_KIND_DRIVER:
	case AS_COMPONENT_KIND_FIRMWARE:
	case AS_COMPONENT_KIND_FONT:
	case AS_COMPONENT_KIND_GENERIC:
	case AS_COMPONENT_KIND_INPUT_METHOD:
	case AS_COMPONENT_KIND_LOCALIZATION:
	case AS_COMPONENT_KIND_OPERATING_SYSTEM:
	case AS_COMPONENT_KIND_RUNTIME:
	case AS_COMPONENT_KIND_REPOSITORY:
		return FALSE;
	case AS_COMPONENT_KIND_UNKNOWN:
	case AS_COMPONENT_KIND_DESKTOP_APP:
	case AS_COMPONENT_KIND_WEB_APP:
	case AS_COMPONENT_KIND_CONSOLE_APP:
	default:
		break;
	}

	return TRUE;
}

/* Check whether the OARS rating for @app is as, or less, extreme than the
 * user’s preferences in @app_filter. If so (i.e. if the app is suitable for
 * this user to use), return %TRUE; otherwise return %FALSE.
 *
 * The #AsContentRating in @app may be %NULL if no OARS ratings are provided for
 * the app. If so, we have to assume the most restrictive ratings. However, if
 * @rating is provided but is empty, we assume that every section in it has
 * value %AS_CONTENT_RATING_VALUE_NONE. See
 * https://github.com/hughsie/oars/blob/HEAD/specification/oars-1.1.md */
static gboolean
app_is_content_rating_appropriate (GsApp *app, MctAppFilter *app_filter)
{
	g_autoptr(AsContentRating) rating = gs_app_dup_content_rating (app);  /* (nullable) */
	g_autofree const gchar **oars_sections = mct_app_filter_get_oars_sections (app_filter);
	AsContentRatingValue default_rating_value;

	if (rating == NULL && !app_is_expected_to_have_content_rating (app)) {
		/* Some apps, such as flatpak runtimes, are not expected to have
		 * content ratings. */
		return TRUE;
	} else if (rating == NULL) {
		g_debug ("No OARS ratings provided for ‘%s’: assuming most extreme",
		         gs_app_get_unique_id (app));
		default_rating_value = AS_CONTENT_RATING_VALUE_INTENSE;
	} else {
		default_rating_value = AS_CONTENT_RATING_VALUE_NONE;
	}

	for (gsize i = 0; oars_sections[i] != NULL; i++) {
		AsContentRatingValue rating_value;
		MctAppFilterOarsValue filter_value;

		filter_value = mct_app_filter_get_oars_value (app_filter, oars_sections[i]);

		if (rating != NULL)
			rating_value = as_content_rating_get_value (rating, oars_sections[i]);
		else
			rating_value = AS_CONTENT_RATING_VALUE_UNKNOWN;

		if (rating_value == AS_CONTENT_RATING_VALUE_UNKNOWN)
			rating_value = default_rating_value;

		if (filter_value == MCT_APP_FILTER_OARS_VALUE_UNKNOWN)
			continue;
		else if (convert_app_filter_oars_value (filter_value) < rating_value)
			return FALSE;
	}

	return TRUE;
}

static gboolean
app_is_parentally_blocklisted (GsApp *app, MctAppFilter *app_filter)
{
	const gchar *desktop_id;
	g_autoptr(GAppInfo) appinfo = NULL;

	desktop_id = gs_app_get_id (app);
	if (desktop_id == NULL)
		return FALSE;
	appinfo = G_APP_INFO (gs_utils_get_desktop_app_info (desktop_id));
	if (appinfo == NULL)
		return FALSE;

	return !mct_app_filter_is_appinfo_allowed (app_filter, appinfo);
}

static gboolean
app_set_parental_quirks (GsPluginMalcontent *self,
                         GsApp              *app,
                         MctAppFilter       *app_filter)
{
	/* note that both quirks can be set on an app at the same time, and they
	 * have slightly different meanings */
	gboolean filtered = FALSE;

	/* check the OARS ratings to see if this app should be installable */
	if (!app_is_content_rating_appropriate (app, app_filter)) {
		g_debug ("Filtering ‘%s’: app OARS rating is too extreme for this user",
		         gs_app_get_unique_id (app));
		gs_app_add_quirk (app, GS_APP_QUIRK_PARENTAL_FILTER);
		filtered = TRUE;
	} else {
		gs_app_remove_quirk (app, GS_APP_QUIRK_PARENTAL_FILTER);
	}

	/* check the app blocklist to see if this app should be launchable */
	if (app_is_parentally_blocklisted (app, app_filter)) {
		g_debug ("Filtering ‘%s’: app is blocklisted for this user",
		         gs_app_get_unique_id (app));
		gs_app_add_quirk (app, GS_APP_QUIRK_PARENTAL_NOT_LAUNCHABLE);
		filtered = TRUE;
	} else {
		gs_app_remove_quirk (app, GS_APP_QUIRK_PARENTAL_NOT_LAUNCHABLE);
	}

	return filtered;
}

static void
reload_app_filter_async (GsPluginMalcontent  *self,
                         gboolean             interactive,
                         GCancellable        *cancellable,
                         GAsyncReadyCallback  callback,
                         gpointer             user_data)
{
	/* Refresh the app filter. This causes a D-Bus request. */
	mct_manager_get_app_filter_async (self->manager,
					  getuid (),
					  interactive ? MCT_MANAGER_GET_VALUE_FLAGS_INTERACTIVE : MCT_MANAGER_GET_VALUE_FLAGS_NONE,
					  cancellable,
					  callback,
					  user_data);
}

static gboolean
reload_app_filter_finish (GsPluginMalcontent  *self,
                          GAsyncResult        *result,
                          GError             **error)
{
	g_autoptr(MctAppFilter) new_app_filter = NULL;
	g_autoptr(MctAppFilter) old_app_filter = NULL;

	new_app_filter = mct_manager_get_app_filter_finish (self->manager,
							    result,
							    error);

	/* on failure, keep the old app filter around since it might be more
	 * useful than nothing */
	if (new_app_filter == NULL)
		return FALSE;

	{
		g_autoptr(GMutexLocker) locker = g_mutex_locker_new (&self->mutex);
		old_app_filter = g_steal_pointer (&self->app_filter);
		self->app_filter = g_steal_pointer (&new_app_filter);
	}

	return TRUE;
}

static void reload_cb (GObject      *source_object,
                       GAsyncResult *result,
                       gpointer      user_data);

static void
app_filter_changed_cb (MctManager *manager,
                       guint64     user_id,
                       gpointer    user_data)
{
	GsPluginMalcontent *self = GS_PLUGIN_MALCONTENT (user_data);

	if (user_id != getuid ())
		return;

	/* The user’s app filter has changed, which means that different
	 * apps could be filtered from before. Reload everything to be
	 * sure of re-filtering correctly. */
	g_debug ("Reloading due to app filter changing for user %" G_GUINT64_FORMAT, user_id);
	reload_app_filter_async (self, FALSE, NULL, reload_cb, g_object_ref (self));
}

static void
reload_cb (GObject      *source_object,
           GAsyncResult *result,
           gpointer      user_data)
{
	g_autoptr(GsPluginMalcontent) self = g_steal_pointer (&user_data);
	g_autoptr(GError) local_error = NULL;

	if (reload_app_filter_finish (self, result, &local_error))
		gs_plugin_reload (GS_PLUGIN (self));
	else
		g_warning ("Failed to reload changed app filter: %s", local_error->message);
}

static void
gs_plugin_malcontent_init (GsPluginMalcontent *self)
{
	GsPlugin *plugin = GS_PLUGIN (self);

	/* need application IDs and content ratings */
	gs_plugin_add_rule (plugin, GS_PLUGIN_RULE_RUN_AFTER, "appstream");
	gs_plugin_add_rule (plugin, GS_PLUGIN_RULE_RUN_AFTER, "flatpak");

	/* set plugin name; it’s not a loadable plugin, but this is descriptive and harmless */
	gs_plugin_set_appstream_id (plugin, "org.lingmo.Appstore.Plugin.Malcontent");
}

static void get_app_filter_cb (GObject      *source_object,
                               GAsyncResult *result,
                               gpointer      user_data);

static void
gs_plugin_malcontent_setup_async (GsPlugin            *plugin,
                                  GCancellable        *cancellable,
                                  GAsyncReadyCallback  callback,
                                  gpointer             user_data)
{
	GsPluginMalcontent *self = GS_PLUGIN_MALCONTENT (plugin);
	g_autoptr(GTask) task = NULL;

	task = g_task_new (self, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_malcontent_setup_async);

	self->manager = mct_manager_new (gs_plugin_get_system_bus_connection (plugin));
	self->manager_app_filter_changed_id = g_signal_connect (self->manager,
								"app-filter-changed",
								(GCallback) app_filter_changed_cb,
								self);

	mct_manager_get_app_filter_async (self->manager, getuid (),
					  /* FIXME: Should this be unconditionally interactive? */
					  MCT_MANAGER_GET_VALUE_FLAGS_INTERACTIVE, cancellable,
					  get_app_filter_cb,
					  g_steal_pointer (&task));
}

static void
get_app_filter_cb (GObject      *source_object,
                   GAsyncResult *result,
                   gpointer      user_data)
{
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	GsPluginMalcontent *self = g_task_get_source_object (task);
	g_autoptr(GMutexLocker) locker = g_mutex_locker_new (&self->mutex);
	g_autoptr(GError) local_error = NULL;

	self->app_filter = mct_manager_get_app_filter_finish (self->manager, result, &local_error);
	if (self->app_filter == NULL) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_malcontent_setup_finish (GsPlugin      *self,
                                   GAsyncResult  *result,
                                   GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static gboolean
refine_app_locked (GsPluginMalcontent   *self,
		   GsApp                *app,
		   GsPluginRefineFlags   flags,
		   GCancellable         *cancellable,
		   GError              **error)
{
	/* not valid */
	if (gs_app_get_id (app) == NULL)
		return TRUE;

	/* Filter by various parental filters. The filter can’t be %NULL,
	 * otherwise setup() would have failed and the plugin would have been
	 * disabled. */
	g_assert (self->app_filter != NULL);

	app_set_parental_quirks (self, app, self->app_filter);

	return TRUE;
}

static void
gs_plugin_malcontent_refine_async (GsPlugin            *plugin,
                                   GsAppList           *list,
                                   GsPluginRefineFlags  flags,
                                   GCancellable        *cancellable,
                                   GAsyncReadyCallback  callback,
                                   gpointer             user_data)
{
	GsPluginMalcontent *self = GS_PLUGIN_MALCONTENT (plugin);
	g_autoptr(GTask) task = NULL;
	g_autoptr(GMutexLocker) locker = g_mutex_locker_new (&self->mutex);
	g_autoptr(GError) local_error = NULL;

	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_malcontent_refine_async);

	for (guint i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app = gs_app_list_index (list, i);
		if (!refine_app_locked (self, app, flags, cancellable, &local_error)) {
			g_task_return_error (task, g_steal_pointer (&local_error));
			return;
		}
	}

	g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_malcontent_refine_finish (GsPlugin      *plugin,
                                    GAsyncResult  *result,
                                    GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void refresh_metadata_cb (GObject      *source_object,
                                 GAsyncResult *result,
                                 gpointer      user_data);

static void
gs_plugin_malcontent_refresh_metadata_async (GsPlugin                     *plugin,
                                             guint64                       cache_age_secs,
                                             GsPluginRefreshMetadataFlags  flags,
                                             GCancellable                 *cancellable,
                                             GAsyncReadyCallback           callback,
                                             gpointer                      user_data)
{
	GsPluginMalcontent *self = GS_PLUGIN_MALCONTENT (plugin);
	g_autoptr(GTask) task = NULL;

	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_malcontent_refresh_metadata_async);

	reload_app_filter_async (self,
				 (flags & GS_PLUGIN_REFRESH_METADATA_FLAGS_INTERACTIVE),
				 cancellable,
				 refresh_metadata_cb,
				 g_steal_pointer (&task));
}

static void
refresh_metadata_cb (GObject      *source_object,
                     GAsyncResult *result,
                     gpointer      user_data)
{
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	GsPluginMalcontent *self = g_task_get_source_object (task);
	g_autoptr(GError) local_error = NULL;

	if (reload_app_filter_finish (self, result, &local_error))
		g_task_return_boolean (task, TRUE);
	else
		g_task_return_error (task, g_steal_pointer (&local_error));
}

static gboolean
gs_plugin_malcontent_refresh_metadata_finish (GsPlugin      *plugin,
                                              GAsyncResult  *result,
                                              GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gs_plugin_malcontent_dispose (GObject *object)
{
	GsPluginMalcontent *self = GS_PLUGIN_MALCONTENT (object);

	g_clear_pointer (&self->app_filter, mct_app_filter_unref);
	if (self->manager != NULL && self->manager_app_filter_changed_id != 0) {
		g_signal_handler_disconnect (self->manager,
					     self->manager_app_filter_changed_id);
		self->manager_app_filter_changed_id = 0;
	}
	g_clear_object (&self->manager);

	G_OBJECT_CLASS (gs_plugin_malcontent_parent_class)->dispose (object);
}

static void
gs_plugin_malcontent_class_init (GsPluginMalcontentClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPluginClass *plugin_class = GS_PLUGIN_CLASS (klass);

	object_class->dispose = gs_plugin_malcontent_dispose;

	plugin_class->setup_async = gs_plugin_malcontent_setup_async;
	plugin_class->setup_finish = gs_plugin_malcontent_setup_finish;
	plugin_class->refine_async = gs_plugin_malcontent_refine_async;
	plugin_class->refine_finish = gs_plugin_malcontent_refine_finish;
	plugin_class->refresh_metadata_async = gs_plugin_malcontent_refresh_metadata_async;
	plugin_class->refresh_metadata_finish = gs_plugin_malcontent_refresh_metadata_finish;
}

GType
gs_plugin_query_type (void)
{
	return GS_TYPE_PLUGIN_MALCONTENT;
}
