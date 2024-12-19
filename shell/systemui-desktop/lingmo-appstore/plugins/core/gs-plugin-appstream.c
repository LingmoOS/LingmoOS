/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2014 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2015-2019 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <config.h>

#include <glib/gi18n.h>
#include <errno.h>
#include <lingmo-appstore.h>
#include <xmlb.h>

#include "gs-appstream.h"
#include "gs-external-appstream-utils.h"
#include "gs-plugin-appstream.h"

/*
 * SECTION:
 * Uses offline AppStream data to populate and refine package results.
 *
 * This plugin calls UpdatesChanged() if any of the AppStream stores are
 * changed in any way.
 *
 * Methods:     | AddCategory
 * Refines:     | [source]->[name,summary,pixbuf,id,kind]
 */

struct _GsPluginAppstream
{
	GsPlugin		 parent;

	GsWorkerThread		*worker;  /* (owned) */

	XbSilo			*silo;
	GRWLock			 silo_lock;
	gchar			*silo_filename;
	GHashTable		*silo_installed_by_desktopid;
	GHashTable		*silo_installed_by_id;
	AsComponentScope	 default_scope;
	GSettings		*settings;

	GPtrArray		*file_monitors; /* (owned) (element-type GFileMonitor) */
	/* The stamps help to avoid locking the silo lock in the main thread
	   and also to detect changes while loading other appstream data. */
	gint			 file_monitor_stamp; /* the file monitor stamp, increased on every file monitor change */
	gint			 file_monitor_stamp_current; /* the currently known file monitor stamp, checked for changes */
};

G_DEFINE_TYPE (GsPluginAppstream, gs_plugin_appstream, GS_TYPE_PLUGIN)

#define assert_in_worker(self) \
	g_assert (gs_worker_thread_is_in_worker_context (self->worker))

static void
gs_plugin_appstream_dispose (GObject *object)
{
	GsPluginAppstream *self = GS_PLUGIN_APPSTREAM (object);

	g_clear_object (&self->silo);
	g_clear_pointer (&self->silo_filename, g_free);
	g_clear_pointer (&self->silo_installed_by_desktopid, g_hash_table_unref);
	g_clear_pointer (&self->silo_installed_by_id, g_hash_table_unref);
	g_clear_object (&self->settings);
	g_rw_lock_clear (&self->silo_lock);
	g_clear_object (&self->worker);
	g_clear_pointer (&self->file_monitors, g_ptr_array_unref);

	G_OBJECT_CLASS (gs_plugin_appstream_parent_class)->dispose (object);
}

static void
gs_plugin_appstream_init (GsPluginAppstream *self)
{
	GApplication *application = g_application_get_default ();

	/* XbSilo needs external locking as we destroy the silo and build a new
	 * one when something changes */
	g_rw_lock_init (&self->silo_lock);

	/* need package name */
	gs_plugin_add_rule (GS_PLUGIN (self), GS_PLUGIN_RULE_RUN_AFTER, "dpkg");

	/* require settings */
	self->settings = g_settings_new ("org.lingmo.appstore");

	/* Can be NULL when running the self tests */
	if (application) {
		g_signal_connect_object (application, "repository-changed",
			G_CALLBACK (gs_plugin_update_cache_state_for_repository), self, G_CONNECT_SWAPPED);
	}

	self->file_monitors = g_ptr_array_new_with_free_func (g_object_unref);
}

static const gchar *
gs_plugin_appstream_convert_component_kind (const gchar *kind)
{
	if (g_strcmp0 (kind, "webapp") == 0)
		return "web-application";
	if (g_strcmp0 (kind, "desktop") == 0)
		return "desktop-application";
	return kind;
}

static gboolean
gs_plugin_appstream_upgrade_cb (XbBuilderFixup *self,
				XbBuilderNode *bn,
				gpointer user_data,
				GError **error)
{
	if (g_strcmp0 (xb_builder_node_get_element (bn), "application") == 0) {
		g_autoptr(XbBuilderNode) id = xb_builder_node_get_child (bn, "id", NULL);
		g_autofree gchar *kind = NULL;
		if (id != NULL) {
			kind = g_strdup (xb_builder_node_get_attr (id, "type"));
			xb_builder_node_remove_attr (id, "type");
		}
		if (kind != NULL)
			xb_builder_node_set_attr (bn, "type", kind);
		xb_builder_node_set_element (bn, "component");
	} else if (g_strcmp0 (xb_builder_node_get_element (bn), "metadata") == 0) {
		xb_builder_node_set_element (bn, "custom");
	} else if (g_strcmp0 (xb_builder_node_get_element (bn), "component") == 0) {
		const gchar *type_old = xb_builder_node_get_attr (bn, "type");
		const gchar *type_new = gs_plugin_appstream_convert_component_kind (type_old);
		if (type_old != type_new)
			xb_builder_node_set_attr (bn, "type", type_new);
	}
	return TRUE;
}

static gboolean
gs_plugin_appstream_add_icons_cb (XbBuilderFixup *self,
				  XbBuilderNode *bn,
				  gpointer user_data,
				  GError **error)
{
	if (g_strcmp0 (xb_builder_node_get_element (bn), "component") != 0)
		return TRUE;
	gs_appstream_component_add_extra_info (bn);
	return TRUE;
}

static gboolean
gs_plugin_appstream_add_origin_keyword_cb (XbBuilderFixup *self,
					   XbBuilderNode *bn,
					   gpointer user_data,
					   GError **error)
{
	if (g_strcmp0 (xb_builder_node_get_element (bn), "components") == 0) {
		const gchar *origin = xb_builder_node_get_attr (bn, "origin");
		GPtrArray *components = xb_builder_node_get_children (bn);
		if (origin == NULL || origin[0] == '\0')
			return TRUE;
		g_debug ("origin %s has %u components", origin, components->len);
		if (components->len < 200) {
			for (guint i = 0; i < components->len; i++) {
				XbBuilderNode *component = g_ptr_array_index (components, i);
				gs_appstream_component_add_keyword (component, origin);
			}
		}
	}
	return TRUE;
}

static void
gs_plugin_appstream_media_baseurl_free (gpointer user_data)
{
	g_string_free ((GString *) user_data, TRUE);
}

static gboolean
gs_plugin_appstream_media_baseurl_cb (XbBuilderFixup *self,
				      XbBuilderNode *bn,
				      gpointer user_data,
				      GError **error)
{
	GString *baseurl = user_data;
	if (g_strcmp0 (xb_builder_node_get_element (bn), "components") == 0) {
		const gchar *url = xb_builder_node_get_attr (bn, "media_baseurl");
		if (url == NULL) {
			g_string_truncate (baseurl, 0);
			return TRUE;
		}
		g_string_assign (baseurl, url);
		return TRUE;
	}

	if (baseurl->len == 0)
		return TRUE;

	if (g_strcmp0 (xb_builder_node_get_element (bn), "icon") == 0) {
		const gchar *type = xb_builder_node_get_attr (bn, "type");
		if (g_strcmp0 (type, "remote") != 0)
			return TRUE;
		gs_appstream_component_fix_url (bn, baseurl->str);
	} else if (g_strcmp0 (xb_builder_node_get_element (bn), "screenshots") == 0) {
		GPtrArray *screenshots = xb_builder_node_get_children (bn);
		for (guint i = 0; i < screenshots->len; i++) {
			XbBuilderNode *screenshot = g_ptr_array_index (screenshots, i);
			GPtrArray *children = NULL;
			/* Type-check for security */
			if (g_strcmp0 (xb_builder_node_get_element (screenshot), "screenshot") != 0) {
				continue;
			}
			children = xb_builder_node_get_children (screenshot);
			for (guint j = 0; j < children->len; j++) {
				XbBuilderNode *child = g_ptr_array_index (children, j);
				const gchar *element = xb_builder_node_get_element (child);
				if (g_strcmp0 (element, "image") != 0 &&
				    g_strcmp0 (element, "video") != 0)
					continue;
				gs_appstream_component_fix_url (child, baseurl->str);
			}
		}
	}
	return TRUE;
}

