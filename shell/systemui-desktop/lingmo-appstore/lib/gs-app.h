/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2018 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2014-2018 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib-object.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libsoup/soup.h>
#include <appstream.h>

#include <gs-app-permissions.h>

G_BEGIN_DECLS

/* Dependency loop means we can’t include the header. */
typedef struct _GsPlugin GsPlugin;
typedef struct _GsAppList GsAppList;

#define GS_TYPE_APP (gs_app_get_type ())

G_DECLARE_DERIVABLE_TYPE (GsApp, gs_app, GS, APP, GObject)

struct _GsAppClass
{
	GObjectClass		 parent_class;
	void			 (*to_string)	(GsApp		*app,
						 GString	*str);
	gpointer		 padding[30];
};

/**
 * GsAppState:
 * @GS_APP_STATE_UNKNOWN:			Unknown state
 * @GS_APP_STATE_INSTALLED:			Application is installed
 * @GS_APP_STATE_AVAILABLE:			Application is available
 * @GS_APP_STATE_AVAILABLE_LOCAL:		Application is locally available as a file
 * @GS_APP_STATE_UPDATABLE:			Application is installed and updatable
 * @GS_APP_STATE_UNAVAILABLE:			Application is referenced, but not available
 * @GS_APP_STATE_QUEUED_FOR_INSTALL:		Application is queued for install
 * @GS_APP_STATE_INSTALLING:			Application is being installed
 * @GS_APP_STATE_REMOVING:			Application is being removed
 * @GS_APP_STATE_UPDATABLE_LIVE:		Application is installed and updatable live
 * @GS_APP_STATE_PURCHASABLE:			Application is available for purchasing
 * @GS_APP_STATE_PURCHASING:			Application is being purchased
 * @GS_APP_STATE_PENDING_INSTALL:		Application is installed, but may have pending some actions,
 *						like restart, to finish it
 * @GS_APP_STATE_PENDING_REMOVE:		Application is removed, but may have pending some actions,
 *						like restart, to finish it
 * @GS_APP_STATE_DOWNLOADING:			Application is being downloaded
 *
 * The application state.
 **/
typedef enum {
	GS_APP_STATE_UNKNOWN,				/* Since: 0.2.2 */
	GS_APP_STATE_INSTALLED,				/* Since: 0.2.2 */
	GS_APP_STATE_AVAILABLE,				/* Since: 0.2.2 */
	GS_APP_STATE_AVAILABLE_LOCAL,			/* Since: 0.2.2 */
	GS_APP_STATE_UPDATABLE,				/* Since: 0.2.2 */
	GS_APP_STATE_UNAVAILABLE,			/* Since: 0.2.2 */
	GS_APP_STATE_QUEUED_FOR_INSTALL,		/* Since: 0.2.2 */
	GS_APP_STATE_INSTALLING,			/* Since: 0.2.2 */
	GS_APP_STATE_REMOVING,				/* Since: 0.2.2 */
	GS_APP_STATE_UPDATABLE_LIVE,			/* Since: 0.5.4 */
	GS_APP_STATE_PURCHASABLE,			/* Since: 0.5.17 */
	GS_APP_STATE_PURCHASING,			/* Since: 0.5.17 */
	GS_APP_STATE_PENDING_INSTALL,			/* Since: 41 */
	GS_APP_STATE_PENDING_REMOVE,			/* Since: 41 */
	GS_APP_STATE_DOWNLOADING,			/* Since: 46 */
	GS_APP_STATE_LAST  /*< skip >*/
} GsAppState;

/**
 * GsAppSpecialKind:
 * @GS_APP_SPECIAL_KIND_NONE:			No special occupation
 * @GS_APP_SPECIAL_KIND_OS_UPDATE:		Application represents an OS update
 *
 * A special occupation for #GsApp. #AsComponentKind can not represent certain
 * LINGMO Appstore specific features, like representing a #GsApp as OS updates
 * which have no associated AppStream entry.
 * They are represented by a #GsApp of kind %AS_COMPONENT_KIND_GENERIC and a value
 * from #GsAppSpecialKind. which does not match any AppStream component type.
 **/
