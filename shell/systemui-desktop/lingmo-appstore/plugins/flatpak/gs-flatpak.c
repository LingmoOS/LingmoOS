/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016 Joaquim Rocha <jrocha@endlessm.com>
 * Copyright (C) 2016-2018 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2016-2019 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/* Notes:
 *
 * All GsApp's created have management-plugin set to flatpak
 * The GsApp:origin is the remote name, e.g. test-repo
 *
 * Two #FlatpakInstallation objects are kept: `installation_noninteractive` and
 * `installation_interactive`. One has flatpak_installation_set_no_interaction()
 * set to %TRUE, the other to %FALSE.
 *
 * This is because multiple #GsFlatpak operations can be ongoing with different
 * interactive states (for example, a background refresh operation while the
 * user is refining an app in the foreground), but the #FlatpakInstallation
 * methods don’t support per-operation interactive state.
 *
 * Internally, each #FlatpakInstallation will use a separate #FlatpakDir
 * pointing to the same repository. Those #FlatpakDirs will lock the repository
 * when using it, so parallel operations won’t race.
 */

#include <config.h>

#include <glib/gi18n.h>
#include <xmlb.h>

#include "gs-appstream.h"
#include "gs-app-private.h"
#include "gs-flatpak-app.h"
#include "gs-flatpak.h"
#include "gs-flatpak-transaction.h"
#include "gs-flatpak-utils.h"
#include "gs-profiler.h"

struct _GsFlatpak {
	GObject			 parent_instance;
	GsFlatpakFlags		 flags;
	FlatpakInstallation	*installation_noninteractive;  /* (owned) */
	FlatpakInstallation	*installation_interactive;  /* (owned) */
	GPtrArray		*installed_refs;  /* must be entirely replaced rather than updated internally */
	GHashTable		*remotes_by_name;
	GMutex			 installed_refs_mutex;
	GHashTable		*broken_remotes;
	GMutex			 broken_remotes_mutex;
	GFileMonitor		*monitor;
	AsComponentScope	 scope;
	GsPlugin		*plugin;
	XbSilo			*silo;
	GRWLock			 silo_lock;
	gchar			*silo_filename;
	GHashTable		*silo_installed_by_desktopid;
	gchar			*id;
	guint			 changed_id;
	GHashTable		*app_silos;
	GMutex			 app_silos_mutex;
	GHashTable		*remote_title; /* gchar *remote name ~> gchar *remote title */
	GMutex			 remote_title_mutex;
	gboolean		 requires_full_rescan;
	gint			 busy; /* (atomic) */
	gboolean		 changed_while_busy;
};

G_DEFINE_TYPE (GsFlatpak, gs_flatpak, G_TYPE_OBJECT)

static void
gs_plugin_refine_item_scope (GsFlatpak *self, GsApp *app)
{
	if (gs_app_get_scope (app) == AS_COMPONENT_SCOPE_UNKNOWN &&
	    (self->flags & GS_FLATPAK_FLAG_IS_TEMPORARY) == 0) {
		gboolean is_user = flatpak_installation_get_is_user (self->installation_noninteractive);
		gs_app_set_scope (app, is_user ? AS_COMPONENT_SCOPE_USER : AS_COMPONENT_SCOPE_SYSTEM);
	}
}

static void
gs_flatpak_claim_app (GsFlatpak *self, GsApp *app)
{
	if (!gs_app_has_management_plugin (app, NULL))
		return;

	gs_app_set_management_plugin (app, self->plugin);
	gs_flatpak_app_set_packaging_info (app);

	/* only when we have a non-temp object */
	if ((self->flags & GS_FLATPAK_FLAG_IS_TEMPORARY) == 0) {
		gs_app_set_scope (app, self->scope);
		gs_flatpak_app_set_object_id (app, gs_flatpak_get_id (self));
	}
}

static void
gs_flatpak_ensure_remote_title (GsFlatpak *self,
				gboolean interactive,
				GCancellable *cancellable)
{
	g_autoptr(GMutexLocker) locker = g_mutex_locker_new (&self->remote_title_mutex);
	g_autoptr(GPtrArray) xremotes = NULL;

	if (g_hash_table_size (self->remote_title))
		return;

	xremotes = flatpak_installation_list_remotes (gs_flatpak_get_installation (self, interactive), cancellable, NULL);
	if (xremotes) {
		guint ii;

		for (ii = 0; ii < xremotes->len; ii++) {
			FlatpakRemote *xremote = g_ptr_array_index (xremotes, ii);

			if (flatpak_remote_get_disabled (xremote) ||
			    !flatpak_remote_get_name (xremote))
				continue;

			g_hash_table_insert (self->remote_title, g_strdup (flatpak_remote_get_name (xremote)), flatpak_remote_get_title (xremote));
		}
	}
}

static void
gs_flatpak_set_app_origin (GsFlatpak *self,
			   GsApp *app,
			   const gchar *origin,
			   FlatpakRemote *xremote,
			   gboolean interactive,
			   GCancellable *cancellable)
{
	g_autoptr(GMutexLocker) locker = NULL;
	g_autofree gchar *tmp = NULL;
	const gchar *title = NULL;

	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (origin != NULL);

	if (xremote) {
		tmp = flatpak_remote_get_title (xremote);
		title = tmp;
	} else {
		locker = g_mutex_locker_new (&self->remote_title_mutex);
		title = g_hash_table_lookup (self->remote_title, origin);
	}

	if (!title) {
		g_autoptr(GPtrArray) xremotes = NULL;

		xremotes = flatpak_installation_list_remotes (gs_flatpak_get_installation (self, interactive), cancellable, NULL);

		if (xremotes) {
			guint ii;

			for (ii = 0; ii < xremotes->len; ii++) {
				FlatpakRemote *yremote = g_ptr_array_index (xremotes, ii);

				if (flatpak_remote_get_disabled (yremote))
					continue;

				if (g_strcmp0 (flatpak_remote_get_name (yremote), origin) == 0) {
					title = flatpak_remote_get_title (yremote);

					if (!locker)
						locker = g_mutex_locker_new (&self->remote_title_mutex);

					/* Takes ownership of the 'title' */
					g_hash_table_insert (self->remote_title, g_strdup (origin), (gpointer) title);
					break;
				}
			}
		}
	}

	if (g_strcmp0 (origin, "flathub-beta") == 0 ||
	    g_strcmp0 (gs_app_get_branch (app), "devel") == 0 ||
	    g_strcmp0 (gs_app_get_branch (app), "master") == 0 ||
	    (gs_app_get_branch (app) && g_str_has_suffix (gs_app_get_branch (app), "beta")))
		gs_app_add_quirk (app, GS_APP_QUIRK_DEVELOPMENT_SOURCE);

	gs_app_set_origin (app, origin);
	gs_app_set_origin_ui (app, title);
}

static void
gs_flatpak_claim_app_list (GsFlatpak *self,
			   GsAppList *list,
			   gboolean   interactive)
{
	for (guint i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app = gs_app_list_index (list, i);

		/* Do not claim ownership of a wildcard app */
		if (gs_app_has_quirk (app, GS_APP_QUIRK_IS_WILDCARD))
			continue;

		if (gs_app_get_origin (app))
			gs_flatpak_set_app_origin (self, app, gs_app_get_origin (app), NULL, interactive, NULL);

		gs_flatpak_claim_app (self, app);
	}
}

static void
gs_flatpak_set_runtime_kind_from_id (GsApp *app)
{
	const gchar *id = gs_app_get_id (app);
	/* this is anything that's not an app, including locales
	 * sources and debuginfo */
	if (g_str_has_suffix (id, ".Locale")) {
		gs_app_set_kind (app, AS_COMPONENT_KIND_LOCALIZATION);
	} else if (g_str_has_suffix (id, ".Debug") ||
		   g_str_has_suffix (id, ".Sources") ||
		   g_str_has_prefix (id, "org.freedesktop.Platform.Icontheme.") ||
		   g_str_has_prefix (id, "org.gtk.Gtk3theme.")) {
		gs_app_set_kind (app, AS_COMPONENT_KIND_GENERIC);
	} else if (gs_app_get_kind (app) == AS_COMPONENT_KIND_UNKNOWN) {
		gs_app_set_kind (app, AS_COMPONENT_KIND_RUNTIME);
	}
}

static void
gs_flatpak_set_kind_from_flatpak (GsApp *app, FlatpakRef *xref)
{
	if (flatpak_ref_get_kind (xref) == FLATPAK_REF_KIND_APP) {
		/* the appstream plugin can set proper kind, like console-application,
		   from the appstream data */
		if (gs_app_get_kind (app) == AS_COMPONENT_KIND_UNKNOWN)
			gs_app_set_kind (app, AS_COMPONENT_KIND_DESKTOP_APP);
	} else if (flatpak_ref_get_kind (xref) == FLATPAK_REF_KIND_RUNTIME) {
		gs_flatpak_set_runtime_kind_from_id (app);
	}
}

static guint
gs_get_strv_index (const gchar * const *strv,
		   const gchar *value)
{
	guint ii;

	for (ii = 0; strv[ii]; ii++) {
		if (g_str_equal (strv[ii], value))
			break;
	}

	return ii;
}

static GsAppPermissions *
perms_from_metadata (GKeyFile *keyfile)
{
	char **strv;
	char *str;
	GsAppPermissions *permissions = gs_app_permissions_new ();
	GsAppPermissionsFlags flags = GS_APP_PERMISSIONS_FLAGS_NONE;

	strv = g_key_file_get_string_list (keyfile, "Context", "sockets", NULL, NULL);
	if (strv != NULL && g_strv_contains ((const gchar * const*)strv, "system-bus"))
		flags |= GS_APP_PERMISSIONS_FLAGS_SYSTEM_BUS;
	if (strv != NULL && g_strv_contains ((const gchar * const*)strv, "session-bus"))
		flags |= GS_APP_PERMISSIONS_FLAGS_SESSION_BUS;
	if (strv != NULL &&
	    !g_strv_contains ((const gchar * const*)strv, "fallback-x11") &&
	    g_strv_contains ((const gchar * const*)strv, "x11"))
		flags |= GS_APP_PERMISSIONS_FLAGS_X11;
	/* "fallback-x11" without "wayland" means X11 */
	if (strv != NULL && g_strv_contains ((const gchar * const*)strv, "fallback-x11") &&
	    !g_strv_contains ((const gchar * const*)strv, "wayland"))
		flags |= GS_APP_PERMISSIONS_FLAGS_X11;
	if (strv != NULL && g_strv_contains ((const gchar * const*)strv, "pulseaudio"))
		flags |= GS_APP_PERMISSIONS_FLAGS_PULSEAUDIO_DEVICES;
	g_strfreev (strv);

	strv = g_key_file_get_string_list (keyfile, "Context", "devices", NULL, NULL);
	if (strv != NULL && g_strv_contains ((const gchar * const*)strv, "all"))
		flags |= GS_APP_PERMISSIONS_FLAGS_DEVICES;
	if (strv != NULL && g_strv_contains ((const gchar * const*)strv, "input"))
		flags |= GS_APP_PERMISSIONS_FLAGS_INPUT_DEVICES;
	if (strv != NULL && (g_strv_contains ((const gchar * const*)strv, "shm") ||
			     g_strv_contains ((const gchar * const*)strv, "kvm")))
		flags |= GS_APP_PERMISSIONS_FLAGS_SYSTEM_DEVICES;
	g_strfreev (strv);

	strv = g_key_file_get_string_list (keyfile, "Context", "shared", NULL, NULL);
	if (strv != NULL && g_strv_contains ((const gchar * const*)strv, "network"))
		flags |= GS_APP_PERMISSIONS_FLAGS_NETWORK;
	g_strfreev (strv);

	strv = g_key_file_get_string_list (keyfile, "Context", "filesystems", NULL, NULL);
	if (strv != NULL) {
		const struct {
			const gchar *key;
			GsAppPermissionsFlags perm;
		} filesystems_access[] = {
			/* Reference: https://docs.flatpak.org/en/latest/flatpak-command-reference.html#idm45858571325264 */
			{ "home", GS_APP_PERMISSIONS_FLAGS_HOME_FULL },
			{ "home:rw", GS_APP_PERMISSIONS_FLAGS_HOME_FULL },
			{ "home:ro", GS_APP_PERMISSIONS_FLAGS_HOME_READ },
			{ "~", GS_APP_PERMISSIONS_FLAGS_HOME_FULL },
			{ "~:rw", GS_APP_PERMISSIONS_FLAGS_HOME_FULL },
			{ "~:ro", GS_APP_PERMISSIONS_FLAGS_HOME_READ },
			{ "host", GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_FULL },
			{ "host:rw", GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_FULL },
			{ "host:ro", GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_READ },
			{ "xdg-config/kdeglobals:ro", GS_APP_PERMISSIONS_FLAGS_NONE },  /* ignore this; all KDE apps need it */
			{ "xdg-download", GS_APP_PERMISSIONS_FLAGS_DOWNLOADS_FULL },
			{ "xdg-download:rw", GS_APP_PERMISSIONS_FLAGS_DOWNLOADS_FULL },
			{ "xdg-download:ro", GS_APP_PERMISSIONS_FLAGS_DOWNLOADS_READ },
			{ "xdg-data/flatpak/overrides:create", GS_APP_PERMISSIONS_FLAGS_ESCAPE_SANDBOX },
			{ "xdg-run/pipewire-0", GS_APP_PERMISSIONS_FLAGS_SCREEN | GS_APP_PERMISSIONS_FLAGS_DEVICES },  /* see https://github.com/LingmoOS/lingmo-appstore/-/issues/2329 */
		};
		guint filesystems_hits = 0;
		guint strv_len = g_strv_length (strv);

		for (guint i = 0; i < G_N_ELEMENTS (filesystems_access); i++) {
			guint index = gs_get_strv_index ((const gchar * const *) strv, filesystems_access[i].key);
			if (index < strv_len) {
				flags |= filesystems_access[i].perm;
				filesystems_hits++;
				/* Mark it as used */
				strv[index][0] = '\0';
			}
		}

		if ((flags & GS_APP_PERMISSIONS_FLAGS_HOME_FULL) != 0)
			flags = flags & ~GS_APP_PERMISSIONS_FLAGS_HOME_READ;
		if ((flags & GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_FULL) != 0)
			flags = flags & ~GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_READ;
		if ((flags & GS_APP_PERMISSIONS_FLAGS_DOWNLOADS_FULL) != 0)
			flags = flags & ~GS_APP_PERMISSIONS_FLAGS_DOWNLOADS_READ;

		if (strv_len > filesystems_hits) {
			/* Cover those not being part of the above filesystem_access array */
			const struct {
				const gchar *prefix;
				const gchar *title;
				const gchar *title_subdir;
			} filesystems_other[] = {
				/* Reference: https://docs.flatpak.org/en/latest/flatpak-command-reference.html#idm45858571325264 */
				{ "/",			NULL,					   N_("System folder %s") },
				{ "home/",		NULL,					   N_("Home subfolder %s") },
				{ "~/",			NULL,					   N_("Home subfolder %s") },
				{ "host-os",		N_("Host system folders"),		   NULL },
				{ "host-etc",		N_("Host system configuration from /etc"), NULL },
				{ "xdg-desktop",	N_("Desktop folder"),			   N_("Desktop subfolder %s") },
				{ "xdg-documents",	N_("Documents folder"),			   N_("Documents subfolder %s") },
				{ "xdg-music",		N_("Music folder"),			   N_("Music subfolder %s") },
				{ "xdg-pictures",	N_("Pictures folder"),			   N_("Pictures subfolder %s") },
				{ "xdg-public-share",	N_("Public Share folder"),		   N_("Public Share subfolder %s") },
				{ "xdg-videos",		N_("Videos folder"),			   N_("Videos subfolder %s") },
				{ "xdg-templates",	N_("Templates folder"),			   N_("Templates subfolder %s") },
				{ "xdg-cache",		N_("User cache folder"),		   N_("User cache subfolder %s") },
				{ "xdg-config",		N_("User configuration folder"),	   N_("User configuration subfolder %s") },
				{ "xdg-data",		N_("User data folder"),			   N_("User data subfolder %s") },
				{ "xdg-run",		N_("User runtime folder"),		   N_("User runtime subfolder %s") }
			};

			flags |= GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_OTHER;

			for (guint j = 0; strv[j]; j++) {
				gchar *perm = strv[j];
				gboolean is_readonly;
				gchar *colon;
				guint i;

				/* Already handled by the flags */
				if (!perm[0])
					continue;

				is_readonly = g_str_has_suffix (perm, ":ro");
				colon = strrchr (perm, ':');
				/* modifiers are ":ro", ":rw", ":create", where ":create" is ":rw" + create
				   and ":rw" is default; treat ":create" as ":rw" */
				if (colon) {
					/* Completeness check */
					if (!g_str_equal (colon, ":ro") &&
					    !g_str_equal (colon, ":rw") &&
					    !g_str_equal (colon, ":create"))
						g_debug ("Unknown filesystem permission modifier '%s' from '%s'", colon, perm);
					/* cut it off */
					*colon = '\0';
				}

				for (i = 0; i < G_N_ELEMENTS (filesystems_other); i++) {
					if (g_str_has_prefix (perm, filesystems_other[i].prefix)) {
						g_autofree gchar *title_tmp = NULL;
						const gchar *slash, *title = NULL;
						slash = strchr (perm, '/');
						/* Catch and ignore invalid permission definitions */
						if (slash && filesystems_other[i].title_subdir != NULL) {
							#pragma GCC diagnostic push
							#pragma GCC diagnostic ignored "-Wformat-nonliteral"
							title_tmp = g_strdup_printf (
								_(filesystems_other[i].title_subdir),
								slash + (slash == perm ? 0 : 1));
							#pragma GCC diagnostic pop
							title = title_tmp;
						} else if (!slash && filesystems_other[i].title != NULL) {
							title = _(filesystems_other[i].title);
						}
						if (title != NULL) {
							if (is_readonly)
								gs_app_permissions_add_filesystem_read (permissions, title);
							else
								gs_app_permissions_add_filesystem_full (permissions, title);
						}
						break;
					}
				}

				/* Nothing matched, use a generic entry */
				if (i == G_N_ELEMENTS (filesystems_other)) {
					g_autofree gchar *title = g_strdup_printf (_("Filesystem access to %s"), perm);
					if (is_readonly)
						gs_app_permissions_add_filesystem_read (permissions, title);
					else
						gs_app_permissions_add_filesystem_full (permissions, title);
				}
			}
		}
	}
	g_strfreev (strv);

	str = g_key_file_get_string (keyfile, "Session Bus Policy", "ca.desrt.dconf", NULL);
	if (str != NULL && g_str_equal (str, "talk"))
		flags |= GS_APP_PERMISSIONS_FLAGS_SETTINGS;
	g_free (str);

	if (!(flags & GS_APP_PERMISSIONS_FLAGS_ESCAPE_SANDBOX)) {
		str = g_key_file_get_string (keyfile, "Session Bus Policy", "org.freedesktop.Flatpak", NULL);
		if (str != NULL && g_str_equal (str, "talk"))
			flags |= GS_APP_PERMISSIONS_FLAGS_ESCAPE_SANDBOX;
		g_free (str);
	}

	if (!(flags & GS_APP_PERMISSIONS_FLAGS_ESCAPE_SANDBOX)) {
		str = g_key_file_get_string (keyfile, "Session Bus Policy", "org.freedesktop.impl.portal.PermissionStore", NULL);
		if (str != NULL && g_str_equal (str, "talk"))
			flags |= GS_APP_PERMISSIONS_FLAGS_ESCAPE_SANDBOX;
		g_free (str);
	}

	gs_app_permissions_set_flags (permissions, flags);
	gs_app_permissions_seal (permissions);

	return permissions;
}

static void
gs_flatpak_set_update_permissions (GsFlatpak           *self,
                                   GsApp               *app,
                                   FlatpakInstalledRef *xref,
                                   gboolean             interactive,
                                   GCancellable        *cancellable)
{
	g_autoptr(GBytes) old_bytes = NULL;
	g_autoptr(GKeyFile) old_keyfile = NULL;
	g_autoptr(GBytes) bytes = NULL;
	g_autoptr(GKeyFile) keyfile = NULL;
	g_autoptr(GsAppPermissions) additional_permissions = gs_app_permissions_new ();
	g_autoptr(GError) error_local = NULL;

	old_bytes = flatpak_installed_ref_load_metadata (FLATPAK_INSTALLED_REF (xref), NULL, &error_local);
	if (old_bytes == NULL) {
		g_debug ("Failed to get metadata for app ‘%s’: %s",
			 gs_app_get_id (app), error_local->message);
		g_clear_error (&error_local);

		/* Permissions are unknown */
		g_clear_object (&additional_permissions);

		goto finish;
	}

	old_keyfile = g_key_file_new ();
	g_key_file_load_from_data (old_keyfile,
	                           g_bytes_get_data (old_bytes, NULL),
	                           g_bytes_get_size (old_bytes),
	                           0, NULL);

	bytes = flatpak_installation_fetch_remote_metadata_sync (gs_flatpak_get_installation (self, interactive),
	                                                         gs_app_get_origin (app),
	                                                         FLATPAK_REF (xref),
	                                                         cancellable,
	                                                         &error_local);
	if (bytes == NULL) {
		g_debug ("Failed to get metadata for remote ‘%s’: %s",
			 gs_app_get_origin (app), error_local->message);
		g_clear_error (&error_local);

		/* Permissions are unknown */
		g_clear_object (&additional_permissions);
	} else {
		g_autoptr(GsAppPermissions) old_permissions = NULL;
		g_autoptr(GsAppPermissions) new_permissions = NULL;
		const GPtrArray *new_paths;

		keyfile = g_key_file_new ();
		g_key_file_load_from_data (keyfile,
			                   g_bytes_get_data (bytes, NULL),
			                   g_bytes_get_size (bytes),
			                   0, NULL);

		old_permissions = perms_from_metadata (old_keyfile);
		new_permissions = perms_from_metadata (keyfile);

		gs_app_permissions_set_flags (additional_permissions,
					      gs_app_permissions_get_flags (new_permissions) &
					     ~gs_app_permissions_get_flags (old_permissions));

		new_paths = gs_app_permissions_get_filesystem_read (new_permissions);
		for (guint i = 0; new_paths && i < new_paths->len; i++) {
			const gchar *new_path = g_ptr_array_index (new_paths, i);
			if (!gs_app_permissions_contains_filesystem_read (old_permissions, new_path))
				gs_app_permissions_add_filesystem_read (additional_permissions, new_path);
		}

		new_paths = gs_app_permissions_get_filesystem_full (new_permissions);
		for (guint i = 0; new_paths && i < new_paths->len; i++) {
			const gchar *new_path = g_ptr_array_index (new_paths, i);
			if (!gs_app_permissions_contains_filesystem_full (old_permissions, new_path))
				gs_app_permissions_add_filesystem_full (additional_permissions, new_path);
		}
	}

finish:
	/* no new permissions set */
	if (additional_permissions != NULL)
		gs_app_permissions_seal (additional_permissions);

	gs_app_set_update_permissions (app, additional_permissions);

	if (additional_permissions != NULL &&
	    gs_app_permissions_get_flags (additional_permissions) != GS_APP_PERMISSIONS_FLAGS_NONE)
		gs_app_add_quirk (app, GS_APP_QUIRK_NEW_PERMISSIONS);
	else
		gs_app_remove_quirk (app, GS_APP_QUIRK_NEW_PERMISSIONS);
}