static void
gs_plugin_appstream_file_monitor_changed_cb (GFileMonitor *monitor,
					     GFile *file,
					     GFile *other_file,
					     GFileMonitorEvent event_type,
					     gpointer user_data)
{
	GsPluginAppstream *self = user_data;
	g_atomic_int_inc (&self->file_monitor_stamp);
}

static void
gs_plugin_appstream_maybe_store_file_monitor (GsPluginAppstream  *self,
					      GFileMonitor *file_monitor) /* (nullable) (transfer none) */
{
	if (!file_monitor)
		return;

	g_signal_connect_object (file_monitor, "changed",
		G_CALLBACK (gs_plugin_appstream_file_monitor_changed_cb), self, 0);

	g_ptr_array_add (self->file_monitors, g_object_ref (file_monitor));
}

static gboolean
gs_plugin_appstream_load_appdata_fn (GsPluginAppstream  *self,
                                     XbBuilder          *builder,
                                     const gchar        *filename,
                                     GCancellable       *cancellable,
                                     GError            **error)
{
	g_autoptr(GFile) file = g_file_new_for_path (filename);
	g_autoptr(XbBuilderFixup) fixup = NULL;
	g_autoptr(XbBuilderNode) info = NULL;
	g_autoptr(XbBuilderSource) source = xb_builder_source_new ();

	/* add source */
	if (!xb_builder_source_load_file (source, file, 0, cancellable, error))
		return FALSE;

	/* fix up any legacy installed files */
	fixup = xb_builder_fixup_new ("AppStreamUpgrade2",
				      gs_plugin_appstream_upgrade_cb,
				      self, NULL);
	xb_builder_fixup_set_max_depth (fixup, 3);
	xb_builder_source_add_fixup (source, fixup);

	/* add metadata */
	info = xb_builder_node_insert (NULL, "info", NULL);
	xb_builder_node_insert_text (info, "filename", filename, NULL);
	xb_builder_source_set_info (source, info);

	/* success */
	xb_builder_import_source (builder, source);
	return TRUE;
}

static gboolean
gs_plugin_appstream_load_appdata (GsPluginAppstream  *self,
                                  XbBuilder          *builder,
                                  const gchar        *path,
                                  GCancellable       *cancellable,
                                  GError            **error)
{
	const gchar *fn;
	g_autoptr(GDir) dir = NULL;
	g_autoptr(GFile) parent = g_file_new_for_path (path);
	g_autoptr(GFileMonitor) file_monitor = NULL;
	g_autoptr(GError) local_error = NULL;
	if (!g_file_query_exists (parent, cancellable)) {
		g_debug ("appstream: Skipping appdata path '%s' as %s", path, g_cancellable_is_cancelled (cancellable) ? "cancelled" : "does not exist");
		return TRUE;
	}

	g_debug ("appstream: Loading appdata path '%s'", path);

	dir = g_dir_open (path, 0, error);
	if (dir == NULL)
		return FALSE;

	file_monitor = g_file_monitor (parent, G_FILE_MONITOR_NONE, cancellable, &local_error);
	if (local_error)
		g_debug ("appstream: Failed to create file monitor for '%s': %s", path, local_error->message);
	gs_plugin_appstream_maybe_store_file_monitor (self, file_monitor);

	while ((fn = g_dir_read_name (dir)) != NULL) {
		if (g_str_has_suffix (fn, ".appdata.xml") ||
		    g_str_has_suffix (fn, ".metainfo.xml")) {
			g_autofree gchar *filename = g_build_filename (path, fn, NULL);
			g_autoptr(GError) error_local = NULL;
			if (!gs_plugin_appstream_load_appdata_fn (self,
								  builder,
								  filename,
								  cancellable,
								  &error_local)) {
				g_debug ("ignoring %s: %s", filename, error_local->message);
				continue;
			}
		}
	}

	/* success */
	return TRUE;
}

static GInputStream *
gs_plugin_appstream_load_dep11_cb (XbBuilderSource *self,
				   XbBuilderSourceCtx *ctx,
				   gpointer user_data,
				   GCancellable *cancellable,
				   GError **error)
{
	g_autoptr(AsMetadata) mdata = as_metadata_new ();
	g_autoptr(GBytes) bytes = NULL;
	g_autoptr(GError) tmp_error = NULL;
	g_autofree gchar *xml = NULL;

	bytes = xb_builder_source_ctx_get_bytes (ctx, cancellable, error);
	if (bytes == NULL)
		return NULL;

	as_metadata_set_format_style (mdata, AS_FORMAT_STYLE_CATALOG);
	as_metadata_parse_bytes (mdata,
				 bytes,
				 AS_FORMAT_KIND_YAML,
				 &tmp_error);
	if (tmp_error != NULL) {
		g_propagate_error (error, g_steal_pointer (&tmp_error));
		return NULL;
	}

	xml = as_metadata_components_to_catalog (mdata, AS_FORMAT_KIND_XML, &tmp_error);
	if (xml == NULL) {
		// This API currently returns NULL if there is nothing to serialize, so we
		// have to test if this is an error or not.
		// See https://github.com/LingmoOS/lingmo-appstore/-/merge_requests/763
		// for discussion about changing this API.
		if (tmp_error != NULL) {
			g_propagate_error (error, g_steal_pointer (&tmp_error));
			return NULL;
		}

		xml = g_strdup("");
	}

	return g_memory_input_stream_new_from_data (g_steal_pointer (&xml), (gssize) -1, g_free);
}

#if LIBXMLB_CHECK_VERSION(0,3,1)
static gboolean
gs_plugin_appstream_tokenize_cb (XbBuilderFixup *self,
				 XbBuilderNode *bn,
				 gpointer user_data,
				 GError **error)
{
	const gchar * const elements_to_tokenize[] = {
		"id",
		"keyword",
		"launchable",
		"mimetype",
		"name",
		"pkgname",
		"summary",
		NULL };
	if (xb_builder_node_get_element (bn) != NULL &&
	    g_strv_contains (elements_to_tokenize, xb_builder_node_get_element (bn)))
		xb_builder_node_tokenize_text (bn);
	return TRUE;
}
#endif

