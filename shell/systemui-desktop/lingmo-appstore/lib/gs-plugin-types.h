/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2012-2018 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * GsPluginStatus:
 * @GS_PLUGIN_STATUS_UNKNOWN:		Unknown status
 * @GS_PLUGIN_STATUS_WAITING:		Waiting
 * @GS_PLUGIN_STATUS_FINISHED:		Finished
 * @GS_PLUGIN_STATUS_SETUP:		Setup in progress
 * @GS_PLUGIN_STATUS_DOWNLOADING:	Downloading in progress
 * @GS_PLUGIN_STATUS_QUERYING:		Querying in progress
 * @GS_PLUGIN_STATUS_INSTALLING:	Installing in progress
 * @GS_PLUGIN_STATUS_REMOVING:		Removing in progress
 *
 * The status of the plugin.
 **/
typedef enum {
	GS_PLUGIN_STATUS_UNKNOWN,
	GS_PLUGIN_STATUS_WAITING,
	GS_PLUGIN_STATUS_FINISHED,
	GS_PLUGIN_STATUS_SETUP,
	GS_PLUGIN_STATUS_DOWNLOADING,
	GS_PLUGIN_STATUS_QUERYING,
	GS_PLUGIN_STATUS_INSTALLING,
	GS_PLUGIN_STATUS_REMOVING,
	GS_PLUGIN_STATUS_LAST  /*< skip >*/
} GsPluginStatus;

/**
 * GsPluginFlags:
 * @GS_PLUGIN_FLAGS_NONE:		No flags set
 * @GS_PLUGIN_FLAGS_INTERACTIVE:	User initiated the job
 *
 * The flags for the plugin at this point in time.
 **/
typedef enum {
	GS_PLUGIN_FLAGS_NONE = 0,
	GS_PLUGIN_FLAGS_INTERACTIVE = 1 << 4,
} GsPluginFlags;

/**
 * GsPluginError:
 * @GS_PLUGIN_ERROR_FAILED:			Generic failure
 * @GS_PLUGIN_ERROR_NOT_SUPPORTED:		Action not supported
 * @GS_PLUGIN_ERROR_CANCELLED:			Action was cancelled
 * @GS_PLUGIN_ERROR_NO_NETWORK:			No network connection available
 * @GS_PLUGIN_ERROR_NO_SECURITY:		Security policy forbid action
 * @GS_PLUGIN_ERROR_NO_SPACE:			No disk space to allow action
 * @GS_PLUGIN_ERROR_AUTH_REQUIRED:		Authentication was required
 * @GS_PLUGIN_ERROR_AUTH_INVALID:		Provided authentication was invalid
 * @GS_PLUGIN_ERROR_PLUGIN_DEPSOLVE_FAILED:	The plugins installed are incompatible
 * @GS_PLUGIN_ERROR_DOWNLOAD_FAILED:		The download action failed
 * @GS_PLUGIN_ERROR_WRITE_FAILED:		The save-to-disk failed
 * @GS_PLUGIN_ERROR_INVALID_FORMAT:		The data format is invalid
 * @GS_PLUGIN_ERROR_DELETE_FAILED:		The delete action failed
 * @GS_PLUGIN_ERROR_RESTART_REQUIRED:		A restart is required
 * @GS_PLUGIN_ERROR_AC_POWER_REQUIRED:		AC power is required
 * @GS_PLUGIN_ERROR_TIMED_OUT:			The job timed out
 * @GS_PLUGIN_ERROR_BATTERY_LEVEL_TOO_LOW:	The system battery level is too low
 *
 * The failure error types.
 **/
typedef enum {
	GS_PLUGIN_ERROR_FAILED,
	GS_PLUGIN_ERROR_NOT_SUPPORTED,
	GS_PLUGIN_ERROR_CANCELLED,
	GS_PLUGIN_ERROR_NO_NETWORK,
	GS_PLUGIN_ERROR_NO_SECURITY,
	GS_PLUGIN_ERROR_NO_SPACE,
	GS_PLUGIN_ERROR_AUTH_REQUIRED,
	GS_PLUGIN_ERROR_AUTH_INVALID,
	GS_PLUGIN_ERROR_PLUGIN_DEPSOLVE_FAILED,
	GS_PLUGIN_ERROR_DOWNLOAD_FAILED,
	GS_PLUGIN_ERROR_WRITE_FAILED,
	GS_PLUGIN_ERROR_INVALID_FORMAT,
	GS_PLUGIN_ERROR_DELETE_FAILED,
	GS_PLUGIN_ERROR_RESTART_REQUIRED,
	GS_PLUGIN_ERROR_AC_POWER_REQUIRED,
	GS_PLUGIN_ERROR_TIMED_OUT,
	GS_PLUGIN_ERROR_BATTERY_LEVEL_TOO_LOW,
	GS_PLUGIN_ERROR_LAST  /*< skip >*/
} GsPluginError;