static void
gs_flatpak_set_metadata (GsFlatpak *self, GsApp *app, FlatpakRef *xref)
{
	g_autofree gchar *ref_tmp = flatpak_ref_format_ref (FLATPAK_REF (xref));
	guint64 installed_size = 0, download_size = 0;

	/* core */
	gs_flatpak_claim_app (self, app);
	gs_app_set_branch (app, flatpak_ref_get_branch (xref));
	gs_app_add_source (app, ref_tmp);
	gs_app_set_metadata (app, "LingmoAppstore::packagename-value",  ref_tmp);
	gs_plugin_refine_item_scope (self, app);

	/* flatpak specific */
	gs_flatpak_app_set_ref_kind (app, flatpak_ref_get_kind (xref));
	gs_flatpak_app_set_ref_name (app, flatpak_ref_get_name (xref));
	gs_flatpak_app_set_ref_arch (app, flatpak_ref_get_arch (xref));
	gs_flatpak_app_set_commit (app, flatpak_ref_get_commit (xref));

	/* map the flatpak kind to the lingmo-appstore kind */
	if (gs_app_get_kind (app) == AS_COMPONENT_KIND_UNKNOWN ||
	    gs_app_get_kind (app) == AS_COMPONENT_KIND_GENERIC) {
		gs_flatpak_set_kind_from_flatpak (app, xref);
	}

	if (FLATPAK_IS_REMOTE_REF (xref) && flatpak_remote_ref_get_eol (FLATPAK_REMOTE_REF (xref)) != NULL)
		gs_app_set_metadata (app, "LingmoAppstore::EolReason", flatpak_remote_ref_get_eol (FLATPAK_REMOTE_REF (xref)));
	else if (FLATPAK_IS_INSTALLED_REF (xref) && flatpak_installed_ref_get_eol (FLATPAK_INSTALLED_REF (xref)) != NULL)
		gs_app_set_metadata (app, "LingmoAppstore::EolReason", flatpak_installed_ref_get_eol (FLATPAK_INSTALLED_REF (xref)));

	if (FLATPAK_IS_REMOTE_REF (xref)) {
		installed_size = flatpak_remote_ref_get_installed_size (FLATPAK_REMOTE_REF (xref));
		download_size = flatpak_remote_ref_get_download_size (FLATPAK_REMOTE_REF (xref));
	} else if (FLATPAK_IS_INSTALLED_REF (xref)) {
		installed_size = flatpak_installed_ref_get_installed_size (FLATPAK_INSTALLED_REF (xref));
	}

	gs_app_set_size_installed (app, (installed_size != 0) ? GS_SIZE_TYPE_VALID : GS_SIZE_TYPE_UNKNOWN, installed_size);
	gs_app_set_size_download (app, (download_size != 0) ? GS_SIZE_TYPE_VALID : GS_SIZE_TYPE_UNKNOWN, download_size);
}

static GsApp *
gs_flatpak_create_app (GsFlatpak *self,
		       const gchar *origin,
		       FlatpakRef *xref,
		       FlatpakRemote *xremote,
		       gboolean interactive,
		       GCancellable *cancellable)
{
	GsApp *app_cached;
	g_autoptr(GsApp) app = NULL;

	/* create a temp GsApp */
	app = gs_app_new (flatpak_ref_get_name (xref));
	gs_flatpak_set_metadata (self, app, xref);
	if (origin != NULL) {
		gs_flatpak_set_app_origin (self, app, origin, xremote, interactive, cancellable);

		if (!(self->flags & GS_FLATPAK_FLAG_IS_TEMPORARY)) {
			/* return the ref'd cached copy, only if the origin is known */
			app_cached = gs_plugin_cache_lookup (self->plugin, gs_app_get_unique_id (app));
			if (app_cached != NULL)
				return app_cached;
		}
	}

	/* fallback values */
	if (gs_flatpak_app_get_ref_kind (app) == FLATPAK_REF_KIND_RUNTIME) {
		g_autoptr(GIcon) icon = NULL;
		gs_app_set_name (app, GS_APP_QUALITY_NORMAL,
				 flatpak_ref_get_name (FLATPAK_REF (xref)));
		gs_app_set_summary (app, GS_APP_QUALITY_NORMAL,
				    "Framework for applications");
		gs_app_set_version (app, flatpak_ref_get_branch (FLATPAK_REF (xref)));
		icon = g_themed_icon_new ("system-component-runtime");
		gs_app_add_icon (app, icon);
	}

	/* Don't add NULL origin apps to the cache. If the app is later set to
	 * origin x the cache may return it as a match for origin y since the cache
	 * hash table uses as_utils_data_id_equal() as the equal func and a NULL
	 * origin becomes a "*" in gs_utils_build_unique_id().
	 */
	if (origin != NULL && !(self->flags & GS_FLATPAK_FLAG_IS_TEMPORARY))
		gs_plugin_cache_add (self->plugin, NULL, app);

	/* no existing match, just steal the temp object */
	return g_steal_pointer (&app);
}

static GsApp *
gs_flatpak_create_source (GsFlatpak *self, FlatpakRemote *xremote)
{
	GsApp *app_cached;
	g_autoptr(GsApp) app = NULL;

	/* create a temp GsApp */
	app = gs_flatpak_app_new_from_remote (self->plugin, xremote,
					      flatpak_installation_get_is_user (self->installation_noninteractive));
	gs_flatpak_claim_app (self, app);

	/* we already have one, returned the ref'd cached copy */
	app_cached = gs_plugin_cache_lookup (self->plugin, gs_app_get_unique_id (app));
	if (app_cached != NULL)
		return app_cached;

	/* no existing match, just steal the temp object */
	gs_plugin_cache_add (self->plugin, NULL, app);
	return g_steal_pointer (&app);
}

static void
gs_flatpak_invalidate_silo (GsFlatpak *self)
{
	g_rw_lock_writer_lock (&self->silo_lock);
	if (self->silo != NULL)
		xb_silo_invalidate (self->silo);
	g_rw_lock_writer_unlock (&self->silo_lock);
}

static void
gs_flatpak_internal_data_changed (GsFlatpak *self)
{
	g_autoptr(GMutexLocker) locker = NULL;

	/* drop the installed refs cache */
	locker = g_mutex_locker_new (&self->installed_refs_mutex);
	g_clear_pointer (&self->installed_refs, g_ptr_array_unref);
	g_clear_pointer (&self->remotes_by_name, g_hash_table_unref);
	g_clear_pointer (&locker, g_mutex_locker_free);

	/* drop the remote title cache */
	locker = g_mutex_locker_new (&self->remote_title_mutex);
	g_hash_table_remove_all (self->remote_title);
	g_clear_pointer (&locker, g_mutex_locker_free);

	/* give all the repos a second chance */
	locker = g_mutex_locker_new (&self->broken_remotes_mutex);
	g_hash_table_remove_all (self->broken_remotes);
	g_clear_pointer (&locker, g_mutex_locker_free);

	gs_flatpak_invalidate_silo (self);

	self->requires_full_rescan = TRUE;
}

static gboolean
gs_flatpak_claim_changed_idle_cb (gpointer user_data)
{
	GsFlatpak *self = user_data;

	gs_flatpak_internal_data_changed (self);
	gs_plugin_cache_invalidate (self->plugin);
	gs_plugin_reload (self->plugin);

	return G_SOURCE_REMOVE;
}

static void
gs_plugin_flatpak_changed_cb (GFileMonitor *monitor,
			      GFile *child,
			      GFile *other_file,
			      GFileMonitorEvent event_type,
			      GsFlatpak *self)
{
	if (gs_flatpak_get_busy (self)) {
		self->changed_while_busy = TRUE;
	} else {
		gs_flatpak_claim_changed_idle_cb (self);
	}
}

static gboolean
gs_flatpak_add_flatpak_keyword_cb (XbBuilderFixup *self,
				   XbBuilderNode *bn,
				   gpointer user_data,
				   GError **error)
{
	if (g_strcmp0 (xb_builder_node_get_element (bn), "component") == 0)
		gs_appstream_component_add_keyword (bn, "flatpak");
	return TRUE;
}

static gboolean
gs_flatpak_fix_id_desktop_suffix_cb (XbBuilderFixup *self,
				     XbBuilderNode *bn,
				     gpointer user_data,
				     GError **error)
{
	if (g_strcmp0 (xb_builder_node_get_element (bn), "component") == 0) {
		g_auto(GStrv) split = NULL;
		g_autoptr(XbBuilderNode) id = xb_builder_node_get_child (bn, "id", NULL);
		g_autoptr(XbBuilderNode) bundle = xb_builder_node_get_child (bn, "bundle", NULL);
		if (id == NULL || bundle == NULL)
			return TRUE;
		split = g_strsplit (xb_builder_node_get_text (bundle), "/", -1);
		if (g_strv_length (split) != 4)
			return TRUE;
		if (g_strcmp0 (xb_builder_node_get_text (id), split[1]) != 0) {
			g_debug ("fixing up <id>%s</id> to %s",
				 xb_builder_node_get_text (id), split[1]);
			gs_appstream_component_add_provide (bn, xb_builder_node_get_text (id));
			xb_builder_node_set_text (id, split[1], -1);
		}
	}
	return TRUE;
}

static gboolean
gs_flatpak_add_bundle_tag_cb (XbBuilderFixup *self,
			      XbBuilderNode *bn,
			      gpointer user_data,
			      GError **error)
{
	const char *app_ref = (char *)user_data;
	if (g_strcmp0 (xb_builder_node_get_element (bn), "component") == 0) {
		g_autoptr(XbBuilderNode) id = xb_builder_node_get_child (bn, "id", NULL);
		g_autoptr(XbBuilderNode) bundle = xb_builder_node_get_child (bn, "bundle", NULL);
		if (id == NULL || bundle != NULL)
			return TRUE;
		g_debug ("adding <bundle> tag for %s", app_ref);
		xb_builder_node_insert_text (bn, "bundle", app_ref, "type", "flatpak", NULL);
	}
	return TRUE;
}

static gboolean
gs_flatpak_fix_metadata_tag_cb (XbBuilderFixup *self,
				XbBuilderNode *bn,
				gpointer user_data,
				GError **error)
{
	if (g_strcmp0 (xb_builder_node_get_element (bn), "component") == 0) {
		g_autoptr(XbBuilderNode) metadata = xb_builder_node_get_child (bn, "metadata", NULL);
		if (metadata != NULL)
			xb_builder_node_set_element (metadata, "custom");
	}
	return TRUE;
}

static gboolean
gs_flatpak_set_origin_cb (XbBuilderFixup *self,
			  XbBuilderNode *bn,
			  gpointer user_data,
			  GError **error)
{
	const char *remote_name = (char *)user_data;
	if (g_strcmp0 (xb_builder_node_get_element (bn), "components") == 0) {
		xb_builder_node_set_attr (bn, "origin",
					  remote_name);
	}
	return TRUE;
}

static gboolean
gs_flatpak_filter_default_branch_cb (XbBuilderFixup *self,
				     XbBuilderNode *bn,
				     gpointer user_data,
				     GError **error)
{
	const gchar *default_branch = (const gchar *) user_data;
	if (g_strcmp0 (xb_builder_node_get_element (bn), "component") == 0) {
		g_autoptr(XbBuilderNode) bc = xb_builder_node_get_child (bn, "bundle", NULL);
		g_auto(GStrv) split = NULL;
		if (bc == NULL) {
			g_debug ("no bundle for component");
			return TRUE;
		}
		split = g_strsplit (xb_builder_node_get_text (bc), "/", -1);
		if (split == NULL || g_strv_length (split) != 4)
			return TRUE;
		if (g_strcmp0 (split[3], default_branch) != 0) {
			g_debug ("not adding app with branch %s as filtering to %s",
				 split[3], default_branch);
			xb_builder_node_add_flag (bn, XB_BUILDER_NODE_FLAG_IGNORE);
		}
	}
	return TRUE;
}

static gboolean
gs_flatpak_filter_noenumerate_cb (XbBuilderFixup *self,
				  XbBuilderNode *bn,
				  gpointer user_data,
				  GError **error)
{
	const gchar *main_ref = (const gchar *) user_data;

	if (g_strcmp0 (xb_builder_node_get_element (bn), "component") == 0) {
		g_autoptr(XbBuilderNode) bc = xb_builder_node_get_child (bn, "bundle", NULL);
		if (bc == NULL) {
			g_debug ("no bundle for component");
			return TRUE;
		}
		if (g_strcmp0 (xb_builder_node_get_text (bc), main_ref) != 0) {
			g_debug ("not adding app %s as filtering to %s",
				 xb_builder_node_get_text (bc), main_ref);
			xb_builder_node_add_flag (bn, XB_BUILDER_NODE_FLAG_IGNORE);
		}
	}
	return TRUE;
}

#if LIBXMLB_CHECK_VERSION(0,3,0)
static gboolean
gs_flatpak_tokenize_cb (XbBuilderFixup *self,
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
		"summary",
		NULL };
	if (xb_builder_node_get_element (bn) != NULL &&
	    g_strv_contains (elements_to_tokenize, xb_builder_node_get_element (bn)))
		xb_builder_node_tokenize_text (bn);
	return TRUE;
}
#endif

static void
fixup_flatpak_appstream_xml (XbBuilderSource *source,
		             const char *origin)
{
	g_autoptr(XbBuilderFixup) fixup1 = NULL;
	g_autoptr(XbBuilderFixup) fixup2 = NULL;
	g_autoptr(XbBuilderFixup) fixup3 = NULL;
#if LIBXMLB_CHECK_VERSION(0,3,0)
	g_autoptr(XbBuilderFixup) fixup5 = NULL;
#endif

	/* add the flatpak search keyword */
	fixup1 = xb_builder_fixup_new ("AddKeywordFlatpak",
				       gs_flatpak_add_flatpak_keyword_cb,
				       NULL, NULL);
	xb_builder_fixup_set_max_depth (fixup1, 2);
	xb_builder_source_add_fixup (source, fixup1);

	/* ensure the <id> matches the flatpak ref ID  */
	fixup2 = xb_builder_fixup_new ("FixIdDesktopSuffix",
				       gs_flatpak_fix_id_desktop_suffix_cb,
				       NULL, NULL);
	xb_builder_fixup_set_max_depth (fixup2, 2);
	xb_builder_source_add_fixup (source, fixup2);

	/* Fixup <metadata> to <custom> for appstream versions >= 0.9 */
	fixup3 = xb_builder_fixup_new ("FixMetadataTag",
				       gs_flatpak_fix_metadata_tag_cb,
				       NULL, NULL);
	xb_builder_fixup_set_max_depth (fixup3, 2);
	xb_builder_source_add_fixup (source, fixup3);

#if LIBXMLB_CHECK_VERSION(0,3,0)
	fixup5 = xb_builder_fixup_new ("TextTokenize",
				       gs_flatpak_tokenize_cb,
				       NULL, NULL);
	xb_builder_fixup_set_max_depth (fixup5, 2);
	xb_builder_source_add_fixup (source, fixup5);
#endif

	if (origin != NULL) {
		g_autoptr(XbBuilderFixup) fixup4 = NULL;

		/* override the *AppStream* origin */
		fixup4 = xb_builder_fixup_new ("SetOrigin",
					       gs_flatpak_set_origin_cb,
					       g_strdup (origin), g_free);
		xb_builder_fixup_set_max_depth (fixup4, 1);
		xb_builder_source_add_fixup (source, fixup4);
	}
}

static gboolean
gs_flatpak_refresh_appstream_remote (GsFlatpak *self,
				     const gchar *remote_name,
				     gboolean interactive,
				     GCancellable *cancellable,
				     GError **error);

static gboolean
gs_flatpak_add_apps_from_xremote (GsFlatpak *self,
				  XbBuilder *builder,
				  FlatpakRemote *xremote,
				  gboolean interactive,
				  GCancellable *cancellable,
				  GError **error)
{
	g_autofree gchar *appstream_dir_fn = NULL;
	g_autofree gchar *appstream_fn = NULL;
	g_autofree gchar *icon_prefix = NULL;
	g_autofree gchar *default_branch = NULL;
	g_autoptr(GFile) appstream_dir = NULL;
	g_autoptr(GFile) file_xml = NULL;
	g_autoptr(GSettings) settings = NULL;
	g_autoptr(XbBuilderNode) info = NULL;
	g_autoptr(XbBuilderSource) source = xb_builder_source_new ();
	const gchar *remote_name = flatpak_remote_get_name (xremote);
	gboolean did_refresh = FALSE;

	/* get the AppStream data location */
	appstream_dir = flatpak_remote_get_appstream_dir (xremote, NULL);
	if (appstream_dir == NULL) {
		g_autoptr(GError) error_local = NULL;
		g_debug ("no appstream dir for %s, trying refresh...",
			 remote_name);

		if (!gs_flatpak_refresh_appstream_remote (self, remote_name, interactive, cancellable, &error_local)) {
			g_debug ("Failed to refresh appstream data for '%s': %s", remote_name, error_local->message);
			if (g_error_matches (error_local, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_FAILED)) {
				g_autoptr(GMutexLocker) locker = NULL;

				locker = g_mutex_locker_new (&self->broken_remotes_mutex);

				/* don't try to fetch this again until refresh() */
				g_hash_table_insert (self->broken_remotes,
						     g_strdup (remote_name),
						     GUINT_TO_POINTER (1));
			}
			return TRUE;
		}

		appstream_dir = flatpak_remote_get_appstream_dir (xremote, NULL);
		if (appstream_dir == NULL) {
			g_debug ("no appstream dir for %s even after refresh, skipping",
				 remote_name);
			return TRUE;
		}

		did_refresh = TRUE;
	}

	/* load the file into a temp silo */
	appstream_dir_fn = g_file_get_path (appstream_dir);
	appstream_fn = g_build_filename (appstream_dir_fn, "appstream.xml.gz", NULL);
	if (!g_file_test (appstream_fn, G_FILE_TEST_EXISTS)) {
		g_autoptr(GError) error_local = NULL;
		g_debug ("no appstream metadata found for '%s' (file: %s), %s",
			 remote_name,
			 appstream_fn,
			 did_refresh ? "skipping" : "trying refresh...");
		if (did_refresh)
			return TRUE;

		if (!gs_flatpak_refresh_appstream_remote (self, remote_name, interactive, cancellable, &error_local)) {
			g_debug ("Failed to refresh appstream data for '%s': %s", remote_name, error_local->message);
			if (g_error_matches (error_local, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_FAILED)) {
				g_autoptr(GMutexLocker) locker = NULL;

				locker = g_mutex_locker_new (&self->broken_remotes_mutex);

				/* don't try to fetch this again until refresh() */
				g_hash_table_insert (self->broken_remotes,
						     g_strdup (remote_name),
						     GUINT_TO_POINTER (1));
			}
			return TRUE;
		}

		if (!g_file_test (appstream_fn, G_FILE_TEST_EXISTS)) {
			g_debug ("no appstream metadata found for '%s', even after refresh (file: %s), skipping",
				 remote_name,
				 appstream_fn);
			return TRUE;
		}
	}

	/* add source */
	file_xml = g_file_new_for_path (appstream_fn);
	if (!xb_builder_source_load_file (source, file_xml,
					  XB_BUILDER_SOURCE_FLAG_WATCH_FILE |
					  XB_BUILDER_SOURCE_FLAG_LITERAL_TEXT,
					  cancellable,
					  error))
		return FALSE;

	fixup_flatpak_appstream_xml (source, remote_name);

	/* add metadata */
	icon_prefix = g_build_filename (appstream_dir_fn, "icons", NULL);
	info = xb_builder_node_insert (NULL, "info", NULL);
	xb_builder_node_insert_text (info, "scope", as_component_scope_to_string (self->scope), NULL);
	xb_builder_node_insert_text (info, "icon-prefix", icon_prefix, NULL);
	xb_builder_source_set_info (source, info);

	/* only add the specific app for noenumerate=true */
	if (flatpak_remote_get_noenumerate (xremote)) {
		g_autofree gchar *main_ref = NULL;

		main_ref = flatpak_remote_get_main_ref (xremote);

		if (main_ref != NULL) {
			g_autoptr(XbBuilderFixup) fixup = NULL;
			fixup = xb_builder_fixup_new ("FilterNoEnumerate",
						      gs_flatpak_filter_noenumerate_cb,
						      g_strdup (main_ref),
						      g_free);
			xb_builder_fixup_set_max_depth (fixup, 2);
			xb_builder_source_add_fixup (source, fixup);
		}
	}

	/* do we want to filter to the default branch */
	settings = g_settings_new ("org.lingmo.appstore");
	default_branch = flatpak_remote_get_default_branch (xremote);
	if (g_settings_get_boolean (settings, "filter-default-branch") &&
	    default_branch != NULL) {
		g_autoptr(XbBuilderFixup) fixup = NULL;
		fixup = xb_builder_fixup_new ("FilterDefaultbranch",
					      gs_flatpak_filter_default_branch_cb,
					      flatpak_remote_get_default_branch (xremote),
					      g_free);
		xb_builder_fixup_set_max_depth (fixup, 2);
		xb_builder_source_add_fixup (source, fixup);
	}

	/* success */
	xb_builder_import_source (builder, source);
	return TRUE;
}

static gchar *
gs_flatpak_get_desktop_files_dir (GsFlatpak *self)
{
	g_autoptr(GFile) path = NULL;
	g_autofree gchar *path_str = NULL;

	path = flatpak_installation_get_path (self->installation_noninteractive);
	path_str = g_file_get_path (path);
	return g_build_filename (path_str, "exports", "share", "applications", NULL);
}

static void
gs_flatpak_rescan_installed (GsFlatpak *self,
			     XbBuilder *builder,
			     GCancellable *cancellable,
			     GError **error)
{
	g_autofree gchar *path = NULL;
	g_autoptr(GError) error_local = NULL;

	/* add all installed desktop files */
	path = gs_flatpak_get_desktop_files_dir (self);
	if (!gs_appstream_load_desktop_files (builder, path, NULL, NULL, cancellable, &error_local))
		g_debug ("Failed to read flatpak .desktop files in %s: %s", path, error_local->message);
}

