/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016-2018 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2019 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <glib.h>

#include "gs-packagekit-helper.h"
#include "packagekit-common.h"

struct _GsPackagekitHelper {
	GObject			 parent_instance;
	GHashTable		*apps;
	GsApp			*progress_app;
	GsAppList		*progress_list;
	GsPlugin		*plugin;
	gboolean		 allow_emit_updates_changed;
};

G_DEFINE_TYPE (GsPackagekitHelper, gs_packagekit_helper, G_TYPE_OBJECT)

void
gs_packagekit_helper_cb (PkProgress *progress, PkProgressType type, gpointer user_data)
{
	GsPackagekitHelper *self = (GsPackagekitHelper *) user_data;
	GsPlugin *plugin = gs_packagekit_helper_get_plugin (self);
	const gchar *package_id = pk_progress_get_package_id (progress);
	GsApp *app = NULL;

	/* optional */
	if (self->progress_app != NULL)
		app = self->progress_app;
	else if (package_id != NULL)
		app = gs_packagekit_helper_get_app_by_id (self, package_id);

	if (type == PK_PROGRESS_TYPE_STATUS) {
		PkStatusEnum status = pk_progress_get_status (progress);
		GsPluginStatus plugin_status = packagekit_status_enum_to_plugin_status (status);
		if (plugin_status != GS_PLUGIN_STATUS_UNKNOWN)
			gs_plugin_status_update (plugin, app, plugin_status);

		/* If we’re installing or removing a package, this may
		 * invalidate a previously-returned pending OS upgrade’s list of
		 * packages.
		 *
		 * FIXME: We can’t currently emit a more specific signal on the
		 * OS upgrade’s #GsApp, because it’s built by the
		 * fedora-pkgdb-collections plugin rather than the PackageKit
		 * plugin. The functionality from fedora-pkgdb-collections would
		 * have to be merged into PackageKit so the right #GsApp is
		 * accessible to modify its download state.
		 * */
		if ((self->allow_emit_updates_changed) &&
		    (plugin_status == GS_PLUGIN_STATUS_INSTALLING ||
		     plugin_status == GS_PLUGIN_STATUS_REMOVING) &&
		    (app == NULL ||
		     (gs_app_get_kind (app) != AS_COMPONENT_KIND_OPERATING_SYSTEM &&
		      gs_app_get_id (app) != NULL))) {
			/* this callback can be called many times in a row; limit how often
			   the GUI part is notified, to not refresh the Updates page too often */
			static gint64 last_notify = 0;
			gint64 current_time = g_get_real_time ();

			/* Just out-of-blue chosen 3 minutes interval between notifications */
			if (current_time - last_notify >= G_USEC_PER_SEC * 60 * 3) {
				g_debug ("notify about updates-changed from %s", G_STRFUNC);
				last_notify = current_time;
				gs_plugin_updates_changed (plugin);
			}
		}
	} else if (type == PK_PROGRESS_TYPE_PERCENTAGE) {
		gint percentage = pk_progress_get_percentage (progress);
		if (app != NULL && percentage >= 0 && percentage <= 100)
			gs_app_set_progress (app, (guint) percentage);
		if (self->progress_list != NULL && percentage >= 0 && percentage <= 100)
			gs_app_list_override_progress (self->progress_list, (guint) percentage);
	}

	/* Only go from TRUE to FALSE - it doesn't make sense for a package
	 * install to become uncancellable later on */
	if (app != NULL && gs_app_get_allow_cancel (app))
		gs_app_set_allow_cancel (app, pk_progress_get_allow_cancel (progress));
}

void
gs_packagekit_helper_add_app (GsPackagekitHelper *self, GsApp *app)
{
	GPtrArray *source_ids = gs_app_get_source_ids (app);

	g_return_if_fail (GS_IS_PACKAGEKIT_HELPER (self));
	g_return_if_fail (GS_IS_APP (app));

	for (guint i = 0; i < source_ids->len; i++) {
		const gchar *source_id = g_ptr_array_index (source_ids, i);
		g_hash_table_insert (self->apps,
				     g_strdup (source_id),
				     g_object_ref (app));
	}
}

void
gs_packagekit_helper_set_progress_app (GsPackagekitHelper *self, GsApp *progress_app)
{
	g_set_object (&self->progress_app, progress_app);
}

void
gs_packagekit_helper_set_progress_list (GsPackagekitHelper *self, GsAppList *progress_list)
{
	g_set_object (&self->progress_list, progress_list);
}

/*
 * gs_packagekit_helper_set_allow_emit_updates_changed:
 * @self: a #GsPackagekitHelper
 * @allow_emit_updates_changed: whether to allow emission of the updates-changed signal
 *
 * Set whether to allow emitting the #GsPlugin:updates-changed signal at any time through the task,
 * or whether to block it.
 *
 * FIXME: This is only needed to work around a signal emission loop caused by interaction
 * between the fedora-pkgdb-collections and PackageKit plugins. When the fedora-pkgdb-collections
 * plugin is removed, this API should be removed.
 * See !1817 and #2462.
 */
void
gs_packagekit_helper_set_allow_emit_updates_changed (GsPackagekitHelper *self,
						     gboolean allow_emit_updates_changed)
{
	self->allow_emit_updates_changed = allow_emit_updates_changed;
}

GsPlugin *
gs_packagekit_helper_get_plugin (GsPackagekitHelper *self)
{
	g_return_val_if_fail (GS_IS_PACKAGEKIT_HELPER (self), NULL);
	return self->plugin;
}

GsApp *
gs_packagekit_helper_get_app_by_id (GsPackagekitHelper *self, const gchar *package_id)
{
	g_return_val_if_fail (GS_IS_PACKAGEKIT_HELPER (self), NULL);
	g_return_val_if_fail (package_id != NULL, NULL);
	return g_hash_table_lookup (self->apps, package_id);
}

static void
gs_packagekit_helper_finalize (GObject *object)
{
	GsPackagekitHelper *self;

	g_return_if_fail (GS_IS_PACKAGEKIT_HELPER (object));

	self = GS_PACKAGEKIT_HELPER (object);

	g_object_unref (self->plugin);
	g_clear_object (&self->progress_app);
	g_clear_object (&self->progress_list);
	g_hash_table_unref (self->apps);

	G_OBJECT_CLASS (gs_packagekit_helper_parent_class)->finalize (object);
}

static void
gs_packagekit_helper_class_init (GsPackagekitHelperClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = gs_packagekit_helper_finalize;
}

static void
gs_packagekit_helper_init (GsPackagekitHelper *self)
{
	self->apps = g_hash_table_new_full (g_str_hash, g_str_equal,
					    g_free, (GDestroyNotify) g_object_unref);
	self->allow_emit_updates_changed = TRUE;
}

GsPackagekitHelper *
gs_packagekit_helper_new (GsPlugin *plugin)
{
	GsPackagekitHelper *self;
	self = g_object_new (GS_TYPE_PACKAGEKIT_HELPER, NULL);
	self->plugin = g_object_ref (plugin);
	return GS_PACKAGEKIT_HELPER (self);
}
