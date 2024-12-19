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
 * SECTION:gs-app-query
 * @short_description: Immutable representation of a query for apps
 *
 * #GsAppQuery is an object to represent a query for apps.
 *
 * It will typically be used with #GsPluginJobListApps, which searches for
 * matching apps, but it may have multiple consumers. #GsAppQuery only
 * represents the query and does not provide an implementation for executing
 * that query.
 *
 * It is immutable after construction, and hence threadsafe. It may be extended
 * in future by adding more query properties. The existing query properties are
 * conjunctive: results should only be returned which match *all* properties
 * which are set, not _any_ properties which are set.
 *
 * The set of apps returned for the query can be controlled with the
 * #GsAppQuery:refine-flags,
 * #GsAppQuery:max-results and
 * #GsAppQuery:dedupe-flags properties. If `refine-flags` is
 * set, all results must be refined using the given set of refine flags (see
 * #GsPluginJobRefine). `max-results` and `dedupe-flags` are used to limit the
 * set of results.
 *
 * Results must always be processed in this order:
 *  - Filtering using #GsAppQuery:filter-func (and any other custom filter
 *    functions the query executor provides).
 *  - Deduplication using #GsAppQuery:dedupe-flags.
 *  - Sorting using #GsAppQuery:sort-func.
 *  - Truncating result list length to #GsAppQuery:max-results.
 *
 * Since: 43
 */

#include "config.h"

#include <glib.h>
#include <glib-object.h>

#include "gs-app.h"
#include "gs-app-list.h"
#include "gs-app-query.h"
#include "gs-enums.h"
#include "gs-plugin-types.h"
#include "gs-utils.h"

struct _GsAppQuery
{
	GObject parent;

	GsPluginRefineFlags refine_flags;
	guint max_results;
	GsAppListFilterFlags dedupe_flags;

	GsAppListSortFunc sort_func;
	gpointer sort_user_data;
	GDestroyNotify sort_user_data_notify;

	GsAppListFilterFunc filter_func;
	gpointer filter_user_data;
	GDestroyNotify filter_user_data_notify;

	/* This is guaranteed to either be %NULL, or a non-empty array */
	gchar **provides_files;  /* (owned) (nullable) (array zero-terminated=1) */
	GDateTime *released_since;  /* (owned) (nullable) */
	GsAppQueryTristate is_curated;
	GsAppQueryTristate is_featured;
	GsCategory *category;  /* (nullable) (owned) */
	GsAppQueryTristate is_installed;

	/* This is guaranteed to either be %NULL, or a non-empty array */
	gchar **deployment_featured;  /* (owned) (nullable) (array zero-terminated=1) */
	/* This is guaranteed to either be %NULL, or a non-empty array */
	gchar **developers;  /* (owned) (nullable) (array zero-terminated=1) */

	gchar **keywords;  /* (owned) (nullable) (array zero-terminated=1) */
	GsApp *alternate_of;  /* (nullable) (owned) */
	gchar *provides_tag;  /* (owned) (nullable) */
	GsAppQueryProvidesType provides_type;
	GsAppQueryLicenseType license_type;
	GsAppQueryDeveloperVerifiedType developer_verified_type;
};

G_DEFINE_TYPE (GsAppQuery, gs_app_query, G_TYPE_OBJECT)

typedef enum {
	PROP_REFINE_FLAGS = 1,
	PROP_MAX_RESULTS,
	PROP_DEDUPE_FLAGS,
	PROP_SORT_FUNC,
	PROP_SORT_USER_DATA,
	PROP_SORT_USER_DATA_NOTIFY,
	PROP_FILTER_FUNC,
	PROP_FILTER_USER_DATA,
	PROP_FILTER_USER_DATA_NOTIFY,
	PROP_DEPLOYMENT_FEATURED,
	PROP_DEVELOPERS,
	PROP_PROVIDES_FILES,
	PROP_RELEASED_SINCE,
	PROP_IS_CURATED,
	PROP_IS_FEATURED,
	PROP_CATEGORY,
	PROP_IS_INSTALLED,
	PROP_KEYWORDS,
	PROP_ALTERNATE_OF,
	PROP_PROVIDES_TAG,
	PROP_PROVIDES_TYPE,
	PROP_LICENSE_TYPE,
	PROP_DEVELOPER_VERIFIED_TYPE,
} GsAppQueryProperty;

static GParamSpec *props[PROP_DEVELOPER_VERIFIED_TYPE + 1] = { NULL, };

static gchar **
gs_app_query_sanitize_keywords (const gchar * const *terms)
{
	g_autoptr(GStrvBuilder) keywords = NULL;
	gboolean any_added = FALSE;

	if (terms == NULL || terms[0] == NULL)
		return NULL;

	keywords = g_strv_builder_new ();

	/* If the caller already split the terms, then use it as is */
	if (terms[1] != NULL) {
		g_strv_builder_addv (keywords, (const gchar **) terms);
		any_added = TRUE;
	} else {
		g_autofree gchar *term = g_strdup (terms[0]);
		g_strstrip (term);
		if (strchr (term, ' ')) {
			g_auto(GStrv) split = g_strsplit (term, " ", -1);
			for (guint i = 0; split[i] != NULL; i++) {
				gchar *word = g_strstrip (split[i]);
				if (*word != '\0') {
					g_strv_builder_add (keywords, word);
					any_added = TRUE;
				}
			}
		} else if (*term != '\0') {
			g_strv_builder_add (keywords, term);
			any_added = TRUE;
		}
	}

	return any_added ? g_strv_builder_end (keywords) : NULL;
}