static gboolean
gs_flatpak_rescan_appstream_store (GsFlatpak *self,
				   gboolean interactive,
				   GCancellable *cancellable,
				   GError **error)
{
	g_autofree gchar *blobfn = NULL;
	g_autoptr(GFile) file = NULL;
	g_autoptr(GPtrArray) xremotes = NULL;
	g_autoptr(GPtrArray) desktop_paths = NULL;
	g_autoptr(GRWLockReaderLocker) reader_locker = NULL;
	g_autoptr(GRWLockWriterLocker) writer_locker = NULL;
	g_autoptr(XbBuilder) builder = NULL;
	g_autoptr(GMainContext) old_thread_default = NULL;

	reader_locker = g_rw_lock_reader_locker_new (&self->silo_lock);
	/* everything is okay */
	if (self->silo != NULL && xb_silo_is_valid (self->silo))
		return TRUE;
	g_clear_pointer (&reader_locker, g_rw_lock_reader_locker_free);

	/* drat! silo needs regenerating */
	writer_locker = g_rw_lock_writer_locker_new (&self->silo_lock);
	g_clear_object (&self->silo);
	g_clear_pointer (&self->silo_filename, g_free);
	g_clear_pointer (&self->silo_installed_by_desktopid, g_hash_table_unref);

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

	/* go through each remote adding metadata */
	xremotes = flatpak_installation_list_remotes (gs_flatpak_get_installation (self, interactive),
						      cancellable,
						      error);
	if (xremotes == NULL) {
		gs_flatpak_error_convert (error);
		return FALSE;
	}
	for (guint i = 0; i < xremotes->len; i++) {
		g_autoptr(GError) error_local = NULL;
		FlatpakRemote *xremote = g_ptr_array_index (xremotes, i);
		if (flatpak_remote_get_disabled (xremote))
			continue;
		g_debug ("found remote %s",
			 flatpak_remote_get_name (xremote));
		if (!gs_flatpak_add_apps_from_xremote (self, builder, xremote, interactive, cancellable, &error_local)) {
			g_debug ("Failed to add apps from remote ‘%s’; skipping: %s",
				 flatpak_remote_get_name (xremote), error_local->message);
			if (g_cancellable_set_error_if_cancelled (cancellable, error)) {
				gs_flatpak_error_convert (error);
				return FALSE;
			}
		}
	}

	/* add any installed files without AppStream info */
	gs_flatpak_rescan_installed (self, builder, cancellable, error);

	/* regenerate with each minor release */
	xb_builder_append_guid (builder, PACKAGE_VERSION);

	/* Merge data from the installed files and the system appstream data,
	   which is always checked, even when the 'appstream_paths' is NULL. */
	desktop_paths = g_ptr_array_new_with_free_func (g_free);
	g_ptr_array_add (desktop_paths, gs_flatpak_get_desktop_files_dir (self));
	gs_appstream_add_data_merge_fixup (builder, NULL, desktop_paths, cancellable);

	/* create per-user cache */
	blobfn = gs_utils_get_cache_filename (gs_flatpak_get_id (self),
					      "components.xmlb",
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
					cancellable, error);

	if (old_thread_default != NULL)
		g_main_context_push_thread_default (old_thread_default);

	if (self->silo != NULL) {
		g_autoptr(GPtrArray) installed = NULL;
		g_autoptr(XbNode) info_filename = NULL;

		self->silo_installed_by_desktopid = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) g_ptr_array_unref);

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

		info_filename = xb_silo_query_first (self->silo, "/info/filename", NULL);
		if (info_filename != NULL)
			self->silo_filename = g_strdup (xb_node_get_text (info_filename));
	}

	/* success */
	return self->silo != NULL;
}

static gboolean
gs_flatpak_rescan_app_data (GsFlatpak *self,
			    gboolean interactive,
			    GCancellable *cancellable,
			    GError **error)
{
	if (self->requires_full_rescan) {
		gboolean res = gs_flatpak_refresh (self, 60, interactive, cancellable, error);
		if (res)
			self->requires_full_rescan = FALSE;
		else
			gs_flatpak_internal_data_changed (self);
		return res;
	}

	if (!gs_flatpak_rescan_appstream_store (self, interactive, cancellable, error)) {
		gs_flatpak_internal_data_changed (self);
		return FALSE;
	}

	return TRUE;
}

/* Returns with a read lock held on @self->silo_lock on success.
   The *locker should be NULL when being called. */
static gboolean
ensure_flatpak_silo_with_locker (GsFlatpak            *self,
				 GRWLockReaderLocker **locker,
				 gboolean              interactive,
				 GCancellable         *cancellable,
				 GError              **error)
{
	/* should not hold the lock when called */
	g_return_val_if_fail (*locker == NULL, FALSE);

	/* ensure valid */
	if (!gs_flatpak_rescan_app_data (self, interactive, cancellable, error))
		return FALSE;

	*locker = g_rw_lock_reader_locker_new (&self->silo_lock);

	while (self->silo == NULL) {
		g_clear_pointer (locker, g_rw_lock_reader_locker_free);

		if (!gs_flatpak_rescan_appstream_store (self, interactive, cancellable, error)) {
			gs_flatpak_internal_data_changed (self);
			return FALSE;
		}

		/* At this point either rescan_appstream_store() returned an error or it successfully
		 * initialised self->silo. There is the possibility that another thread will invalidate
		 * the silo before we regain the lock. If so, we’ll have to rescan again. */
		*locker = g_rw_lock_reader_locker_new (&self->silo_lock);
	}

	return TRUE;
}

gboolean
gs_flatpak_setup (GsFlatpak *self, GCancellable *cancellable, GError **error)
{
	/* watch for changes */
	self->monitor = flatpak_installation_create_monitor (self->installation_noninteractive,
							     cancellable,
							     error);
	if (self->monitor == NULL) {
		gs_flatpak_error_convert (error);
		return FALSE;
	}
	self->changed_id =
		g_signal_connect (self->monitor, "changed",
				  G_CALLBACK (gs_plugin_flatpak_changed_cb), self);

	/* success */
	return TRUE;
}

typedef struct {
	GsPlugin	*plugin;
	GsApp		*app;
} GsFlatpakProgressHelper;

static void
gs_flatpak_progress_helper_free (GsFlatpakProgressHelper *phelper)
{
	g_object_unref (phelper->plugin);
	if (phelper->app != NULL)
		g_object_unref (phelper->app);
	g_slice_free (GsFlatpakProgressHelper, phelper);
}

static GsFlatpakProgressHelper *
gs_flatpak_progress_helper_new (GsPlugin *plugin, GsApp *app)
{
	GsFlatpakProgressHelper *phelper;
	phelper = g_slice_new0 (GsFlatpakProgressHelper);
	phelper->plugin = g_object_ref (plugin);
	if (app != NULL)
		phelper->app = g_object_ref (app);
	return phelper;
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC(GsFlatpakProgressHelper, gs_flatpak_progress_helper_free)

static void
gs_flatpak_progress_cb (const gchar *status,
			guint progress,
			gboolean estimating,
			gpointer user_data)
{
	GsFlatpakProgressHelper *phelper = (GsFlatpakProgressHelper *) user_data;
	GsPluginStatus plugin_status = GS_PLUGIN_STATUS_DOWNLOADING;

	if (phelper->app != NULL) {
		if (estimating)
			gs_app_set_progress (phelper->app, GS_APP_PROGRESS_UNKNOWN);
		else
			gs_app_set_progress (phelper->app, progress);

		switch (gs_app_get_state (phelper->app)) {
		case GS_APP_STATE_INSTALLING:
			plugin_status = GS_PLUGIN_STATUS_INSTALLING;
			break;
		case GS_APP_STATE_REMOVING:
			plugin_status = GS_PLUGIN_STATUS_REMOVING;
			break;
		default:
			break;
		}
	}
	gs_plugin_status_update (phelper->plugin, phelper->app, plugin_status);
}

static gboolean
gs_flatpak_refresh_appstream_remote (GsFlatpak *self,
				     const gchar *remote_name,
				     gboolean interactive,
				     GCancellable *cancellable,
				     GError **error)
{
	g_autofree gchar *str = NULL;
	g_autoptr(GsApp) app_dl = gs_app_new (gs_plugin_get_name (self->plugin));
	g_autoptr(GsFlatpakProgressHelper) phelper = NULL;
	FlatpakInstallation *installation = gs_flatpak_get_installation (self, interactive);
	g_autoptr(GError) error_local = NULL;

	/* TRANSLATORS: status text when downloading new metadata */
	str = g_strdup_printf (_("Getting flatpak metadata for %s…"), remote_name);
	gs_app_set_summary_missing (app_dl, str);
	gs_plugin_status_update (self->plugin, app_dl, GS_PLUGIN_STATUS_DOWNLOADING);

	if (!flatpak_installation_update_remote_sync (installation,
						      remote_name,
						      cancellable,
						      &error_local)) {
		g_debug ("Failed to update metadata for remote %s: %s",
			 remote_name, error_local->message);
		gs_flatpak_error_convert (&error_local);
		g_propagate_error (error, g_steal_pointer (&error_local));
		return FALSE;
	}
	phelper = gs_flatpak_progress_helper_new (self->plugin, app_dl);
	if (!flatpak_installation_update_appstream_full_sync (installation,
							      remote_name,
							      NULL, /* arch */
							      gs_flatpak_progress_cb,
							      phelper,
							      NULL, /* out_changed */
							      cancellable,
							      error)) {
		gs_flatpak_error_convert (error);
		return FALSE;
	}

	/* success */
	gs_app_set_progress (app_dl, 100);
	return TRUE;
}

static gboolean
gs_flatpak_refresh_appstream (GsFlatpak     *self,
                              guint64        cache_age_secs,
                              gboolean       interactive,
                              GCancellable  *cancellable,
                              GError       **error)
{
	gboolean ret;
	g_autoptr(GPtrArray) xremotes = NULL;

	/* get remotes */
	xremotes = flatpak_installation_list_remotes (gs_flatpak_get_installation (self, interactive),
						      cancellable,
						      error);
	if (xremotes == NULL) {
		gs_flatpak_error_convert (error);
		return FALSE;
	}
	for (guint i = 0; i < xremotes->len; i++) {
		const gchar *remote_name;
		guint64 tmp;
		g_autoptr(GError) error_local = NULL;
		g_autoptr(GFile) file = NULL;
		g_autoptr(GFile) file_timestamp = NULL;
		g_autofree gchar *appstream_fn = NULL;
		FlatpakRemote *xremote = g_ptr_array_index (xremotes, i);
		g_autoptr(GMutexLocker) locker = NULL;

		/* not enabled */
		if (flatpak_remote_get_disabled (xremote))
			continue;

		remote_name = flatpak_remote_get_name (xremote);
		locker = g_mutex_locker_new (&self->broken_remotes_mutex);

		/* skip known-broken repos */
		if (g_hash_table_lookup (self->broken_remotes, remote_name) != NULL) {
			g_debug ("skipping known broken remote: %s", remote_name);
			continue;
		}

		g_clear_pointer (&locker, g_mutex_locker_free);

		/* is the timestamp new enough */
		file_timestamp = flatpak_remote_get_appstream_timestamp (xremote, NULL);
		tmp = gs_utils_get_file_age (file_timestamp);
		if (tmp < cache_age_secs) {
			g_autofree gchar *fn = g_file_get_path (file_timestamp);
			g_debug ("%s is only %" G_GUINT64_FORMAT " seconds old, so ignoring refresh",
				 fn, tmp);
			continue;
		}

		/* download new data */
		g_debug ("%s is %" G_GUINT64_FORMAT " seconds old, so downloading new data",
			 remote_name, tmp);
		ret = gs_flatpak_refresh_appstream_remote (self,
							   remote_name,
							   interactive,
							   cancellable,
							   &error_local);
		if (!ret) {
			g_autoptr(GsPluginEvent) event = NULL;
			if (g_error_matches (error_local,
					     GS_PLUGIN_ERROR,
					     GS_PLUGIN_ERROR_FAILED)) {
				g_debug ("Failed to get AppStream metadata: %s",
					 error_local->message);

				locker = g_mutex_locker_new (&self->broken_remotes_mutex);

				/* don't try to fetch this again until refresh() */
				g_hash_table_insert (self->broken_remotes,
						     g_strdup (remote_name),
						     GUINT_TO_POINTER (1));
				continue;
			}

			/* allow the plugin loader to decide if this should be
			 * shown the user, possibly only for interactive jobs */
			gs_flatpak_error_convert (&error_local);
			event = gs_plugin_event_new ("error", error_local,
						     NULL);
			gs_plugin_event_add_flag (event, GS_PLUGIN_EVENT_FLAG_WARNING);
			gs_plugin_report_event (self->plugin, event);
			continue;
		}

		/* add the new AppStream repo to the shared silo */
		file = flatpak_remote_get_appstream_dir (xremote, NULL);
		appstream_fn = g_file_get_path (file);
		g_debug ("using AppStream metadata found at: %s", appstream_fn);
	}

	/* ensure the AppStream silo is up to date */
	if (!gs_flatpak_rescan_appstream_store (self, interactive, cancellable, error)) {
		gs_flatpak_internal_data_changed (self);
		return FALSE;
	}

	return TRUE;
}

static void
gs_flatpak_set_metadata_installed (GsFlatpak *self,
				   GsApp *app,
				   FlatpakInstalledRef *xref,
				   gboolean interactive,
				   GCancellable *cancellable)
{
	const gchar *appdata_version;
	guint64 mtime;
	guint64 size_installed;
	g_autofree gchar *metadata_fn = NULL;
	g_autoptr(GFile) file = NULL;
	g_autoptr(GFileInfo) info = NULL;

	/* for all types */
	gs_flatpak_set_metadata (self, app, FLATPAK_REF (xref));
	if (gs_app_get_metadata_item (app, "LingmoAppstore::Creator") == NULL) {
		gs_app_set_metadata (app, "LingmoAppstore::Creator",
				     gs_plugin_get_name (self->plugin));
	}

	/* get the last time the app was updated */
	metadata_fn = g_build_filename (flatpak_installed_ref_get_deploy_dir (xref),
					"..",
					"active",
					"metadata",
					NULL);
	file = g_file_new_for_path (metadata_fn);
	info = g_file_query_info (file,
				  G_FILE_ATTRIBUTE_TIME_MODIFIED,
				  G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
				  NULL, NULL);
	if (info != NULL) {
		mtime = g_file_info_get_attribute_uint64 (info, G_FILE_ATTRIBUTE_TIME_MODIFIED);
		gs_app_set_install_date (app, mtime);
	}

	/* If it's a runtime, check if the main-app info should be set. Note that
	 * checking the app for AS_COMPONENT_KIND_RUNTIME is not good enough because it
	 * could be e.g. AS_COMPONENT_KIND_LOCALIZATION and still be a runtime from
	 * Flatpak's perspective.
	 */
	if (gs_flatpak_app_get_ref_kind (app) == FLATPAK_REF_KIND_RUNTIME &&
	    gs_flatpak_app_get_main_app_ref_name (app) == NULL) {
		g_autoptr(GError) error = NULL;
		g_autoptr(GKeyFile) metadata_file = NULL;
		metadata_file = g_key_file_new ();
		if (g_key_file_load_from_file (metadata_file, metadata_fn,
					       G_KEY_FILE_NONE, &error)) {
			g_autofree gchar *main_app = g_key_file_get_string (metadata_file,
									    "ExtensionOf",
									    "ref", NULL);
			if (main_app != NULL)
				gs_flatpak_app_set_main_app_ref_name (app, main_app);
		} else {
			g_warning ("Error loading the metadata file for '%s': %s",
				   gs_app_get_unique_id (app), error->message);
		}
	}

	/* this is faster than resolving */
	if (gs_app_get_origin (app) == NULL)
		gs_flatpak_set_app_origin (self, app, flatpak_installed_ref_get_origin (xref), NULL, interactive, cancellable);

	/* this is faster than flatpak_installation_fetch_remote_size_sync() */
	size_installed = flatpak_installed_ref_get_installed_size (xref);
	gs_app_set_size_installed (app, (size_installed != 0) ? GS_SIZE_TYPE_VALID : GS_SIZE_TYPE_UNKNOWN, size_installed);

	appdata_version = flatpak_installed_ref_get_appdata_version (xref);
	if (appdata_version != NULL)
		gs_app_set_version (app, appdata_version);
}

static GsApp *
gs_flatpak_create_installed (GsFlatpak *self,
			     FlatpakInstalledRef *xref,
			     FlatpakRemote *xremote,
			     gboolean interactive,
			     GCancellable *cancellable)
{
	g_autoptr(GsApp) app = NULL;
	const gchar *origin;

	g_return_val_if_fail (xref != NULL, NULL);

	/* create new object */
	origin = flatpak_installed_ref_get_origin (xref);
	app = gs_flatpak_create_app (self, origin, FLATPAK_REF (xref), xremote, interactive, cancellable);

	/* Set the state to installed only from some states, to not override the updatable-live or other states */
	if (gs_app_get_state (app) == GS_APP_STATE_UNKNOWN ||
	    gs_app_get_state (app) == GS_APP_STATE_AVAILABLE) {
		gs_app_set_state (app, GS_APP_STATE_UNKNOWN);
		gs_app_set_state (app, GS_APP_STATE_INSTALLED);
	}

	gs_flatpak_set_metadata_installed (self, app, xref, interactive, cancellable);
	return g_steal_pointer (&app);
}

gboolean
gs_flatpak_add_installed (GsFlatpak *self,
			  GsAppList *list,
			  gboolean interactive,
			  GCancellable *cancellable,
			  GError **error)
{
	g_autoptr(GPtrArray) xrefs = NULL;

	/* get apps and runtimes */
	xrefs = flatpak_installation_list_installed_refs (gs_flatpak_get_installation (self, interactive),
							  cancellable, error);
	if (xrefs == NULL) {
		gs_flatpak_error_convert (error);
		return FALSE;
	}

	gs_flatpak_ensure_remote_title (self, interactive, cancellable);

	for (guint i = 0; i < xrefs->len; i++) {
		FlatpakInstalledRef *xref = g_ptr_array_index (xrefs, i);
		g_autoptr(GsApp) app = gs_flatpak_create_installed (self, xref, NULL, interactive, cancellable);
		gs_app_list_add (list, app);
	}

	return TRUE;
}

gboolean
gs_flatpak_add_sources (GsFlatpak *self,
			GsAppList *list,
			gboolean interactive,
			GCancellable *cancellable,
			GError **error)
{
	g_autoptr(GPtrArray) xrefs = NULL;
	g_autoptr(GPtrArray) xremotes = NULL;
	FlatpakInstallation *installation = gs_flatpak_get_installation (self, interactive);

	/* refresh */
	if (!gs_flatpak_rescan_app_data (self, interactive, cancellable, error))
		return FALSE;

	/* get installed apps and runtimes */
	xrefs = flatpak_installation_list_installed_refs (installation,
							  cancellable,
							  error);
	if (xrefs == NULL) {
		gs_flatpak_error_convert (error);
		return FALSE;
	}

	/* get available remotes */
	xremotes = flatpak_installation_list_remotes (installation,
						      cancellable,
						      error);
	if (xremotes == NULL) {
		gs_flatpak_error_convert (error);
		return FALSE;
	}
	for (guint i = 0; i < xremotes->len; i++) {
		FlatpakRemote *xremote = g_ptr_array_index (xremotes, i);
		g_autoptr(GsApp) app = NULL;

		/* apps installed from bundles add their own remote that only
		 * can be used for updating that app only -- so hide them */
		if (flatpak_remote_get_noenumerate (xremote))
			continue;

		/* create app */
		app = gs_flatpak_create_source (self, xremote);
		gs_app_list_add (list, app);

		/* add related apps, i.e. what was installed from there */
		for (guint j = 0; j < xrefs->len; j++) {
			FlatpakInstalledRef *xref = g_ptr_array_index (xrefs, j);
			g_autoptr(GsApp) related = NULL;

			/* only apps */
			if (flatpak_ref_get_kind (FLATPAK_REF (xref)) != FLATPAK_REF_KIND_APP)
				continue;
			if (g_strcmp0 (flatpak_installed_ref_get_origin (xref),
				       flatpak_remote_get_name (xremote)) != 0)
				continue;
			related = gs_flatpak_create_installed (self, xref, xremote, interactive, cancellable);
			gs_app_add_related (app, related);
		}
	}
	return TRUE;
}

GsApp *
gs_flatpak_find_source_by_url (GsFlatpak *self,
			       const gchar *url,
			       gboolean interactive,
			       GCancellable *cancellable,
			       GError **error)
{
	g_autoptr(GPtrArray) xremotes = NULL;

	g_return_val_if_fail (url != NULL, NULL);

	xremotes = flatpak_installation_list_remotes (gs_flatpak_get_installation (self, interactive), cancellable, error);
	if (xremotes == NULL)
		return NULL;
	for (guint i = 0; i < xremotes->len; i++) {
		FlatpakRemote *xremote = g_ptr_array_index (xremotes, i);
		g_autofree gchar *url_tmp = flatpak_remote_get_url (xremote);
		if (g_strcmp0 (url, url_tmp) == 0)
			return gs_flatpak_create_source (self, xremote);
	}
	g_set_error (error,
		     GS_PLUGIN_ERROR,
		     GS_PLUGIN_ERROR_NOT_SUPPORTED,
		     "cannot find %s", url);
	return NULL;
}

/* transfer full */
GsApp *
gs_flatpak_ref_to_app (GsFlatpak *self,
		       const gchar *ref,
		       gboolean interactive,
		       GCancellable *cancellable,
		       GError **error)
{
	g_autoptr(GPtrArray) xremotes = NULL;
	FlatpakInstallation *installation = gs_flatpak_get_installation (self, interactive);

	g_return_val_if_fail (ref != NULL, NULL);

	g_mutex_lock (&self->installed_refs_mutex);

	if (self->installed_refs == NULL) {
		self->installed_refs = flatpak_installation_list_installed_refs (installation,
								 cancellable, error);

		if (self->installed_refs == NULL) {
			g_mutex_unlock (&self->installed_refs_mutex);
			gs_flatpak_error_convert (error);
			return NULL;
		}
	}

	for (guint i = 0; i < self->installed_refs->len; i++) {
		g_autoptr(FlatpakInstalledRef) xref = g_object_ref (g_ptr_array_index (self->installed_refs, i));
		g_autofree gchar *ref_tmp = flatpak_ref_format_ref (FLATPAK_REF (xref));
		if (g_strcmp0 (ref, ref_tmp) == 0) {
			g_mutex_unlock (&self->installed_refs_mutex);
			return gs_flatpak_create_installed (self, xref, NULL, interactive, cancellable);
		}
	}

	g_mutex_unlock (&self->installed_refs_mutex);

	/* look at each remote xref */
	xremotes = flatpak_installation_list_remotes (installation,
						      cancellable, error);
	if (xremotes == NULL) {
		gs_flatpak_error_convert (error);
		return NULL;
	}
	for (guint i = 0; i < xremotes->len; i++) {
		FlatpakRemote *xremote = g_ptr_array_index (xremotes, i);
		g_autoptr(GError) error_local = NULL;
		g_autoptr(GPtrArray) refs_remote = NULL;

		/* disabled */
		if (flatpak_remote_get_disabled (xremote))
			continue;
		refs_remote = flatpak_installation_list_remote_refs_sync (installation,
									  flatpak_remote_get_name (xremote),
									  cancellable,
									  &error_local);
		if (refs_remote == NULL) {
			g_debug ("failed to list refs in '%s': %s",
				 flatpak_remote_get_name (xremote),
				 error_local->message);
			continue;
		}
		for (guint j = 0; j < refs_remote->len; j++) {
			FlatpakRef *xref = g_ptr_array_index (refs_remote, j);
			g_autofree gchar *ref_tmp = flatpak_ref_format_ref (xref);
			if (g_strcmp0 (ref, ref_tmp) == 0) {
				const gchar *origin = flatpak_remote_get_name (xremote);
				return gs_flatpak_create_app (self, origin, xref, xremote, interactive, cancellable);
			}
		}
	}

	/* nothing found */
	g_set_error (error,
		     GS_PLUGIN_ERROR,
		     GS_PLUGIN_ERROR_NOT_SUPPORTED,
		     "cannot find %s", ref);
	return NULL;
}

/* This is essentially the inverse of gs_flatpak_app_new_from_repo_file() */
static void
gs_flatpak_update_remote_from_app (GsFlatpak     *self,
                                   FlatpakRemote *xremote,
                                   GsApp         *app)
{
	const gchar *gpg_key;
	const gchar *branch;
	const gchar *title, *homepage, *comment, *description;
	const gchar *filter;
	g_autoptr(GPtrArray) icons = NULL;

	flatpak_remote_set_disabled (xremote, FALSE);

	flatpak_remote_set_url (xremote, gs_flatpak_app_get_repo_url (app));
	flatpak_remote_set_noenumerate (xremote, FALSE);

	title = gs_app_get_name (app);
	if (title != NULL)
		flatpak_remote_set_title (xremote, title);

	/* decode GPG key if set */
	gpg_key = gs_flatpak_app_get_repo_gpgkey (app);
	if (gpg_key != NULL) {
		gsize data_len = 0;
		g_autofree guchar *data = NULL;
		g_autoptr(GBytes) bytes = NULL;
		data = g_base64_decode (gpg_key, &data_len);
		bytes = g_bytes_new (data, data_len);
		flatpak_remote_set_gpg_verify (xremote, TRUE);
		flatpak_remote_set_gpg_key (xremote, bytes);
	} else {
		flatpak_remote_set_gpg_verify (xremote, FALSE);
	}

	/* default branch */
	branch = gs_app_get_branch (app);
	if (branch != NULL)
		flatpak_remote_set_default_branch (xremote, branch);

	/* optional data */
	homepage = gs_app_get_url (app, AS_URL_KIND_HOMEPAGE);
	if (homepage != NULL)
		flatpak_remote_set_homepage (xremote, homepage);

	comment = gs_app_get_summary (app);
	if (comment != NULL)
		flatpak_remote_set_comment (xremote, comment);

	description = gs_app_get_description (app);
	if (description != NULL)
		flatpak_remote_set_description (xremote, description);

	icons = gs_app_dup_icons (app);
	for (guint i = 0; icons != NULL && i < icons->len; i++) {
		GIcon *icon = g_ptr_array_index (icons, i);

		if (GS_IS_REMOTE_ICON (icon)) {
			flatpak_remote_set_icon (xremote,
						 gs_remote_icon_get_uri (GS_REMOTE_ICON (icon)));
			break;
		}
	}

	/* With the other fields, we always want to add as much information as
	 * we can to the @xremote. With the filter, though, we want to drop it
	 * if no filter is set on the @app. Importing an updated flatpakrepo
	 * file is one of the methods for switching from (for example) filtered
	 * flathub to unfiltered flathub. So if @app doesn’t have a filter set,
	 * clear it on the @xremote (i.e. don’t check for NULL). */
	filter = gs_flatpak_app_get_repo_filter (app);
	flatpak_remote_set_filter (xremote, filter);
}

static FlatpakRemote *
gs_flatpak_create_new_remote (GsFlatpak *self,
                              GsApp *app,
                              GCancellable *cancellable,
                              GError **error)
{
	g_autoptr(FlatpakRemote) xremote = NULL;

	/* create a new remote */
	xremote = flatpak_remote_new (gs_app_get_id (app));
	gs_flatpak_update_remote_from_app (self, xremote, app);

	return g_steal_pointer (&xremote);
}

static FlatpakRemote * /* (transfer full) */
gs_flatpak_remote_by_name (GsFlatpak *self,
			   const gchar *lookup_name,
			   gboolean interactive,
			   GCancellable *cancellable,
			   GError **error)
{
	g_autoptr(GMutexLocker) locker = g_mutex_locker_new (&self->installed_refs_mutex);
	FlatpakRemote *res = NULL;

	if (self->remotes_by_name == NULL) {
		g_autoptr(GPtrArray) remotes = flatpak_installation_list_remotes (gs_flatpak_get_installation (self, interactive),
										  cancellable, error);
		if (remotes == NULL)
			return NULL;
		self->remotes_by_name = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
		for (guint i = 0; i < remotes->len; i++) {
			FlatpakRemote *remote = g_ptr_array_index (remotes, i);
			const gchar *name = flatpak_remote_get_name (remote);
			if (name != NULL) {
				g_hash_table_insert (self->remotes_by_name, g_strdup (name), g_object_ref (remote));
				if (res == NULL && g_strcmp0 (name, lookup_name) == 0)
					res = g_object_ref (remote);
			}
		}
	} else {
		res = g_hash_table_lookup (self->remotes_by_name, lookup_name);
		if (res != NULL)
			g_object_ref (res);
	}

	if (res == NULL && error != NULL && *error == NULL)
		g_set_error (error, FLATPAK_ERROR, FLATPAK_ERROR_REMOTE_NOT_FOUND, "Remote '%s' not found", lookup_name);

	return res;
}

/* @is_install is %TRUE if the repo is being installed, or %FALSE if it’s being
 * enabled. If it’s being enabled, no properties apart from enabled/disabled
 * should be modified. */
gboolean
gs_flatpak_app_install_source (GsFlatpak *self,
			       GsApp *app,
			       gboolean is_install,
			       gboolean interactive,
			       GCancellable *cancellable,
			       GError **error)
{
	g_autoptr(FlatpakRemote) xremote = NULL;
	FlatpakInstallation *installation = gs_flatpak_get_installation (self, interactive);

	xremote = gs_flatpak_remote_by_name (self, gs_app_get_id (app), interactive, cancellable, NULL);
	if (xremote != NULL) {
		/* if the remote already exists, just enable it and update it */
		g_debug ("modifying existing remote %s", flatpak_remote_get_name (xremote));
		flatpak_remote_set_disabled (xremote, FALSE);

		if (is_install &&
		    gs_flatpak_app_get_file_kind (app) == GS_FLATPAK_APP_FILE_KIND_REPO) {
			gs_flatpak_update_remote_from_app (self, xremote, app);
		}
	} else if (!is_install) {
		g_set_error (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_FAILED, "Cannot enable flatpak remote '%s', remote not found", gs_app_get_id (app));
	} else {
		/* create a new remote */
		xremote = gs_flatpak_create_new_remote (self, app, cancellable, error);
	}

	/* install it */
	gs_app_set_state (app, GS_APP_STATE_INSTALLING);
	if (!flatpak_installation_modify_remote (installation,
						 xremote,
						 cancellable,
						 error)) {
		gs_flatpak_error_convert (error);
		g_prefix_error (error, "cannot modify remote: ");
		gs_app_set_state_recover (app);
		gs_flatpak_internal_data_changed (self);
		return FALSE;
	}

	/* Mark the internal cache as obsolete. */
	gs_flatpak_internal_data_changed (self);

	/* success */
	gs_app_set_state (app, GS_APP_STATE_INSTALLED);

	gs_plugin_repository_changed (self->plugin, app);

	return TRUE;
}

static GsApp *
get_main_app_of_related (GsFlatpak *self,
			 GsApp *related_app,
			 gboolean interactive,
			 GCancellable *cancellable,
			 GError **error)
{
	g_autoptr(FlatpakInstalledRef) ref = NULL;
	const gchar *ref_name;
	g_auto(GStrv) app_tokens = NULL;
	FlatpakRefKind ref_kind = FLATPAK_REF_KIND_RUNTIME;

	ref_name = gs_flatpak_app_get_main_app_ref_name (related_app);
	if (ref_name == NULL) {
		g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND,
			     "%s doesn't have a main app set to it.",
			     gs_app_get_unique_id (related_app));
		return NULL;
	}

	app_tokens = g_strsplit (ref_name, "/", -1);
	if (g_strv_length (app_tokens) != 4) {
		g_set_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA,
			     "The main app of %s has an invalid name: %s",
			     gs_app_get_unique_id (related_app), ref_name);
		return NULL;
	}

	/* get the right ref kind for the main app */
	if (g_strcmp0 (app_tokens[0], "app") == 0)
		ref_kind = FLATPAK_REF_KIND_APP;

	/* this function only returns G_IO_ERROR_NOT_FOUND when the metadata file
	 * is missing, but if that's the case then things should have broken before
	 * this point */
	ref = flatpak_installation_get_installed_ref (gs_flatpak_get_installation (self, interactive),
						      ref_kind,
						      app_tokens[1],
						      app_tokens[2],
						      app_tokens[3],
						      cancellable,
						      error);
	if (ref == NULL)
		return NULL;

	return gs_flatpak_create_installed (self, ref, NULL, interactive, cancellable);
}