/**
 * GsPluginRefineFlags:
 * @GS_PLUGIN_REFINE_FLAGS_NONE:			No explicit flags set
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_ID:			Require the app’s ID; this is the minimum possible requirement
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_LICENSE:		Require the license
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_URL:			Require the URL
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_DESCRIPTION:		Require the long description
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_SIZE:		Require the installed and download sizes
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_RATING:		Require the rating
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_VERSION:		Require the version
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_HISTORY:		Require the history
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_SETUP_ACTION:	Require enough to install or remove the package
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_UPDATE_DETAILS:	Require update details
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_ORIGIN:		Require the origin
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_RELATED:		Require related packages
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_ADDONS:		Require available addons
 * @GS_PLUGIN_REFINE_FLAGS_ALLOW_PACKAGES:		Allow packages to be returned
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_UPDATE_SEVERITY:	Require update severity
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_UPGRADE_REMOVED:	Require distro upgrades
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_PROVENANCE:		Require the provenance
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_REVIEWS:		Require user-reviews
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_REVIEW_RATINGS:	Require user-ratings
 * @GS_PLUGIN_REFINE_FLAGS_DISABLE_FILTERING:		Normally the results of a refine are
 * 	filtered to remove non-valid apps; if this flag is set, that won’t happen.
 * 	This is intended to be used by internal #GsPluginLoader code. (Since: 42)
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON:		Require the icon to be loaded
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_PERMISSIONS:		Require the needed permissions
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_ORIGIN_HOSTNAME:	Require the origin hostname
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_ORIGIN_UI:		Require the origin for UI
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_RUNTIME:		Require the runtime
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_SCREENSHOTS:		Require screenshot information
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_CATEGORIES:		Require categories
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_PROJECT_GROUP:	Require project group
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_DEVELOPER_NAME:	Require developer name
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_KUDOS:		Require kudos
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_CONTENT_RATING:	Require content rating
 * @GS_PLUGIN_REFINE_FLAGS_REQUIRE_SIZE_DATA:		Require user and cache data sizes (Since: 41)
 * @GS_PLUGIN_REFINE_FLAGS_MASK:			All flags (Since: 40)
 *
 * The refine flags.
 **/
typedef enum {
	GS_PLUGIN_REFINE_FLAGS_NONE			= 0,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_ID		= 1 << 0,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_LICENSE		= 1 << 1,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_URL		= 1 << 2,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_DESCRIPTION	= 1 << 3,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_SIZE		= 1 << 4,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_RATING		= 1 << 5,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_VERSION		= 1 << 6,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_HISTORY		= 1 << 7,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_SETUP_ACTION	= 1 << 8,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_UPDATE_DETAILS	= 1 << 9,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_ORIGIN		= 1 << 10,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_RELATED		= 1 << 11,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_SIZE_DATA	= 1 << 12,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_ADDONS		= 1 << 13,
	GS_PLUGIN_REFINE_FLAGS_ALLOW_PACKAGES		= 1 << 14, /* TODO: move to request */
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_UPDATE_SEVERITY	= 1 << 15,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_UPGRADE_REMOVED	= 1 << 16,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_PROVENANCE	= 1 << 17,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_REVIEWS		= 1 << 18,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_REVIEW_RATINGS	= 1 << 19,
	GS_PLUGIN_REFINE_FLAGS_DISABLE_FILTERING	= 1 << 20,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON		= 1 << 21,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_PERMISSIONS	= 1 << 22,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_ORIGIN_HOSTNAME	= 1 << 23,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_ORIGIN_UI	= 1 << 24,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_RUNTIME		= 1 << 25,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_SCREENSHOTS	= 1 << 26,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_CATEGORIES	= 1 << 27,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_PROJECT_GROUP	= 1 << 28,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_DEVELOPER_NAME	= 1 << 29,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_KUDOS		= 1 << 30,
	GS_PLUGIN_REFINE_FLAGS_REQUIRE_CONTENT_RATING	= 1 << 31,
	GS_PLUGIN_REFINE_FLAGS_MASK			= ~0,
} GsPluginRefineFlags;

/**
 * GsPluginListAppsFlags:
 * @GS_PLUGIN_LIST_APPS_FLAGS_NONE: No flags set.
 * @GS_PLUGIN_LIST_APPS_FLAGS_INTERACTIVE: User initiated the job.
 *
 * Flags for an operation to list apps matching a given query.
 *
 * Since: 43
 */
typedef enum {
	GS_PLUGIN_LIST_APPS_FLAGS_NONE = 0,
	GS_PLUGIN_LIST_APPS_FLAGS_INTERACTIVE = 1 << 0,
} GsPluginListAppsFlags;