typedef enum {
	GS_APP_SPECIAL_KIND_NONE,		/* Since: 40 */
	GS_APP_SPECIAL_KIND_OS_UPDATE,		/* Since: 40 */
} GsAppSpecialKind;

/**
 * GsAppKudo:
 * @GS_APP_KUDO_MY_LANGUAGE:		Localised in my language
 * @GS_APP_KUDO_RECENT_RELEASE:		Released recently
 * @GS_APP_KUDO_FEATURED_RECOMMENDED:	Chosen for the front page
 * @GS_APP_KUDO_HAS_KEYWORDS:		Has at least 1 keyword
 * @GS_APP_KUDO_HAS_SCREENSHOTS:	Supplies screenshots
 * @GS_APP_KUDO_HI_DPI_ICON:		Installs a HiDPI icon
 * @GS_APP_KUDO_SANDBOXED:		Application is sandboxed
 * @GS_APP_KUDO_SANDBOXED_SECURE:	Application is sandboxed securely
 *
 * Any awards given to the application.
 **/
typedef enum {
	GS_APP_KUDO_MY_LANGUAGE			= 1 << 0,
	GS_APP_KUDO_RECENT_RELEASE		= 1 << 1,
	GS_APP_KUDO_FEATURED_RECOMMENDED	= 1 << 2,
	GS_APP_KUDO_HAS_KEYWORDS		= 1 << 7,
	GS_APP_KUDO_HAS_SCREENSHOTS		= 1 << 9,
	GS_APP_KUDO_HI_DPI_ICON			= 1 << 14,
	GS_APP_KUDO_SANDBOXED			= 1 << 15,
	GS_APP_KUDO_SANDBOXED_SECURE		= 1 << 16,
	GS_APP_KUDO_LAST  /*< skip >*/
} GsAppKudo;

/**
 * GsAppQuirk:
 * @GS_APP_QUIRK_NONE:			No special attributes
 * @GS_APP_QUIRK_PROVENANCE:		Installed by OS vendor
 * @GS_APP_QUIRK_COMPULSORY:		Cannot be removed
 * @GS_APP_QUIRK_HAS_SOURCE:		Has a source to allow staying up-to-date
 * @GS_APP_QUIRK_IS_WILDCARD:		Matches applications from any plugin
 * @GS_APP_QUIRK_NEEDS_REBOOT:		A reboot is required after the action
 * @GS_APP_QUIRK_NOT_REVIEWABLE:	The app is not reviewable
 * @GS_APP_QUIRK_NOT_LAUNCHABLE:	The app is not launchable (run-able)
 * @GS_APP_QUIRK_NEEDS_USER_ACTION:	The component requires some kind of user action
 * @GS_APP_QUIRK_IS_PROXY:		Is a proxy app that operates on other applications
 * @GS_APP_QUIRK_UNUSABLE_DURING_UPDATE:The device is unusable whilst the action is performed
 * @GS_APP_QUIRK_DEVELOPER_VERIFIED:	The app developer has been verified
 * @GS_APP_QUIRK_PARENTAL_FILTER:	The app has been filtered by parental controls, and should be hidden
 * @GS_APP_QUIRK_NEW_PERMISSIONS:	The update requires new permissions
 * @GS_APP_QUIRK_PARENTAL_NOT_LAUNCHABLE:	The app cannot be run by the current user due to parental controls, and should not be launchable
 * @GS_APP_QUIRK_HIDE_FROM_SEARCH:	The app should not be shown in search results
 * @GS_APP_QUIRK_HIDE_EVERYWHERE:	The app should not be shown anywhere (it’s blocklisted)
 * @GS_APP_QUIRK_DO_NOT_AUTO_UPDATE:	The app should not be automatically updated
 * @GS_APP_QUIRK_DEVELOPMENT_SOURCE:	The app is from a development source (Since: 43)
 *
 * The application attributes.
 **/
