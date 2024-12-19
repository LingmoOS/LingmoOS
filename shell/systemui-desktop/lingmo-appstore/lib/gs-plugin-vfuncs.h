/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2012-2017 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

/**
 * SECTION:gs-plugin-vfuncs
 * @title: GsPlugin Exports
 * @include: lingmo-appstore.h
 * @stability: Unstable
 * @short_description: Vfuncs that plugins can implement
 */

#include <appstream.h>
#include <glib-object.h>
#include <gmodule.h>
#include <gio/gio.h>
#include <libsoup/soup.h>

#include "gs-app.h"
#include "gs-app-list.h"
#include "gs-category.h"

G_BEGIN_DECLS

/**
 * gs_plugin_query_type:
 *
 * Returns the #GType for a subclass of #GsPlugin provided by this plugin
 * module. It should not do any other computation.
 *
 * The init function for that type should initialize the plugin. If the plugin
 * should not be run then gs_plugin_set_enabled() should be called from the
 * init function.
 *
 * NOTE: Do not do any failable actions in the plugin class’ init function; use
 * #GsPluginClass.setup_async instead.
 *
 * Since: 42
 */
GType		 gs_plugin_query_type			(void);

/**
 * gs_plugin_adopt_app:
 * @plugin: a #GsPlugin
 * @app: a #GsApp
 *
 * Called when an #GsApp has not been claimed (i.e. a management plugin has not
 * been set).
 *
 * A claimed app means other plugins will not try to perform actions
 * such as install, remove or update. Most apps are claimed when they
 * are created.
 *
 * If a plugin can adopt this app then it should call
 * gs_app_set_management_plugin() on @app.
 **/
void		 gs_plugin_adopt_app			(GsPlugin	*plugin,
							 GsApp		*app);

/**
 * gs_plugin_add_updates:
 * @plugin: a #GsPlugin
 * @list: a #GsAppList
 * @cancellable: a #GCancellable, or %NULL
 * @error: a #GError, or %NULL
 *
 * Get the list of updates.
 *
 * NOTE: Actually downloading the updates can be done in
 * #GsPlugin.update_apps_async().
 *
 * Plugins are expected to add new apps using gs_app_list_add().
 *
 * Returns: %TRUE for success or if not relevant
 **/
gboolean	 gs_plugin_add_updates			(GsPlugin	*plugin,
							 GsAppList	*list,
							 GCancellable	*cancellable,
							 GError		**error);

/**
 * gs_plugin_add_sources:
 * @plugin: a #GsPlugin
 * @list: a #GsAppList
 * @cancellable: a #GCancellable, or %NULL
 * @error: a #GError, or %NULL
 *
 * Get the list of sources, for example the repos listed in /etc/yum.repos.d
 * or the remotes configured in flatpak.
 *
 * Plugins are expected to add new apps using gs_app_list_add() of type
 * %AS_COMPONENT_KIND_REPOSITORY.
 *
 * Returns: %TRUE for success or if not relevant
 **/
gboolean	 gs_plugin_add_sources			(GsPlugin	*plugin,
							 GsAppList	*list,
							 GCancellable	*cancellable,
							 GError		**error);

/**
 * gs_plugin_add_updates_historical
 * @plugin: a #GsPlugin
 * @list: a #GsAppList
 * @cancellable: a #GCancellable, or %NULL
 * @error: a #GError, or %NULL
 *
 * Get the list of historical updates, i.e. the updates that have just been
 * installed.
 *
 * Plugins are expected to add new apps using gs_app_list_add().
 *
 * Returns: %TRUE for success or if not relevant
 **/
gboolean	 gs_plugin_add_updates_historical	(GsPlugin	*plugin,
							 GsAppList	*list,
							 GCancellable	*cancellable,
							 GError		**error);

/**
 * gs_plugin_launch:
 * @plugin: a #GsPlugin
 * @app: a #GsApp
 * @cancellable: a #GCancellable, or %NULL
 * @error: a #GError, or %NULL
 *
 * Launch the specified app using a plugin-specific method.
 * This is normally setting some environment or launching a specific binary.
 *
 * Plugins can simply use gs_plugin_app_launch() if no plugin-specific
 * functionality is required.
 *
 * Returns: %TRUE for success or if not relevant
 **/
gboolean	 gs_plugin_launch			(GsPlugin	*plugin,
							 GsApp		*app,
							 GCancellable	*cancellable,
							 GError		**error);

/**
 * gs_plugin_update_cancel:
 * @plugin: a #GsPlugin
 * @app: a #GsApp
 * @cancellable: a #GCancellable, or %NULL
 * @error: a #GError, or %NULL
 *
 * Cancels the offline update of @app.
 *
 * Returns: %TRUE for success or if not relevant
 **/
gboolean	 gs_plugin_update_cancel		(GsPlugin	*plugin,
							 GsApp		*app,
							 GCancellable	*cancellable,
							 GError		**error);

/**
 * gs_plugin_app_install:
 * @plugin: a #GsPlugin
 * @app: a #GsApp
 * @cancellable: a #GCancellable, or %NULL
 * @error: a #GError, or %NULL
 *
 * Install the app.
 *
 * Plugins are expected to send progress notifications to the UI using
 * gs_app_set_progress() using the passed in @app.
 *
 * All functions can block, but should sent progress notifications, e.g. using
 * gs_app_set_progress() if they will take more than tens of milliseconds
 * to complete.
 *
 * On failure the error message returned will usually only be shown on the
 * console, but they can also be retrieved using gs_plugin_loader_get_events().
 *
 * NOTE: Once the action is complete, the plugin must set the new state of @app
 * to %GS_APP_STATE_INSTALLED.
 *
 * Returns: %TRUE for success or if not relevant
 **/