static void
gs_app_query_constructed (GObject *object)
{
	GsAppQuery *self = GS_APP_QUERY (object);

	G_OBJECT_CLASS (gs_app_query_parent_class)->constructed (object);

	g_assert ((self->provides_tag != NULL) == (self->provides_type != GS_APP_QUERY_PROVIDES_UNKNOWN));
}

static void
gs_app_query_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
	GsAppQuery *self = GS_APP_QUERY (object);

	switch ((GsAppQueryProperty) prop_id) {
	case PROP_REFINE_FLAGS:
		g_value_set_flags (value, self->refine_flags);
		break;
	case PROP_MAX_RESULTS:
		g_value_set_uint (value, self->max_results);
		break;
	case PROP_DEDUPE_FLAGS:
		g_value_set_flags (value, self->dedupe_flags);
		break;
	case PROP_SORT_FUNC:
		g_value_set_pointer (value, self->sort_func);
		break;
	case PROP_SORT_USER_DATA:
		g_value_set_pointer (value, self->sort_user_data);
		break;
	case PROP_SORT_USER_DATA_NOTIFY:
		g_value_set_pointer (value, self->sort_user_data_notify);
		break;
	case PROP_FILTER_FUNC:
		g_value_set_pointer (value, self->filter_func);
		break;
	case PROP_FILTER_USER_DATA:
		g_value_set_pointer (value, self->filter_user_data);
		break;
	case PROP_FILTER_USER_DATA_NOTIFY:
		g_value_set_pointer (value, self->filter_user_data_notify);
		break;
	case PROP_DEPLOYMENT_FEATURED:
		g_value_set_boxed (value, self->deployment_featured);
		break;
	case PROP_DEVELOPERS:
		g_value_set_boxed (value, self->developers);
		break;
	case PROP_PROVIDES_FILES:
		g_value_set_boxed (value, self->provides_files);
		break;
	case PROP_RELEASED_SINCE:
		g_value_set_boxed (value, self->released_since);
		break;
	case PROP_IS_CURATED:
		g_value_set_enum (value, self->is_curated);
		break;
	case PROP_IS_FEATURED:
		g_value_set_enum (value, self->is_featured);
		break;
	case PROP_CATEGORY:
		g_value_set_object (value, self->category);
		break;
	case PROP_IS_INSTALLED:
		g_value_set_enum (value, self->is_installed);
		break;
	case PROP_KEYWORDS:
		g_value_set_boxed (value, self->keywords);
		break;
	case PROP_ALTERNATE_OF:
		g_value_set_object (value, self->alternate_of);
		break;
	case PROP_PROVIDES_TAG:
		g_value_set_string (value, self->provides_tag);
		break;
	case PROP_PROVIDES_TYPE:
		g_value_set_enum (value, self->provides_type);
		break;
	case PROP_LICENSE_TYPE:
		g_value_set_enum (value, self->license_type);
		break;
	case PROP_DEVELOPER_VERIFIED_TYPE:
		g_value_set_enum (value, self->developer_verified_type);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_app_query_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
	GsAppQuery *self = GS_APP_QUERY (object);

	switch ((GsAppQueryProperty) prop_id) {
	case PROP_REFINE_FLAGS:
		/* Construct only. */
		g_assert (self->refine_flags == 0);
		self->refine_flags = g_value_get_flags (value);
		break;
	case PROP_MAX_RESULTS:
		/* Construct only. */
		g_assert (self->max_results == 0);
		self->max_results = g_value_get_uint (value);
		break;
	case PROP_DEDUPE_FLAGS:
		/* Construct only. */
		g_assert (self->dedupe_flags == 0);
		self->dedupe_flags = g_value_get_flags (value);
		break;
	case PROP_SORT_FUNC:
		/* Construct only. */
		g_assert (self->sort_func == NULL);
		self->sort_func = g_value_get_pointer (value);
		break;
	case PROP_SORT_USER_DATA:
		/* Construct only. */
		g_assert (self->sort_user_data == NULL);
		self->sort_user_data = g_value_get_pointer (value);
		break;
	case PROP_SORT_USER_DATA_NOTIFY:
		/* Construct only. */
		g_assert (self->sort_user_data_notify == NULL);
		self->sort_user_data_notify = g_value_get_pointer (value);
		break;
	case PROP_FILTER_FUNC:
		/* Construct only. */
		g_assert (self->filter_func == NULL);
		self->filter_func = g_value_get_pointer (value);
		break;
	case PROP_FILTER_USER_DATA:
		/* Construct only. */
		g_assert (self->filter_user_data == NULL);
		self->filter_user_data = g_value_get_pointer (value);
		break;
	case PROP_FILTER_USER_DATA_NOTIFY:
		/* Construct only. */
		g_assert (self->filter_user_data_notify == NULL);
		self->filter_user_data_notify = g_value_get_pointer (value);
		break;
	case PROP_DEPLOYMENT_FEATURED:
		/* Construct only. */
		g_assert (self->deployment_featured == NULL);
		self->deployment_featured = g_value_dup_boxed (value);

		/* Squash empty arrays to %NULL. */
		if (self->deployment_featured != NULL && self->deployment_featured[0] == NULL)
			g_clear_pointer (&self->deployment_featured, g_strfreev);

		break;
	case PROP_DEVELOPERS:
		/* Construct only. */
		g_assert (self->developers == NULL);
		self->developers = g_value_dup_boxed (value);

		/* Squash empty arrays to %NULL. */
		if (self->developers != NULL && self->developers[0] == NULL)
			g_clear_pointer (&self->developers, g_strfreev);

		break;
	case PROP_PROVIDES_FILES:
		/* Construct only. */
		g_assert (self->provides_files == NULL);
		self->provides_files = g_value_dup_boxed (value);

		/* Squash empty arrays to %NULL. */
		if (self->provides_files != NULL && self->provides_files[0] == NULL)
			g_clear_pointer (&self->provides_files, g_strfreev);

		break;
	case PROP_RELEASED_SINCE:
		/* Construct only. */
		g_assert (self->released_since == NULL);
		self->released_since = g_value_dup_boxed (value);
		break;
	case PROP_IS_CURATED:
		/* Construct only. */
		g_assert (self->is_curated == GS_APP_QUERY_TRISTATE_UNSET);
		self->is_curated = g_value_get_enum (value);
		break;
	case PROP_IS_FEATURED:
		/* Construct only. */
		g_assert (self->is_featured == GS_APP_QUERY_TRISTATE_UNSET);
		self->is_featured = g_value_get_enum (value);
		break;
	case PROP_CATEGORY:
		/* Construct only. */
		g_assert (self->category == NULL);
		self->category = g_value_dup_object (value);
		break;
	case PROP_IS_INSTALLED:
		/* Construct only. */
		g_assert (self->is_installed == GS_APP_QUERY_TRISTATE_UNSET);
		self->is_installed = g_value_get_enum (value);
		break;
	case PROP_KEYWORDS:
		/* Construct only. */
		g_assert (self->keywords == NULL);
		self->keywords = gs_app_query_sanitize_keywords (g_value_get_boxed (value));
		break;
	case PROP_ALTERNATE_OF:
		/* Construct only. */
		g_assert (self->alternate_of == NULL);
		self->alternate_of = g_value_dup_object (value);
		break;
	case PROP_PROVIDES_TAG:
		/* Construct only. */
		g_assert (self->provides_tag == NULL);
		self->provides_tag = g_value_dup_string (value);
		break;
	case PROP_PROVIDES_TYPE:
		/* Construct only. */
		g_assert (self->provides_type == GS_APP_QUERY_PROVIDES_UNKNOWN);
		self->provides_type = g_value_get_enum (value);
		break;
	case PROP_LICENSE_TYPE:
		/* Construct only. */
		g_assert (self->license_type == GS_APP_QUERY_LICENSE_ANY);
		self->license_type = g_value_get_enum (value);
		break;
	case PROP_DEVELOPER_VERIFIED_TYPE:
		/* Construct only. */
		g_assert (self->developer_verified_type == GS_APP_QUERY_DEVELOPER_VERIFIED_ANY);
		self->developer_verified_type = g_value_get_enum (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_app_query_dispose (GObject *object)
{
	GsAppQuery *self = GS_APP_QUERY (object);

	if (self->sort_user_data_notify != NULL && self->sort_user_data != NULL) {
		self->sort_user_data_notify (g_steal_pointer (&self->sort_user_data));
		self->sort_user_data_notify = NULL;
	}

	if (self->filter_user_data_notify != NULL && self->filter_user_data != NULL) {
		self->filter_user_data_notify (g_steal_pointer (&self->filter_user_data));
		self->filter_user_data_notify = NULL;
	}

	g_clear_object (&self->category);
	g_clear_object (&self->alternate_of);

	G_OBJECT_CLASS (gs_app_query_parent_class)->dispose (object);
}

static void
gs_app_query_finalize (GObject *object)
{
	GsAppQuery *self = GS_APP_QUERY (object);

	g_clear_pointer (&self->deployment_featured, g_strfreev);
	g_clear_pointer (&self->developers, g_strfreev);
	g_clear_pointer (&self->provides_files, g_strfreev);
	g_clear_pointer (&self->released_since, g_date_time_unref);
	g_clear_pointer (&self->keywords, g_strfreev);
	g_clear_pointer (&self->provides_tag, g_free);

	G_OBJECT_CLASS (gs_app_query_parent_class)->finalize (object);
}

static void
gs_app_query_class_init (GsAppQueryClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->constructed = gs_app_query_constructed;
	object_class->get_property = gs_app_query_get_property;
	object_class->set_property = gs_app_query_set_property;
	object_class->dispose = gs_app_query_dispose;
	object_class->finalize = gs_app_query_finalize;

	/**
	 * GsAppQuery:refine-flags:
	 *
	 * Flags to specify how the returned apps must be refined, if at all.
	 *
	 * Since: 43
	 */
	props[PROP_REFINE_FLAGS] =
		g_param_spec_flags ("refine-flags", "Refine Flags",
				    "Flags to specify how the returned apps must be refined, if at all.",
				    GS_TYPE_PLUGIN_REFINE_FLAGS, GS_PLUGIN_REFINE_FLAGS_NONE,
				    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				    G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppQuery:max-results:
	 *
	 * Maximum number of results to return, or 0 for no limit.
	 *
	 * Since: 43
	 */
	props[PROP_MAX_RESULTS] =
		g_param_spec_uint ("max-results", "Max Results",
				   "Maximum number of results to return, or 0 for no limit.",
				   0, G_MAXUINT, 0,
				   G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				   G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppQuery:dedupe-flags:
	 *
	 * Flags to specify how to deduplicate the returned apps, if at all.
	 *
	 * Since: 43
	 */
	props[PROP_DEDUPE_FLAGS] =
		g_param_spec_flags ("dedupe-flags", "Dedupe Flags",
				    "Flags to specify how to deduplicate the returned apps, if at all.",
				    GS_TYPE_APP_LIST_FILTER_FLAGS, GS_APP_LIST_FILTER_FLAG_NONE,
				    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				    G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppQuery:sort-func: (nullable)
	 *
	 * A sort function to sort the returned apps.
	 *
	 * This must be of type #GsAppListSortFunc.
	 *
	 * Since: 43
	 */
	props[PROP_SORT_FUNC] =
		g_param_spec_pointer ("sort-func", "Sort Function",
				      "A sort function to sort the returned apps.",
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				      G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppQuery:sort-user-data: (nullable)
	 *
	 * User data to pass to #GsAppQuery:sort-func.
	 *
	 * Since: 43
	 */
	props[PROP_SORT_USER_DATA] =
		g_param_spec_pointer ("sort-user-data", "Sort User Data",
				      "User data to pass to #GsAppQuery:sort-func.",
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				      G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppQuery:sort-user-data-notify: (nullable)
	 *
	 * A function to free #GsAppQuery:sort-user-data once it is no longer
	 * needed.
	 *
	 * This must be of type #GDestroyNotify.
	 *
	 * This will be called exactly once between being set and when the
	 * #GsAppQuery is finalized.
	 *
	 * Since: 43
	 */
	props[PROP_SORT_USER_DATA_NOTIFY] =
		g_param_spec_pointer ("sort-user-data-notify", "Sort User Data Notify",
				      "A function to free #GsAppQuery:sort-user-data once it is no longer needed.",
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				      G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppQuery:filter-func: (nullable)
	 *
	 * A filter function to filter the returned apps.
	 *
	 * This must be of type #GsAppListFilterFunc.
	 *
	 * Since: 43
	 */
	props[PROP_FILTER_FUNC] =
		g_param_spec_pointer ("filter-func", "Filter Function",
				      "A filter function to filter the returned apps.",
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				      G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppQuery:filter-user-data: (nullable)
	 *
	 * User data to pass to #GsAppQuery:filter-func.
	 *
	 * Since: 43
	 */
	props[PROP_FILTER_USER_DATA] =
		g_param_spec_pointer ("filter-user-data", "Filter User Data",
				      "User data to pass to #GsAppQuery:filter-func.",
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				      G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppQuery:filter-user-data-notify: (nullable)
	 *
	 * A function to free #GsAppQuery:filter-user-data once it is no longer
	 * needed.
	 *
	 * This must be of type #GDestroyNotify.
	 *
	 * This will be called exactly once between being set and when the
	 * #GsAppQuery is finalized.
	 *
	 * Since: 43
	 */
	props[PROP_FILTER_USER_DATA_NOTIFY] =
		g_param_spec_pointer ("filter-user-data-notify", "Filter User Data Notify",
				      "A function to free #GsAppQuery:filter-user-data once it is no longer needed.",
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				      G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppQuery:deployment-featured: (nullable)
	 *
	 * A list of `LingmoAppstore::DeploymentFeatured` app keys.
	 *
	 * Search for applications that should be featured in a deployment-specific
	 * section on the overview page.
	 * This is expected to be a curated list of applications that are high quality
	 * and feature-complete. Only apps matching at least one of the keys in this
	 * list are returned.
	 *
	 * This may be %NULL to not filter on it. An empty array is
	 * considered equivalent to %NULL.
	 *
	 * Since: 43
	 */
	props[PROP_DEPLOYMENT_FEATURED] =
		g_param_spec_boxed ("deployment-featured", "Deployment Featured",
				    "A list of `LingmoAppstore::DeploymentFeatured` app keys.",
				    G_TYPE_STRV,
				    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				    G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppQuery:developers: (nullable)
	 *
	 * A list of developers to search the apps for.
	 *
	 * Used to search for apps which are provided by given developer(s).
	 *
	 * This may be %NULL to not filter on by them. An empty array is
	 * considered equivalent to %NULL.
	 *
	 * Since: 43
	 */
	props[PROP_DEVELOPERS] =
		g_param_spec_boxed ("developers", "Developers",
				    "A list of developers who provide the apps.",
				    G_TYPE_STRV,
				    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				    G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppQuery:provides-files: (nullable)
	 *
	 * A list of file paths which the apps must provide.
	 *
	 * Used to search for apps which provide specific files on the local
	 * file system.
	 *
	 * This may be %NULL to not filter on file paths. An empty array is
	 * considered equivalent to %NULL.
	 *
	 * Since: 43
	 */
	props[PROP_PROVIDES_FILES] =
		g_param_spec_boxed ("provides-files", "Provides Files",
				    "A list of file paths which the apps must provide.",
				    G_TYPE_STRV,
				    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				    G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppQuery:released-since: (nullable)
	 *
	 * A date/time which apps must have been released since (exclusive).
	 *
	 * Used to search for apps which have been updated recently.
	 *
	 * This may be %NULL to not filter on release date.
	 *
	 * Since: 43
	 */
	props[PROP_RELEASED_SINCE] =
		g_param_spec_boxed ("released-since", "Released Since",
				    "A date/time which apps must have been released since (exclusive).",
				    G_TYPE_DATE_TIME,
				    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				    G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppQuery:is-curated:
	 *
	 * Whether apps must be curated (%GS_APP_QUERY_TRISTATE_TRUE), or not
	 * curated (%GS_APP_QUERY_TRISTATE_FALSE).
	 *
	 * If this is %GS_APP_QUERY_TRISTATE_UNSET, apps are not filtered by
	 * their curation state.
	 *
	 * ‘Curated’ apps have been reviewed and picked by an editor to be
	 * promoted to users in some way. They should be high quality and
	 * feature complete.
	 *
	 * Since: 43
	 */
	props[PROP_IS_CURATED] =
		g_param_spec_enum ("is-curated", "Is Curated",
				   "Whether apps must be curated, or not curated.",
				   GS_TYPE_APP_QUERY_TRISTATE,
				   GS_APP_QUERY_TRISTATE_UNSET,
				   G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				   G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppQuery:is-featured:
	 *
	 * Whether apps must be featured (%GS_APP_QUERY_TRISTATE_TRUE), or not
	 * featured (%GS_APP_QUERY_TRISTATE_FALSE).
	 *
	 * If this is %GS_APP_QUERY_TRISTATE_UNSET, apps are not filtered by
	 * their featured state.
	 *
	 * ‘Featured’ apps have been selected by the distribution or software
	 * source to be highlighted or promoted to users in some way. They
	 * should be high quality and feature complete.
	 *
	 * Since: 43
	 */
	props[PROP_IS_FEATURED] =
		g_param_spec_enum ("is-featured", "Is Featured",
				   "Whether apps must be featured, or not featured.",
				   GS_TYPE_APP_QUERY_TRISTATE,
				   GS_APP_QUERY_TRISTATE_UNSET,
				   G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				   G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppQuery:category: (nullable)
	 *
	 * A category which apps must be in.
	 *
	 * If this is %NULL, apps are not filtered by category.
	 *
	 * Since: 43
	 */
	props[PROP_CATEGORY] =
		g_param_spec_object ("category", "Category",
				     "A category which apps must be in.",
				     GS_TYPE_CATEGORY,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppQuery:is-installed:
	 *
	 * Whether apps must be installed (%GS_APP_QUERY_TRISTATE_TRUE), or not
	 * installed (%GS_APP_QUERY_TRISTATE_FALSE).
	 *
	 * If this is %GS_APP_QUERY_TRISTATE_UNSET, apps are not filtered by
	 * their installed state.
	 *
	 * Since: 43
	 */
	props[PROP_IS_INSTALLED] =
		g_param_spec_enum ("is-installed", "Is Installed",
				   "Whether apps must be installed, or not installed.",
				   GS_TYPE_APP_QUERY_TRISTATE,
				   GS_APP_QUERY_TRISTATE_UNSET,
				   G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				   G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppQuery:keywords:
	 *
	 * A set of search keywords which apps must match.
	 *
	 * Search matches may be done against multiple properties of the app,
	 * such as its name, description, supported content types, defined
	 * keywords, etc. The keywords in this property may be stemmed in an
	 * undefined way after being retrieved from #GsAppQuery.
	 *
	 * If this is %NULL, apps are not filtered by matches to this set of
	 * keywords. An empty array is considered equivalent to %NULL.
	 *
	 * Since: 43
	 */
	props[PROP_KEYWORDS] =
		g_param_spec_boxed ("keywords", "Keywords",
				    "A set of search keywords which apps must match.",
				    G_TYPE_STRV,
				    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				    G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppQuery:alternate-of: (nullable)
	 *
	 * An app which apps must be related to.
	 *
	 * The definition of ‘related to’ depends on the code consuming
	 * #GsAppQuery, but it will typically be other applications which
	 * implement the same feature, or other applications which are packaged
	 * together with this one.
	 *
	 * If this is %NULL, apps are not filtered by alternatives.
	 *
	 * Since: 43
	 */
	props[PROP_ALTERNATE_OF] =
		g_param_spec_object ("alternate-of", "Alternate Of",
				     "An app which apps must be related to.",
				     GS_TYPE_APP,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppQuery:provides-tag: (nullable)
	 *
	 * A tag which apps must provide.
	 *
	 * The interpretation of the tag depends on #GsAppQuery:provides-type,
	 * which must not be %GS_APP_QUERY_PROVIDES_UNKNOWN if this is
	 * non-%NULL. Typically a tag will be a content type which the app
	 * implements, or the name of a printer which the app provides the
	 * driver for, etc.
	 *
	 * If this is %NULL, apps are not filtered by what they provide.
	 *
	 * Since: 43
	 */
	props[PROP_PROVIDES_TAG] =
		g_param_spec_string ("provides-tag", "Provides Tag",
				     "A tag which apps must provide.",
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppQuery:provides-type:
	 *
	 * The type of #GsAppQuery:provides-tag.
	 *
	 * If this is %GS_APP_QUERY_PROVIDES_UNKNOWN, apps are not filtered by
	 * what they provide.
	 *
	 * Since: 43
	 */
	props[PROP_PROVIDES_TYPE] =
		g_param_spec_enum ("provides-type", "Provides Type",
				   "The type of #GsAppQuery:provides-tag.",
				   GS_TYPE_APP_QUERY_PROVIDES_TYPE, GS_APP_QUERY_PROVIDES_UNKNOWN,
				   G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				   G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppQuery:license-type:
	 *
	 * The type of license the app must be under.
	 *
	 * If this is %GS_APP_QUERY_LICENSE_ANY, apps are not filtered by
	 * their license type.
	 *
	 * Since: 44
	 */
	props[PROP_LICENSE_TYPE] =
		g_param_spec_enum ("license-type", "License Type",
				   "The type of license the app must be under.",
				   GS_TYPE_APP_QUERY_LICENSE_TYPE, GS_APP_QUERY_LICENSE_ANY,
				   G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				   G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsAppQuery:developer-verified-type:
	 *
	 * The type of developer verified state filter.
	 *
	 * If this is %GS_APP_QUERY_DEVELOPER_VERIFIED_ANY, apps are not filtered by
	 * the developer verified state.
	 *
	 * Since: 46
	 */
	props[PROP_DEVELOPER_VERIFIED_TYPE] =
		g_param_spec_enum ("developer-verified-type", "Developer Verified Type",
				   "The type of developer verified state filter.",
				   GS_TYPE_APP_QUERY_DEVELOPER_VERIFIED_TYPE,
				   GS_APP_QUERY_DEVELOPER_VERIFIED_ANY,
				   G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				   G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (props), props);
}

static void
gs_app_query_init (GsAppQuery *self)
{
	self->is_curated = GS_APP_QUERY_TRISTATE_UNSET;
	self->is_featured = GS_APP_QUERY_TRISTATE_UNSET;
	self->is_installed = GS_APP_QUERY_TRISTATE_UNSET;
	self->provides_type = GS_APP_QUERY_PROVIDES_UNKNOWN;
	self->license_type = GS_APP_QUERY_LICENSE_ANY;
	self->developer_verified_type = GS_APP_QUERY_DEVELOPER_VERIFIED_ANY;
}

/**
 * gs_app_query_new:
 * @first_property_name: name of the first #GObject property
 * @...: value for the first property, followed by additional property/value
 *   pairs, then a terminating %NULL
 *
 * Create a new #GsAppQuery containing the given query properties.
 *
 * Returns: (transfer full): a new #GsAppQuery
 * Since: 43
 */
GsAppQuery *
gs_app_query_new (const gchar *first_property_name,
		  ...)
{
	va_list args;
	g_autoptr(GsAppQuery) query = NULL;

	va_start (args, first_property_name);
	query = GS_APP_QUERY (g_object_new_valist (GS_TYPE_APP_QUERY, first_property_name, args));
	va_end (args);

	return g_steal_pointer (&query);
}

/**
 * gs_app_query_get_refine_flags:
 * @self: a #GsAppQuery
 *
 * Get the value of #GsAppQuery:refine-flags.
 *
 * Returns: the refine flags for the query
 * Since: 43
 */
GsPluginRefineFlags
gs_app_query_get_refine_flags (GsAppQuery *self)
{
	g_return_val_if_fail (GS_IS_APP_QUERY (self), GS_PLUGIN_REFINE_FLAGS_NONE);

	return self->refine_flags;
}

/**
 * gs_app_query_get_max_results:
 * @self: a #GsAppQuery
 *
 * Get the value of #GsAppQuery:max-results.
 *
 * Returns: the maximum number of results to return for the query, or `0` to
 *   indicate no limit
 * Since: 43
 */
guint
gs_app_query_get_max_results (GsAppQuery *self)
{
	g_return_val_if_fail (GS_IS_APP_QUERY (self), 0);

	return self->max_results;
}

/**
 * gs_app_query_get_dedupe_flags:
 * @self: a #GsAppQuery
 *
 * Get the value of #GsAppQuery:dedupe-flags.
 *
 * Returns: the dedupe flags for the query
 * Since: 43
 */
GsAppListFilterFlags
gs_app_query_get_dedupe_flags (GsAppQuery *self)
{
	g_return_val_if_fail (GS_IS_APP_QUERY (self), GS_APP_LIST_FILTER_FLAG_NONE);

	return self->dedupe_flags;
}

/**
 * gs_app_query_get_sort_func:
 * @self: a #GsAppQuery
 * @user_data_out: (out) (transfer none) (optional) (nullable): return location
 *   for the #GsAppQuery:sort-user-data, or %NULL to ignore
 *
 * Get the value of #GsAppQuery:sort-func.
 *
 * Returns: (nullable): the sort function for the query
 * Since: 43
 */
GsAppListSortFunc
gs_app_query_get_sort_func (GsAppQuery *self,
                            gpointer   *user_data_out)
{
	g_return_val_if_fail (GS_IS_APP_QUERY (self), NULL);

	if (user_data_out != NULL)
		*user_data_out = self->sort_user_data;

	return self->sort_func;
}

/**
 * gs_app_query_get_filter_func:
 * @self: a #GsAppQuery
 * @user_data_out: (out) (transfer none) (optional) (nullable): return location
 *   for the #GsAppQuery:filter-user-data, or %NULL to ignore
 *
 * Get the value of #GsAppQuery:filter-func.
 *
 * Returns: (nullable): the filter function for the query
 * Since: 43
 */
GsAppListFilterFunc
gs_app_query_get_filter_func (GsAppQuery *self,
                              gpointer   *user_data_out)
{
	g_return_val_if_fail (GS_IS_APP_QUERY (self), NULL);

	if (user_data_out != NULL)
		*user_data_out = self->filter_user_data;

	return self->filter_func;
}

/**
 * gs_app_query_get_n_properties_set:
 * @self: a #GsAppQuery
 *
 * Get the number of query properties which have been set.
 *
 * These are the properties which determine the query results, rather than ones
 * which control refining the results (#GsAppQuery:refine-flags,
 * #GsAppQuery:max-results, #GsAppQuery:dedupe-flags, #GsAppQuery:sort-func and
 * its user data, #GsAppQuery:filter-func and its user data,
 * #GsAppQuery:license-type).
 *
 * Returns: number of properties set so they will affect query results
 * Since: 43
 */
guint
gs_app_query_get_n_properties_set (GsAppQuery *self)
{
	guint n = 0;

	g_return_val_if_fail (GS_IS_APP_QUERY (self), 0);

	if (self->provides_files != NULL)
		n++;
	if (self->released_since != NULL)
		n++;
	if (self->is_curated != GS_APP_QUERY_TRISTATE_UNSET)
		n++;
	if (self->is_featured != GS_APP_QUERY_TRISTATE_UNSET)
		n++;
	if (self->category != NULL)
		n++;
	if (self->is_installed != GS_APP_QUERY_TRISTATE_UNSET)
		n++;
	if (self->deployment_featured != NULL)
		n++;
	if (self->developers != NULL)
		n++;
	if (self->keywords != NULL)
		n++;
	if (self->alternate_of != NULL)
		n++;
	if (self->provides_tag != NULL)
		n++;

	return n;
}

/**
 * gs_app_query_get_provides_files:
 * @self: a #GsAppQuery
 *
 * Get the value of #GsAppQuery:provides-files.
 *
 * Returns: (nullable): a list of file paths which the apps must provide,
 *   or %NULL to not filter on file paths
 * Since: 43
 */
const gchar * const *
gs_app_query_get_provides_files (GsAppQuery *self)
{
	g_return_val_if_fail (GS_IS_APP_QUERY (self), NULL);

	/* Always return %NULL or a non-empty array */
	g_assert (self->provides_files == NULL || self->provides_files[0] != NULL);

	return (const gchar * const *) self->provides_files;
}

/**
 * gs_app_query_get_released_since:
 * @self: a #GsAppQuery
 *
 * Get the value of #GsAppQuery:released-since.
 *
 * Returns: (nullable): a date/time which apps must have been released since,
 *   or %NULL to not filter on release date
 * Since: 43
 */
GDateTime *
gs_app_query_get_released_since (GsAppQuery *self)
{
	g_return_val_if_fail (GS_IS_APP_QUERY (self), NULL);

	return self->released_since;
}

/**
 * gs_app_query_get_is_curated:
 * @self: a #GsAppQuery
 *
 * Get the value of #GsAppQuery:is-curated.
 *
 * Returns: %GS_APP_QUERY_TRISTATE_TRUE if apps must be curated,
 *   %GS_APP_QUERY_TRISTATE_FALSE if they must be not curated, or
 *   %GS_APP_QUERY_TRISTATE_UNSET if it doesn’t matter
 * Since: 43
 */
GsAppQueryTristate
gs_app_query_get_is_curated (GsAppQuery *self)
{
	g_return_val_if_fail (GS_IS_APP_QUERY (self), GS_APP_QUERY_TRISTATE_UNSET);

	return self->is_curated;
}

/**
 * gs_app_query_get_is_featured:
 * @self: a #GsAppQuery
 *
 * Get the value of #GsAppQuery:is-featured.
 *
 * Returns: %GS_APP_QUERY_TRISTATE_TRUE if apps must be featured,
 *   %GS_APP_QUERY_TRISTATE_FALSE if they must be not featured, or
 *   %GS_APP_QUERY_TRISTATE_UNSET if it doesn’t matter
 * Since: 43
 */
GsAppQueryTristate
gs_app_query_get_is_featured (GsAppQuery *self)
{
	g_return_val_if_fail (GS_IS_APP_QUERY (self), GS_APP_QUERY_TRISTATE_UNSET);

	return self->is_featured;
}

/**
 * gs_app_query_get_category:
 * @self: a #GsAppQuery
 *
 * Get the value of #GsAppQuery:category.
 *
 * Returns: (nullable) (transfer none): a category which apps must be part of,
 *   or %NULL to not filter on category
 * Since: 43
 */
GsCategory *
gs_app_query_get_category (GsAppQuery *self)
{
	g_return_val_if_fail (GS_IS_APP_QUERY (self), NULL);

	return self->category;
}

/**
 * gs_app_query_get_is_installed:
 * @self: a #GsAppQuery
 *
 * Get the value of #GsAppQuery:is-installed.
 *
 * Returns: %GS_APP_QUERY_TRISTATE_TRUE if apps must be installed,
 *   %GS_APP_QUERY_TRISTATE_FALSE if they must be not installed, or
 *   %GS_APP_QUERY_TRISTATE_UNSET if it doesn’t matter
 * Since: 43
 */
GsAppQueryTristate
gs_app_query_get_is_installed (GsAppQuery *self)
{
	g_return_val_if_fail (GS_IS_APP_QUERY (self), GS_APP_QUERY_TRISTATE_UNSET);

	return self->is_installed;
}

/**
 * gs_app_query_get_deployment_featured:
 * @self: a #GsAppQuery
 *
 * Get the value of #GsAppQuery:deployment-featured.
 *
 * Returns: (nullable): a list of `LingmoAppstore::DeploymentFeatured` app keys,
 *   which the apps have set in a custom key, or %NULL to not filter on this
 * Since: 43
 */
const gchar * const *
gs_app_query_get_deployment_featured (GsAppQuery *self)
{
	g_return_val_if_fail (GS_IS_APP_QUERY (self), NULL);

	/* Always return %NULL or a non-empty array */
	g_assert (self->deployment_featured == NULL || self->deployment_featured[0] != NULL);

	return (const gchar * const *) self->deployment_featured;
}

/**
 * gs_app_query_get_developers:
 * @self: a #GsAppQuery
 *
 * Get the value of #GsAppQuery:developers.
 *
 * Returns: (nullable): a list of developers who provide the apps,
 *   or %NULL to not filter by it
 * Since: 43
 */
const gchar * const *
gs_app_query_get_developers (GsAppQuery *self)
{
	g_return_val_if_fail (GS_IS_APP_QUERY (self), NULL);

	/* Always return %NULL or a non-empty array */
	g_assert (self->developers == NULL || self->developers[0] != NULL);

	return (const gchar * const *) self->developers;
}

/**
 * gs_app_query_get_keywords:
 * @self: a #GsAppQuery
 *
 * Get the value of #GsAppQuery:keywords.
 *
 * Returns: a set of search keywords which apps must match, or %NULL to not
 *   filter by it
 * Since: 43
 */
const gchar * const *
gs_app_query_get_keywords (GsAppQuery *self)
{
	g_return_val_if_fail (GS_IS_APP_QUERY (self), NULL);

	/* Always return %NULL or a non-empty array */
	g_assert (self->keywords == NULL || self->keywords[0] != NULL);

	return (const gchar * const *) self->keywords;
}

/**
 * gs_app_query_get_alternate_of:
 * @self: a #GsAppQuery
 *
 * Get the value of #GsAppQuery:alternate-of.
 *
 * Returns: (nullable) (transfer none): an app which apps must be related to,
 *   or %NULL to not filter on alternates
 * Since: 43
 */
GsApp *
gs_app_query_get_alternate_of (GsAppQuery *self)
{
	g_return_val_if_fail (GS_IS_APP_QUERY (self), NULL);

	return self->alternate_of;
}

/**
 * gs_app_query_get_provides:
 * @self: a #GsAppQuery
 * @out_provides_tag: (transfer none) (optional) (nullable) (out): return
 *   location for the value of #GsAppQuery:provides-tag, or %NULL to ignore
 *
 * Get the value of #GsAppQuery:provides-type and #GsAppQuery:provides-tag.
 *
 * Returns: the type of tag to filter on, or %GS_APP_QUERY_PROVIDES_UNKNOWN to
 *   not filter on provides
 * Since: 43
 */
GsAppQueryProvidesType
gs_app_query_get_provides (GsAppQuery   *self,
                           const gchar **out_provides_tag)
{
	g_return_val_if_fail (GS_IS_APP_QUERY (self), GS_APP_QUERY_PROVIDES_UNKNOWN);

	if (out_provides_tag != NULL)
		*out_provides_tag = self->provides_tag;

	return self->provides_type;
}

/**
 * gs_app_query_get_license_type:
 * @self: a #GsAppQuery
 *
 * Get the value of #GsAppQuery:license-type.
 *
 * Returns: the type of license the app must be under, or
 *   %GS_APP_QUERY_LICENSE_ANY to not filter by license
 * Since: 44
 */
GsAppQueryLicenseType
gs_app_query_get_license_type (GsAppQuery *self)
{
	g_return_val_if_fail (GS_IS_APP_QUERY (self), GS_APP_QUERY_LICENSE_ANY);

	return self->license_type;
}

/**
 * gs_app_query_get_developer_verified_type:
 * @self: a #GsAppQuery
 *
 * Get the value of #GsAppQuery:developer-verified-type.
 *
 * Returns: the type of developer verified state filter, or
 *   %GS_APP_QUERY_DEVELOPER_VERIFIED_ANY to not filter by it
 * Since: 46
 */
GsAppQueryDeveloperVerifiedType
gs_app_query_get_developer_verified_type (GsAppQuery *self)
{
	g_return_val_if_fail (GS_IS_APP_QUERY (self), GS_APP_QUERY_DEVELOPER_VERIFIED_ANY);

	return self->developer_verified_type;
}