typedef enum {
	GS_APP_QUIRK_NONE		= 0,		/* Since: 3.32 */
	GS_APP_QUIRK_PROVENANCE		= 1 << 0,	/* Since: 3.32 */
	GS_APP_QUIRK_COMPULSORY		= 1 << 1,	/* Since: 3.32 */
	GS_APP_QUIRK_HAS_SOURCE		= 1 << 2,	/* Since: 3.32 */
	GS_APP_QUIRK_IS_WILDCARD	= 1 << 3,	/* Since: 3.32 */
	GS_APP_QUIRK_NEEDS_REBOOT	= 1 << 4,	/* Since: 3.32 */
	GS_APP_QUIRK_NOT_REVIEWABLE	= 1 << 5,	/* Since: 3.32 */
	/* there’s a hole here where GS_APP_QUIRK_HAS_SHORTCUT used to be */
	GS_APP_QUIRK_NOT_LAUNCHABLE	= 1 << 7,	/* Since: 3.32 */
	GS_APP_QUIRK_NEEDS_USER_ACTION	= 1 << 8,	/* Since: 3.32 */
	GS_APP_QUIRK_IS_PROXY 		= 1 << 9,	/* Since: 3.32 */
	GS_APP_QUIRK_UNUSABLE_DURING_UPDATE	= 1 << 10,	/* Since: 44 */
	GS_APP_QUIRK_DEVELOPER_VERIFIED	= 1 << 11,	/* Since: 3.32 */
	GS_APP_QUIRK_PARENTAL_FILTER	= 1 << 12,	/* Since: 3.32 */
	GS_APP_QUIRK_NEW_PERMISSIONS	= 1 << 13,	/* Since: 3.32 */
	GS_APP_QUIRK_PARENTAL_NOT_LAUNCHABLE	= 1 << 14,	/* Since: 3.32 */
	GS_APP_QUIRK_HIDE_FROM_SEARCH	= 1 << 15,	/* Since: 3.32 */
	GS_APP_QUIRK_HIDE_EVERYWHERE	= 1 << 16,	/* Since: 3.36 */
	GS_APP_QUIRK_DO_NOT_AUTO_UPDATE	= 1 << 17,	/* Since: 3.36 */
	GS_APP_QUIRK_DEVELOPMENT_SOURCE	= 1 << 18,	/* Since: 43 */
	GS_APP_QUIRK_LAST  /*< skip >*/
} GsAppQuirk;

#define	GS_APP_INSTALL_DATE_UNSET		0
#define	GS_APP_INSTALL_DATE_UNKNOWN		1 /* 1s past the epoch */

/**
 * GsSizeType:
 * @GS_SIZE_TYPE_UNKNOWN:	Size is unknown
 * @GS_SIZE_TYPE_UNKNOWABLE:	Size is unknown and is impossible to calculate
 * @GS_SIZE_TYPE_VALID:		Size is known and valid
 *
 * Types of download or file size for applications.
 *
 * These are used to represent the validity of properties like
 * #GsApp:size-download.
 *
 * Since: 43
 */
typedef enum {
	GS_SIZE_TYPE_UNKNOWN,
	GS_SIZE_TYPE_UNKNOWABLE,
	GS_SIZE_TYPE_VALID,
} GsSizeType;

/**
 * GsAppQuality:
 * @GS_APP_QUALITY_UNKNOWN:	The quality value is unknown
 * @GS_APP_QUALITY_LOWEST:	Lowest quality
 * @GS_APP_QUALITY_NORMAL:	Normal quality
 * @GS_APP_QUALITY_HIGHEST:	Highest quality
 *
 * Any awards given to the application.
 **/
typedef enum {
	GS_APP_QUALITY_UNKNOWN,
	GS_APP_QUALITY_LOWEST,
	GS_APP_QUALITY_NORMAL,
	GS_APP_QUALITY_HIGHEST,
	GS_APP_QUALITY_LAST  /*< skip >*/
} GsAppQuality;

/**
 * GsAppIconsState:
 * @GS_APP_ICONS_STATE_UNKNOWN:		The state of the icons is unknown
 * @GS_APP_ICONS_STATE_PENDING_DOWNLOAD:	Icons are in queue to be downloaded
 * @GS_APP_ICONS_STATE_DOWNLOADING:	Icons are downloading
 * @GS_APP_ICONS_STATE_AVAILABLE:	Icons are available
 *
 * State of the icons of the application.
 *
 * Since: 44
 **/
