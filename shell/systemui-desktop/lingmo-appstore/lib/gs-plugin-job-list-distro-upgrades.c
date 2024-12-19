/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2022 Endless OS Foundation LLC
 *
 * Author: Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-plugin-job-list-distro-upgrades
 * @short_description: A plugin job to list distro upgrades
 *
 * #GsPluginJobListDistroUpgrades is a #GsPluginJob representing an operation to
 * list available upgrades for the distro, from all #GsPlugins.
 *
 * Upgrades for the distro are large upgrades, such as from Fedora 34 to
 * Fedora 35. They are not small package updates.
 *
 * This job will list the available upgrades, but will not download them or
 * install them. Due to the typical size of an upgrade, these should not be
 * downloaded until the user has explicitly requested it.
 *
 * The known properties on the set of apps returned by this operation can be
 * controlled with the #GsPluginJobListDistroUpgrades:refine-flags property. All
 * results will be refined using %GS_PLUGIN_REFINE_FLAGS_REQUIRE_SETUP_ACTION
 * plus the given set of refine flags. See #GsPluginJobRefine.
 *
 * This class is a wrapper around #GsPluginClass.list_distro_upgrades_async,
 * calling it for all loaded plugins, with some additional filtering
 * done on the results and #GsPluginJobRefine used to refine them.
 *
 * Retrieve the resulting #GsAppList using
 * gs_plugin_job_list_distro_upgrades_get_result_list(). Components in the list
 * are expected to be of type %AS_COMPONENT_KIND_OPERATING_SYSTEM.
 *
 * See also: #GsPluginClass.list_distro_upgrades_async
 * Since: 42
 */

#include "config.h"

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>

#include "gs-app.h"
#include "gs-app-list-private.h"
#include "gs-enums.h"
#include "gs-plugin-job-private.h"
#include "gs-plugin-job-list-distro-upgrades.h"
#include "gs-plugin-job-refine.h"
#include "gs-plugin-private.h"
#include "gs-plugin-types.h"
#include "gs-utils.h"

struct _GsPluginJobListDistroUpgrades
{
	GsPluginJob parent;

	/* Input arguments. */
	GsPluginListDistroUpgradesFlags flags;
	GsPluginRefineFlags refine_flags;

	/* In-progress data. */
	GsAppList *merged_list;  /* (owned) (nullable) */
	GError *saved_error;  /* (owned) (nullable) */
	guint n_pending_ops;

	/* Results. */
	GsAppList *result_list;  /* (owned) (nullable) */
};

G_DEFINE_TYPE (GsPluginJobListDistroUpgrades, gs_plugin_job_list_distro_upgrades, GS_TYPE_PLUGIN_JOB)

typedef enum {
	PROP_REFINE_FLAGS = 1,
	PROP_FLAGS,
} GsPluginJobListDistroUpgradesProperty;

static GParamSpec *props[PROP_FLAGS + 1] = { NULL, };

static void
gs_plugin_job_list_distro_upgrades_dispose (GObject *object)
{
	GsPluginJobListDistroUpgrades *self = GS_PLUGIN_JOB_LIST_DISTRO_UPGRADES (object);

	g_assert (self->merged_list == NULL);
	g_assert (self->saved_error == NULL);
	g_assert (self->n_pending_ops == 0);

	g_clear_object (&self->result_list);

	G_OBJECT_CLASS (gs_plugin_job_list_distro_upgrades_parent_class)->dispose (object);
}