static gboolean
gs_plugin_appstream_load_appstream_fn (GsPluginAppstream  *self,
                                       XbBuilder          *builder,
                                       const gchar        *filename,
                                       GCancellable       *cancellable,
                                       GError            **error)
{
	g_autoptr(GFile) file = g_file_new_for_path (filename);
	g_autoptr(XbBuilderNode) info = NULL;
	g_autoptr(XbBuilderFixup) fixup1 = NULL;
	g_autoptr(XbBuilderFixup) fixup2 = NULL;
	g_autoptr(XbBuilderFixup) fixup3 = NULL;
#if LIBXMLB_CHECK_VERSION(0,3,1)
	g_autoptr(XbBuilderFixup) fixup4 = NULL;
#endif
	g_autoptr(XbBuilderFixup) fixup5 = NULL;
	g_autoptr(XbBuilderSource) source = xb_builder_source_new ();

	/* add support for DEP-11 files */
	xb_builder_source_add_adapter (source,
				       "application/yaml",
				       gs_plugin_appstream_load_dep11_cb,
				       NULL, NULL);
	xb_builder_source_add_adapter (source,
				       "application/x-yaml",
				       gs_plugin_appstream_load_dep11_cb,
				       NULL, NULL);

	/* add source */
	if (!xb_builder_source_load_file (source, file, 0, cancellable, error))
		return FALSE;

	/* add metadata */
	info = xb_builder_node_insert (NULL, "info", NULL);
	xb_builder_node_insert_text (info, "scope", "system", NULL);
	xb_builder_node_insert_text (info, "filename", filename, NULL);
	xb_builder_source_set_info (source, info);

	/* add missing icons as required */
	fixup1 = xb_builder_fixup_new ("AddIcons",
				       gs_plugin_appstream_add_icons_cb,
				       self, NULL);
	xb_builder_fixup_set_max_depth (fixup1, 2);
	xb_builder_source_add_fixup (source, fixup1);

	/* fix up any legacy installed files */
	fixup2 = xb_builder_fixup_new ("AppStreamUpgrade2",
				       gs_plugin_appstream_upgrade_cb,
				       self, NULL);
	xb_builder_fixup_set_max_depth (fixup2, 3);
	xb_builder_source_add_fixup (source, fixup2);

	/* add the origin as a search keyword for small repos */
	fixup3 = xb_builder_fixup_new ("AddOriginKeyword",
				       gs_plugin_appstream_add_origin_keyword_cb,
				       self, NULL);
	xb_builder_fixup_set_max_depth (fixup3, 1);
	xb_builder_source_add_fixup (source, fixup3);

#if LIBXMLB_CHECK_VERSION(0,3,1)
	fixup4 = xb_builder_fixup_new ("TextTokenize",
				       gs_plugin_appstream_tokenize_cb,
				       NULL, NULL);
	xb_builder_fixup_set_max_depth (fixup4, 2);
	xb_builder_source_add_fixup (source, fixup4);
#endif

	/* prepend media_baseurl to remote relative URLs */
	fixup5 = xb_builder_fixup_new ("MediaBaseUrl",
				       gs_plugin_appstream_media_baseurl_cb,
				       g_string_new (NULL),
				       gs_plugin_appstream_media_baseurl_free);
	xb_builder_fixup_set_max_depth (fixup5, 3);
	xb_builder_source_add_fixup (source, fixup5);

	/* success */
	xb_builder_import_source (builder, source);
	return TRUE;
}

static gboolean
gs_plugin_appstream_load_appstream (GsPluginAppstream  *self,
                                    XbBuilder          *builder,
                                    const gchar        *path,
                                    GCancellable       *cancellable,
                                    GError            **error)
{
	const gchar *fn;
	g_autoptr(GDir) dir = NULL;
	g_autoptr(GFile) parent = g_file_new_for_path (path);
	g_autoptr(GFileMonitor) file_monitor = NULL;
	g_autoptr(GError) local_error = NULL;

	/* parent path does not exist */
	if (!g_file_query_exists (parent, cancellable)) {
		g_debug ("appstream: Skipping appstream path '%s' as %s", path, g_cancellable_is_cancelled (cancellable) ? "cancelled" : "does not exist");
		return TRUE;
	}
	g_debug ("appstream: Loading appstream path '%s'", path);
	dir = g_dir_open (path, 0, error);
	if (dir == NULL)
		return FALSE;

	file_monitor = g_file_monitor (parent, G_FILE_MONITOR_NONE, cancellable, &local_error);
	if (local_error)
		g_debug ("appstream: Failed to create file monitor for '%s': %s", path, local_error->message);
	gs_plugin_appstream_maybe_store_file_monitor (self, file_monitor);

	while ((fn = g_dir_read_name (dir)) != NULL) {
#ifdef ENABLE_EXTERNAL_APPSTREAM
		/* Ignore our own system-installed files when
		   external-appstream-system-wide is FALSE */
		if (!g_settings_get_boolean (self->settings, "external-appstream-system-wide") &&
		    g_strcmp0 (path, gs_external_appstream_utils_get_system_dir ()) == 0 &&
		    g_str_has_prefix (fn, EXTERNAL_APPSTREAM_PREFIX))
			continue;
#endif
		if (g_str_has_suffix (fn, ".xml") ||
		    g_str_has_suffix (fn, ".yml") ||
		    g_str_has_suffix (fn, ".yml.gz") ||
		    g_str_has_suffix (fn, ".xml.gz")) {
			g_autofree gchar *filename = g_build_filename (path, fn, NULL);
			g_autoptr(GError) error_local = NULL;
			if (!gs_plugin_appstream_load_appstream_fn (self,
								    builder,
								    filename,
								    cancellable,
								    &error_local)) {
				g_debug ("ignoring %s: %s", filename, error_local->message);
				continue;
			}
		}
	}

	/* success */
	return TRUE;
}

static void
gs_add_appstream_metainfo_location (GPtrArray *locations, const gchar *root)
{
	g_ptr_array_add (locations,
			 g_build_filename (root, "metainfo", NULL));
	g_ptr_array_add (locations,
			 g_build_filename (root, "appdata", NULL));
}