static GsApp *
get_real_app_for_update (GsFlatpak *self,
			 GsApp *app,
			 gboolean interactive,
			 GCancellable *cancellable,
			 GError **error)
{
	GsApp *main_app = NULL;
	g_autoptr(GError) error_local = NULL;

	if (gs_flatpak_app_get_ref_kind (app) == FLATPAK_REF_KIND_RUNTIME)
		main_app = get_main_app_of_related (self, app, interactive, cancellable, &error_local);

	if (main_app == NULL) {
		/* not all runtimes are extensions, and in that case we get the
		 * not-found error, so we only report other types of errors */
		if (error_local != NULL &&
		    !g_error_matches (error_local, G_IO_ERROR, G_IO_ERROR_NOT_FOUND)) {
			g_propagate_error (error, g_steal_pointer (&error_local));
			gs_flatpak_error_convert (error);
			return NULL;
		}

		main_app = g_object_ref (app);
	} else {
		g_debug ("Related extension app %s of main app %s is updatable, so "
			 "setting the latter's state instead.", gs_app_get_unique_id (app),
			 gs_app_get_unique_id (main_app));
		gs_app_set_state (main_app, GS_APP_STATE_UPDATABLE_LIVE);
		/* Make sure the 'app' is not forgotten, it'll be added into the transaction later */
		gs_app_add_related (main_app, app);
	}

	return main_app;
}

gboolean
gs_flatpak_add_updates (GsFlatpak *self,
			GsAppList *list,
			gboolean interactive,
			GCancellable *cancellable,
			GError **error)
{
	g_autoptr(GPtrArray) xrefs = NULL;
	FlatpakInstallation *installation = gs_flatpak_get_installation (self, interactive);

	/* ensure valid */
	if (!gs_flatpak_rescan_app_data (self, interactive, cancellable, error))
		return FALSE;

	/* get all the updatable apps and runtimes */
	xrefs = flatpak_installation_list_installed_refs_for_update (installation,
								     cancellable,
								     error);
	if (xrefs == NULL) {
		gs_flatpak_error_convert (error);
		return FALSE;
	}

	gs_flatpak_ensure_remote_title (self, interactive, cancellable);

	/* look at each installed xref */
	for (guint i = 0; i < xrefs->len; i++) {
		FlatpakInstalledRef *xref = g_ptr_array_index (xrefs, i);
		const gchar *commit;
		const gchar *latest_commit;
		g_autoptr(GsApp) app = NULL;
		g_autoptr(GError) error_local = NULL;
		g_autoptr(GsApp) main_app = NULL;

		/* check the application has already been downloaded */
		commit = flatpak_ref_get_commit (FLATPAK_REF (xref));
		latest_commit = flatpak_installed_ref_get_latest_commit (xref);
		app = gs_flatpak_create_installed (self, xref, NULL, interactive, cancellable);
		main_app = get_real_app_for_update (self, app, interactive, cancellable, &error_local);
		if (main_app == NULL) {
			g_debug ("Couldn't get the main app for updatable app extension %s: "
				 "%s; adding the app itself to the updates list...",
				 gs_app_get_unique_id (app), error_local->message);
			g_clear_error (&error_local);
			main_app = g_object_ref (app);
		}

		/* if for some reason the app is already getting updated, then
		 * don't change its state */
		if (gs_app_get_state (main_app) != GS_APP_STATE_INSTALLING)
			gs_app_set_state (main_app, GS_APP_STATE_UPDATABLE_LIVE);

		/* set updatable state on the extension too, as it will have
		 * its state updated to installing then installed later on */
		if (gs_app_get_state (app) != GS_APP_STATE_INSTALLING)
			gs_app_set_state (app, GS_APP_STATE_UPDATABLE_LIVE);

		/* already downloaded */
		if (latest_commit && g_strcmp0 (commit, latest_commit) != 0) {
			g_debug ("%s has a downloaded update %s->%s",
				 flatpak_ref_get_name (FLATPAK_REF (xref)),
				 commit, latest_commit);
			gs_app_set_update_details_markup (main_app, NULL);
			gs_app_set_update_version (main_app, NULL);
			gs_app_set_update_urgency (main_app, AS_URGENCY_KIND_UNKNOWN);
			gs_app_set_size_download (main_app, GS_SIZE_TYPE_VALID, 0);

		/* needs download */
		} else {
			guint64 download_size = 0;
			g_debug ("%s needs update",
				 flatpak_ref_get_name (FLATPAK_REF (xref)));

			/* get the current download size */
			if (gs_app_get_size_download (main_app, NULL) != GS_SIZE_TYPE_VALID) {
				if (!flatpak_installation_fetch_remote_size_sync (installation,
										  gs_app_get_origin (app),
										  FLATPAK_REF (xref),
										  &download_size,
										  NULL,
										  cancellable,
										  &error_local)) {
					g_warning ("failed to get download size: %s",
						   error_local->message);
					g_clear_error (&error_local);
					gs_app_set_size_download (main_app, GS_SIZE_TYPE_UNKNOWABLE, 0);
				} else {
					gs_app_set_size_download (main_app, GS_SIZE_TYPE_VALID, download_size);
				}
			}
		}
		gs_flatpak_set_update_permissions (self, main_app, xref, interactive, cancellable);
		gs_app_list_add (list, main_app);
	}

	/* success */
	return TRUE;
}

gboolean
gs_flatpak_refresh (GsFlatpak *self,
		    guint64 cache_age_secs,
		    gboolean interactive,
		    GCancellable *cancellable,
		    GError **error)
{
	/* give all the repos a second chance */
	g_mutex_lock (&self->broken_remotes_mutex);
	g_hash_table_remove_all (self->broken_remotes);
	g_mutex_unlock (&self->broken_remotes_mutex);

	/* manually drop the cache in both installation instances;
	 * it's needed to have them both agree on the content. */
	if (!flatpak_installation_drop_caches (gs_flatpak_get_installation (self, FALSE),
					       cancellable,
					       error)) {
		gs_flatpak_error_convert (error);
		return FALSE;
	}

	if (!flatpak_installation_drop_caches (gs_flatpak_get_installation (self, TRUE),
					       cancellable,
					       error)) {
		gs_flatpak_error_convert (error);
		return FALSE;
	}

	/* drop the installed refs cache */
	g_mutex_lock (&self->installed_refs_mutex);
	g_clear_pointer (&self->installed_refs, g_ptr_array_unref);
	g_clear_pointer (&self->remotes_by_name, g_hash_table_unref);
	g_mutex_unlock (&self->installed_refs_mutex);

	/* manually do this in case we created the first appstream file */
	gs_flatpak_invalidate_silo (self);

	/* update AppStream metadata */
	if (!gs_flatpak_refresh_appstream (self, cache_age_secs, interactive, cancellable, error))
		return FALSE;

	/* success */
	return TRUE;
}

static gboolean
gs_plugin_refine_item_origin_hostname (GsFlatpak *self,
				       GsApp *app,
				       gboolean interactive,
				       GCancellable *cancellable,
				       GError **error)
{
	g_autoptr(FlatpakRemote) xremote = NULL;
	g_autofree gchar *url = NULL;
	g_autoptr(GError) error_local = NULL;

	/* already set */
	if (gs_app_get_origin_hostname (app) != NULL)
		return TRUE;

	/* no origin */
	if (gs_app_get_origin (app) == NULL)
		return TRUE;

	/* get the remote  */
	xremote = gs_flatpak_remote_by_name (self, gs_app_get_origin (app), interactive, cancellable, &error_local);
	if (xremote == NULL) {
		if (g_error_matches (error_local,
				     FLATPAK_ERROR,
				     FLATPAK_ERROR_REMOTE_NOT_FOUND)) {
			/* if the user deletes the -origin remote for a locally
			 * installed flatpakref file then we should just show
			 * 'localhost' and not return an error */
			gs_app_set_origin_hostname (app, "");
			return TRUE;
		}
		g_propagate_error (error, g_steal_pointer (&error_local));
		gs_flatpak_error_convert (error);
		return FALSE;
	}
	url = flatpak_remote_get_url (xremote);
	if (url == NULL) {
		g_set_error (error,
			     GS_PLUGIN_ERROR,
			     GS_PLUGIN_ERROR_INVALID_FORMAT,
			     "no URL for remote %s",
			     flatpak_remote_get_name (xremote));
		return FALSE;
	}
	gs_app_set_origin_hostname (app, url);
	return TRUE;
}

static gboolean
gs_refine_item_metadata (GsFlatpak  *self,
                         GsApp      *app,
                         GError    **error)
{
	g_autoptr(FlatpakRef) xref = NULL;

	/* already set */
	if (gs_flatpak_app_get_ref_name (app) != NULL)
		return TRUE;

	/* not a valid type */
	if (gs_app_get_kind (app) == AS_COMPONENT_KIND_REPOSITORY)
		return TRUE;

	/* AppStream sets the source to appname/arch/branch, if this isn't set
	 * we can't break out the fields */
	if (gs_app_get_source_default (app) == NULL) {
		g_autofree gchar *tmp = gs_app_to_string (app);
		g_warning ("no source set by appstream for %s: %s",
			   gs_plugin_get_name (self->plugin), tmp);
		return TRUE;
	}

	/* parse the ref */
	xref = flatpak_ref_parse (gs_app_get_source_default (app), error);
	if (xref == NULL) {
		gs_flatpak_error_convert (error);
		g_prefix_error (error, "failed to parse '%s': ",
				gs_app_get_source_default (app));
		return FALSE;
	}
	gs_flatpak_set_metadata (self, app, xref);

	/* success */
	return TRUE;
}

static gboolean
gs_plugin_refine_item_origin (GsFlatpak *self,
			      GsApp *app,
			      gboolean interactive,
			      GCancellable *cancellable,
			      GError **error)
{
	g_autofree gchar *ref_display = NULL;
	g_autoptr(GPtrArray) xremotes = NULL;
	FlatpakInstallation *installation = gs_flatpak_get_installation (self, interactive);

	/* already set */
	if (gs_app_get_origin (app) != NULL)
		return TRUE;

	/* not applicable */
	if (gs_app_get_state (app) == GS_APP_STATE_AVAILABLE_LOCAL)
		return TRUE;

	/* ensure metadata exists */
	if (!gs_refine_item_metadata (self, app, error))
		return FALSE;

	/* find list of remotes */
	ref_display = gs_flatpak_app_get_ref_display (app);
	g_debug ("looking for a remote for %s", ref_display);
	xremotes = flatpak_installation_list_remotes (installation,
						      cancellable, error);
	if (xremotes == NULL) {
		gs_flatpak_error_convert (error);
		return FALSE;
	}
	for (guint i = 0; i < xremotes->len; i++) {
		const gchar *remote_name;
		FlatpakRemote *xremote = g_ptr_array_index (xremotes, i);
		g_autoptr(FlatpakRemoteRef) xref = NULL;
		g_autoptr(GError) error_local = NULL;

		/* not enabled */
		if (flatpak_remote_get_disabled (xremote))
			continue;

		/* sync */
		remote_name = flatpak_remote_get_name (xremote);
		g_debug ("looking at remote %s", remote_name);
		xref = flatpak_installation_fetch_remote_ref_sync (installation,
								   remote_name,
								   gs_flatpak_app_get_ref_kind (app),
								   gs_flatpak_app_get_ref_name (app),
								   gs_flatpak_app_get_ref_arch (app),
								   gs_app_get_branch (app),
								   cancellable,
								   &error_local);
		if (xref != NULL) {
			g_debug ("found remote %s", remote_name);
			gs_flatpak_set_app_origin (self, app, remote_name, xremote, interactive, cancellable);
			gs_flatpak_app_set_commit (app, flatpak_ref_get_commit (FLATPAK_REF (xref)));
			gs_plugin_refine_item_scope (self, app);
			return TRUE;
		}
		g_debug ("%s failed to find remote %s: %s",
			 ref_display, remote_name, error_local->message);
	}

	/* not found */
	g_set_error (error,
		     GS_PLUGIN_ERROR,
		     GS_PLUGIN_ERROR_NOT_SUPPORTED,
		     "%s not found in any remote",
		     ref_display);
	return FALSE;
}

static FlatpakRef *
gs_flatpak_create_fake_ref (GsApp *app, GError **error)
{
	FlatpakRef *xref;
	g_autofree gchar *id = NULL;
	id = g_strdup_printf ("%s/%s/%s/%s",
			      gs_flatpak_app_get_ref_kind_as_str (app),
			      gs_flatpak_app_get_ref_name (app),
			      gs_flatpak_app_get_ref_arch (app),
			      gs_app_get_branch (app));
	xref = flatpak_ref_parse (id, error);
	if (xref == NULL) {
		gs_flatpak_error_convert (error);
		return NULL;
	}
	return xref;
}

/* the _unlocked() version doesn't call gs_flatpak_rescan_app_data,
 * in order to avoid taking the writer lock on self->silo_lock */