gboolean	 gs_plugin_app_install			(GsPlugin	*plugin,
							 GsApp		*app,
							 GCancellable	*cancellable,
							 GError		**error);

/**
 * gs_plugin_app_remove:
 * @plugin: a #GsPlugin
 * @app: a #GsApp
 * @cancellable: a #GCancellable, or %NULL
 * @error: a #GError, or %NULL
 *
 * Remove the app.
 *
 * Plugins are expected to send progress notifications to the UI using
 * gs_app_set_progress() using the passed in @app.
 *
 * All functions can block, but should sent progress notifications, e.g. using
 * gs_app_set_progress() if they will take more than tens of milliseconds
 * to complete.
 *
 * On failure the error message returned will usually only be shown on the
 * console, but they can also be retrieved using gs_plugin_loader_get_events().
 *
 * NOTE: Once the action is complete, the plugin must set the new state of @app
 * to %GS_APP_STATE_AVAILABLE or %GS_APP_STATE_UNKNOWN if not known.
 *
 * Returns: %TRUE for success or if not relevant
 **/
gboolean	 gs_plugin_app_remove			(GsPlugin	*plugin,
							 GsApp		*app,
							 GCancellable	*cancellable,
							 GError		**error);

/**
 * gs_plugin_app_upgrade_download:
 * @plugin: a #GsPlugin
 * @app: a #GsApp, with kind %AS_COMPONENT_KIND_OPERATING_SYSTEM
 * @cancellable: a #GCancellable, or %NULL
 * @error: a #GError, or %NULL
 *
 * Starts downloading a distribution upgrade in the background.
 *
 * All functions can block, but should sent progress notifications, e.g. using
 * gs_app_set_progress() if they will take more than tens of milliseconds
 * to complete.
 *
 * Returns: %TRUE for success or if not relevant
 **/
gboolean	 gs_plugin_app_upgrade_download		(GsPlugin	*plugin,
							 GsApp		*app,
							 GCancellable	*cancellable,
							 GError		**error);

/**
 * gs_plugin_app_upgrade_trigger:
 * @plugin: a #GsPlugin
 * @app: a #GsApp, with kind %AS_COMPONENT_KIND_OPERATING_SYSTEM
 * @cancellable: a #GCancellable, or %NULL
 * @error: a #GError, or %NULL
 *
 * Triggers the distribution upgrade to be installed on next boot.
 *
 * Returns: %TRUE for success or if not relevant
 **/
gboolean	 gs_plugin_app_upgrade_trigger		(GsPlugin	*plugin,
							 GsApp		*app,
							 GCancellable	*cancellable,
							 GError		**error);

/**
 * gs_plugin_file_to_app:
 * @plugin: a #GsPlugin
 * @list: a #GsAppList
 * @file: a #GFile
 * @cancellable: a #GCancellable, or %NULL
 * @error: a #GError, or %NULL
 *
 * Converts a local file to a #GsApp. It's expected that only one plugin will
 * match the mimetype of @file and that a single #GsApp will be in the returned
 * list. If no plugins can handle the file, the list will be empty.
 *
 * For example, the PackageKit plugin can turn a .rpm file into a app
 * of kind %AS_COMPONENT_KIND_UNKNOWN but that in some cases it will be further refined
 * into a %AS_COMPONENT_KIND_DESKTOP_APP (with all the extra metadata) by the appstream
 * plugin.
 *
 * Plugins are expected to add new apps using gs_app_list_add().
 *
 * Returns: %TRUE for success or if not relevant
 **/
gboolean	 gs_plugin_file_to_app			(GsPlugin	*plugin,
							 GsAppList	*list,
							 GFile		*file,
							 GCancellable	*cancellable,
							 GError		**error);

/**
 * gs_plugin_url_to_app:
 * @plugin: a #GsPlugin
 * @list: a #GsAppList
 * @url: a #URL, e.g. "apt://gimp"
 * @cancellable: a #GCancellable, or %NULL
 * @error: a #GError, or %NULL
 *
 * Converts a URL to a #GsApp. It's expected that only one plugin will
 * match the scheme of @url and that a single #GsApp will be in the returned
 * list. If no plugins can handle the file, the list will be empty.
 *
 * For example, the apt plugin can turn apt://gimp into a application.
 *
 * Plugins are expected to add new apps using gs_app_list_add().
 *
 * Returns: %TRUE for success or if not relevant
 **/
gboolean	 gs_plugin_url_to_app			(GsPlugin	*plugin,
							 GsAppList	*list,
							 const gchar	*url,
							 GCancellable	*cancellable,
							 GError		**error);

/**
 * gs_plugin_add_langpacks:
 * @plugin: a #GsPlugin
 * @list: a #GsAppList
 * @locale: a #LANGUAGE_CODE or #LOCALE, e.g. "ja" or "ja_JP"
 * @cancellable: a #GCancellable, or %NULL
 * @error: a #GError, or %NULL
 *
 * Returns a list of language packs, as per input language code or locale.
 *
 * Returns: %TRUE for success or if not relevant
 **/
gboolean	 gs_plugin_add_langpacks		(GsPlugin	*plugin,
							 GsAppList	*list,
							 const gchar    *locale,
							 GCancellable	*cancellable,
							 GError		**error);

G_END_DECLS