typedef enum {
	GS_APP_ICONS_STATE_UNKNOWN,
	GS_APP_ICONS_STATE_PENDING_DOWNLOAD,
	GS_APP_ICONS_STATE_DOWNLOADING,
	GS_APP_ICONS_STATE_AVAILABLE,
} GsAppIconsState;

/**
 * GS_APP_PROGRESS_UNKNOWN:
 *
 * A value returned by gs_app_get_progress() if the app’s progress is unknown
 * or has a wide confidence interval. Typically this would be represented in the
 * UI using a pulsing progress bar or spinner.
 *
 * Since: 3.38
 */
#define GS_APP_PROGRESS_UNKNOWN G_MAXUINT

const gchar 	*gs_app_state_to_string (GsAppState state);

GsApp		*gs_app_new			(const gchar	*id);
G_DEPRECATED_FOR(gs_app_set_from_unique_id)
GsApp		*gs_app_new_from_unique_id	(const gchar	*unique_id);
void		 gs_app_set_from_unique_id	(GsApp		*app,
						 const gchar	*unique_id,
						 AsComponentKind kind);
gchar		*gs_app_to_string		(GsApp		*app);
void		 gs_app_to_string_append	(GsApp		*app,
						 GString	*str);

const gchar	*gs_app_get_id			(GsApp		*app);
void		 gs_app_set_id			(GsApp		*app,
						 const gchar	*id);
AsComponentKind	 gs_app_get_kind		(GsApp		*app);
void		 gs_app_set_kind		(GsApp		*app,
						 AsComponentKind kind);
GsAppState	 gs_app_get_state		(GsApp		*app);
void		 gs_app_set_state		(GsApp		*app,
						 GsAppState	 state);
AsComponentScope gs_app_get_scope		(GsApp		*app);
void		 gs_app_set_scope		(GsApp		*app,
						 AsComponentScope scope);
AsBundleKind	 gs_app_get_bundle_kind		(GsApp		*app);
void		 gs_app_set_bundle_kind		(GsApp		*app,
						 AsBundleKind	 bundle_kind);
GsAppSpecialKind gs_app_get_special_kind	(GsApp		*app);
void		 gs_app_set_special_kind	(GsApp		*app,
						 GsAppSpecialKind kind);
void		 gs_app_set_state_recover	(GsApp		*app);
guint		 gs_app_get_progress		(GsApp		*app);
void		 gs_app_set_progress		(GsApp		*app,
						 guint		 percentage);
gboolean	gs_app_get_allow_cancel		(GsApp	*app);
void		gs_app_set_allow_cancel		 (GsApp	*app,
						  gboolean	allow_cancel);
const gchar	*gs_app_get_unique_id		(GsApp		*app);
const gchar	*gs_app_get_branch		(GsApp		*app);
void		 gs_app_set_branch		(GsApp		*app,
						 const gchar	*branch);
const gchar	*gs_app_get_name		(GsApp		*app);
void		 gs_app_set_name		(GsApp		*app,
						 GsAppQuality	 quality,
						 const gchar	*name);
const gchar	*gs_app_get_renamed_from	(GsApp		*app);
void		 gs_app_set_renamed_from	(GsApp		*app,
						 const gchar	*renamed_from);
const gchar	*gs_app_get_source_default	(GsApp		*app);
void		 gs_app_add_source		(GsApp		*app,
						 const gchar	*source);
GPtrArray	*gs_app_get_sources		(GsApp		*app);
void		 gs_app_set_sources		(GsApp		*app,
						 GPtrArray	*sources);
const gchar	*gs_app_get_source_id_default	(GsApp		*app);
void		 gs_app_add_source_id		(GsApp		*app,
						 const gchar	*source_id);
GPtrArray	*gs_app_get_source_ids		(GsApp		*app);
void		 gs_app_set_source_ids		(GsApp		*app,
						 GPtrArray	*source_ids);
void		 gs_app_clear_source_ids	(GsApp		*app);
const gchar	*gs_app_get_project_group	(GsApp		*app);
void		 gs_app_set_project_group	(GsApp		*app,
						 const gchar	*project_group);