static gboolean
gs_plugin_appstream_check_silo (GsPluginAppstream  *self,
                                GCancellable       *cancellable,
                                GError            **error)
{
	const gchar *test_xml;
	g_autofree gchar *blobfn = NULL;
	g_autoptr(XbBuilder) builder = NULL;
	g_autoptr(XbNode) n = NULL;
	g_autoptr(GFile) file = NULL;
	g_autoptr(GPtrArray) installed = NULL;
	g_autoptr(GRWLockReaderLocker) reader_locker = NULL;
	g_autoptr(GRWLockWriterLocker) writer_locker = NULL;
	g_autoptr(GPtrArray) parent_appdata = g_ptr_array_new_with_free_func (g_free);
	g_autoptr(GPtrArray) parent_appstream = NULL;
	g_autoptr(GMainContext) old_thread_default = NULL;

	reader_locker = g_rw_lock_reader_locker_new (&self->silo_lock);
	/* everything is okay */
	if (self->silo != NULL && xb_silo_is_valid (self->silo) &&
	    g_atomic_int_get (&self->file_monitor_stamp_current) == g_atomic_int_get (&self->file_monitor_stamp))
		return TRUE;
	g_clear_pointer (&reader_locker, g_rw_lock_reader_locker_free);

	/* drat! silo needs regenerating */
	writer_locker = g_rw_lock_writer_locker_new (&self->silo_lock);
 reload:
	g_clear_object (&self->silo);
	g_clear_pointer (&self->silo_filename, g_free);
	g_clear_pointer (&self->silo_installed_by_desktopid, g_hash_table_unref);
	g_clear_pointer (&self->silo_installed_by_id, g_hash_table_unref);
	self->default_scope = AS_COMPONENT_SCOPE_UNKNOWN;
	g_ptr_array_set_size (self->file_monitors, 0);
	g_atomic_int_set (&self->file_monitor_stamp_current, g_atomic_int_get (&self->file_monitor_stamp));

	/* FIXME: https://github.com/LingmoOS/lingmo-appstore/-/issues/1422 */
	old_thread_default = g_main_context_ref_thread_default ();
	if (old_thread_default == g_main_context_default ())
		g_clear_pointer (&old_thread_default, g_main_context_unref);
	if (old_thread_default != NULL)
		g_main_context_pop_thread_default (old_thread_default);
	builder = xb_builder_new ();
	if (old_thread_default != NULL)
		g_main_context_push_thread_default (old_thread_default);
	g_clear_pointer (&old_thread_default, g_main_context_unref);

	/* verbose profiling */
	if (g_getenv ("GS_XMLB_VERBOSE") != NULL) {
		xb_builder_set_profile_flags (builder,
					      XB_SILO_PROFILE_FLAG_XPATH |
					      XB_SILO_PROFILE_FLAG_DEBUG);
	}

	gs_appstream_add_current_locales (builder);

	/* only when in self test */
	test_xml = g_getenv ("GS_SELF_TEST_APPSTREAM_XML");
	if (test_xml != NULL) {
		g_autoptr(XbBuilderFixup) fixup1 = NULL;
		g_autoptr(XbBuilderFixup) fixup2 = NULL;
		g_autoptr(XbBuilderSource) source = xb_builder_source_new ();
		if (!xb_builder_source_load_xml (source, test_xml,
						 XB_BUILDER_SOURCE_FLAG_NONE,
						 error))
			return FALSE;
		fixup1 = xb_builder_fixup_new ("AddOriginKeywords",
					       gs_plugin_appstream_add_origin_keyword_cb,
					       self, NULL);
		xb_builder_fixup_set_max_depth (fixup1, 1);
		xb_builder_source_add_fixup (source, fixup1);
		fixup2 = xb_builder_fixup_new ("AddIcons",
					       gs_plugin_appstream_add_icons_cb,
					       self, NULL);
		xb_builder_fixup_set_max_depth (fixup2, 2);
		xb_builder_source_add_fixup (source, fixup2);
		xb_builder_import_source (builder, source);

		/* Nothing to watch in the tests */
		parent_appstream = g_ptr_array_new_with_free_func (g_free);
	} else {
		g_autoptr(GPtrArray) parent_desktop = g_ptr_array_new ();

		g_ptr_array_add (parent_desktop, (gpointer) DATADIR "/applications");
		if (g_strcmp0 (DATADIR, "/usr/share") != 0)
			g_ptr_array_add (parent_desktop, (gpointer) "/usr/share/applications");

		/* add search paths */
		parent_appstream = gs_appstream_get_appstream_data_dirs ();
		gs_add_appstream_metainfo_location (parent_appdata, DATADIR);

		/* Add the normal system directories if the installation prefix
		 * is different from normal — typically this happens when doing
		 * development builds. It’s useful to still list the system apps
		 * during development. */
		if (g_strcmp0 (DATADIR, "/usr/share") != 0)
			gs_add_appstream_metainfo_location (parent_appdata, "/usr/share");

		/* FIXME: https://github.com/LingmoOS/lingmo-appstore/-/issues/1422 */
		old_thread_default = g_main_context_ref_thread_default ();
		if (old_thread_default == g_main_context_default ())
			g_clear_pointer (&old_thread_default, g_main_context_unref);
		if (old_thread_default != NULL)
			g_main_context_pop_thread_default (old_thread_default);

		/* import all files */
		for (guint i = 0; i < parent_appstream->len; i++) {
			const gchar *fn = g_ptr_array_index (parent_appstream, i);
			if (!gs_plugin_appstream_load_appstream (self, builder, fn, cancellable, error)) {
				if (old_thread_default != NULL)
					g_main_context_push_thread_default (old_thread_default);
				return FALSE;
			}
		}
		for (guint i = 0; i < parent_appdata->len; i++) {
			const gchar *fn = g_ptr_array_index (parent_appdata, i);
			if (!gs_plugin_appstream_load_appdata (self, builder, fn, cancellable, error)) {
				if (old_thread_default != NULL)
					g_main_context_push_thread_default (old_thread_default);
				return FALSE;
			}
		}
		for (guint i = 0; i < parent_desktop->len; i++) {
			g_autoptr(GFileMonitor) file_monitor = NULL;
			const gchar *dir = g_ptr_array_index (parent_desktop, i);
			if (!gs_appstream_load_desktop_files (builder, dir, NULL, &file_monitor, cancellable, error)) {
				if (old_thread_default != NULL)
					g_main_context_push_thread_default (old_thread_default);
				return FALSE;
			}
			gs_plugin_appstream_maybe_store_file_monitor (self, file_monitor);
		}

		gs_appstream_add_data_merge_fixup (builder, parent_appstream, parent_desktop, cancellable);

		if (old_thread_default != NULL)
			g_main_context_push_thread_default (old_thread_default);
		g_clear_pointer (&old_thread_default, g_main_context_unref);
	}

	/* regenerate with each minor release */
	xb_builder_append_guid (builder, PACKAGE_VERSION);

	/* create per-user cache */
	blobfn = gs_utils_get_cache_filename ("appstream", "components.xmlb",
					      GS_UTILS_CACHE_FLAG_WRITEABLE |
					      GS_UTILS_CACHE_FLAG_CREATE_DIRECTORY,
					      error);
	if (blobfn == NULL)
		return FALSE;
	file = g_file_new_for_path (blobfn);
	g_debug ("ensuring %s", blobfn);

	/* FIXME: https://github.com/LingmoOS/lingmo-appstore/-/issues/1422 */
	old_thread_default = g_main_context_ref_thread_default ();
	if (old_thread_default == g_main_context_default ())
		g_clear_pointer (&old_thread_default, g_main_context_unref);
	if (old_thread_default != NULL)
		g_main_context_pop_thread_default (old_thread_default);

	self->silo = xb_builder_ensure (builder, file,
					XB_BUILDER_COMPILE_FLAG_IGNORE_INVALID |
					XB_BUILDER_COMPILE_FLAG_SINGLE_LANG,
					NULL, error);
	if (self->silo == NULL) {
		if (old_thread_default != NULL)
			g_main_context_push_thread_default (old_thread_default);
		return FALSE;
	}

	if (old_thread_default != NULL)
		g_main_context_push_thread_default (old_thread_default);

	if (g_atomic_int_get (&self->file_monitor_stamp_current) != g_atomic_int_get (&self->file_monitor_stamp)) {
		g_ptr_array_set_size (parent_appdata, 0);
		g_ptr_array_set_size (parent_appstream, 0);
		g_debug ("appstream: File monitors reported change while loading appstream data, reloading...");
		goto reload;
	}

	/* test we found something */
	n = xb_silo_query_first (self->silo, "components/component", NULL);
	if (n == NULL) {
		g_warning ("No AppStream data, try 'make install-sample-data' in data/");
		g_set_error (error,
			     GS_PLUGIN_ERROR,
			     GS_PLUGIN_ERROR_NOT_SUPPORTED,
			     "No AppStream data found");
		return FALSE;
	}

	g_clear_object (&n);

	self->silo_installed_by_desktopid = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) g_ptr_array_unref);
	self->silo_installed_by_id = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

	installed = xb_silo_query (self->silo, "/component[@type='desktop-application']/launchable[@type='desktop-id']", 0, NULL);
	for (guint i = 0; installed != NULL && i < installed->len; i++) {
		XbNode *launchable = g_ptr_array_index (installed, i);
		const gchar *id = xb_node_get_text (launchable);
		if (id != NULL && *id != '\0') {
			GPtrArray *nodes = g_hash_table_lookup (self->silo_installed_by_desktopid, id);
			if (nodes == NULL) {
				nodes = g_ptr_array_new_with_free_func (g_object_unref);
				g_hash_table_insert (self->silo_installed_by_desktopid, g_strdup (id), nodes);
			}
			g_ptr_array_add (nodes, xb_node_get_parent (launchable));
		}
	}

	g_clear_pointer (&installed, g_ptr_array_unref);
	installed = xb_silo_query (self->silo, "/component/id", 0, NULL);
	for (guint i = 0; installed != NULL && i < installed->len; i++) {
		XbNode *id_node = g_ptr_array_index (installed, i);
		const gchar *id = xb_node_get_text (id_node);
		if (id != NULL && *id != '\0')
			g_hash_table_add (self->silo_installed_by_id, g_strdup (id));
	}

	n = xb_silo_query_first (self->silo, "info", NULL);
	if (n != NULL) {
		g_autoptr(XbNode) child = NULL;
		g_autoptr(XbNode) next = NULL;
		for (child = xb_node_get_child (n);
		     child != NULL && (self->silo_filename == NULL || self->default_scope == AS_COMPONENT_SCOPE_UNKNOWN);
		     g_object_unref (child), child = g_steal_pointer (&next)) {
			const gchar *elem = xb_node_get_element (child);
			next = xb_node_get_next (child);
			if (self->silo_filename == NULL && g_strcmp0 (elem, "filename") == 0) {
				self->silo_filename = g_strdup (xb_node_get_text (child));
			} else if (self->default_scope == AS_COMPONENT_SCOPE_UNKNOWN && g_strcmp0 (elem, "scope") == 0) {
				const gchar *tmp = xb_node_get_text (child);
				if (tmp != NULL)
					self->default_scope = as_component_scope_from_string (tmp);
			}
		}
	}

	/* success */
	return TRUE;
}