static gboolean
gs_flatpak_refine_app_state_unlocked (GsFlatpak *self,
                                      GsApp *app,
                                      gboolean interactive,
				      gboolean force_state_update,
                                      GCancellable *cancellable,
                                      GError **error)
{
	g_autoptr(FlatpakInstalledRef) ref = NULL;
	g_autoptr(GPtrArray) installed_refs = NULL;
	FlatpakInstallation *installation = gs_flatpak_get_installation (self, interactive);

	/* already found */
	if (!force_state_update &&
	    gs_app_get_state (app) != GS_APP_STATE_UNKNOWN)
		return TRUE;

	/* need broken out metadata */
	if (!gs_refine_item_metadata (self, app, error))
		return FALSE;

	/* ensure origin set */
	if (!gs_plugin_refine_item_origin (self, app, interactive, cancellable, error))
		return FALSE;

	/* find the app using the origin and the ID */
	g_mutex_lock (&self->installed_refs_mutex);

	if (self->installed_refs == NULL) {
		self->installed_refs = flatpak_installation_list_installed_refs (installation,
								 cancellable, error);

		if (self->installed_refs == NULL) {
			g_mutex_unlock (&self->installed_refs_mutex);
			gs_flatpak_error_convert (error);
			return FALSE;
		}
	}

	installed_refs = g_ptr_array_ref (self->installed_refs);

	for (guint i = 0; i < installed_refs->len; i++) {
		FlatpakInstalledRef *ref_tmp = g_ptr_array_index (installed_refs, i);
		const gchar *origin = flatpak_installed_ref_get_origin (ref_tmp);
		const gchar *name = flatpak_ref_get_name (FLATPAK_REF (ref_tmp));
		const gchar *arch = flatpak_ref_get_arch (FLATPAK_REF (ref_tmp));
		const gchar *branch = flatpak_ref_get_branch (FLATPAK_REF (ref_tmp));
		if (g_strcmp0 (origin, gs_app_get_origin (app)) == 0 &&
		    g_strcmp0 (name, gs_flatpak_app_get_ref_name (app)) == 0 &&
		    g_strcmp0 (arch, gs_flatpak_app_get_ref_arch (app)) == 0 &&
		    g_strcmp0 (branch, gs_app_get_branch (app)) == 0) {
			ref = g_object_ref (ref_tmp);
			break;
		}
	}
	g_mutex_unlock (&self->installed_refs_mutex);
	if (ref != NULL) {
		g_debug ("marking %s as installed with flatpak",
			 gs_app_get_unique_id (app));
		gs_flatpak_set_metadata_installed (self, app, ref, interactive, cancellable);
		if (force_state_update || gs_app_get_state (app) == GS_APP_STATE_UNKNOWN)
			gs_app_set_state (app, GS_APP_STATE_INSTALLED);

		/* flatpak only allows one installed app to be launchable */
		if (flatpak_installed_ref_get_is_current (ref)) {
			gs_app_remove_quirk (app, GS_APP_QUIRK_NOT_LAUNCHABLE);
		} else {
			g_debug ("%s is not current, and therefore not launchable",
				 gs_app_get_unique_id (app));
			gs_app_add_quirk (app, GS_APP_QUIRK_NOT_LAUNCHABLE);
		}
		return TRUE;
	}

	/* anything not installed just check the remote is still present */
	if ((force_state_update || gs_app_get_state (app) == GS_APP_STATE_UNKNOWN) &&
	    gs_app_get_origin (app) != NULL) {
		g_autoptr(FlatpakRemote) xremote = NULL;
		xremote = gs_flatpak_remote_by_name (self, gs_app_get_origin (app), interactive, cancellable, NULL);
		if (xremote != NULL) {
			if (flatpak_remote_get_disabled (xremote)) {
				g_debug ("%s is available with flatpak "
					 "but %s is disabled",
					 gs_app_get_unique_id (app),
					 flatpak_remote_get_name (xremote));
				gs_app_set_state (app, GS_APP_STATE_UNAVAILABLE);
			} else {
				g_debug ("marking %s as available with flatpak",
					 gs_app_get_unique_id (app));
				gs_app_set_state (app, GS_APP_STATE_AVAILABLE);
			}
		} else {
			gs_app_set_state (app, GS_APP_STATE_UNKNOWN);
			g_debug ("failed to find %s remote %s for %s",
				 self->id,
				 gs_app_get_origin (app),
				 gs_app_get_unique_id (app));
		}
	}

	/* success */
	return TRUE;
}

gboolean
gs_flatpak_refine_app_state (GsFlatpak *self,
                             GsApp *app,
                             gboolean interactive,
			     gboolean force_state_update,
                             GCancellable *cancellable,
                             GError **error)
{
	/* ensure valid */
	if (!gs_flatpak_rescan_app_data (self, interactive, cancellable, error))
		return FALSE;

	return gs_flatpak_refine_app_state_unlocked (self, app, interactive, force_state_update, cancellable, error);
}

static GsApp *
gs_flatpak_create_runtime (GsFlatpak   *self,
                           GsApp       *parent,
                           const gchar *runtime,
                           gboolean     interactive,
                           GCancellable *cancellable)
{
	g_autofree gchar *source = NULL;
	g_auto(GStrv) split = NULL;
	g_autoptr(GsApp) app_cache = NULL;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GError) local_error = NULL;
	const gchar *origin;

	/* get the name/arch/branch */
	split = g_strsplit (runtime, "/", -1);
	if (g_strv_length (split) != 3)
		return NULL;

	/* create the complete GsApp from the single string */
	app = gs_app_new (split[0]);
	gs_flatpak_claim_app (self, app);
	source = g_strdup_printf ("runtime/%s", runtime);
	gs_app_add_source (app, source);
	gs_app_set_metadata (app, "LingmoAppstore::packagename-value",  source);
	gs_app_set_kind (app, AS_COMPONENT_KIND_RUNTIME);
	gs_app_set_branch (app, split[2]);

	origin = gs_app_get_origin (parent);
	if (origin != NULL) {
		g_autoptr(FlatpakRemoteRef) xref = NULL;

		xref = flatpak_installation_fetch_remote_ref_sync (gs_flatpak_get_installation (self, interactive),
								   origin,
								   FLATPAK_REF_KIND_RUNTIME,
								   gs_app_get_id (app),
								   gs_flatpak_app_get_ref_arch (parent),
								   gs_app_get_branch (app),
								   cancellable,
								   NULL);

		/* Prefer runtime from the same origin as the parent application */
		if (xref)
			gs_app_set_origin (app, origin);
	}

	/* search in the cache */
	app_cache = gs_plugin_cache_lookup (self->plugin, gs_app_get_unique_id (app));
	if (app_cache != NULL &&
	    g_strcmp0 (gs_flatpak_app_get_ref_name (app_cache), split[0]) == 0 &&
	    g_strcmp0 (gs_flatpak_app_get_ref_arch (app_cache), split[1]) == 0 &&
	    g_strcmp0 (gs_app_get_branch (app_cache), split[2]) == 0) {
		/* since the cached runtime can have been created somewhere else
		 * (we're using a global cache), we need to make sure that a
		 * source is set */
		if (gs_app_get_source_default (app_cache) == NULL) {
			gs_app_add_source (app_cache, source);
			gs_app_set_metadata (app_cache, "LingmoAppstore::packagename-value",  source);
		}
		return g_steal_pointer (&app_cache);
	} else {
		g_clear_object (&app_cache);
	}

	/* if the app is per-user we can also use the installed system runtime */
	if (gs_app_get_scope (parent) == AS_COMPONENT_SCOPE_USER) {
		gs_app_set_scope (app, AS_COMPONENT_SCOPE_UNKNOWN);
		app_cache = gs_plugin_cache_lookup (self->plugin, gs_app_get_unique_id (app));
		if (app_cache != NULL &&
		    g_strcmp0 (gs_flatpak_app_get_ref_name (app_cache), split[0]) == 0 &&
		    g_strcmp0 (gs_flatpak_app_get_ref_arch (app_cache), split[1]) == 0 &&
		    g_strcmp0 (gs_app_get_branch (app_cache), split[2]) == 0) {
			return g_steal_pointer (&app_cache);
		} else {
			g_clear_object (&app_cache);
		}
	}

	/* set superclassed app properties */
	gs_flatpak_app_set_ref_kind (app, FLATPAK_REF_KIND_RUNTIME);
	gs_flatpak_app_set_ref_name (app, split[0]);
	gs_flatpak_app_set_ref_arch (app, split[1]);

	if (!gs_flatpak_refine_app_state_unlocked (self, app, interactive, FALSE, NULL, &local_error))
		g_debug ("Failed to refine state for runtime '%s': %s", gs_app_get_unique_id (app), local_error->message);

	/* save in the cache */
	gs_plugin_cache_add (self->plugin, NULL, app);
	return g_steal_pointer (&app);
}

static gboolean
gs_flatpak_set_app_metadata (GsFlatpak *self,
			     GsApp *app,
			     const gchar *data,
			     gsize length,
			     gboolean interactive,
			     GCancellable *cancellable,
			     GError **error)
{
	gboolean secure = TRUE;
	g_autofree gchar *name = NULL;
	g_autofree gchar *runtime = NULL;
	g_autoptr(GKeyFile) kf = NULL;
	g_autoptr(GsApp) app_runtime = NULL;
	g_autoptr(GsAppPermissions) permissions = NULL;
	g_auto(GStrv) shared = NULL;
	g_auto(GStrv) sockets = NULL;
	g_auto(GStrv) filesystems = NULL;

	kf = g_key_file_new ();
	if (!g_key_file_load_from_data (kf, data, length, G_KEY_FILE_NONE, error)) {
		gs_flatpak_error_convert (error);
		return FALSE;
	}
	name = g_key_file_get_string (kf, "Application", "name", error);
	if (name == NULL) {
		gs_flatpak_error_convert (error);
		return FALSE;
	}
	gs_flatpak_app_set_ref_name (app, name);
	runtime = g_key_file_get_string (kf, "Application", "runtime", error);
	if (runtime == NULL) {
		gs_flatpak_error_convert (error);
		return FALSE;
	}

	shared = g_key_file_get_string_list (kf, "Context", "shared", NULL, NULL);
	if (shared != NULL) {
		/* SHM isn't secure enough */
		if (g_strv_contains ((const gchar * const *) shared, "ipc"))
			secure = FALSE;
	}
	sockets = g_key_file_get_string_list (kf, "Context", "sockets", NULL, NULL);
	if (sockets != NULL) {
		/* X11 isn't secure enough */
		if (g_strv_contains ((const gchar * const *) sockets, "x11"))
			secure = FALSE;
	}
	filesystems = g_key_file_get_string_list (kf, "Context", "filesystems", NULL, NULL);
	if (filesystems != NULL) {
		/* secure apps should be using portals */
		if (g_strv_contains ((const gchar * const *) filesystems, "home"))
			secure = FALSE;
	}

	permissions = perms_from_metadata (kf);
	gs_app_set_permissions (app, permissions);
	/* this is actually quite hard to achieve */
	if (secure)
		gs_app_add_kudo (app, GS_APP_KUDO_SANDBOXED_SECURE);

	/* create runtime */
	app_runtime = gs_flatpak_create_runtime (self, app, runtime, interactive, cancellable);
	if (app_runtime != NULL) {
		gs_plugin_refine_item_scope (self, app_runtime);
		gs_app_set_runtime (app, app_runtime);
	}

	/* we always get this, but it's a low bar... */
	gs_app_add_kudo (app, GS_APP_KUDO_SANDBOXED);

	return TRUE;
}

static GBytes *
gs_flatpak_fetch_remote_metadata (GsFlatpak *self,
				  GsApp *app,
				  gboolean interactive,
				  GCancellable *cancellable,
				  GError **error)
{
	g_autoptr(GBytes) data = NULL;
	g_autoptr(FlatpakRef) xref = NULL;
	g_autoptr(GError) local_error = NULL;

	/* no origin */
	if (gs_app_get_origin (app) == NULL) {
		g_set_error (error,
			     GS_PLUGIN_ERROR,
			     GS_PLUGIN_ERROR_NOT_SUPPORTED,
			     "no origin set when getting metadata for %s",
			     gs_app_get_unique_id (app));
		return NULL;
	}

	/* fetch from the server */
	xref = gs_flatpak_create_fake_ref (app, error);
	if (xref == NULL)
		return NULL;
	data = flatpak_installation_fetch_remote_metadata_sync (gs_flatpak_get_installation (self, interactive),
								gs_app_get_origin (app),
								xref,
								cancellable,
								&local_error);
	if (data == NULL) {
		if (g_error_matches (local_error, FLATPAK_ERROR, FLATPAK_ERROR_REF_NOT_FOUND) &&
		    !gs_plugin_get_network_available (self->plugin)) {
			local_error->code = GS_PLUGIN_ERROR_NO_NETWORK;
			local_error->domain = GS_PLUGIN_ERROR;
		} else {
			gs_flatpak_error_convert (&local_error);
		}
		g_propagate_error (error, g_steal_pointer (&local_error));
		return NULL;
	}
	return g_steal_pointer (&data);
}

static gboolean
gs_plugin_refine_item_metadata (GsFlatpak *self,
				GsApp *app,
				gboolean interactive,
				GCancellable *cancellable,
				GError **error)
{
	const gchar *str;
	gsize len = 0;
	g_autofree gchar *contents = NULL;
	g_autofree gchar *installation_path_str = NULL;
	g_autofree gchar *install_path = NULL;
	g_autoptr(GBytes) data = NULL;
	g_autoptr(GFile) installation_path = NULL;

	/* not applicable */
	if (gs_app_get_kind (app) == AS_COMPONENT_KIND_REPOSITORY)
		return TRUE;
	if (gs_flatpak_app_get_ref_kind (app) != FLATPAK_REF_KIND_APP)
		return TRUE;

	/* already done */
	if (gs_app_has_kudo (app, GS_APP_KUDO_SANDBOXED))
		return TRUE;

	/* this is quicker than doing network IO */
	installation_path = flatpak_installation_get_path (self->installation_noninteractive);
	installation_path_str = g_file_get_path (installation_path);
	install_path = g_build_filename (installation_path_str,
					 gs_flatpak_app_get_ref_kind_as_str (app),
					 gs_flatpak_app_get_ref_name (app),
					 gs_flatpak_app_get_ref_arch (app),
					 gs_app_get_branch (app),
					 "active",
					 "metadata",
					 NULL);
	if (g_file_test (install_path, G_FILE_TEST_EXISTS)) {
		if (!g_file_get_contents (install_path, &contents, &len, error))
			return FALSE;
		str = contents;
	} else {
		data = gs_flatpak_fetch_remote_metadata (self, app, interactive,
							 cancellable,
							 error);
		if (data == NULL)
			return FALSE;
		str = g_bytes_get_data (data, &len);
	}

	/* parse key file */
	if (!gs_flatpak_set_app_metadata (self, app, str, len, interactive, cancellable, error))
		return FALSE;
	return TRUE;
}

static FlatpakInstalledRef *
gs_flatpak_get_installed_ref (GsFlatpak *self,
			      GsApp *app,
			      gboolean interactive,
			      GCancellable *cancellable,
			      GError **error)
{
	FlatpakInstalledRef *ref;
	ref = flatpak_installation_get_installed_ref (gs_flatpak_get_installation (self, interactive),
						      gs_flatpak_app_get_ref_kind (app),
						      gs_flatpak_app_get_ref_name (app),
						      gs_flatpak_app_get_ref_arch (app),
						      gs_app_get_branch (app),
						      cancellable,
						      error);
	if (ref == NULL)
		gs_flatpak_error_convert (error);
	return ref;
}

static gboolean
gs_flatpak_prune_addons_list (GsFlatpak *self,
			      GsApp *app,
			      gboolean interactive,
			      GCancellable *cancellable,
			      GError **error)
{
	g_autoptr(GsAppList) addons_list = NULL;
	g_autoptr(GPtrArray) installed_related_refs = NULL;
	g_autoptr(GPtrArray) remote_related_refs = NULL;
	g_autoptr(GPtrArray) remove_addons = NULL;
	g_autofree gchar *ref = NULL;
	FlatpakInstallation *installation = gs_flatpak_get_installation (self, interactive);
	g_autoptr(GError) error_local = NULL;

	addons_list = gs_app_dup_addons (app);
	if (addons_list == NULL || gs_app_list_length (addons_list) == 0)
		return TRUE;

	if (gs_app_get_origin (app) == NULL)
		return TRUE;

	/* return early if the addons haven't been refined */
	for (guint i = 0; i < gs_app_list_length (addons_list); i++) {
		GsApp *app_addon = gs_app_list_index (addons_list, i);

		if (gs_flatpak_app_get_ref_name (app_addon) == NULL ||
		    gs_flatpak_app_get_ref_arch (app_addon) == NULL ||
		    gs_app_get_branch (app_addon) == NULL)
			return TRUE;
	}

	/* return early if the API we need isn't available */
#if !FLATPAK_CHECK_VERSION(1,11,1)
	if (gs_app_get_state (app) == GS_APP_STATE_INSTALLED)
		return TRUE;
#endif

	ref = g_strdup_printf ("%s/%s/%s/%s",
			      gs_flatpak_app_get_ref_kind_as_str (app),
			      gs_flatpak_app_get_ref_name (app),
			      gs_flatpak_app_get_ref_arch (app),
			      gs_app_get_branch (app));

	/* Find installed related refs in case the app is installed */
	installed_related_refs = flatpak_installation_list_installed_related_refs_sync (installation,
											gs_app_get_origin (app),
											ref,
											cancellable,
											&error_local);
	if (installed_related_refs == NULL &&
	    !g_error_matches (error_local,
			      FLATPAK_ERROR,
			      FLATPAK_ERROR_NOT_INSTALLED)) {
		gs_flatpak_error_convert (&error_local);
		g_propagate_error (error, g_steal_pointer (&error_local));
		return FALSE;
	}

	g_clear_error (&error_local);

#if FLATPAK_CHECK_VERSION(1,11,1)
	/* Find remote related refs that match the installed version in case the app is installed */
	remote_related_refs = flatpak_installation_list_remote_related_refs_for_installed_sync (installation,
												gs_app_get_origin (app),
												ref,
												cancellable,
												&error_local);
	if (remote_related_refs == NULL &&
	    !g_error_matches (error_local,
			      FLATPAK_ERROR,
			      FLATPAK_ERROR_NOT_INSTALLED)) {
		gs_flatpak_error_convert (&error_local);
		g_propagate_error (error, g_steal_pointer (&error_local));
		return FALSE;
	}

	g_clear_error (&error_local);
#endif

	/* Find remote related refs in case the app is not installed */
	if (remote_related_refs == NULL) {
		remote_related_refs = flatpak_installation_list_remote_related_refs_sync (installation,
											  gs_app_get_origin (app),
											  ref,
											  cancellable,
											  &error_local);
		/* don't make the error fatal in case we're offline */
		if (error_local != NULL)
			g_debug ("failed to list remote related refs of %s: %s",
				 gs_app_get_unique_id (app), error_local->message);
	}

	g_clear_error (&error_local);

	remove_addons = g_ptr_array_new_full (gs_app_list_length (addons_list), g_object_unref);
	/* For each addon, if it is neither installed nor available, hide it
	 * since it may be intended for a different version of the app. We
	 * don't want to show both org.videolan.VLC.Plugin.bdj//3-19.08 and
	 * org.videolan.VLC.Plugin.bdj//3-20.08 in the UI; only one will work
	 * for the installed app
	 */
	for (guint i = 0; i < gs_app_list_length (addons_list); i++) {
		GsApp *app_addon = gs_app_list_index (addons_list, i);
		gboolean found = FALSE;
		g_autofree char *addon_ref = NULL;

		addon_ref = g_strdup_printf ("%s/%s/%s/%s",
					     gs_flatpak_app_get_ref_kind_as_str (app_addon),
					     gs_flatpak_app_get_ref_name (app_addon),
					     gs_flatpak_app_get_ref_arch (app_addon),
					     gs_app_get_branch (app_addon));
		for (guint j = 0; !found && installed_related_refs && j < installed_related_refs->len; j++) {
			FlatpakRelatedRef *rel = g_ptr_array_index (installed_related_refs, j);
			g_autofree char *rel_ref = flatpak_ref_format_ref (FLATPAK_REF (rel));
			if (g_strcmp0 (addon_ref, rel_ref) == 0)
				found = TRUE;
		}
		for (guint j = 0; !found && remote_related_refs && j < remote_related_refs->len; j++) {
			FlatpakRelatedRef *rel = g_ptr_array_index (remote_related_refs, j);
			g_autofree char *rel_ref = flatpak_ref_format_ref (FLATPAK_REF (rel));
			if (g_strcmp0 (addon_ref, rel_ref) == 0)
				found = TRUE;
		}

		if (!found)
			g_ptr_array_add (remove_addons, g_object_ref (app_addon));
	}

	for (guint i = 0; i < remove_addons->len; i++) {
		GsApp *addon = g_ptr_array_index (remove_addons, i);
		g_debug ("removing addon '%s' from app '%s', because not related to it",
			 gs_app_get_unique_id (addon), gs_app_get_unique_id (app));
		gs_app_remove_addon (app, addon);
	}

	return TRUE;
}

static guint64
gs_flatpak_get_app_directory_size (GsApp *app,
				   const gchar *subdir_name,
				   GCancellable *cancellable)
{
	g_autofree gchar *filename = NULL;
	filename = g_build_filename (g_get_home_dir (), ".var", "app", gs_app_get_id (app), subdir_name, NULL);
	return gs_utils_get_file_size (filename, NULL, NULL, cancellable);
}

static gboolean
gs_plugin_refine_item_size (GsFlatpak *self,
			    GsApp *app,
			    gboolean interactive,
			    GCancellable *cancellable,
			    GError **error)
{
	gboolean ret;
	guint64 download_size = 0;
	guint64 installed_size = 0;
	GsSizeType size_type = GS_SIZE_TYPE_UNKNOWABLE;

	/* not applicable */
	if (gs_app_get_state (app) == GS_APP_STATE_AVAILABLE_LOCAL)
		return TRUE;
	if (gs_app_get_kind (app) == AS_COMPONENT_KIND_REPOSITORY)
		return TRUE;

	/* already set */
	if (gs_app_is_installed (app)) {
		/* only care about the installed size if the app is installed */
		if (gs_app_get_size_installed (app, NULL) == GS_SIZE_TYPE_VALID)
			return TRUE;
	} else {
		if (gs_app_get_size_installed (app, NULL) == GS_SIZE_TYPE_VALID &&
		    gs_app_get_size_download (app, NULL) == GS_SIZE_TYPE_VALID)
		return TRUE;
	}

	/* need runtime */
	if (!gs_plugin_refine_item_metadata (self, app, interactive, cancellable, error))
		return FALSE;

	/* calculate the platform size too if the app is not installed */
	if (gs_app_get_state (app) == GS_APP_STATE_AVAILABLE &&
	    gs_flatpak_app_get_ref_kind (app) == FLATPAK_REF_KIND_APP) {
		GsApp *app_runtime;

		/* is the app_runtime already installed? */
		app_runtime = gs_app_get_runtime (app);
		if (!gs_flatpak_refine_app_state_unlocked (self,
		                                           app_runtime,
		                                           interactive,
							   FALSE,
		                                           cancellable,
		                                           error))
			return FALSE;
		if (gs_app_get_state (app_runtime) == GS_APP_STATE_INSTALLED) {
			g_debug ("runtime %s is already installed, so not adding size",
				 gs_app_get_unique_id (app_runtime));
		} else {
			if (!gs_plugin_refine_item_size (self,
							 app_runtime,
							 interactive,
							 cancellable,
							 error))
				return FALSE;
		}
	}

	/* just get the size of the app */
	if (!gs_plugin_refine_item_origin (self, app, interactive,
					   cancellable, error))
		return FALSE;

	/* if the app is installed we use the ref to fetch the installed size
	 * and ignore the download size as this is faster */
	if (gs_app_is_installed (app)) {
		g_autoptr(FlatpakInstalledRef) xref = NULL;
		xref = gs_flatpak_get_installed_ref (self, app, interactive, cancellable, error);
		if (xref == NULL)
			return FALSE;
		installed_size = flatpak_installed_ref_get_installed_size (xref);
		size_type = (installed_size > 0) ? GS_SIZE_TYPE_VALID : GS_SIZE_TYPE_UNKNOWABLE;
	} else {
		g_autoptr(FlatpakRef) xref = NULL;
		g_autoptr(GError) error_local = NULL;

		/* no origin */
		if (gs_app_get_origin (app) == NULL) {
			g_set_error (error,
				     GS_PLUGIN_ERROR,
				     GS_PLUGIN_ERROR_NOT_SUPPORTED,
				     "no origin set for %s",
				     gs_app_get_unique_id (app));
			return FALSE;
		}
		xref = gs_flatpak_create_fake_ref (app, error);
		if (xref == NULL)
			return FALSE;
		ret = flatpak_installation_fetch_remote_size_sync (gs_flatpak_get_installation (self, interactive),
								   gs_app_get_origin (app),
								   xref,
								   &download_size,
								   &installed_size,
								   cancellable,
								   &error_local);

		if (!ret) {
			/* This can happen when the remote is filtered */
			g_debug ("libflatpak failed to return application size: %s", error_local->message);
			g_clear_error (&error_local);
		} else {
			size_type = GS_SIZE_TYPE_VALID;
		}
	}

	gs_app_set_size_installed (app, size_type, installed_size);
	gs_app_set_size_download (app, size_type, download_size);

	return TRUE;
}