const gchar	*gs_app_get_developer_name	(GsApp		*app);
void		 gs_app_set_developer_name	(GsApp		*app,
						 const gchar	*developer_name);
const gchar	*gs_app_get_agreement		(GsApp		*app);
void		 gs_app_set_agreement		(GsApp		*app,
						 const gchar	*agreement);
const gchar	*gs_app_get_version		(GsApp		*app);
const gchar	*gs_app_get_version_ui		(GsApp		*app);
void		 gs_app_set_version		(GsApp		*app,
						 const gchar	*version);
const gchar	*gs_app_get_summary		(GsApp		*app);
void		 gs_app_set_summary		(GsApp		*app,
						 GsAppQuality	 quality,
						 const gchar	*summary);
const gchar	*gs_app_get_summary_missing	(GsApp		*app);
void		 gs_app_set_summary_missing	(GsApp		*app,
						 const gchar	*summary_missing);
const gchar	*gs_app_get_description		(GsApp		*app);
void		 gs_app_set_description		(GsApp		*app,
						 GsAppQuality	 quality,
						 const gchar	*description);
const gchar	*gs_app_get_url			(GsApp		*app,
						 AsUrlKind	 kind);
void		 gs_app_set_url			(GsApp		*app,
						 AsUrlKind	 kind,
						 const gchar	*url);
const gchar	*gs_app_get_url_missing		(GsApp		*app);
void		 gs_app_set_url_missing		(GsApp		*app,
						 const gchar	*url);
const gchar	*gs_app_get_launchable		(GsApp		*app,
						 AsLaunchableKind kind);
void		 gs_app_set_launchable		(GsApp		*app,
						 AsLaunchableKind kind,
						 const gchar	*launchable);
const gchar	*gs_app_get_license		(GsApp		*app);
gboolean	 gs_app_get_license_is_free	(GsApp		*app);
void		 gs_app_set_license		(GsApp		*app,
						 GsAppQuality	 quality,
						 const gchar	*license);
gchar		**gs_app_get_menu_path		(GsApp		*app);
void		 gs_app_set_menu_path		(GsApp		*app,
						 gchar		**menu_path);
const gchar	*gs_app_get_origin		(GsApp		*app);
void		 gs_app_set_origin		(GsApp		*app,
						 const gchar	*origin);
const gchar	*gs_app_get_origin_appstream	(GsApp		*app);
void		 gs_app_set_origin_appstream	(GsApp		*app,
						 const gchar	*origin_appstream);
const gchar	*gs_app_get_origin_hostname	(GsApp		*app);
void		 gs_app_set_origin_hostname	(GsApp		*app,
						 const gchar	*origin_hostname);
GPtrArray	*gs_app_get_screenshots		(GsApp		*app);
void		 gs_app_add_screenshot		(GsApp		*app,
						 AsScreenshot	*screenshot);
AsScreenshot	*gs_app_get_action_screenshot	(GsApp		*app);
void		 gs_app_set_action_screenshot	(GsApp		*app,
						 AsScreenshot	*screenshot);
const gchar	*gs_app_get_update_version	(GsApp		*app);
const gchar	*gs_app_get_update_version_ui	(GsApp		*app);
void		 gs_app_set_update_version	(GsApp		*app,
						 const gchar	*update_version);
const gchar	*gs_app_get_update_details_markup
						(GsApp		*app);
void		 gs_app_set_update_details_markup
						(GsApp		*app,
						 const gchar	*markup);
void		 gs_app_set_update_details_text	(GsApp		*app,
						 const gchar	*text);
gboolean	 gs_app_get_update_details_set	(GsApp		*app);
AsUrgencyKind	 gs_app_get_update_urgency	(GsApp		*app);
void		 gs_app_set_update_urgency	(GsApp		*app,
						 AsUrgencyKind	 update_urgency);
GsPlugin	*gs_app_dup_management_plugin	(GsApp		*app);
gboolean	 gs_app_has_management_plugin	(GsApp		*app,
						 GsPlugin	*plugin);
void		 gs_app_set_management_plugin	(GsApp		*app,
						 GsPlugin	*management_plugin);