static void
gs_plugin_appstream_reload (GsPlugin *plugin)
{
	GsPluginAppstream *self;
	g_autoptr(GsAppList) list = NULL;
	g_autoptr(GRWLockWriterLocker) writer_locker = NULL;
	guint sz;

	g_return_if_fail (GS_IS_PLUGIN_APPSTREAM (plugin));

	list = gs_plugin_list_cached (plugin);
	sz = gs_app_list_length (list);
	for (guint i = 0; i < sz; i++) {
		GsApp *app = gs_app_list_index (list, i);
		/* to ensure the app states are refined */
		gs_app_set_state (app, GS_APP_STATE_UNKNOWN);
	}

	self = GS_PLUGIN_APPSTREAM (plugin);
	writer_locker = g_rw_lock_writer_locker_new (&self->silo_lock);
	if (self->silo != NULL)
		xb_silo_invalidate (self->silo);
}

static gint
get_priority_for_interactivity (gboolean interactive)
{
	return interactive ? G_PRIORITY_DEFAULT : G_PRIORITY_LOW;
}

static void setup_thread_cb (GTask        *task,
                             gpointer      source_object,
                             gpointer      task_data,
                             GCancellable *cancellable);

static void
gs_plugin_appstream_setup_async (GsPlugin            *plugin,
                                 GCancellable        *cancellable,
                                 GAsyncReadyCallback  callback,
                                 gpointer             user_data)
{
	GsPluginAppstream *self = GS_PLUGIN_APPSTREAM (plugin);
	g_autoptr(GTask) task = NULL;

	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_appstream_setup_async);

	/* Start up a worker thread to process all the plugin’s function calls. */
	self->worker = gs_worker_thread_new ("gs-plugin-appstream");

	/* Queue a job to check the silo, which will cause it to be loaded. */
	gs_worker_thread_queue (self->worker, G_PRIORITY_DEFAULT,
				setup_thread_cb, g_steal_pointer (&task));
}

