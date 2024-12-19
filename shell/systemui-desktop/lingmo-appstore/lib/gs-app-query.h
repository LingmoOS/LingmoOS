/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2022 Endless OS Foundation LLC
 *
 * Author: Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "gs-app-list.h"
#include "gs-category.h"
#include "gs-plugin-types.h"

G_BEGIN_DECLS

/**
 * GsAppQueryTristate:
 * @GS_APP_QUERY_TRISTATE_UNSET: Value is unset.
 * @GS_APP_QUERY_TRISTATE_FALSE: False. Equal in value to %FALSE.
 * @GS_APP_QUERY_TRISTATE_TRUE: True. Equal in value to %TRUE.
 *
 * A type for storing a boolean value which can also have an ‘unknown’ or
 * ‘unset’ state.
 *
 * Within #GsAppQuery this is used for boolean query properties which are unset
 * by default so that they don’t affect the query.
 *
 * Since: 43
 */
typedef enum
{
	GS_APP_QUERY_TRISTATE_UNSET = -1,
	GS_APP_QUERY_TRISTATE_FALSE = 0,
	GS_APP_QUERY_TRISTATE_TRUE = 1,
} GsAppQueryTristate;

/**
 * GsAppQueryProvidesType:
 * @GS_APP_QUERY_PROVIDES_UNKNOWN: Format is unknown and value is unset.
 * @GS_APP_QUERY_PROVIDES_PACKAGE_NAME: A package name in whatever ID format is
 *   used natively by the current distro.
 * @GS_APP_QUERY_PROVIDES_GSTREAMER: A GStreamer plugin name which the app must
 *   provide.
 * @GS_APP_QUERY_PROVIDES_FONT: A font name which the app must provide.
 * @GS_APP_QUERY_PROVIDES_MIME_HANDLER: A MIME type/content type which the app
 *   must support.
 * @GS_APP_QUERY_PROVIDES_PS_DRIVER: A printer/PostScript driver which the app
 *   must provide.
 * @GS_APP_QUERY_PROVIDES_PLASMA: A Plasma ID which the app must provide.
 *   (FIXME: It’s not really clear what this means, but it’s historically been
 *   supported.)
 *
 * A type for identifying the format or meaning of #GsAppQuery:provides-tag.
 *
 * This allows querying for apps which provide various types of functionality,
 * such as printer drivers or fonts.
 *
 * Since: 43
 */
typedef enum {
	GS_APP_QUERY_PROVIDES_UNKNOWN = 0,
	GS_APP_QUERY_PROVIDES_PACKAGE_NAME,
	GS_APP_QUERY_PROVIDES_GSTREAMER,
	GS_APP_QUERY_PROVIDES_FONT,
	GS_APP_QUERY_PROVIDES_MIME_HANDLER,
	GS_APP_QUERY_PROVIDES_PS_DRIVER,
	GS_APP_QUERY_PROVIDES_PLASMA,
} GsAppQueryProvidesType;

/**
 * GsAppQueryLicenseType:
 * @GS_APP_QUERY_LICENSE_ANY: Any license, proprietary or free
 * @GS_APP_QUERY_LICENSE_FOSS: Only free licenses (FOSS or open source)
 *
 * A type for categorising licenses, so that apps can be filtered by the type of
 * license they have.
 *
 * Since: 44
 */
typedef enum {
	GS_APP_QUERY_LICENSE_ANY,
	GS_APP_QUERY_LICENSE_FOSS,
} GsAppQueryLicenseType;

/**
 * GsAppQueryDeveloperVerifiedType:
 * @GS_APP_QUERY_DEVELOPER_VERIFIED_ANY: Any app, with or without verified developer identity
 * @GS_APP_QUERY_DEVELOPER_VERIFIED_ONLY: Only apps with verified developer identity
 *
 * A type to filter apps by developer verified identity.
 *
 * Since: 46
 */
typedef enum {
	GS_APP_QUERY_DEVELOPER_VERIFIED_ANY,
	GS_APP_QUERY_DEVELOPER_VERIFIED_ONLY,
} GsAppQueryDeveloperVerifiedType;

#define GS_TYPE_APP_QUERY (gs_app_query_get_type ())

G_DECLARE_FINAL_TYPE (GsAppQuery, gs_app_query, GS, APP_QUERY, GObject)

GsAppQuery	*gs_app_query_new	(const gchar *first_property_name,
					 ...) G_GNUC_NULL_TERMINATED;

GsPluginRefineFlags	 gs_app_query_get_refine_flags	(GsAppQuery *self);
guint			 gs_app_query_get_max_results	(GsAppQuery *self);
GsAppListFilterFlags	 gs_app_query_get_dedupe_flags	(GsAppQuery *self);
GsAppListSortFunc	 gs_app_query_get_sort_func	(GsAppQuery *self,
							 gpointer   *user_data_out);
GsAppListFilterFunc	 gs_app_query_get_filter_func	(GsAppQuery *self,
							 gpointer   *user_data_out);

guint			 gs_app_query_get_n_properties_set (GsAppQuery *self);

const gchar * const	*gs_app_query_get_provides_files (GsAppQuery *self);
GDateTime		*gs_app_query_get_released_since (GsAppQuery *self);
GsAppQueryTristate	 gs_app_query_get_is_curated	 (GsAppQuery *self);
GsAppQueryTristate	 gs_app_query_get_is_featured	 (GsAppQuery *self);
GsCategory		*gs_app_query_get_category	 (GsAppQuery *self);
GsAppQueryTristate	 gs_app_query_get_is_installed	 (GsAppQuery *self);
const gchar * const	*gs_app_query_get_deployment_featured
							 (GsAppQuery *self);
const gchar * const	*gs_app_query_get_developers	 (GsAppQuery *self);
const gchar * const	*gs_app_query_get_keywords	 (GsAppQuery *self);
GsApp			*gs_app_query_get_alternate_of	 (GsAppQuery *self);
GsAppQueryProvidesType	 gs_app_query_get_provides	 (GsAppQuery *self,
							  const gchar **out_provides_tag);
GsAppQueryLicenseType	 gs_app_query_get_license_type	 (GsAppQuery *self);
GsAppQueryDeveloperVerifiedType
			 gs_app_query_get_developer_verified_type
							 (GsAppQuery *self);

G_END_DECLS