GIcon		*gs_app_get_icon_for_size	(GsApp		*app,
						 guint		 size,
						 guint		 scale,
						 const gchar	*fallback_icon_name);
G_DEPRECATED_FOR(gs_app_dup_icons)
GPtrArray	*gs_app_get_icons		(GsApp		*app);
GPtrArray	*gs_app_dup_icons		(GsApp		*app);
gboolean	 gs_app_has_icons		(GsApp		*app);
void		 gs_app_add_icon		(GsApp		*app,
						 GIcon		*icon);
void		 gs_app_remove_all_icons	(GsApp		*app);
GFile		*gs_app_get_local_file		(GsApp		*app);
void		 gs_app_set_local_file		(GsApp		*app,
						 GFile		*local_file);
AsContentRating	*gs_app_dup_content_rating	(GsApp		*app);
void		 gs_app_set_content_rating	(GsApp		*app,
						 AsContentRating *content_rating);
GsApp		*gs_app_get_runtime		(GsApp		*app);
void		 gs_app_set_runtime		(GsApp		*app,
						 GsApp		*runtime);
const gchar	*gs_app_get_metadata_item	(GsApp		*app,
						 const gchar	*key);
GVariant	*gs_app_get_metadata_variant	(GsApp		*app,
						 const gchar	*key);
void		 gs_app_set_metadata		(GsApp		*app,
						 const gchar	*key,
						 const gchar	*value);
void		 gs_app_set_metadata_variant	(GsApp		*app,
						 const gchar	*key,
						 GVariant	*value);
gint		 gs_app_get_rating		(GsApp		*app);
void		 gs_app_set_rating		(GsApp		*app,
						 gint		 rating);
GArray		*gs_app_get_review_ratings	(GsApp		*app);
void		 gs_app_set_review_ratings	(GsApp		*app,
						 GArray		*review_ratings);
GPtrArray	*gs_app_get_reviews		(GsApp		*app);
void		 gs_app_add_review		(GsApp		*app,
						 AsReview	*review);
void		 gs_app_remove_review		(GsApp		*app,
						 AsReview	*review);
GPtrArray	*gs_app_get_provided		(GsApp		*app);
AsProvided	*gs_app_get_provided_for_kind	(GsApp		*app,
						 AsProvidedKind kind);
void		 gs_app_add_provided_item	(GsApp		*app,
						 AsProvidedKind kind,
						 const gchar	*item);
GsSizeType	 gs_app_get_size_installed	(GsApp		*app,
						 guint64	*size_bytes_out);
void		 gs_app_set_size_installed	(GsApp		*app,
						 GsSizeType	 size_type,
						 guint64	 size_bytes);
GsSizeType	 gs_app_get_size_installed_dependencies
						(GsApp		*app,
						 guint64	*size_bytes_out);
GsSizeType	 gs_app_get_size_user_data	(GsApp		*app,
						 guint64	*size_bytes_out);
void		 gs_app_set_size_user_data	(GsApp		*app,
						 GsSizeType	 size_type,
						 guint64	 size_bytes);
GsSizeType	 gs_app_get_size_cache_data	(GsApp		*app,
						 guint64	*size_bytes_out);
void		 gs_app_set_size_cache_data	(GsApp		*app,
						 GsSizeType	 size_type,
						 guint64	 size_bytes);
GsSizeType	 gs_app_get_size_download	(GsApp		*app,
						 guint64	*size_bytes_out);
void		 gs_app_set_size_download	(GsApp		*app,
						 GsSizeType	 size_type,
						 guint64	 size_bytes);
GsSizeType	 gs_app_get_size_download_dependencies
						(GsApp		*app,
						 guint64	*size_bytes_out);
void		 gs_app_add_related		(GsApp		*app,
						 GsApp		*app2);
void		 gs_app_add_addons		(GsApp		*app,
						 GsAppList	*addons);
void		 gs_app_add_history		(GsApp		*app,
						 GsApp		*app2);
guint64		 gs_app_get_install_date	(GsApp		*app);
void		 gs_app_set_install_date	(GsApp		*app,
						 guint64	 install_date);
guint64		 gs_app_get_release_date	(GsApp		*app);
void		 gs_app_set_release_date	(GsApp		*app,
						 guint64	 release_date);