static void
gs_flatpak_refine_appstream_release (XbNode *component, GsApp *app)
{
	const gchar *version;

	/* get first release */
	version = xb_node_query_attr (component, "releases/release", "version", NULL);
	if (version == NULL)
		return;
	switch (gs_app_get_state (app)) {
	case GS_APP_STATE_INSTALLED:
	case GS_APP_STATE_AVAILABLE:
	case GS_APP_STATE_AVAILABLE_LOCAL:
		gs_app_set_version (app, version);
		break;
	case GS_APP_STATE_UPDATABLE:
	case GS_APP_STATE_UPDATABLE_LIVE:
		gs_app_set_update_version (app, version);
		break;
	default:
		g_debug ("%s is not installed, so ignoring version of %s",
			 gs_app_get_unique_id (app), version);
		break;
	}
}

/* This function is like gs_flatpak_refine_appstream(), but takes gzip
 * compressed appstream data as a GBytes and assumes they are already uniquely
 * tied to the app (and therefore app ID alone can be used to find the right
 * component).
 */
static gboolean
gs_flatpak_refine_appstream_from_bytes (GsFlatpak *self,
					GsApp *app,
					const char *origin, /* (nullable) */
					FlatpakInstalledRef *installed_ref, /* (nullable) */
					GBytes *appstream_gz,
					GsPluginRefineFlags flags,
					gboolean interactive,
					GCancellable *cancellable,
					GError **error)
{
	g_autofree gchar *xpath = NULL;
	g_autoptr(XbBuilder) builder = NULL;
	g_autoptr(XbBuilderSource) source = xb_builder_source_new ();
	g_autoptr(XbNode) component_node = NULL;
	g_autoptr(XbNode) n = NULL;
	g_autoptr(XbSilo) silo = NULL;
	g_autoptr(XbBuilderFixup) bundle_fixup = NULL;
	g_autoptr(GBytes) appstream = NULL;
	g_autoptr(GInputStream) stream_data = NULL;
	g_autoptr(GInputStream) stream_gz = NULL;
	g_autoptr(GZlibDecompressor) decompressor = NULL;
	g_autoptr(GMainContext) old_thread_default = NULL;

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

	gs_appstream_add_current_locales (builder);

	/* decompress data */
	decompressor = g_zlib_decompressor_new (G_ZLIB_COMPRESSOR_FORMAT_GZIP);
	stream_gz = g_memory_input_stream_new_from_bytes (appstream_gz);
	if (stream_gz == NULL) {
		g_set_error (error,
			     GS_PLUGIN_ERROR,
			     GS_PLUGIN_ERROR_INVALID_FORMAT,
			     "unable to decompress appstream data");
		return FALSE;
	}
	stream_data = g_converter_input_stream_new (stream_gz,
						    G_CONVERTER (decompressor));

	appstream = g_input_stream_read_bytes (stream_data,
					       0x100000, /* 1Mb */
					       cancellable,
					       error);
	if (appstream == NULL) {
		gs_flatpak_error_convert (error);
		return FALSE;
	}

	/* build silo */
	if (!xb_builder_source_load_bytes (source, appstream,
					   XB_BUILDER_SOURCE_FLAG_NONE,
					   error))
		return FALSE;

	/* Appdata from flatpak_installed_ref_load_appdata() may be missing the
	 * <bundle> tag but for this function we know it's the right component.
	 */
	bundle_fixup = xb_builder_fixup_new ("AddBundle",
				       gs_flatpak_add_bundle_tag_cb,
				       gs_flatpak_app_get_ref_display (app), g_free);
	xb_builder_fixup_set_max_depth (bundle_fixup, 2);
	xb_builder_source_add_fixup (source, bundle_fixup);

	fixup_flatpak_appstream_xml (source, origin);

	/* add metadata */
	if (installed_ref != NULL) {
		g_autoptr(XbBuilderNode) info = NULL;
		g_autofree char *icon_prefix = NULL;

		info = xb_builder_node_insert (NULL, "info", NULL);
		xb_builder_node_insert_text (info, "scope", as_component_scope_to_string (self->scope), NULL);
		icon_prefix = g_build_filename (flatpak_installed_ref_get_deploy_dir (installed_ref),
						"files", "share", "app-info", "icons", "flatpak", NULL);
		xb_builder_node_insert_text (info, "icon-prefix", icon_prefix, NULL);
		xb_builder_source_set_info (source, info);
	}

	xb_builder_import_source (builder, source);

	/* FIXME: https://github.com/LingmoOS/lingmo-appstore/-/issues/1422 */
	old_thread_default = g_main_context_ref_thread_default ();
	if (old_thread_default == g_main_context_default ())
		g_clear_pointer (&old_thread_default, g_main_context_unref);
	if (old_thread_default != NULL)
		g_main_context_pop_thread_default (old_thread_default);

	silo = xb_builder_compile (builder,
				   XB_BUILDER_COMPILE_FLAG_SINGLE_LANG,
				   cancellable,
				   error);

	if (old_thread_default != NULL)
		g_main_context_push_thread_default (old_thread_default);

	if (silo == NULL)
		return FALSE;
	if (g_getenv ("GS_XMLB_VERBOSE") != NULL) {
		g_autofree gchar *xml = NULL;
		xml = xb_silo_export (silo,
				      XB_NODE_EXPORT_FLAG_FORMAT_INDENT |
				      XB_NODE_EXPORT_FLAG_FORMAT_MULTILINE,
				      NULL);
		g_debug ("showing AppStream data: %s", xml);
	}

	/* check for sanity */
	n = xb_silo_query_first (silo, "components/component", NULL);
	if (n == NULL) {
		g_set_error_literal (error,
				     GS_PLUGIN_ERROR,
				     GS_PLUGIN_ERROR_NOT_SUPPORTED,
				     "no apps found in AppStream data");
		return FALSE;
	}

	/* find app */
	xpath = g_strdup_printf ("components/component/id[text()='%s']/..",
				 gs_flatpak_app_get_ref_name (app));
	component_node = xb_silo_query_first (silo, xpath, NULL);
	if (component_node == NULL) {
		g_set_error (error,
			     GS_PLUGIN_ERROR,
			     GS_PLUGIN_ERROR_INVALID_FORMAT,
			     "application %s not found",
			     gs_flatpak_app_get_ref_name (app));
		return FALSE;
	}

	/* copy details from AppStream to app */
	if (!gs_appstream_refine_app (self->plugin, app, silo, component_node, flags, self->silo_installed_by_desktopid,
				      self->silo_filename ? self->silo_filename : "", self->scope, error))
		return FALSE;

	if (gs_app_get_origin (app))
		gs_flatpak_set_app_origin (self, app, gs_app_get_origin (app), NULL, interactive, cancellable);

	/* use the default release as the version number */
	gs_flatpak_refine_appstream_release (component_node, app);

	/* save the silo so it can be used for searches */
	{
		g_autoptr(GMutexLocker) locker = g_mutex_locker_new (&self->app_silos_mutex);
		g_hash_table_replace (self->app_silos,
				      gs_flatpak_app_get_ref_display (app),
				      g_steal_pointer (&silo));
	}

	return TRUE;
}

static XbNode *
get_renamed_component (GsFlatpak *self,
		       GsApp *app,
		       XbSilo *silo,
		       gboolean interactive,
		       GCancellable *cancellable,
		       GError **error)
{
	const gchar *origin = gs_app_get_origin (app);
	const gchar *renamed_to;
#if LIBXMLB_CHECK_VERSION(0, 3, 0)
	g_autoptr(XbQuery) query = NULL;
	g_auto(XbQueryContext) context = XB_QUERY_CONTEXT_INIT ();
#else
	g_autofree gchar *xpath = NULL;
	g_autofree gchar *source_safe = NULL;
#endif
	g_autoptr(FlatpakRemoteRef) remote_ref = NULL;
	g_autoptr(XbNode) component = NULL;
	FlatpakInstallation *installation = gs_flatpak_get_installation (self, interactive);

	remote_ref = flatpak_installation_fetch_remote_ref_sync (installation,
								 origin,
								 gs_flatpak_app_get_ref_kind (app),
								 gs_flatpak_app_get_ref_name (app),
								 gs_flatpak_app_get_ref_arch (app),
								 gs_app_get_branch (app),
								 cancellable, error);
	if (remote_ref == NULL)
		return NULL;

	renamed_to = flatpak_remote_ref_get_eol_rebase (remote_ref);
	if (renamed_to == NULL)
		return NULL;

#if LIBXMLB_CHECK_VERSION(0, 3, 0)
	query = xb_silo_lookup_query (silo, "components[@origin=?]/component/bundle[@type='flatpak'][text()=?]/..");
	xb_value_bindings_bind_str (xb_query_context_get_bindings (&context), 0, origin, NULL);
	xb_value_bindings_bind_str (xb_query_context_get_bindings (&context), 1, renamed_to, NULL);
	component = xb_silo_query_first_with_context (silo, query, &context, NULL);
#else
	source_safe = xb_string_escape (renamed_to);
	xpath = g_strdup_printf ("components[@origin='%s']/component/bundle[@type='flatpak'][text()='%s']/..",
				 origin, source_safe);
	component = xb_silo_query_first (silo, xpath, NULL);
#endif

	/* Get the previous name so it can be displayed in the UI */
	if (component != NULL) {
		g_autoptr(FlatpakInstalledRef) installed_ref = NULL;
		const gchar *installed_name = NULL;

		installed_ref = flatpak_installation_get_installed_ref (installation,
									gs_flatpak_app_get_ref_kind (app),
									gs_flatpak_app_get_ref_name (app),
									gs_flatpak_app_get_ref_arch (app),
									gs_app_get_branch (app),
									cancellable, error);
		if (installed_ref != NULL)
			installed_name = flatpak_installed_ref_get_appdata_name (installed_ref);
		if (installed_name != NULL)
			gs_app_set_renamed_from (app, installed_name);
	}

	return g_steal_pointer (&component);
}

/* Returns %TRUE if @error exists and is set to G_IO_ERROR_CANCELLED */
static inline gboolean
propagate_cancelled_error (GError **dest,
                           GError **error)
{
	g_assert (error != NULL);

	if (*error && g_error_matches (*error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
		g_propagate_error (dest, g_steal_pointer (error));
		return TRUE;
	}

	return FALSE;
}

static gboolean
gs_flatpak_refine_appstream (GsFlatpak *self,
			     GsApp *app,
			     XbSilo *silo,
			     GsPluginRefineFlags flags,
			     GHashTable *components_by_bundle,
			     gboolean interactive,
			     GCancellable *cancellable,
			     GError **error)
{
	const gchar *origin = gs_app_get_origin (app);
	const gchar *source = gs_app_get_source_default (app);
	g_autoptr(GError) error_local = NULL;
	g_autoptr(XbNode) component = NULL;

	if (origin == NULL || source == NULL)
		return TRUE;

	/* find using source and origin */
	if (components_by_bundle != NULL) {
		g_autofree gchar *key = g_strconcat (origin, "\n", source, NULL);
		component = g_hash_table_lookup (components_by_bundle, key);
		if (component != NULL)
			g_object_ref (component);
	} else {
		g_autofree gchar *source_safe = NULL;
		g_autofree gchar *xpath = NULL;

		source_safe = xb_string_escape (source);
		xpath = g_strdup_printf ("components[@origin='%s']/component/bundle[@type='flatpak'][text()='%s']/..",
					 origin, source_safe);
		component = xb_silo_query_first (silo, xpath, &error_local);
		if (propagate_cancelled_error (error, &error_local))
			return FALSE;

		g_clear_error (&error_local);
	}

	/* Ensure the gs_flatpak_app_get_ref_*() metadata are set */
	gs_refine_item_metadata (self, app, NULL);

	/* If the app was renamed, use the appstream data from the new name;
	 * usually it will not exist under the old name */
	if (component == NULL && gs_flatpak_app_get_ref_kind (app) == FLATPAK_REF_KIND_APP) {
		g_autoptr(GError) renamed_component_error = NULL;

		component = get_renamed_component (self, app, silo,
						   interactive,
						   cancellable,
						   &renamed_component_error);

		if (propagate_cancelled_error (error, &renamed_component_error))
			return FALSE;

		g_clear_error (&error_local);
	}

	if (component == NULL) {
		g_autoptr(FlatpakInstalledRef) installed_ref = NULL;
		g_autoptr(GBytes) appstream_gz = NULL;

		/* For apps installed from .flatpak bundles there may not be any remote
		 * appstream data in @silo for it, so use the appstream data from
		 * within the app.
		 */
		installed_ref = flatpak_installation_get_installed_ref (gs_flatpak_get_installation (self, interactive),
									gs_flatpak_app_get_ref_kind (app),
									gs_flatpak_app_get_ref_name (app),
									gs_flatpak_app_get_ref_arch (app),
									gs_app_get_branch (app),
									cancellable,
									&error_local);

		if (installed_ref == NULL)
			return !propagate_cancelled_error (error, &error_local); /* the app may not be installed */

		appstream_gz = flatpak_installed_ref_load_appdata (installed_ref,
								   cancellable,
								   &error_local);
		if (appstream_gz == NULL)
			return !propagate_cancelled_error (error, &error_local);

		g_debug ("using installed appdata for %s", gs_flatpak_app_get_ref_name (app));
		return gs_flatpak_refine_appstream_from_bytes (self,
				                               app,
							       flatpak_installed_ref_get_origin (installed_ref),
							       installed_ref,
							       appstream_gz,
							       flags,
							       interactive,
							       cancellable, error);
	}

	if (!gs_appstream_refine_app (self->plugin, app, silo, component, flags, self->silo_installed_by_desktopid,
				      self->silo_filename ? self->silo_filename : "", self->scope, error))
		return FALSE;

	/* use the default release as the version number */
	gs_flatpak_refine_appstream_release (component, app);
	return TRUE;
}

static gboolean
gs_flatpak_refine_app_unlocked (GsFlatpak *self,
                                GsApp *app,
                                GsPluginRefineFlags flags,
                                gboolean interactive,
				gboolean force_state_update,
				GHashTable *components_by_bundle,
                                GRWLockReaderLocker **locker,
                                GCancellable *cancellable,
                                GError **error)
{
	GsAppState old_state = gs_app_get_state (app);
	g_autoptr(GError) local_error = NULL;

	/* not us */
	if (gs_app_get_bundle_kind (app) != AS_BUNDLE_KIND_FLATPAK)
		return TRUE;

	g_clear_pointer (locker, g_rw_lock_reader_locker_free);

	if (!ensure_flatpak_silo_with_locker (self, locker, interactive, cancellable, error))
		return FALSE;

	/* always do AppStream properties */
	if (!gs_flatpak_refine_appstream (self, app, self->silo, flags, components_by_bundle, interactive, cancellable, error))
		return FALSE;

	/* AppStream sets the source to appname/arch/branch */
	if (!gs_refine_item_metadata (self, app, error)) {
		g_prefix_error (error, "failed to get metadata: ");
		return FALSE;
	}

	/* check the installed state */
	if (!gs_flatpak_refine_app_state_unlocked (self, app, interactive, force_state_update, cancellable, error)) {
		g_prefix_error (error, "failed to get state: ");
		return FALSE;
	}

	/* hide any addons that aren't for this app */
	if (!gs_flatpak_prune_addons_list (self, app, interactive, cancellable, &local_error)) {
		g_warning ("failed to prune addons: %s", local_error->message);
		g_clear_error (&local_error);
	}

	/* scope is fast, do unconditionally */
	if (gs_app_get_state (app) != GS_APP_STATE_AVAILABLE_LOCAL)
		gs_plugin_refine_item_scope (self, app);

	/* if the state was changed, perhaps set the version from the release */
	if (old_state != gs_app_get_state (app)) {
		if (!gs_flatpak_refine_appstream (self, app, self->silo, flags, components_by_bundle, interactive, cancellable, error))
			return FALSE;
	}

	/* version fallback */
	if (flags & GS_PLUGIN_REFINE_FLAGS_REQUIRE_VERSION) {
		if (gs_app_get_version (app) == NULL) {
			const gchar *branch;
			branch = gs_app_get_branch (app);
			gs_app_set_version (app, branch);
		}
	}

	/* size */
	if (flags & GS_PLUGIN_REFINE_FLAGS_REQUIRE_SIZE) {
		g_autoptr(GError) error_local = NULL;
		if (!gs_plugin_refine_item_size (self, app, interactive,
						 cancellable, &error_local)) {
			if (g_error_matches (error_local, GS_PLUGIN_ERROR,
					     GS_PLUGIN_ERROR_NO_NETWORK)) {
				g_debug ("failed to get size while "
					 "refining app %s: %s",
					 gs_app_get_unique_id (app),
					 error_local->message);
			} else {
				g_prefix_error (&error_local, "failed to get size: ");
				g_propagate_error (error, g_steal_pointer (&error_local));
				return FALSE;
			}
		}
	}

	if ((flags & GS_PLUGIN_REFINE_FLAGS_REQUIRE_SIZE_DATA) != 0 &&
	    gs_app_is_installed (app) &&
	    gs_app_get_kind (app) != AS_COMPONENT_KIND_RUNTIME) {
		if (gs_app_get_size_cache_data (app, NULL) != GS_SIZE_TYPE_VALID)
			gs_app_set_size_cache_data (app, GS_SIZE_TYPE_VALID,
						    gs_flatpak_get_app_directory_size (app, "cache", cancellable));
		if (gs_app_get_size_user_data (app, NULL) != GS_SIZE_TYPE_VALID)
			gs_app_set_size_user_data (app, GS_SIZE_TYPE_VALID,
						   gs_flatpak_get_app_directory_size (app, "config", cancellable) +
						   gs_flatpak_get_app_directory_size (app, "data", cancellable));

		if (g_cancellable_is_cancelled (cancellable)) {
			gs_app_set_size_cache_data (app, GS_SIZE_TYPE_UNKNOWABLE, 0);
			gs_app_set_size_user_data (app, GS_SIZE_TYPE_UNKNOWABLE, 0);
		}
	}

	/* origin-hostname */
	if (flags & GS_PLUGIN_REFINE_FLAGS_REQUIRE_ORIGIN_HOSTNAME) {
		if (!gs_plugin_refine_item_origin_hostname (self, app, interactive,
							    cancellable,
							    error)) {
			g_prefix_error (error, "failed to get origin-hostname: ");
			return FALSE;
		}
	}

	/* permissions */
	if (flags & GS_PLUGIN_REFINE_FLAGS_REQUIRE_RUNTIME ||
	    flags & GS_PLUGIN_REFINE_FLAGS_REQUIRE_PERMISSIONS) {
		g_autoptr(GError) error_local = NULL;
		if (!gs_plugin_refine_item_metadata (self, app, interactive,
						     cancellable, &error_local)) {
			if (!gs_plugin_get_network_available (self->plugin) &&
			    g_error_matches (error_local, GS_PLUGIN_ERROR,
					     GS_PLUGIN_ERROR_NO_NETWORK)) {
				g_debug ("failed to get permissions while "
					 "refining app %s: %s",
					 gs_app_get_unique_id (app),
					 error_local->message);
			} else {
				g_prefix_error (&error_local, "failed to read permissions from app '%s' metadata: ", gs_app_get_unique_id (app));
				g_propagate_error (error, g_steal_pointer (&error_local));
				return FALSE;
			}
		}
	}

	if (gs_app_get_origin (app))
		gs_flatpak_set_app_origin (self, app, gs_app_get_origin (app), NULL, interactive, cancellable);

	return TRUE;
}

void
gs_flatpak_refine_addons (GsFlatpak *self,
			  GsApp *parent_app,
			  GsPluginRefineFlags flags,
			  GsAppState state,
			  gboolean interactive,
			  GCancellable *cancellable)
{
	g_autoptr(GRWLockReaderLocker) locker = NULL;
	g_autoptr(GsAppList) addons = NULL;
	g_autoptr(GString) errors = NULL;
	guint ii, sz;

	addons = gs_app_dup_addons (parent_app);
	sz = addons ? gs_app_list_length (addons) : 0;

	for (ii = 0; ii < sz; ii++) {
		GsApp *addon = gs_app_list_index (addons, ii);
		g_autoptr(GError) local_error = NULL;

		if (state != gs_app_get_state (addon))
			continue;

		if (!gs_flatpak_refine_app_unlocked (self, addon, flags, interactive, TRUE, NULL, &locker, cancellable, &local_error)) {
			if (errors)
				g_string_append_c (errors, '\n');
			else
				errors = g_string_new (NULL);
			g_string_append_printf (errors, _("Failed to refine addon ‘%s’: %s"),
				gs_app_get_name (addon), local_error->message);
		}
	}

	if (errors) {
		g_autoptr(GsPluginEvent) event = NULL;
		g_autoptr(GError) error_local = g_error_new_literal (GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_FAILED,
			errors->str);

		event = gs_plugin_event_new ("error", error_local,
					     NULL);
		gs_plugin_event_add_flag (event, GS_PLUGIN_EVENT_FLAG_WARNING);
		gs_plugin_report_event (self->plugin, event);
	}
}

gboolean
gs_flatpak_refine_app (GsFlatpak *self,
		       GsApp *app,
		       GsPluginRefineFlags flags,
		       gboolean interactive,
		       gboolean force_state_update,
		       GCancellable *cancellable,
		       GError **error)
{
	g_autoptr(GRWLockReaderLocker) locker = NULL;

	/* ensure valid */
	if (!gs_flatpak_rescan_app_data (self, interactive, cancellable, error))
		return FALSE;

	return gs_flatpak_refine_app_unlocked (self, app, flags, interactive, force_state_update, NULL, &locker, cancellable, error);
}

gboolean
gs_flatpak_refine_wildcard (GsFlatpak *self, GsApp *app,
			    GsAppList *list, GsPluginRefineFlags refine_flags,
			    gboolean interactive,
			    GHashTable **inout_components_by_id,
			    GHashTable **inout_components_by_bundle,
			    GCancellable *cancellable, GError **error)
{
	const gchar *id;
	GPtrArray* components = NULL;
	g_autoptr(GError) error_local = NULL;
	g_autoptr(GRWLockReaderLocker) locker = NULL;

	GS_PROFILER_BEGIN_SCOPED (FlatpakRefineWildcard, "Flatpak (refine wildcard)", NULL);

	/* not enough info to find */
	id = gs_app_get_id (app);
	if (id == NULL)
		return TRUE;

	if (!ensure_flatpak_silo_with_locker (self, &locker, interactive, cancellable, error))
		return FALSE;

	GS_PROFILER_BEGIN_SCOPED (FlatpakRefineWildcardQuerySilo, "Flatpak (query silo)", NULL);

	if (*inout_components_by_id != NULL) {
		components = g_hash_table_lookup (*inout_components_by_id, gs_app_get_id (app));
	} else {
		g_autoptr(GPtrArray) components_with_id = NULL;
		*inout_components_by_id = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) g_ptr_array_unref);
		components_with_id = xb_silo_query (self->silo, "components/component/id", 0, &error_local);
		if (components_with_id == NULL) {
			if (g_error_matches (error_local, G_IO_ERROR, G_IO_ERROR_NOT_FOUND))
				return TRUE;
			g_propagate_error (error, g_steal_pointer (&error_local));
			return FALSE;
		}
		for (guint i = 0; i < components_with_id->len; i++) {
			XbNode *node = g_ptr_array_index (components_with_id, i);
			XbNode *comp_node = xb_node_get_parent (node);
			const gchar *comp_id = xb_node_get_text (node);
			GPtrArray *comps = g_hash_table_lookup (*inout_components_by_id, comp_id);
			if (comps == NULL) {
				comps = g_ptr_array_new_with_free_func (g_object_unref);
				g_hash_table_insert (*inout_components_by_id, g_strdup (comp_id), comps);
			}
			g_ptr_array_add (comps, comp_node);
			if (components == NULL && g_strcmp0 (id, comp_id) == 0)
				components = comps;
		}
	}

	GS_PROFILER_END_SCOPED (FlatpakRefineWildcardQuerySilo);

	if (components == NULL)
		return TRUE;

	gs_flatpak_ensure_remote_title (self, interactive, cancellable);

	if (*inout_components_by_bundle == NULL) {
		g_autoptr(GPtrArray) bundles = NULL;

		*inout_components_by_bundle = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
		bundles = xb_silo_query (self->silo, "/components/component/bundle[@type='flatpak']", 0, NULL);
		for (guint b = 0; bundles != NULL && b < bundles->len; b++) {
			XbNode *bundle_node = g_ptr_array_index (bundles, b);
			g_autoptr(XbNode) component_node = xb_node_get_parent (bundle_node);
			g_autoptr(XbNode) components_node = xb_node_get_parent (component_node);
			const gchar *origin = xb_node_get_attr (components_node, "origin");
			if (origin != NULL) {
				const gchar *bundle = xb_node_get_text (bundle_node);
				if (bundle != NULL) {
					g_autofree gchar *key = g_strconcat (origin, "\n", bundle, NULL);
					g_hash_table_insert (*inout_components_by_bundle, g_steal_pointer (&key), g_steal_pointer (&component_node));
				}
			}
		}
	}


	GS_PROFILER_BEGIN_SCOPED (FlatpakRefineWildcardGenerateApps, "Flatpak (create app)", NULL);
	for (guint i = 0; i < components->len; i++) {
		XbNode *component = g_ptr_array_index (components, i);
		g_autoptr(GsApp) new = NULL;

		GS_PROFILER_BEGIN_SCOPED (FlatpakRefineWildcardCreateAppstreamApp, "Flatpak (create Appstream app)", NULL);
		new = gs_appstream_create_app (self->plugin, self->silo, component, self->silo_filename ? self->silo_filename : "",
					       self->scope, error);
		GS_PROFILER_END_SCOPED (FlatpakRefineWildcardCreateAppstreamApp);

		if (new == NULL)
			return FALSE;

		gs_flatpak_claim_app (self, new);

		/* The appstream plugin did not find the component in the plugin's cache,
		   thus read the required info from the 'bundle' element. */
		if (gs_flatpak_app_get_ref_name (new) == NULL ||
		    gs_flatpak_app_get_ref_arch (new) == NULL) {
			const gchar *xref_str = NULL;
			g_autoptr(XbNode) child = NULL;
			g_autoptr(XbNode) next = NULL;
			for (child = xb_node_get_child (component); child != NULL && xref_str == NULL;
			     g_object_unref (child), child = g_steal_pointer (&next)) {
				next = xb_node_get_next (child);
				if (g_strcmp0 (xb_node_get_element (child), "bundle") == 0 &&
				    g_strcmp0 (xb_node_get_attr (child, "type"), "flatpak") == 0) {
					xref_str = xb_node_get_text (child);
					break;
				}
			}
			if (xref_str != NULL) {
				g_auto(GStrv) split = NULL;

				/* get the kind/name/arch/branch */
				split = g_strsplit (xref_str, "/", -1);
				if (g_strv_length (split) == 4) {
					const gchar *comp_type = xb_node_get_attr (component, "type");
					AsComponentKind kind = as_component_kind_from_string (comp_type);
					if (kind != AS_COMPONENT_KIND_UNKNOWN)
						gs_app_set_kind (new, kind);
					else if (g_ascii_strcasecmp (split[0], "app") == 0)
						gs_app_set_kind (new, AS_COMPONENT_KIND_DESKTOP_APP);
					else if (g_ascii_strcasecmp (split[0], "runtime") == 0)
						gs_flatpak_set_runtime_kind_from_id (new);
					gs_flatpak_app_set_ref_name (new, split[1]);
					gs_flatpak_app_set_ref_arch (new, split[2]);
					gs_app_set_branch (new, split[3]);
					gs_app_set_metadata (new, "LingmoAppstore::packagename-value", xref_str);
				}
			}
		}

		if (gs_flatpak_app_get_ref_name (new) == NULL ||
		    gs_flatpak_app_get_ref_arch (new) == NULL) {
			g_debug ("Failed to get ref info for '%s' from wildcard '%s', skipping it...", gs_app_get_id (new), id);
		} else {
			GS_PROFILER_BEGIN_SCOPED (FlatpakRefineWildcardRefineNewApp, "Flatpak (refine new app)", NULL);
			if (!gs_flatpak_refine_app_unlocked (self, new, refine_flags, interactive, FALSE, *inout_components_by_bundle, &locker, cancellable, error))
				return FALSE;
			GS_PROFILER_END_SCOPED (FlatpakRefineWildcardRefineNewApp);

			GS_PROFILER_BEGIN_SCOPED (FlatpakRefineWildcardSubsumeMetadata, "Flatpak (subsume metadata)", NULL);
			gs_app_subsume_metadata (new, app);
			GS_PROFILER_END_SCOPED (FlatpakRefineWildcardSubsumeMetadata);

			gs_app_list_add (list, new);
		}
	}
	GS_PROFILER_END_SCOPED (FlatpakRefineWildcardGenerateApps);

	GS_PROFILER_END_SCOPED (FlatpakRefineWildcard);

	/* success */
	return TRUE;
}