/**
 * GsPluginRefineCategoriesFlags:
 * @GS_PLUGIN_REFINE_CATEGORIES_FLAGS_NONE: No flags set.
 * @GS_PLUGIN_REFINE_CATEGORIES_FLAGS_INTERACTIVE: User initiated the job.
 * @GS_PLUGIN_REFINE_CATEGORIES_FLAGS_SIZE: Work out the number of apps in each category.
 *
 * Flags for an operation to refine categories.
 *
 * Since: 43
 */
typedef enum {
	GS_PLUGIN_REFINE_CATEGORIES_FLAGS_NONE = 0,
	GS_PLUGIN_REFINE_CATEGORIES_FLAGS_INTERACTIVE = 1 << 0,
	GS_PLUGIN_REFINE_CATEGORIES_FLAGS_SIZE = 1 << 1,
} GsPluginRefineCategoriesFlags;

/**
 * GsPluginRefreshMetadataFlags:
 * @GS_PLUGIN_REFRESH_METADATA_FLAGS_NONE: No flags set.
 * @GS_PLUGIN_REFRESH_METADATA_FLAGS_INTERACTIVE: User initiated the job.
 *
 * Flags for an operation to refresh metadata.
 *
 * Since: 42
 */
typedef enum {
	GS_PLUGIN_REFRESH_METADATA_FLAGS_NONE = 0,
	GS_PLUGIN_REFRESH_METADATA_FLAGS_INTERACTIVE = 1 << 0,
} GsPluginRefreshMetadataFlags;

/**
 * GsPluginListDistroUpgradesFlags:
 * @GS_PLUGIN_LIST_DISTRO_UPGRADES_FLAGS_NONE: No flags set.
 * @GS_PLUGIN_LIST_DISTRO_UPGRADES_FLAGS_INTERACTIVE: User initiated the job.
 *
 * Flags for an operation to list available distro upgrades.
 *
 * Since: 42
 */
typedef enum {
	GS_PLUGIN_LIST_DISTRO_UPGRADES_FLAGS_NONE = 0,
	GS_PLUGIN_LIST_DISTRO_UPGRADES_FLAGS_INTERACTIVE = 1 << 0,
} GsPluginListDistroUpgradesFlags;

/**
 * GsPluginManageRepositoryFlags:
 * @GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_NONE: No flags set.
 * @GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_INTERACTIVE: User initiated the job.
 * @GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_INSTALL: Install the repository.
 * @GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_REMOVE: Remove the repository.
 * @GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_ENABLE: Enable the repository.
 * @GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_DISABLE: Disable the repository.
 *
 * Flags for an operation on a repository.
 *
 * Since: 42
 */
typedef enum {
	GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_NONE		= 0,
	GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_INTERACTIVE	= 1 << 0,
	GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_INSTALL	= 1 << 1,
	GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_REMOVE	= 1 << 2,
	GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_ENABLE	= 1 << 3,
	GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_DISABLE	= 1 << 4,
} GsPluginManageRepositoryFlags;

/**
 * GsPluginUpdateAppsFlags:
 * @GS_PLUGIN_UPDATE_APPS_FLAGS_NONE: No flags set.
 * @GS_PLUGIN_UPDATE_APPS_FLAGS_INTERACTIVE: User initiated the job.
 * @GS_PLUGIN_UPDATE_APPS_FLAGS_NO_DOWNLOAD: Only use locally cached resources,
 *   and error if they don’t exist.
 * @GS_PLUGIN_UPDATE_APPS_FLAGS_NO_APPLY: Only download the resources, and don’t
 *   apply the updates.
 *
 * Flags for an operation to download or update apps.
 *
 * Since: 44
 */
typedef enum {
	GS_PLUGIN_UPDATE_APPS_FLAGS_NONE = 0,
	GS_PLUGIN_UPDATE_APPS_FLAGS_INTERACTIVE = 1 << 0,
	GS_PLUGIN_UPDATE_APPS_FLAGS_NO_DOWNLOAD = 1 << 1,
	GS_PLUGIN_UPDATE_APPS_FLAGS_NO_APPLY = 1 << 2,
} GsPluginUpdateAppsFlags;

/**
 * GsPluginProgressCallback:
 * @plugin: the #GsPlugin reporting its progress
 * @progress: the percentage completion (0–100 inclusive), or
 *   %GS_APP_PROGRESS_UNKNOWN for unknown
 * @user_data: user data passed to the calling function
 *
 * Callback to report the progress of a particular @plugin through a particular
 * operation.
 *
 * Since: 44
 */
typedef void (* GsPluginProgressCallback)		(GsPlugin	*plugin,
							 guint		 progress,
							 gpointer	 user_data);