GPtrArray	*gs_app_get_categories		(GsApp		*app);
void		 gs_app_set_categories		(GsApp		*app,
						 GPtrArray	*categories);
GArray		*gs_app_get_key_colors		(GsApp		*app);
void		 gs_app_set_key_colors		(GsApp		*app,
						 GArray		*key_colors);
void		 gs_app_add_key_color		(GsApp		*app,
						 GdkRGBA	*key_color);
gboolean	gs_app_get_user_key_colors	(GsApp		*app);

G_DEPRECATED
void            gs_app_set_is_update_downloaded (GsApp         *app,
                                                 gboolean       is_update_downloaded);
G_DEPRECATED
gboolean        gs_app_get_is_update_downloaded (GsApp         *app);

gboolean	 gs_app_has_category		(GsApp		*app,
						 const gchar	*category);
void		 gs_app_add_category		(GsApp		*app,
						 const gchar	*category);
gboolean	 gs_app_remove_category		(GsApp		*app,
						 const gchar	*category);
void		 gs_app_add_kudo		(GsApp		*app,
						 GsAppKudo	 kudo);
void		 gs_app_remove_kudo		(GsApp		*app,
						 GsAppKudo	 kudo);
gboolean	 gs_app_has_kudo		(GsApp		*app,
						 GsAppKudo	 kudo);
guint64		 gs_app_get_kudos		(GsApp		*app);
guint		 gs_app_get_kudos_percentage	(GsApp		*app);
gboolean	 gs_app_get_to_be_installed	(GsApp		*app);
void		 gs_app_set_to_be_installed	(GsApp		*app,
						 gboolean	 to_be_installed);
void		 gs_app_set_match_value		(GsApp		*app,
						 guint		 match_value);
guint		 gs_app_get_match_value		(GsApp		*app);

gboolean	 gs_app_has_quirk		(GsApp		*app,
						 GsAppQuirk	 quirk);
void		 gs_app_add_quirk		(GsApp		*app,
						 GsAppQuirk	 quirk);
void		 gs_app_remove_quirk		(GsApp		*app,
						 GsAppQuirk	 quirk);
gboolean	 gs_app_is_installed		(GsApp		*app);
gboolean	 gs_app_is_updatable		(GsApp		*app);
gchar		*gs_app_dup_origin_ui		(GsApp		*app,
						 gboolean	 with_packaging_format);
void		 gs_app_set_origin_ui		(GsApp		*app,
						 const gchar	*origin_ui);
gchar		*gs_app_get_packaging_format	(GsApp		*app);
const gchar	*gs_app_get_packaging_format_raw(GsApp *app);
void		 gs_app_subsume_metadata	(GsApp		*app,
						 GsApp		*donor);
GsAppPermissions *
		 gs_app_dup_permissions		(GsApp		*app);
void		 gs_app_set_permissions		(GsApp		*app,
						 GsAppPermissions *permissions);
GsAppPermissions *
		 gs_app_dup_update_permissions	(GsApp		*app);
void		 gs_app_set_update_permissions	(GsApp		*app,
						 GsAppPermissions *update_permissions);
GPtrArray	*gs_app_get_version_history	(GsApp		*app);
void		 gs_app_set_version_history	(GsApp		*app,
						 GPtrArray	*version_history);
void		gs_app_ensure_icons_downloaded	(GsApp		*app,
						 SoupSession	*soup_session,
						 guint		 maximum_icon_size,
						 GCancellable	*cancellable);

GPtrArray	*gs_app_get_relations		(GsApp		*app);
void		 gs_app_add_relation		(GsApp		*app,
						 AsRelation	*relation);
void		 gs_app_set_relations		(GsApp		*app,
						 GPtrArray	*relations);

gboolean	 gs_app_get_has_translations	(GsApp		*app);
void		 gs_app_set_has_translations	(GsApp		*app,
						 gboolean	 has_translations);
gboolean	 gs_app_is_downloaded		(GsApp		*app);

GsAppIconsState	 gs_app_get_icons_state		(GsApp		*app);
gboolean	 gs_app_is_application		(GsApp		*app);

G_END_DECLS