gboolean
gs_flatpak_launch (GsFlatpak *self,
		   GsApp *app,
		   gboolean interactive,
		   GCancellable *cancellable,
		   GError **error)
{
	/* launch the app */
	if (!flatpak_installation_launch (gs_flatpak_get_installation (self, interactive),
					  gs_flatpak_app_get_ref_name (app),
					  gs_flatpak_app_get_ref_arch (app),
					  gs_app_get_branch (app),
					  NULL,
					  cancellable,
					  error)) {
		gs_flatpak_error_convert (error);
		return FALSE;
	}
	return TRUE;
}

gboolean
gs_flatpak_app_remove_source (GsFlatpak *self,
			      GsApp *app,
			      gboolean is_remove,
			      gboolean interactive,
			      GCancellable *cancellable,
			      GError **error)
{
	g_autoptr(FlatpakRemote) xremote = NULL;
	gboolean success;
	FlatpakInstallation *installation = gs_flatpak_get_installation (self, interactive);

	/* find the remote */
	xremote = gs_flatpak_remote_by_name (self, gs_app_get_id (app), interactive, cancellable, error);
	if (xremote == NULL) {
		gs_flatpak_error_convert (error);
		g_prefix_error (error,
				"flatpak source %s not found: ",
				gs_app_get_id (app));
		return FALSE;
	}

	/* remove */
	gs_app_set_state (app, GS_APP_STATE_REMOVING);
	if (is_remove) {
		success = flatpak_installation_remove_remote (installation, gs_app_get_id (app), cancellable, error);
	} else {
		gboolean was_disabled = flatpak_remote_get_disabled (xremote);
		flatpak_remote_set_disabled (xremote, TRUE);
		success = flatpak_installation_modify_remote (installation, xremote, cancellable, error);
		if (!success)
			flatpak_remote_set_disabled (xremote, was_disabled);
	}

	if (!success) {
		gs_flatpak_error_convert (error);
		gs_app_set_state_recover (app);
		return FALSE;
	}

	/* invalidate cache */
	gs_flatpak_invalidate_silo (self);

	gs_app_set_state (app, is_remove ? GS_APP_STATE_UNAVAILABLE : GS_APP_STATE_AVAILABLE);

	gs_plugin_repository_changed (self->plugin, app);

	return TRUE;
}

GsApp *
gs_flatpak_file_to_app_bundle (GsFlatpak *self,
			       GFile *file,
			       gboolean unrefined,
			       gboolean interactive,
			       GCancellable *cancellable,
			       GError **error)
{
	g_autoptr(GBytes) appstream_gz = NULL;
	g_autoptr(GBytes) icon_data64 = NULL, icon_data128 = NULL;
	g_autoptr(GBytes) metadata = NULL;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(FlatpakBundleRef) xref_bundle = NULL;

	/* load bundle */
	xref_bundle = flatpak_bundle_ref_new (file, error);
	if (xref_bundle == NULL) {
		gs_flatpak_error_convert (error);
		g_prefix_error (error, "error loading bundle: ");
		return NULL;
	}

	/* load metadata */
	app = gs_flatpak_create_app (self, NULL, FLATPAK_REF (xref_bundle), NULL, interactive, cancellable);
	if (unrefined)
		return g_steal_pointer (&app);

	gs_flatpak_app_set_file_kind (app, GS_FLATPAK_APP_FILE_KIND_BUNDLE);
	gs_app_set_state (app, GS_APP_STATE_AVAILABLE_LOCAL);
	gs_app_set_size_installed (app, GS_SIZE_TYPE_VALID, flatpak_bundle_ref_get_installed_size (xref_bundle));
	gs_flatpak_set_metadata (self, app, FLATPAK_REF (xref_bundle));
	metadata = flatpak_bundle_ref_get_metadata (xref_bundle);
	if (!gs_flatpak_set_app_metadata (self, app,
					  g_bytes_get_data (metadata, NULL),
					  g_bytes_get_size (metadata),
					  interactive,
					  cancellable,
					  error))
		return NULL;

	/* load AppStream */
	appstream_gz = flatpak_bundle_ref_get_appstream (xref_bundle);
	if (appstream_gz != NULL) {
		if (!gs_flatpak_refine_appstream_from_bytes (self, app, NULL, NULL,
							     appstream_gz,
							     GS_PLUGIN_REFINE_FLAGS_REQUIRE_ID,
							     interactive,
							     cancellable, error))
			return NULL;
	} else {
		g_warning ("no appstream metadata in file");
		gs_app_set_name (app, GS_APP_QUALITY_LOWEST,
				 gs_flatpak_app_get_ref_name (app));
		gs_app_set_summary (app, GS_APP_QUALITY_LOWEST,
				    "A flatpak application");
		gs_app_set_description (app, GS_APP_QUALITY_LOWEST, "");
	}

	/* Load icons. Currently flatpak only supports exactly 64px or 128px
	 * icons in bundles. */
	icon_data64 = flatpak_bundle_ref_get_icon (xref_bundle, 64);
	if (icon_data64 != NULL) {
		g_autoptr(GIcon) icon = g_bytes_icon_new (icon_data64);
		gs_icon_set_width (icon, 64);
		gs_icon_set_height (icon, 64);
		gs_app_add_icon (app, icon);
	}

	icon_data128 = flatpak_bundle_ref_get_icon (xref_bundle, 128);
	if (icon_data128 != NULL) {
		g_autoptr(GIcon) icon = g_bytes_icon_new (icon_data128);
		gs_icon_set_width (icon, 128);
		gs_icon_set_height (icon, 128);
		gs_app_add_icon (app, icon);
	}

	/* Fallback */
	if (icon_data64 == NULL && icon_data128 == NULL) {
		g_autoptr(GIcon) icon = g_themed_icon_new ("system-component-application");
		gs_app_add_icon (app, icon);
	}

	/* not quite true: this just means we can update this specific app */
	if (flatpak_bundle_ref_get_origin (xref_bundle))
		gs_app_add_quirk (app, GS_APP_QUIRK_HAS_SOURCE);

	/* success */
	return g_steal_pointer (&app);
}

static gboolean
_txn_abort_on_ready (FlatpakTransaction *transaction)
{
	return FALSE;
}

static gboolean
_txn_add_new_remote (FlatpakTransaction *transaction,
		     FlatpakTransactionRemoteReason reason,
		     const char *from_id,
		     const char *remote_name,
		     const char *url)
{
	return TRUE;
}

static int
_txn_choose_remote_for_ref (FlatpakTransaction *transaction,
			    const char *for_ref,
			    const char *runtime_ref,
			    const char * const *remotes)
{
	/* This transaction is just for displaying the app not installing it so
	 * this choice shouldn't matter */
	return 0;
}

GsApp *
gs_flatpak_file_to_app_ref (GsFlatpak *self,
			    GFile *file,
			    gboolean unrefined,
			    gboolean interactive,
			    GCancellable *cancellable,
			    GError **error)
{
	GsApp *runtime;
	const gchar *remote_name;
	gboolean is_runtime, success;
	gsize len = 0;
	GList *txn_ops;
#if !FLATPAK_CHECK_VERSION(1,13,1)
	guint64 app_installed_size = 0, app_download_size = 0;
#endif
	g_autofree gchar *contents = NULL;
	g_autoptr(FlatpakTransaction) transaction = NULL;
	g_autoptr(FlatpakRef) parsed_ref = NULL;
	g_autoptr(FlatpakRemoteRef) remote_ref = NULL;
	g_autoptr(FlatpakRemote) xremote = NULL;
	g_autoptr(GBytes) ref_file_data = NULL;
	g_autoptr(GError) error_local = NULL;
	g_autoptr(GKeyFile) kf = NULL;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(XbBuilder) builder = xb_builder_new ();
	g_autoptr(XbSilo) silo = NULL;
	g_autofree gchar *origin_url = NULL;
	g_autofree gchar *ref_comment = NULL;
	g_autofree gchar *ref_description = NULL;
	g_autofree gchar *ref_homepage = NULL;
	g_autofree gchar *ref_icon = NULL;
	g_autofree gchar *ref_title = NULL;
	g_autofree gchar *ref_name = NULL;
	g_autofree gchar *ref_branch = NULL;
	FlatpakInstallation *installation = gs_flatpak_get_installation (self, interactive);

	gs_appstream_add_current_locales (builder);

	/* get file data */
	if (!g_file_load_contents (file,
				   cancellable,
				   &contents,
				   &len,
				   NULL,
				   error)) {
		gs_utils_error_convert_gio (error);
		return NULL;
	}

	/* load the file */
	kf = g_key_file_new ();
	if (!g_key_file_load_from_data (kf, contents, len, G_KEY_FILE_NONE, error)) {
		gs_utils_error_convert_gio (error);
		return NULL;
	}

	/* check version */
	if (g_key_file_has_key (kf, "Flatpak Ref", "Version", NULL)) {
		guint64 ver = g_key_file_get_uint64 (kf, "Flatpak Ref", "Version", NULL);
		if (ver != 1) {
			g_set_error (error,
				     GS_PLUGIN_ERROR,
				     GS_PLUGIN_ERROR_NOT_SUPPORTED,
				     "unsupported version %" G_GUINT64_FORMAT, ver);
			return NULL;
		}
	}

	/* get name, branch, kind */
	ref_name = g_key_file_get_string (kf, "Flatpak Ref", "Name", error);
	if (ref_name == NULL) {
		gs_utils_error_convert_gio (error);
		return NULL;
	}
	if (g_key_file_has_key (kf, "Flatpak Ref", "Branch", NULL)) {
		ref_branch = g_key_file_get_string (kf, "Flatpak Ref", "Branch", error);
		if (ref_branch == NULL) {
			gs_utils_error_convert_gio (error);
			return NULL;
		}
	} else {
		ref_branch = g_strdup ("master");
	}
	if (g_key_file_has_key (kf, "Flatpak Ref", "IsRuntime", NULL)) {
		is_runtime = g_key_file_get_boolean (kf, "Flatpak Ref", "IsRuntime", error);
		if (error != NULL && *error != NULL) {
			gs_utils_error_convert_gio (error);
			return NULL;
		}
	} else {
		is_runtime = FALSE;
	}

	if (unrefined) {
		/* Note: we don't support non-default arch here but it's not a
		 * regression since we never have for a flatpakref
		 */
		g_autofree char *app_ref = g_strdup_printf ("%s/%s/%s/%s",
				                            is_runtime ? "runtime" : "app",
							    ref_name,
							    flatpak_get_default_arch (),
							    ref_branch);
		parsed_ref = flatpak_ref_parse (app_ref, error);
		if (parsed_ref == NULL) {
			gs_flatpak_error_convert (error);
			return NULL;
		}

		/* early return */
		app = gs_flatpak_create_app (self, NULL, parsed_ref, NULL, interactive, cancellable);
		return g_steal_pointer (&app);
	}

	/* Add the remote (to the temporary installation) but abort the
	 * transaction before it installs the app
	 */
	transaction = flatpak_transaction_new_for_installation (installation, cancellable, error);
	if (transaction == NULL) {
		gs_flatpak_error_convert (error);
		return NULL;
	}
	flatpak_transaction_set_no_interaction (transaction, TRUE);
	g_signal_connect (transaction, "ready-pre-auth", G_CALLBACK (_txn_abort_on_ready), NULL);
	g_signal_connect (transaction, "add-new-remote", G_CALLBACK (_txn_add_new_remote), NULL);
	g_signal_connect (transaction, "choose-remote-for-ref", G_CALLBACK (_txn_choose_remote_for_ref), NULL);
	ref_file_data = g_bytes_new (contents, len);
	if (!flatpak_transaction_add_install_flatpakref (transaction, ref_file_data, error)) {
		gs_flatpak_error_convert (error);
		return NULL;
	}
	success = flatpak_transaction_run (transaction, cancellable, &error_local);
	g_assert (!success); /* aborted in _txn_abort_on_ready */

	/* We don't check for FLATPAK_ERROR_ALREADY_INSTALLED here because it's
	 * a temporary installation
	 */
	if (!g_error_matches (error_local, FLATPAK_ERROR, FLATPAK_ERROR_ABORTED)) {
		g_propagate_error (error, g_steal_pointer (&error_local));
		gs_flatpak_error_convert (error);
		return NULL;
	}

	g_clear_error (&error_local);

	/* find the operation for the flatpakref */
	txn_ops = flatpak_transaction_get_operations (transaction);
	for (GList *l = txn_ops; l != NULL; l = l->next) {
		FlatpakTransactionOperation *op = l->data;
		const char *op_ref = flatpak_transaction_operation_get_ref (op);
		parsed_ref = flatpak_ref_parse (op_ref, error);
		if (parsed_ref == NULL) {
			gs_flatpak_error_convert (error);
			return NULL;
		}
		if (g_strcmp0 (flatpak_ref_get_name (parsed_ref), ref_name) != 0) {
			g_clear_object (&parsed_ref);
		} else {
			remote_name = flatpak_transaction_operation_get_remote (op);
			g_debug ("auto-created remote name: %s", remote_name);
#if !FLATPAK_CHECK_VERSION(1,13,1)
			app_download_size = flatpak_transaction_operation_get_download_size (op);
			app_installed_size = flatpak_transaction_operation_get_installed_size (op);
#endif
			break;
		}
	}
	g_assert (parsed_ref != NULL);
	g_list_free_full (g_steal_pointer (&txn_ops), g_object_unref);

#if FLATPAK_CHECK_VERSION(1,13,1)
	/* fetch remote ref */
	remote_ref = flatpak_installation_fetch_remote_ref_sync (installation,
							   remote_name,
							   flatpak_ref_get_kind (parsed_ref),
							   flatpak_ref_get_name (parsed_ref),
							   flatpak_ref_get_arch (parsed_ref),
							   flatpak_ref_get_branch (parsed_ref),
							   cancellable,
							   error);
	if (remote_ref == NULL) {
		gs_flatpak_error_convert (error);
		return NULL;
	}
	app = gs_flatpak_create_app (self, remote_name, FLATPAK_REF (remote_ref), NULL, interactive, cancellable);
#else
	app = gs_flatpak_create_app (self, remote_name, parsed_ref, NULL, interactive, cancellable);
	gs_app_set_size_download (app, (app_download_size != 0) ? GS_SIZE_TYPE_VALID : GS_SIZE_TYPE_UNKNOWN, app_download_size);
	gs_app_set_size_installed (app, (app_installed_size != 0) ? GS_SIZE_TYPE_VALID : GS_SIZE_TYPE_UNKNOWN, app_installed_size);
#endif

	gs_app_add_quirk (app, GS_APP_QUIRK_HAS_SOURCE);
	gs_flatpak_app_set_file_kind (app, GS_FLATPAK_APP_FILE_KIND_REF);
	gs_app_set_state (app, GS_APP_STATE_AVAILABLE);

	runtime = gs_app_get_runtime (app);
	if (runtime != NULL) {
		g_autofree char *runtime_ref = gs_flatpak_app_get_ref_display (runtime);
		if (gs_app_get_state (runtime) == GS_APP_STATE_UNKNOWN) {
			g_autofree gchar *uri = NULL;
			/* the new runtime is available from the RuntimeRepo */
			uri = g_key_file_get_string (kf, "Flatpak Ref", "RuntimeRepo", NULL);
			gs_flatpak_app_set_runtime_url (runtime, uri);
		}

		/* find the operation for the runtime to set its size data. Since this
		 * is all happening on a tmp installation, it won't be available later
		 * during the refine step
		 */
		txn_ops = flatpak_transaction_get_operations (transaction);
		for (GList *l = txn_ops; l != NULL; l = l->next) {
			FlatpakTransactionOperation *op = l->data;
			const char *op_ref = flatpak_transaction_operation_get_ref (op);
			if (g_strcmp0 (runtime_ref, op_ref) == 0) {
				guint64 installed_size = 0, download_size = 0;
				download_size = flatpak_transaction_operation_get_download_size (op);
				gs_app_set_size_download (runtime, (download_size != 0) ? GS_SIZE_TYPE_VALID : GS_SIZE_TYPE_UNKNOWN, download_size);
				installed_size = flatpak_transaction_operation_get_installed_size (op);
				gs_app_set_size_installed (runtime, (installed_size != 0) ? GS_SIZE_TYPE_VALID : GS_SIZE_TYPE_UNKNOWN, installed_size);
				break;
			}
		}
		g_list_free_full (g_steal_pointer (&txn_ops), g_object_unref);
	}

	/* use the data from the flatpakref file as a fallback */
	ref_title = g_key_file_get_string (kf, "Flatpak Ref", "Title", NULL);
	if (ref_title != NULL)
		gs_app_set_name (app, GS_APP_QUALITY_NORMAL, ref_title);
	ref_comment = g_key_file_get_string (kf, "Flatpak Ref", "Comment", NULL);
	if (ref_comment != NULL)
		gs_app_set_summary (app, GS_APP_QUALITY_NORMAL, ref_comment);
	ref_description = g_key_file_get_string (kf, "Flatpak Ref", "Description", NULL);
	if (ref_description != NULL)
		gs_app_set_description (app, GS_APP_QUALITY_NORMAL, ref_description);
	ref_homepage = g_key_file_get_string (kf, "Flatpak Ref", "Homepage", NULL);
	if (ref_homepage != NULL)
		gs_app_set_url (app, AS_URL_KIND_HOMEPAGE, ref_homepage);
	ref_icon = g_key_file_get_string (kf, "Flatpak Ref", "Icon", NULL);
	if (ref_icon != NULL &&
	    (g_str_has_prefix (ref_icon, "http:") ||
	     g_str_has_prefix (ref_icon, "https:"))) {
		g_autoptr(GIcon) icon = gs_remote_icon_new (ref_icon);
		gs_app_add_icon (app, icon);
	}

	/* set the origin data */
	xremote = gs_flatpak_remote_by_name (self, remote_name, interactive, cancellable, error);
	if (xremote == NULL) {
		gs_flatpak_error_convert (error);
		return NULL;
	}
	origin_url = flatpak_remote_get_url (xremote);
	if (origin_url == NULL) {
		g_set_error (error,
			     GS_PLUGIN_ERROR,
			     GS_PLUGIN_ERROR_INVALID_FORMAT,
			     "no URL for remote %s",
			     flatpak_remote_get_name (xremote));
		return NULL;
	}
	gs_app_set_origin_hostname (app, origin_url);

	/* get the new appstream data (nonfatal for failure) */
	if (!gs_flatpak_refresh_appstream_remote (self, remote_name, interactive,
						  cancellable, &error_local)) {
		g_autoptr(GsPluginEvent) event = NULL;

		gs_flatpak_error_convert (&error_local);

		event = gs_plugin_event_new ("app", app,
					     "error", error_local,
					     NULL);
		gs_plugin_event_add_flag (event, GS_PLUGIN_EVENT_FLAG_WARNING);
		gs_plugin_report_event (self->plugin, event);
		g_clear_error (&error_local);
	}

	/* get this now, as it's not going to be available at install time */
	if (!gs_plugin_refine_item_metadata (self, app, interactive, cancellable, error))
		return NULL;

	/* parse it */
	if (!gs_flatpak_add_apps_from_xremote (self, builder, xremote, interactive, cancellable, error))
		return NULL;

	/* build silo */
	/* No need to change the thread-default main context because the silo
	 * doesn’t live beyond this function */
	silo = xb_builder_compile (builder,
				   XB_BUILDER_COMPILE_FLAG_SINGLE_LANG,
				   cancellable,
				   error);
	if (silo == NULL)
		return NULL;
	if (g_getenv ("GS_XMLB_VERBOSE") != NULL) {
		g_autofree gchar *xml = NULL;
		xml = xb_silo_export (silo,
				      XB_NODE_EXPORT_FLAG_FORMAT_INDENT |
				      XB_NODE_EXPORT_FLAG_FORMAT_MULTILINE,
				      NULL);
		g_debug ("showing AppStream data: %s", xml);
	}

	/* get extra AppStream data if available */
	if (!gs_flatpak_refine_appstream (self, app, silo,
					  GS_PLUGIN_REFINE_FLAGS_MASK,
					  NULL,
					  interactive,
					  cancellable,
					  error))
		return NULL;

	/* success */
	return g_steal_pointer (&app);
}