/**
 * GsPluginAppNeedsUserActionCallback:
 * @plugin: the #GsPlugin asking for user action
 * @app: (nullable) (transfer none): the related #GsApp, or %NULL if no app is
 *   explicitly related to the necessary user action
 * @action_screenshot: (nullable) (transfer none): a screenshot (with caption
 *   set) which gives the user instructions about what action to take, or %NULL
 *   if no instructions are available
 * @user_data: user data passed to the calling function
 *
 * Callback to ask the user to perform a physical action during a plugin
 * operation.
 *
 * This will typically be something like unplugging and reconnecting a hardware
 * device, and instructions will be given via @action_screenshot.
 *
 * Since: 44
 */
typedef void (* GsPluginAppNeedsUserActionCallback)	(GsPlugin	*plugin,
							 GsApp		*app,
							 AsScreenshot	*action_screenshot,
							 gpointer	 user_data);

/**
 * GsPluginRule:
 * @GS_PLUGIN_RULE_CONFLICTS:		The plugin conflicts with another
 * @GS_PLUGIN_RULE_RUN_AFTER:		Order the plugin after another
 * @GS_PLUGIN_RULE_RUN_BEFORE:		Order the plugin before another
 * @GS_PLUGIN_RULE_BETTER_THAN:		Results are better than another
 *
 * The rules used for ordering plugins.
 * Plugins are expected to add rules in the init function for their #GsPlugin
 * subclass.
 **/
typedef enum {
	GS_PLUGIN_RULE_CONFLICTS,
	GS_PLUGIN_RULE_RUN_AFTER,
	GS_PLUGIN_RULE_RUN_BEFORE,
	GS_PLUGIN_RULE_BETTER_THAN,
	GS_PLUGIN_RULE_LAST  /*< skip >*/
} GsPluginRule;

/**
 * GsPluginAction:
 * @GS_PLUGIN_ACTION_UNKNOWN:			Action is unknown
 * @GS_PLUGIN_ACTION_INSTALL:			Install an app
 * @GS_PLUGIN_ACTION_REMOVE:			Remove an app
 * @GS_PLUGIN_ACTION_UPGRADE_DOWNLOAD:		Download a distro upgrade
 * @GS_PLUGIN_ACTION_UPGRADE_TRIGGER:		Trigger a distro upgrade
 * @GS_PLUGIN_ACTION_LAUNCH:			Launch an app
 * @GS_PLUGIN_ACTION_UPDATE_CANCEL:		Cancel the update
 * @GS_PLUGIN_ACTION_GET_UPDATES:		Get the list of updates
 * @GS_PLUGIN_ACTION_GET_SOURCES:		Get the list of sources
 * @GS_PLUGIN_ACTION_FILE_TO_APP:		Convert the file to an app
 * @GS_PLUGIN_ACTION_URL_TO_APP:		Convert the URI to an app
 * @GS_PLUGIN_ACTION_GET_UPDATES_HISTORICAL:    Get the list of historical updates
 * @GS_PLUGIN_ACTION_GET_LANGPACKS:		Get appropriate language pack
 * @GS_PLUGIN_ACTION_INSTALL_REPO:		Install a repository (Since: 41)
 * @GS_PLUGIN_ACTION_REMOVE_REPO:		Remove a repository (Since: 41)
 * @GS_PLUGIN_ACTION_ENABLE_REPO:		Enable a repository (Since: 41)
 * @GS_PLUGIN_ACTION_DISABLE_REPO:		Disable a repository (Since: 41)
 *
 * The plugin action.
 **/
typedef enum {
	GS_PLUGIN_ACTION_UNKNOWN,
	GS_PLUGIN_ACTION_INSTALL,
	GS_PLUGIN_ACTION_REMOVE,
	GS_PLUGIN_ACTION_UPGRADE_DOWNLOAD,
	GS_PLUGIN_ACTION_UPGRADE_TRIGGER,
	GS_PLUGIN_ACTION_LAUNCH,
	GS_PLUGIN_ACTION_UPDATE_CANCEL,
	GS_PLUGIN_ACTION_GET_UPDATES,
	GS_PLUGIN_ACTION_GET_SOURCES,
	GS_PLUGIN_ACTION_FILE_TO_APP,
	GS_PLUGIN_ACTION_URL_TO_APP,
	GS_PLUGIN_ACTION_GET_UPDATES_HISTORICAL,
	GS_PLUGIN_ACTION_GET_LANGPACKS,
	GS_PLUGIN_ACTION_INSTALL_REPO,
	GS_PLUGIN_ACTION_REMOVE_REPO,
	GS_PLUGIN_ACTION_ENABLE_REPO,
	GS_PLUGIN_ACTION_DISABLE_REPO,
	GS_PLUGIN_ACTION_LAST  /*< skip >*/
} GsPluginAction;

G_END_DECLS