/* Run in @worker. */
static void
setup_thread_cb (GTask        *task,
                 gpointer      source_object,
                 gpointer      task_data,
                 GCancellable *cancellable)
{
	GsPluginAppstream *self = GS_PLUGIN_APPSTREAM (source_object);
	g_autoptr(GError) local_error = NULL;

	assert_in_worker (self);

	if (!gs_plugin_appstream_check_silo (self, cancellable, &local_error))
		g_task_return_error (task, g_steal_pointer (&local_error));
	else
		g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_appstream_setup_finish (GsPlugin      *plugin,
                                  GAsyncResult  *result,
                                  GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void shutdown_cb (GObject      *source_object,
                         GAsyncResult *result,
                         gpointer      user_data);

static void
gs_plugin_appstream_shutdown_async (GsPlugin            *plugin,
                                    GCancellable        *cancellable,
                                    GAsyncReadyCallback  callback,
                                    gpointer             user_data)
{
	GsPluginAppstream *self = GS_PLUGIN_APPSTREAM (plugin);
	g_autoptr(GTask) task = NULL;

	task = g_task_new (self, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_appstream_shutdown_async);

	/* Stop the worker thread. */
	gs_worker_thread_shutdown_async (self->worker, cancellable, shutdown_cb, g_steal_pointer (&task));
}

static void
shutdown_cb (GObject      *source_object,
             GAsyncResult *result,
             gpointer      user_data)
{
	g_autoptr(GTask) task = G_TASK (user_data);
	GsPluginAppstream *self = g_task_get_source_object (task);
	g_autoptr(GsWorkerThread) worker = NULL;
	g_autoptr(GError) local_error = NULL;

	worker = g_steal_pointer (&self->worker);

	if (!gs_worker_thread_shutdown_finish (worker, result, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_appstream_shutdown_finish (GsPlugin      *plugin,
                                     GAsyncResult  *result,
                                     GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

gboolean
gs_plugin_url_to_app (GsPlugin *plugin,
		      GsAppList *list,
		      const gchar *url,
		      GCancellable *cancellable,
		      GError **error)
{
	GsPluginAppstream *self = GS_PLUGIN_APPSTREAM (plugin);
	g_autoptr(GRWLockReaderLocker) locker = NULL;

	/* check silo is valid */
	if (!gs_plugin_appstream_check_silo (self, cancellable, error))
		return FALSE;

	locker = g_rw_lock_reader_locker_new (&self->silo_lock);

	return gs_appstream_url_to_app (plugin, self->silo, list, url, cancellable, error);
}

static void
gs_plugin_appstream_set_compulsory_quirk (GsApp *app, XbNode *component)
{
	g_autoptr(GPtrArray) array = NULL;
	const gchar *current_desktop;

	/*
	 * Set the core applications for the current desktop that cannot be
	 * removed.
	 *
	 * If XDG_CURRENT_DESKTOP contains ":", indicating that it is made up
	 * of multiple components per the Desktop Entry Specification, an app
	 * is compulsory if any of the components in XDG_CURRENT_DESKTOP match
	 * any value in <compulsory_for_desktops />. In that way,
	 * "LINGMO-Classic:LINGMO" shares compulsory apps with LINGMO.
	 *
	 * As a special case, if the <compulsory_for_desktop /> value contains
	 * a ":", we match the entire XDG_CURRENT_DESKTOP. This lets people set
	 * compulsory apps for such compound desktops if they want.
	 *
	 */
	array = xb_node_query (component, "compulsory_for_desktop", 0, NULL);
	if (array == NULL)
		return;
	current_desktop = g_getenv ("XDG_CURRENT_DESKTOP");
	if (current_desktop != NULL) {
		g_auto(GStrv) xdg_current_desktops = g_strsplit (current_desktop, ":", 0);
		for (guint i = 0; i < array->len; i++) {
			XbNode *n = g_ptr_array_index (array, i);
			const gchar *tmp = xb_node_get_text (n);
			/* if the value has a :, check the whole string */
			if (g_strstr_len (tmp, -1, ":")) {
				if (g_strcmp0 (current_desktop, tmp) == 0) {
					gs_app_add_quirk (app, GS_APP_QUIRK_COMPULSORY);
					break;
				}
			/* otherwise check if any element matches this one */
			} else if (g_strv_contains ((const gchar * const *) xdg_current_desktops, tmp)) {
				gs_app_add_quirk (app, GS_APP_QUIRK_COMPULSORY);
				break;
			}
		}
	}
}

static gboolean
gs_plugin_appstream_refine_state (GsPluginAppstream  *self,
                                  GsApp              *app,
                                  GError            **error)
{
	g_autoptr(GRWLockReaderLocker) locker = NULL;

	/* Ignore apps with no ID */
	if (gs_app_get_id (app) == NULL)
		return TRUE;

	locker = g_rw_lock_reader_locker_new (&self->silo_lock);

	if (g_hash_table_contains (self->silo_installed_by_id, gs_app_get_id (app)))
		gs_app_set_state (app, GS_APP_STATE_INSTALLED);
	return TRUE;
}

static gboolean
gs_plugin_refine_from_id (GsPluginAppstream    *self,
                          GsApp                *app,
                          GsPluginRefineFlags   flags,
			  GHashTable           *apps_by_id,
			  GHashTable           *apps_by_origin_and_id,
                          gboolean             *found,
                          GError              **error)
{
	const gchar *id, *origin;
	GPtrArray *components;
	g_autoptr(GRWLockReaderLocker) locker = NULL;

	/* not enough info to find */
	id = gs_app_get_id (app);
	if (id == NULL)
		return TRUE;

	locker = g_rw_lock_reader_locker_new (&self->silo_lock);

	origin = gs_app_get_origin_appstream (app);

	/* look in AppStream then fall back to AppData */
	if (origin && *origin) {
		g_autofree gchar *key = g_strconcat (origin, "\n", id, NULL);
		components = g_hash_table_lookup (apps_by_origin_and_id, key);
	} else {
		components = g_hash_table_lookup (apps_by_id, id);
	}
	if (components != NULL) {
		for (guint i = 0; i < components->len; i++) {
			XbNode *component = g_ptr_array_index (components, i);
			if (!gs_appstream_refine_app (GS_PLUGIN (self), app, self->silo, component, flags, self->silo_installed_by_desktopid,
						      self->silo_filename ? self->silo_filename : "", self->default_scope, error))
				return FALSE;
			gs_plugin_appstream_set_compulsory_quirk (app, component);
		}
	}

	/* if an installed desktop or appdata file exists set to installed */
	if (gs_app_get_state (app) == GS_APP_STATE_UNKNOWN) {
		if (!gs_plugin_appstream_refine_state (self, app, error))
			return FALSE;
	}

	/* success */
	*found = TRUE;
	return TRUE;
}

static gboolean
gs_plugin_refine_from_pkgname (GsPluginAppstream    *self,
                               GsApp                *app,
                               GsPluginRefineFlags   flags,
                               GError              **error)
{
	GPtrArray *sources = gs_app_get_sources (app);
	g_autoptr(GError) error_local = NULL;

	/* not enough info to find */
	if (sources->len == 0)
		return TRUE;

	/* find all apps when matching any prefixes */
	for (guint j = 0; j < sources->len; j++) {
		const gchar *pkgname = g_ptr_array_index (sources, j);
		g_autoptr(GRWLockReaderLocker) locker = NULL;
		g_autoptr(GString) xpath = g_string_new (NULL);
		g_autoptr(XbNode) component = NULL;

		locker = g_rw_lock_reader_locker_new (&self->silo_lock);

		/* prefer actual apps and then fallback to anything else */
		xb_string_append_union (xpath, "components/component[@type='desktop-application']/pkgname[text()='%s']/..", pkgname);
		xb_string_append_union (xpath, "components/component[@type='console-application']/pkgname[text()='%s']/..", pkgname);
		xb_string_append_union (xpath, "components/component[@type='web-application']/pkgname[text()='%s']/..", pkgname);
		xb_string_append_union (xpath, "components/component/pkgname[text()='%s']/..", pkgname);
		component = xb_silo_query_first (self->silo, xpath->str, &error_local);
		if (component == NULL) {
			if (g_error_matches (error_local, G_IO_ERROR, G_IO_ERROR_NOT_FOUND))
				continue;
			g_propagate_error (error, g_steal_pointer (&error_local));
			return FALSE;
		}
		if (!gs_appstream_refine_app (GS_PLUGIN (self), app, self->silo, component, flags, self->silo_installed_by_desktopid,
					      self->silo_filename ? self->silo_filename : "", self->default_scope, error))
			return FALSE;
		gs_plugin_appstream_set_compulsory_quirk (app, component);
	}

	/* if an installed desktop or appdata file exists set to installed */
	if (gs_app_get_state (app) == GS_APP_STATE_UNKNOWN) {
		if (!gs_plugin_appstream_refine_state (self, app, error))
			return FALSE;
	}

	/* success */
	return TRUE;
}

static void refine_thread_cb (GTask        *task,
                              gpointer      source_object,
                              gpointer      task_data,
                              GCancellable *cancellable);

static void
gs_plugin_appstream_refine_async (GsPlugin            *plugin,
                                  GsAppList           *list,
                                  GsPluginRefineFlags  flags,
                                  GCancellable        *cancellable,
                                  GAsyncReadyCallback  callback,
                                  gpointer             user_data)
{
	GsPluginAppstream *self = GS_PLUGIN_APPSTREAM (plugin);
	g_autoptr(GTask) task = NULL;
	gboolean interactive = gs_plugin_has_flags (GS_PLUGIN (self), GS_PLUGIN_FLAGS_INTERACTIVE);

	task = gs_plugin_refine_data_new_task (plugin, list, flags, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_appstream_refine_async);

	/* Queue a job for the refine. */
	gs_worker_thread_queue (self->worker, get_priority_for_interactivity (interactive),
				refine_thread_cb, g_steal_pointer (&task));
}

static gboolean refine_wildcard (GsPluginAppstream    *self,
                                 GsApp                *app,
                                 GsAppList            *list,
                                 GsPluginRefineFlags   refine_flags,
				 GHashTable           *apps_by_id,
                                 GCancellable         *cancellable,
                                 GError              **error);

/* Run in @worker. */
static void
refine_thread_cb (GTask        *task,
                  gpointer      source_object,
                  gpointer      task_data,
                  GCancellable *cancellable)
{
	GsPluginAppstream *self = GS_PLUGIN_APPSTREAM (source_object);
	GsPluginRefineData *data = task_data;
	GsAppList *list = data->list;
	GsPluginRefineFlags flags = data->flags;
	gboolean found = FALSE;
	g_autoptr(GsAppList) app_list = NULL;
	g_autoptr(GHashTable) apps_by_id = NULL;
	g_autoptr(GHashTable) apps_by_origin_and_id = NULL;
	g_autoptr(GPtrArray) components = NULL;
	g_autoptr(GRWLockReaderLocker) locker = NULL;
	g_autoptr(GError) local_error = NULL;

	assert_in_worker (self);

	/* check silo is valid */
	if (!gs_plugin_appstream_check_silo (self, cancellable, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	apps_by_id = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) g_ptr_array_unref);
	apps_by_origin_and_id = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) g_ptr_array_unref);

	locker = g_rw_lock_reader_locker_new (&self->silo_lock);

	components = xb_silo_query (self->silo, "components/component/id", 0, NULL);
	for (guint i = 0; components != NULL && i < components->len; i++) {
		XbNode *node = g_ptr_array_index (components, i);
		g_autoptr(XbNode) component_node = xb_node_get_parent (node);
		g_autoptr(XbNode) components_node = xb_node_get_parent (component_node);
		GPtrArray *comps;
		const gchar *comp_id = xb_node_get_text (node);
		const gchar *origin;

		/* include only web-apps and those with pkgname */
		if (g_strcmp0 (xb_node_get_attr (component_node, "type"), "web-application") != 0) {
			g_autoptr(XbNode) child = NULL;
			g_autoptr(XbNode) next = NULL;
			gboolean found_pkgname = FALSE;

			for (child = xb_node_get_child (component_node); child != NULL; g_object_unref (child), child = g_steal_pointer (&next)) {
				next = xb_node_get_next (child);
				if (g_strcmp0 (xb_node_get_element (child), "pkgname") == 0) {
					found_pkgname = TRUE;
					break;
				}
			}

			if (!found_pkgname)
				continue;
		}

		comps = g_hash_table_lookup (apps_by_id, comp_id);
		if (comps == NULL) {
			comps = g_ptr_array_new_with_free_func (g_object_unref);
			g_hash_table_insert (apps_by_id, g_strdup (comp_id), comps);
		}
		g_ptr_array_add (comps, g_object_ref (component_node));

		origin = xb_node_get_attr (components_node, "origin");
		if (origin != NULL) {
			g_autofree gchar *key = g_strconcat (origin, "\n", comp_id, NULL);
			comps = g_hash_table_lookup (apps_by_origin_and_id, key);
			if (comps == NULL) {
				comps = g_ptr_array_new_with_free_func (g_object_unref);
				g_hash_table_insert (apps_by_origin_and_id, g_steal_pointer (&key), comps);
			}
			g_ptr_array_add (comps, g_object_ref (component_node));
		}
	}

	g_clear_pointer (&components, g_ptr_array_unref);
	components = xb_silo_query (self->silo, "component/id", 0, NULL);
	for (guint i = 0; components != NULL && i < components->len; i++) {
		XbNode *node = g_ptr_array_index (components, i);
		g_autoptr(XbNode) component_node = xb_node_get_parent (node);
		GPtrArray *comps;
		const gchar *comp_id = xb_node_get_text (node);

		comps = g_hash_table_lookup (apps_by_id, comp_id);
		if (comps == NULL) {
			comps = g_ptr_array_new_with_free_func (g_object_unref);
			g_hash_table_insert (apps_by_id, g_strdup (comp_id), comps);
		}
		g_ptr_array_add (comps, g_object_ref (component_node));
	}

	for (guint i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app = gs_app_list_index (list, i);

		/* not us */
		if (gs_app_get_bundle_kind (app) != AS_BUNDLE_KIND_PACKAGE &&
		    gs_app_get_bundle_kind (app) != AS_BUNDLE_KIND_UNKNOWN)
			continue;

		if (gs_app_has_quirk (app, GS_APP_QUIRK_IS_WILDCARD))
			continue;

		/* find by ID then fall back to package name */
		if (!gs_plugin_refine_from_id (self, app, flags, apps_by_id, apps_by_origin_and_id, &found, &local_error)) {
			g_task_return_error (task, g_steal_pointer (&local_error));
			return;
		}
		if (!found) {
			if (!gs_plugin_refine_from_pkgname (self, app, flags, &local_error)) {
				g_task_return_error (task, g_steal_pointer (&local_error));
				return;
			}
		}
	}

	/* Refine wildcards.
	 *
	 * Use a copy of the list for the loop because a function called
	 * on the plugin may affect the list which can lead to problems
	 * (e.g. inserting an app in the list on every call results in
	 * an infinite loop) */
	app_list = gs_app_list_copy (list);

	for (guint j = 0; j < gs_app_list_length (app_list); j++) {
		GsApp *app = gs_app_list_index (app_list, j);

		if (gs_app_has_quirk (app, GS_APP_QUIRK_IS_WILDCARD) &&
		    !refine_wildcard (self, app, list, flags, apps_by_id, cancellable, &local_error)) {
			g_task_return_error (task, g_steal_pointer (&local_error));
			return;
		}
	}

	/* success */
	g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_appstream_refine_finish (GsPlugin      *plugin,
                                   GAsyncResult  *result,
                                   GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

/* Run in @worker. Silo must be valid */
static gboolean
refine_wildcard (GsPluginAppstream    *self,
                 GsApp                *app,
                 GsAppList            *list,
                 GsPluginRefineFlags   refine_flags,
		 GHashTable           *apps_by_id,
                 GCancellable         *cancellable,
                 GError              **error)
{
	const gchar *id;
	GPtrArray *components;
	g_autoptr(GRWLockReaderLocker) locker = NULL;

	/* not enough info to find */
	id = gs_app_get_id (app);
	if (id == NULL)
		return TRUE;

	locker = g_rw_lock_reader_locker_new (&self->silo_lock);

	components = g_hash_table_lookup (apps_by_id, id);
	if (components == NULL)
		return TRUE;
	for (guint i = 0; i < components->len; i++) {
		XbNode *component = g_ptr_array_index (components, i);
		g_autoptr(GsApp) new = NULL;

		/* new app */
		new = gs_appstream_create_app (GS_PLUGIN (self), self->silo, component, self->silo_filename ? self->silo_filename : "",
					       self->default_scope, error);
		if (new == NULL)
			return FALSE;
		gs_app_set_scope (new, AS_COMPONENT_SCOPE_SYSTEM);
		gs_app_subsume_metadata (new, app);
		if (!gs_appstream_refine_app (GS_PLUGIN (self), new, self->silo, component, refine_flags, self->silo_installed_by_desktopid,
					      self->silo_filename ? self->silo_filename : "", self->default_scope, error))
			return FALSE;
		gs_plugin_appstream_set_compulsory_quirk (new, component);

		/* if an installed desktop or appdata file exists set to installed */
		if (gs_app_get_state (new) == GS_APP_STATE_UNKNOWN) {
			if (!gs_plugin_appstream_refine_state (self, new, error))
				return FALSE;
		}

		gs_app_list_add (list, new);
	}

	/* success */
	return TRUE;
}

static void refine_categories_thread_cb (GTask        *task,
                                         gpointer      source_object,
                                         gpointer      task_data,
                                         GCancellable *cancellable);

static void
gs_plugin_appstream_refine_categories_async (GsPlugin                      *plugin,
                                             GPtrArray                     *list,
                                             GsPluginRefineCategoriesFlags  flags,
                                             GCancellable                  *cancellable,
                                             GAsyncReadyCallback            callback,
                                             gpointer                       user_data)
{
	GsPluginAppstream *self = GS_PLUGIN_APPSTREAM (plugin);
	g_autoptr(GTask) task = NULL;
	gboolean interactive = (flags & GS_PLUGIN_REFINE_CATEGORIES_FLAGS_INTERACTIVE);

	task = gs_plugin_refine_categories_data_new_task (plugin, list, flags,
							  cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_appstream_refine_categories_async);

	/* All we actually do is add the sizes of each category. If that’s
	 * not been requested, avoid queueing a worker job. */
	if (!(flags & GS_PLUGIN_REFINE_CATEGORIES_FLAGS_SIZE)) {
		g_task_return_boolean (task, TRUE);
		return;
	}

	/* Queue a job to get the apps. */
	gs_worker_thread_queue (self->worker, get_priority_for_interactivity (interactive),
				refine_categories_thread_cb, g_steal_pointer (&task));
}

/* Run in @worker. */
static void
refine_categories_thread_cb (GTask        *task,
                             gpointer      source_object,
                             gpointer      task_data,
                             GCancellable *cancellable)
{
	GsPluginAppstream *self = GS_PLUGIN_APPSTREAM (source_object);
	g_autoptr(GRWLockReaderLocker) locker = NULL;
	GsPluginRefineCategoriesData *data = task_data;
	g_autoptr(GError) local_error = NULL;

	assert_in_worker (self);

	/* check silo is valid */
	if (!gs_plugin_appstream_check_silo (self, cancellable, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	locker = g_rw_lock_reader_locker_new (&self->silo_lock);

	if (!gs_appstream_refine_category_sizes (self->silo, data->list, cancellable, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_appstream_refine_categories_finish (GsPlugin      *plugin,
                                              GAsyncResult  *result,
                                              GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void list_apps_thread_cb (GTask        *task,
                                 gpointer      source_object,
                                 gpointer      task_data,
                                 GCancellable *cancellable);

static void
gs_plugin_appstream_list_apps_async (GsPlugin              *plugin,
                                     GsAppQuery            *query,
                                     GsPluginListAppsFlags  flags,
                                     GCancellable          *cancellable,
                                     GAsyncReadyCallback    callback,
                                     gpointer               user_data)
{
	GsPluginAppstream *self = GS_PLUGIN_APPSTREAM (plugin);
	g_autoptr(GTask) task = NULL;
	gboolean interactive = (flags & GS_PLUGIN_LIST_APPS_FLAGS_INTERACTIVE);

	task = gs_plugin_list_apps_data_new_task (plugin, query, flags,
						  cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_appstream_list_apps_async);

	/* Queue a job to get the apps. */
	gs_worker_thread_queue (self->worker, get_priority_for_interactivity (interactive),
				list_apps_thread_cb, g_steal_pointer (&task));
}

/* Run in @worker. */
static void
list_apps_thread_cb (GTask        *task,
                     gpointer      source_object,
                     gpointer      task_data,
                     GCancellable *cancellable)
{
	GsPluginAppstream *self = GS_PLUGIN_APPSTREAM (source_object);
	g_autoptr(GRWLockReaderLocker) locker = NULL;
	g_autoptr(GsAppList) list = gs_app_list_new ();
	GsPluginListAppsData *data = task_data;
	GDateTime *released_since = NULL;
	GsAppQueryTristate is_curated = GS_APP_QUERY_TRISTATE_UNSET;
	GsAppQueryTristate is_featured = GS_APP_QUERY_TRISTATE_UNSET;
	GsCategory *category = NULL;
	GsAppQueryTristate is_installed = GS_APP_QUERY_TRISTATE_UNSET;
	guint64 age_secs = 0;
	const gchar * const *deployment_featured = NULL;
	const gchar * const *developers = NULL;
	const gchar * const *keywords = NULL;
	GsApp *alternate_of = NULL;
	g_autoptr(GError) local_error = NULL;

	assert_in_worker (self);

	if (data->query != NULL) {
		released_since = gs_app_query_get_released_since (data->query);
		is_curated = gs_app_query_get_is_curated (data->query);
		is_featured = gs_app_query_get_is_featured (data->query);
		category = gs_app_query_get_category (data->query);
		is_installed = gs_app_query_get_is_installed (data->query);
		deployment_featured = gs_app_query_get_deployment_featured (data->query);
		developers = gs_app_query_get_developers (data->query);
		keywords = gs_app_query_get_keywords (data->query);
		alternate_of = gs_app_query_get_alternate_of (data->query);
	}
	if (released_since != NULL) {
		g_autoptr(GDateTime) now = g_date_time_new_now_utc ();
		age_secs = g_date_time_difference (now, released_since) / G_TIME_SPAN_SECOND;
	}

	/* Currently only support a subset of query properties, and only one set at once.
	 * Also don’t currently support GS_APP_QUERY_TRISTATE_FALSE. */
	if ((released_since == NULL &&
	     is_curated == GS_APP_QUERY_TRISTATE_UNSET &&
	     is_featured == GS_APP_QUERY_TRISTATE_UNSET &&
	     category == NULL &&
	     is_installed == GS_APP_QUERY_TRISTATE_UNSET &&
	     deployment_featured == NULL &&
	     developers == NULL &&
	     keywords == NULL &&
	     alternate_of == NULL) ||
	    is_curated == GS_APP_QUERY_TRISTATE_FALSE ||
	    is_featured == GS_APP_QUERY_TRISTATE_FALSE ||
	    is_installed == GS_APP_QUERY_TRISTATE_FALSE ||
	    gs_app_query_get_n_properties_set (data->query) != 1) {
		g_task_return_new_error (task, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
					 "Unsupported query");
		return;
	}

	/* check silo is valid */
	if (!gs_plugin_appstream_check_silo (self, cancellable, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	locker = g_rw_lock_reader_locker_new (&self->silo_lock);

	if (released_since != NULL &&
	    !gs_appstream_add_recent (GS_PLUGIN (self), self->silo, list, age_secs,
				      cancellable, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	if (is_curated != GS_APP_QUERY_TRISTATE_UNSET &&
	    !gs_appstream_add_popular (self->silo, list, cancellable, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	if (is_featured != GS_APP_QUERY_TRISTATE_UNSET &&
	    !gs_appstream_add_featured (self->silo, list, cancellable, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	if (category != NULL &&
	    !gs_appstream_add_category_apps (GS_PLUGIN (self), self->silo, category, list, cancellable, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	if (is_installed == GS_APP_QUERY_TRISTATE_TRUE &&
	    !gs_appstream_add_installed (GS_PLUGIN (self), self->silo, list, cancellable, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	if (deployment_featured != NULL &&
	    !gs_appstream_add_deployment_featured (self->silo, deployment_featured, list,
						   cancellable, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	if (developers != NULL &&
	    !gs_appstream_search_developer_apps (GS_PLUGIN (self), self->silo, developers, list, cancellable, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	if (keywords != NULL &&
	    !gs_appstream_search (GS_PLUGIN (self), self->silo, keywords, list, cancellable, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	if (alternate_of != NULL &&
	    !gs_appstream_add_alternates (self->silo, alternate_of, list, cancellable, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	g_task_return_pointer (task, g_steal_pointer (&list), g_object_unref);
}

static GsAppList *
gs_plugin_appstream_list_apps_finish (GsPlugin      *plugin,
                                      GAsyncResult  *result,
                                      GError       **error)
{
	return g_task_propagate_pointer (G_TASK (result), error);
}

static void refresh_metadata_thread_cb (GTask        *task,
                                        gpointer      source_object,
                                        gpointer      task_data,
                                        GCancellable *cancellable);

static void
gs_plugin_appstream_refresh_metadata_async (GsPlugin                     *plugin,
                                            guint64                       cache_age_secs,
                                            GsPluginRefreshMetadataFlags  flags,
                                            GCancellable                 *cancellable,
                                            GAsyncReadyCallback           callback,
                                            gpointer                      user_data)
{
	GsPluginAppstream *self = GS_PLUGIN_APPSTREAM (plugin);
	g_autoptr(GTask) task = NULL;
	gboolean interactive = (flags & GS_PLUGIN_REFRESH_METADATA_FLAGS_INTERACTIVE);

	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_appstream_refresh_metadata_async);

	/* Queue a job to check the silo, which will cause it to be refreshed if needed. */
	gs_worker_thread_queue (self->worker, get_priority_for_interactivity (interactive),
				refresh_metadata_thread_cb, g_steal_pointer (&task));
}

/* Run in @worker. */
static void
refresh_metadata_thread_cb (GTask        *task,
                            gpointer      source_object,
                            gpointer      task_data,
                            GCancellable *cancellable)
{
	GsPluginAppstream *self = GS_PLUGIN_APPSTREAM (source_object);
	g_autoptr(GError) local_error = NULL;

	assert_in_worker (self);

	/* Checking the silo will refresh it if needed. */
	if (!gs_plugin_appstream_check_silo (self, cancellable, &local_error))
		g_task_return_error (task, g_steal_pointer (&local_error));
	else
		g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_appstream_refresh_metadata_finish (GsPlugin      *plugin,
                                             GAsyncResult  *result,
                                             GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gs_plugin_appstream_class_init (GsPluginAppstreamClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPluginClass *plugin_class = GS_PLUGIN_CLASS (klass);

	object_class->dispose = gs_plugin_appstream_dispose;

	plugin_class->reload = gs_plugin_appstream_reload;
	plugin_class->setup_async = gs_plugin_appstream_setup_async;
	plugin_class->setup_finish = gs_plugin_appstream_setup_finish;
	plugin_class->shutdown_async = gs_plugin_appstream_shutdown_async;
	plugin_class->shutdown_finish = gs_plugin_appstream_shutdown_finish;
	plugin_class->refine_async = gs_plugin_appstream_refine_async;
	plugin_class->refine_finish = gs_plugin_appstream_refine_finish;
	plugin_class->list_apps_async = gs_plugin_appstream_list_apps_async;
	plugin_class->list_apps_finish = gs_plugin_appstream_list_apps_finish;
	plugin_class->refresh_metadata_async = gs_plugin_appstream_refresh_metadata_async;
	plugin_class->refresh_metadata_finish = gs_plugin_appstream_refresh_metadata_finish;
	plugin_class->refine_categories_async = gs_plugin_appstream_refine_categories_async;
	plugin_class->refine_categories_finish = gs_plugin_appstream_refine_categories_finish;
}

GType
gs_plugin_query_type (void)
{
	return GS_TYPE_PLUGIN_APPSTREAM;
}