gboolean
gs_flatpak_search (GsFlatpak *self,
		   const gchar * const *values,
		   GsAppList *list,
		   gboolean interactive,
		   GCancellable *cancellable,
		   GError **error)
{
	g_autoptr(GsAppList) list_tmp = gs_app_list_new ();
	g_autoptr(GRWLockReaderLocker) locker = NULL;
	g_autoptr(GMutexLocker) app_silo_locker = NULL;
	g_autoptr(GPtrArray) silos_to_remove = g_ptr_array_new ();
	GHashTableIter iter;
	gpointer key, value;

	if (!ensure_flatpak_silo_with_locker (self, &locker, interactive, cancellable, error))
		return FALSE;

	if (!gs_appstream_search (self->plugin, self->silo, values, list_tmp,
				  cancellable, error))
		return FALSE;

	gs_flatpak_ensure_remote_title (self, interactive, cancellable);

	gs_flatpak_claim_app_list (self, list_tmp, interactive);
	gs_app_list_add_list (list, list_tmp);

	/* Also search silos from installed apps which were missing from self->silo */
	app_silo_locker = g_mutex_locker_new (&self->app_silos_mutex);
	g_hash_table_iter_init (&iter, self->app_silos);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		g_autoptr(XbSilo) app_silo = g_object_ref (value);
		g_autoptr(GsAppList) app_list_tmp = gs_app_list_new ();
		const char *app_ref = (char *)key;
		g_autoptr(FlatpakInstalledRef) installed_ref = NULL;
		g_auto(GStrv) split = NULL;
		FlatpakRefKind kind;

		/* Ignore any silos of apps that have since been removed.
		 * FIXME: can we use self->installed_refs here? */
		split = g_strsplit (app_ref, "/", -1);
		g_assert (g_strv_length (split) == 4);
		if (g_strcmp0 (split[0], "app") == 0)
			kind = FLATPAK_REF_KIND_APP;
		else
			kind = FLATPAK_REF_KIND_RUNTIME;
		installed_ref = flatpak_installation_get_installed_ref (gs_flatpak_get_installation (self, interactive),
									kind,
									split[1],
									split[2],
									split[3],
									NULL, NULL);
		if (installed_ref == NULL) {
			g_ptr_array_add (silos_to_remove, (gpointer) app_ref);
			continue;
		}

		if (!gs_appstream_search (self->plugin, app_silo, values, app_list_tmp,
					  cancellable, error))
			return FALSE;

		gs_flatpak_claim_app_list (self, app_list_tmp, interactive);
		gs_app_list_add_list (list, app_list_tmp);
	}

	for (guint i = 0; i < silos_to_remove->len; i++) {
		const char *silo = g_ptr_array_index (silos_to_remove, i);
		g_hash_table_remove (self->app_silos, silo);
	}

	return TRUE;
}

gboolean
gs_flatpak_search_developer_apps (GsFlatpak *self,
				  const gchar * const *values,
				  GsAppList *list,
				  gboolean interactive,
				  GCancellable *cancellable,
				  GError **error)
{
	g_autoptr(GsAppList) list_tmp = gs_app_list_new ();
	g_autoptr(GRWLockReaderLocker) locker = NULL;
	g_autoptr(GMutexLocker) app_silo_locker = NULL;
	g_autoptr(GPtrArray) silos_to_remove = g_ptr_array_new ();
	GHashTableIter iter;
	gpointer key, value;

	if (!ensure_flatpak_silo_with_locker (self, &locker, interactive, cancellable, error))
		return FALSE;

	if (!gs_appstream_search_developer_apps (self->plugin, self->silo, values, list_tmp,
						 cancellable, error))
		return FALSE;

	gs_flatpak_ensure_remote_title (self, interactive, cancellable);

	gs_flatpak_claim_app_list (self, list_tmp, interactive);
	gs_app_list_add_list (list, list_tmp);

	/* Also search silos from installed apps which were missing from self->silo */
	app_silo_locker = g_mutex_locker_new (&self->app_silos_mutex);
	g_hash_table_iter_init (&iter, self->app_silos);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		g_autoptr(XbSilo) app_silo = g_object_ref (value);
		g_autoptr(GsAppList) app_list_tmp = gs_app_list_new ();
		const char *app_ref = (char *)key;
		g_autoptr(FlatpakInstalledRef) installed_ref = NULL;
		g_auto(GStrv) split = NULL;
		FlatpakRefKind kind;

		/* Ignore any silos of apps that have since been removed.
		 * FIXME: can we use self->installed_refs here? */
		split = g_strsplit (app_ref, "/", -1);
		g_assert (g_strv_length (split) == 4);
		if (g_strcmp0 (split[0], "app") == 0)
			kind = FLATPAK_REF_KIND_APP;
		else
			kind = FLATPAK_REF_KIND_RUNTIME;
		installed_ref = flatpak_installation_get_installed_ref (gs_flatpak_get_installation (self, interactive),
									kind,
									split[1],
									split[2],
									split[3],
									NULL, NULL);
		if (installed_ref == NULL) {
			g_ptr_array_add (silos_to_remove, (gpointer) app_ref);
			continue;
		}

		if (!gs_appstream_search_developer_apps (self->plugin, app_silo, values, app_list_tmp,
							 cancellable, error))
			return FALSE;

		gs_flatpak_claim_app_list (self, app_list_tmp, interactive);
		gs_app_list_add_list (list, app_list_tmp);
	}

	for (guint i = 0; i < silos_to_remove->len; i++) {
		const char *silo = g_ptr_array_index (silos_to_remove, i);
		g_hash_table_remove (self->app_silos, silo);
	}

	return TRUE;
}

gboolean
gs_flatpak_add_category_apps (GsFlatpak *self,
			      GsCategory *category,
			      GsAppList *list,
			      gboolean interactive,
			      GCancellable *cancellable,
			      GError **error)
{
	g_autoptr(GRWLockReaderLocker) locker = NULL;

	if (!ensure_flatpak_silo_with_locker (self, &locker, interactive, cancellable, error))
		return FALSE;

	return gs_appstream_add_category_apps (self->plugin, self->silo,
					       category, list,
					       cancellable, error);
}

gboolean
gs_flatpak_refine_category_sizes (GsFlatpak     *self,
                                  GPtrArray     *list,
                                  gboolean       interactive,
                                  GCancellable  *cancellable,
                                  GError       **error)
{
	g_autoptr(GRWLockReaderLocker) locker = NULL;

	if (!ensure_flatpak_silo_with_locker (self, &locker, interactive, cancellable, error))
		return FALSE;

	return gs_appstream_refine_category_sizes (self->silo, list, cancellable, error);
}

gboolean
gs_flatpak_add_popular (GsFlatpak *self,
			GsAppList *list,
			gboolean interactive,
			GCancellable *cancellable,
			GError **error)
{
	g_autoptr(GsAppList) list_tmp = gs_app_list_new ();
	g_autoptr(GRWLockReaderLocker) locker = NULL;

	if (!ensure_flatpak_silo_with_locker (self, &locker, interactive, cancellable, error))
		return FALSE;

	if (!gs_appstream_add_popular (self->silo, list_tmp,
				       cancellable, error))
		return FALSE;

	gs_app_list_add_list (list, list_tmp);

	return TRUE;
}

gboolean
gs_flatpak_add_featured (GsFlatpak *self,
			 GsAppList *list,
			 gboolean interactive,
			 GCancellable *cancellable,
			 GError **error)
{
	g_autoptr(GsAppList) list_tmp = gs_app_list_new ();
	g_autoptr(GRWLockReaderLocker) locker = NULL;

	if (!ensure_flatpak_silo_with_locker (self, &locker, interactive, cancellable, error))
		return FALSE;

	if (!gs_appstream_add_featured (self->silo, list_tmp,
					cancellable, error))
		return FALSE;

	gs_app_list_add_list (list, list_tmp);

	return TRUE;
}

gboolean
gs_flatpak_add_deployment_featured (GsFlatpak *self,
				    GsAppList *list,
				    gboolean interactive,
				    const gchar *const *deployments,
				    GCancellable *cancellable,
				    GError **error)
{
	g_autoptr(GRWLockReaderLocker) locker = NULL;

	if (!ensure_flatpak_silo_with_locker (self, &locker, interactive, cancellable, error))
		return FALSE;

	return gs_appstream_add_deployment_featured (self->silo, deployments, list, cancellable, error);
}

gboolean
gs_flatpak_add_alternates (GsFlatpak *self,
			   GsApp *app,
			   GsAppList *list,
			   gboolean interactive,
			   GCancellable *cancellable,
			   GError **error)
{
	g_autoptr(GsAppList) list_tmp = gs_app_list_new ();
	g_autoptr(GRWLockReaderLocker) locker = NULL;

	if (!ensure_flatpak_silo_with_locker (self, &locker, interactive, cancellable, error))
		return FALSE;

	if (!gs_appstream_add_alternates (self->silo, app, list_tmp,
					  cancellable, error))
		return FALSE;

	gs_app_list_add_list (list, list_tmp);

	return TRUE;
}

gboolean
gs_flatpak_add_recent (GsFlatpak *self,
		       GsAppList *list,
		       guint64 age,
		       gboolean interactive,
		       GCancellable *cancellable,
		       GError **error)
{
	g_autoptr(GsAppList) list_tmp = gs_app_list_new ();
	g_autoptr(GRWLockReaderLocker) locker = NULL;

	if (!ensure_flatpak_silo_with_locker (self, &locker, interactive, cancellable, error))
		return FALSE;

	if (!gs_appstream_add_recent (self->plugin, self->silo, list_tmp, age,
				      cancellable, error))
		return FALSE;

	gs_flatpak_claim_app_list (self, list_tmp, interactive);
	gs_app_list_add_list (list, list_tmp);

	return TRUE;
}

gboolean
gs_flatpak_url_to_app (GsFlatpak *self,
		       GsAppList *list,
		       const gchar *url,
		       gboolean interactive,
		       GCancellable *cancellable,
		       GError **error)
{
	g_autoptr(GsAppList) list_tmp = gs_app_list_new ();
	g_autoptr(GRWLockReaderLocker) locker = NULL;

	if (!ensure_flatpak_silo_with_locker (self, &locker, interactive, cancellable, error))
		return FALSE;

	if (!gs_appstream_url_to_app (self->plugin, self->silo, list_tmp, url, cancellable, error))
		return FALSE;

	gs_flatpak_claim_app_list (self, list_tmp, interactive);
	gs_app_list_add_list (list, list_tmp);

	return TRUE;
}

const gchar *
gs_flatpak_get_id (GsFlatpak *self)
{
	if (self->id == NULL) {
		GString *str = g_string_new ("flatpak");
		g_string_append_printf (str, "-%s",
					as_component_scope_to_string (self->scope));
		if (flatpak_installation_get_id (self->installation_noninteractive) != NULL) {
			g_string_append_printf (str, "-%s",
						flatpak_installation_get_id (self->installation_noninteractive));
		}
		if (self->flags & GS_FLATPAK_FLAG_IS_TEMPORARY)
			g_string_append (str, "-temp");
		self->id = g_string_free (str, FALSE);
	}
	return self->id;
}

AsComponentScope
gs_flatpak_get_scope (GsFlatpak *self)
{
	return self->scope;
}

FlatpakInstallation *
gs_flatpak_get_installation (GsFlatpak *self,
                             gboolean   interactive)
{
	return interactive ? self->installation_interactive : self->installation_noninteractive;
}

static void
gs_flatpak_finalize (GObject *object)
{
	GsFlatpak *self;
	g_return_if_fail (GS_IS_FLATPAK (object));
	self = GS_FLATPAK (object);

	if (self->changed_id > 0) {
		g_signal_handler_disconnect (self->monitor, self->changed_id);
		self->changed_id = 0;
	}
	if (self->silo != NULL)
		g_object_unref (self->silo);
	if (self->monitor != NULL)
		g_object_unref (self->monitor);
	g_clear_pointer (&self->silo_filename, g_free);
	g_clear_pointer (&self->silo_installed_by_desktopid, g_hash_table_unref);

	g_free (self->id);
	g_object_unref (self->installation_noninteractive);
	g_object_unref (self->installation_interactive);
	g_clear_pointer (&self->installed_refs, g_ptr_array_unref);
	g_clear_pointer (&self->remotes_by_name, g_hash_table_unref);
	g_mutex_clear (&self->installed_refs_mutex);
	g_object_unref (self->plugin);
	g_hash_table_unref (self->broken_remotes);
	g_mutex_clear (&self->broken_remotes_mutex);
	g_rw_lock_clear (&self->silo_lock);
	g_hash_table_unref (self->app_silos);
	g_mutex_clear (&self->app_silos_mutex);
	g_clear_pointer (&self->remote_title, g_hash_table_unref);
	g_mutex_clear (&self->remote_title_mutex);

	G_OBJECT_CLASS (gs_flatpak_parent_class)->finalize (object);
}

static void
gs_flatpak_class_init (GsFlatpakClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = gs_flatpak_finalize;
}

static void
gs_flatpak_init (GsFlatpak *self)
{
	/* XbSilo needs external locking as we destroy the silo and build a new
	 * one when something changes */
	g_rw_lock_init (&self->silo_lock);

	g_mutex_init (&self->installed_refs_mutex);
	self->installed_refs = NULL;
	self->remotes_by_name = NULL;
	g_mutex_init (&self->broken_remotes_mutex);
	self->broken_remotes = g_hash_table_new_full (g_str_hash, g_str_equal,
						      g_free, NULL);
	self->app_silos = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
	g_mutex_init (&self->app_silos_mutex);
	self->remote_title = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
	g_mutex_init (&self->remote_title_mutex);
}

GsFlatpak *
gs_flatpak_new (GsPlugin *plugin, FlatpakInstallation *installation, GsFlatpakFlags flags)
{
	GsFlatpak *self;
	g_autoptr(GFile) path = NULL;
	gboolean is_user;

	path = flatpak_installation_get_path (installation);
	is_user = flatpak_installation_get_is_user (installation);

	self = g_object_new (GS_TYPE_FLATPAK, NULL);

	self->installation_noninteractive = g_object_ref (installation);
	flatpak_installation_set_no_interaction (self->installation_noninteractive, TRUE);

	/* Cloning it should never fail as the repo should already exist on disk. */
	self->installation_interactive = flatpak_installation_new_for_path (path, is_user, NULL, NULL);
	g_assert (self->installation_interactive != NULL);
	flatpak_installation_set_no_interaction (self->installation_interactive, FALSE);

	self->scope = is_user ? AS_COMPONENT_SCOPE_USER : AS_COMPONENT_SCOPE_SYSTEM;
	self->plugin = g_object_ref (plugin);
	self->flags = flags;
	return GS_FLATPAK (self);
}

void
gs_flatpak_set_busy (GsFlatpak *self,
		     gboolean busy)
{
	g_return_if_fail (GS_IS_FLATPAK (self));

	if (busy) {
		g_atomic_int_inc (&self->busy);
	} else {
		g_return_if_fail (g_atomic_int_get (&self->busy) > 0);
		if (g_atomic_int_dec_and_test (&self->busy)) {
			if (self->changed_while_busy) {
				self->changed_while_busy = FALSE;
				g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, gs_flatpak_claim_changed_idle_cb,
					g_object_ref (self), g_object_unref);
			}
		}
	}
}

gboolean
gs_flatpak_get_busy (GsFlatpak *self)
{
	g_return_val_if_fail (GS_IS_FLATPAK (self), FALSE);
	return g_atomic_int_get (&self->busy) > 0;
}

gboolean
gs_flatpak_purge_sync (GsFlatpak    *self,
		       GCancellable *cancellable,
		       GError      **error)
{
	FlatpakInstallation *installation;
	g_autoptr(GPtrArray) unused_refs = NULL;

	installation = gs_flatpak_get_installation (self, FALSE);
	if (installation == NULL) {
		g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND,
				     "Non-interactive installation not found");
		return FALSE;
	}

	unused_refs = flatpak_installation_list_unused_refs (installation, NULL, cancellable, error);
	if (unused_refs == NULL)
		return FALSE;

	g_debug ("Installation '%s' has %u unused refs", gs_flatpak_get_id (self), unused_refs->len);

	if (unused_refs->len > 0) {
		g_autoptr(FlatpakTransaction) transaction = NULL;
		transaction = gs_flatpak_transaction_new (installation, GS_FLATPAK_ERROR_MODE_STOP_ON_FIRST_ERROR, cancellable, error);
		if (transaction == NULL) {
			g_prefix_error_literal (error, "failed to build transaction: ");
			return FALSE;
		}
		flatpak_transaction_set_no_interaction (transaction, TRUE);
		flatpak_transaction_set_no_pull (transaction, TRUE);

		/* use system installations as dependency sources for user installations */
		flatpak_transaction_add_default_dependency_sources (transaction);

		for (guint i = 0; i < unused_refs->len; i++) {
			g_autoptr(GsApp) app = NULL;
			FlatpakRef *ref = g_ptr_array_index (unused_refs, i);
			const gchar *ref_str = flatpak_ref_format_ref_cached (ref);
			app = gs_flatpak_ref_to_app (self, ref_str, FALSE, cancellable, error);
			if (app == NULL) {
				g_prefix_error (error, "failed to create app from ref '%s': ", ref_str);
				return FALSE;
			}
			gs_flatpak_transaction_add_app (transaction, app);
			if (!flatpak_transaction_add_uninstall (transaction, ref_str, error)) {
				g_prefix_error (error, "failed to add ref to transaction: ");
				return FALSE;
			}
			g_debug ("Going to uninstall '%s'", ref_str);
		}

		return gs_flatpak_transaction_run (transaction, cancellable, error);
	} else {
		/* Nothing to uninstall. */
		return TRUE;
	}
}