static void
gs_plugin_job_list_distro_upgrades_get_property (GObject    *object,
                                                 guint       prop_id,
                                                 GValue     *value,
                                                 GParamSpec *pspec)
{
	GsPluginJobListDistroUpgrades *self = GS_PLUGIN_JOB_LIST_DISTRO_UPGRADES (object);

	switch ((GsPluginJobListDistroUpgradesProperty) prop_id) {
	case PROP_REFINE_FLAGS:
		g_value_set_flags (value, self->refine_flags);
		break;
	case PROP_FLAGS:
		g_value_set_flags (value, self->flags);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_plugin_job_list_distro_upgrades_set_property (GObject      *object,
                                                 guint         prop_id,
                                                 const GValue *value,
                                                 GParamSpec   *pspec)
{
	GsPluginJobListDistroUpgrades *self = GS_PLUGIN_JOB_LIST_DISTRO_UPGRADES (object);

	switch ((GsPluginJobListDistroUpgradesProperty) prop_id) {
	case PROP_REFINE_FLAGS:
		/* Construct only. */
		g_assert (self->refine_flags == 0);
		self->refine_flags = g_value_get_flags (value);
		g_object_notify_by_pspec (object, props[prop_id]);
		break;
	case PROP_FLAGS:
		/* Construct only. */
		g_assert (self->flags == 0);
		self->flags = g_value_get_flags (value);
		g_object_notify_by_pspec (object, props[prop_id]);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static gint
app_sort_version_cb (GsApp    *app1,
                     GsApp    *app2,
                     gpointer  user_data)
{
	return as_vercmp_simple (gs_app_get_version (app1),
				 gs_app_get_version (app2));
}

static void plugin_list_distro_upgrades_cb (GObject      *source_object,
                                            GAsyncResult *result,
                                            gpointer      user_data);
static void finish_op (GTask  *task,
                       GError *error);
static void refine_cb (GObject      *source_object,
                       GAsyncResult *result,
                       gpointer      user_data);
static void finish_task (GTask     *task,
                         GsAppList *merged_list);

static void
gs_plugin_job_list_distro_upgrades_run_async (GsPluginJob         *job,
                                              GsPluginLoader      *plugin_loader,
                                              GCancellable        *cancellable,
                                              GAsyncReadyCallback  callback,
                                              gpointer             user_data)
{
	GsPluginJobListDistroUpgrades *self = GS_PLUGIN_JOB_LIST_DISTRO_UPGRADES (job);
	g_autoptr(GTask) task = NULL;
	GPtrArray *plugins;  /* (element-type GsPlugin) */
	gboolean anything_ran = FALSE;
	g_autoptr(GError) local_error = NULL;

	/* check required args */
	task = g_task_new (job, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_job_list_distro_upgrades_run_async);
	g_task_set_task_data (task, g_object_ref (plugin_loader), (GDestroyNotify) g_object_unref);

	/* run each plugin, keeping a counter of pending operations which is
	 * initialised to 1 until all the operations are started */
	self->n_pending_ops = 1;
	self->merged_list = gs_app_list_new ();
	plugins = gs_plugin_loader_get_plugins (plugin_loader);

	for (guint i = 0; i < plugins->len; i++) {
		GsPlugin *plugin = g_ptr_array_index (plugins, i);
		GsPluginClass *plugin_class = GS_PLUGIN_GET_CLASS (plugin);

		if (!gs_plugin_get_enabled (plugin))
			continue;
		if (plugin_class->list_distro_upgrades_async == NULL)
			continue;

		/* at least one plugin supports this vfunc */
		anything_ran = TRUE;

		/* Handle cancellation */
		if (g_cancellable_set_error_if_cancelled (cancellable, &local_error))
			break;

		/* run the plugin */
		self->n_pending_ops++;
		plugin_class->list_distro_upgrades_async (plugin, self->flags, cancellable, plugin_list_distro_upgrades_cb, g_object_ref (task));
	}

	if (!anything_ran)
		g_debug ("no plugin could handle listing distro upgrades");

	finish_op (task, g_steal_pointer (&local_error));
}

static void
plugin_list_distro_upgrades_cb (GObject      *source_object,
                                GAsyncResult *result,
                                gpointer      user_data)
{
	GsPlugin *plugin = GS_PLUGIN (source_object);
	GsPluginClass *plugin_class = GS_PLUGIN_GET_CLASS (plugin);
	g_autoptr(GTask) task = G_TASK (user_data);
	GsPluginJobListDistroUpgrades *self = g_task_get_source_object (task);
	g_autoptr(GsAppList) plugin_apps = NULL;
	g_autoptr(GError) local_error = NULL;

	plugin_apps = plugin_class->list_distro_upgrades_finish (plugin, result, &local_error);
	gs_plugin_status_update (plugin, NULL, GS_PLUGIN_STATUS_FINISHED);

	if (plugin_apps != NULL)
		gs_app_list_add_list (self->merged_list, plugin_apps);

	finish_op (task, g_steal_pointer (&local_error));
}

/* @error is (transfer full) if non-%NULL */
static void
finish_op (GTask  *task,
           GError *error)
{
	GsPluginJobListDistroUpgrades *self = g_task_get_source_object (task);
	GCancellable *cancellable = g_task_get_cancellable (task);
	GsPluginLoader *plugin_loader = g_task_get_task_data (task);
	g_autoptr(GsAppList) merged_list = NULL;
	g_autoptr(GError) error_owned = g_steal_pointer (&error);

	if (error_owned != NULL && self->saved_error == NULL)
		self->saved_error = g_steal_pointer (&error_owned);
	else if (error_owned != NULL)
		g_debug ("Additional error while listing distro upgrades: %s", error_owned->message);

	g_assert (self->n_pending_ops > 0);
	self->n_pending_ops--;

	if (self->n_pending_ops > 0)
		return;

	/* Get the results of the parallel ops. */
	merged_list = g_steal_pointer (&self->merged_list);

	if (self->saved_error != NULL) {
		g_task_return_error (task, g_steal_pointer (&self->saved_error));
		g_signal_emit_by_name (G_OBJECT (self), "completed");
		return;
	}

	/* run refine() on each one if required */
	if (merged_list != NULL &&
	    gs_app_list_length (merged_list) > 0) {
		g_autoptr(GsPluginJob) refine_job = NULL;

		/* Always specify REQUIRE_SETUP_ACTION, as that requires enough
		 * information to be able to install the upgrade later if
		 * requested. */
		refine_job = gs_plugin_job_refine_new (merged_list,
						       self->refine_flags |
						       GS_PLUGIN_REFINE_FLAGS_REQUIRE_SETUP_ACTION |
						       GS_PLUGIN_REFINE_FLAGS_DISABLE_FILTERING);
		gs_plugin_loader_job_process_async (plugin_loader, refine_job,
						    cancellable,
						    refine_cb,
						    g_object_ref (task));
	} else {
		g_debug ("No distro upgrades to refine");
		finish_task (task, merged_list);
	}
}

static void
refine_cb (GObject      *source_object,
           GAsyncResult *result,
           gpointer      user_data)
{
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source_object);
	g_autoptr(GTask) task = G_TASK (user_data);
	GsPluginJobListDistroUpgrades *self = g_task_get_source_object (task);
	g_autoptr(GsAppList) new_list = NULL;
	g_autoptr(GError) local_error = NULL;

	new_list = gs_plugin_loader_job_process_finish (plugin_loader, result, &local_error);
	if (new_list == NULL) {
		gs_utils_error_convert_gio (&local_error);
		g_task_return_error (task, g_steal_pointer (&local_error));
		g_signal_emit_by_name (G_OBJECT (self), "completed");
		return;
	}

	finish_task (task, new_list);
}

static void
finish_task (GTask     *task,
             GsAppList *merged_list)
{
	GsPluginJobListDistroUpgrades *self = g_task_get_source_object (task);
	g_autofree gchar *job_debug = NULL;

	/* Sort the results. The refine may have added useful metadata. */
	gs_app_list_sort (merged_list, app_sort_version_cb, NULL);

	/* show elapsed time */
	job_debug = gs_plugin_job_to_string (GS_PLUGIN_JOB (self));
	g_debug ("%s", job_debug);

	/* Check the intermediate working values are all cleared. */
	g_assert (self->merged_list == NULL);
	g_assert (self->saved_error == NULL);
	g_assert (self->n_pending_ops == 0);

	/* success */
	g_set_object (&self->result_list, merged_list);
	g_task_return_boolean (task, TRUE);
	g_signal_emit_by_name (G_OBJECT (self), "completed");
}

static gboolean
gs_plugin_job_list_distro_upgrades_run_finish (GsPluginJob   *self,
                                               GAsyncResult  *result,
                                               GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gs_plugin_job_list_distro_upgrades_class_init (GsPluginJobListDistroUpgradesClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPluginJobClass *job_class = GS_PLUGIN_JOB_CLASS (klass);

	object_class->dispose = gs_plugin_job_list_distro_upgrades_dispose;
	object_class->get_property = gs_plugin_job_list_distro_upgrades_get_property;
	object_class->set_property = gs_plugin_job_list_distro_upgrades_set_property;

	job_class->run_async = gs_plugin_job_list_distro_upgrades_run_async;
	job_class->run_finish = gs_plugin_job_list_distro_upgrades_run_finish;

	/**
	 * GsPluginJobListDistroUpgrades:refine-flags:
	 *
	 * Flags to specify how to refine the returned apps.
	 *
	 * %GS_PLUGIN_REFINE_FLAGS_REQUIRE_SETUP_ACTION will always be used.
	 *
	 * Since: 42
	 */
	props[PROP_REFINE_FLAGS] =
		g_param_spec_flags ("refine-flags", "Refine Flags",
				    "Flags to specify how to refine the returned apps.",
				    GS_TYPE_PLUGIN_REFINE_FLAGS, GS_PLUGIN_REFINE_FLAGS_NONE,
				    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				    G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsPluginJobListDistroUpgrades:flags:
	 *
	 * Flags to specify how the operation should run.
	 *
	 * Since: 42
	 */
	props[PROP_FLAGS] =
		g_param_spec_flags ("flags", "Flags",
				    "Flags to specify how the operation should run.",
				    GS_TYPE_PLUGIN_LIST_DISTRO_UPGRADES_FLAGS,
				    GS_PLUGIN_LIST_DISTRO_UPGRADES_FLAGS_NONE,
				    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				    G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (props), props);
}

static void
gs_plugin_job_list_distro_upgrades_init (GsPluginJobListDistroUpgrades *self)
{
}

/**
 * gs_plugin_job_list_distro_upgrades_new:
 * @flags: flags affecting how the operation runs
 * @refine_flags: flags to affect how the results are refined
 *
 * Create a new #GsPluginJobListDistroUpgrades for listing the available distro
 * upgrades.
 *
 * Returns: (transfer full): a new #GsPluginJobListDistroUpgrades
 * Since: 42
 */
GsPluginJob *
gs_plugin_job_list_distro_upgrades_new (GsPluginListDistroUpgradesFlags flags,
                                        GsPluginRefineFlags             refine_flags)
{
	return g_object_new (GS_TYPE_PLUGIN_JOB_LIST_DISTRO_UPGRADES,
			     "refine-flags", refine_flags,
			     "flags", flags,
			     NULL);
}

/**
 * gs_plugin_job_list_distro_upgrades_get_result_list:
 * @self: a #GsPluginJobListDistroUpgrades
 *
 * Get the full list of available distro upgrades.
 *
 * If this is called before the job is complete, %NULL will be returned.
 *
 * Returns: (transfer none) (nullable): the job results, or %NULL on error
 *   or if called before the job has completed
 * Since: 42
 */
GsAppList *
gs_plugin_job_list_distro_upgrades_get_result_list (GsPluginJobListDistroUpgrades *self)
{
	g_return_val_if_fail (GS_IS_PLUGIN_JOB_LIST_DISTRO_UPGRADES (self), NULL);

	return self->result_list;
}
