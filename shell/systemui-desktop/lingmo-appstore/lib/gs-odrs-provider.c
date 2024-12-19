/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2016-2018 Kalev Lember <klember@redhat.com>
 * Copyright (C) 2021 Endless OS Foundation LLC
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * SECTION:gs-odrs-provider
 * @short_description: Provides review data from the Open Desktop Ratings Service.
 *
 * To test this plugin locally you will probably want to build and run the
 * `odrs-web` container, following the instructions in the
 * [`odrs-web` repository](https://github.com/LingmoOS/Infrastructure/odrs-web/-/blob/HEAD/app_data/README.md),
 * and then get lingmo-appstore to use your local review server by running:
 * ```
 * gsettings set org.lingmo.appstore review-server 'http://127.0.0.1:5000/1.0/reviews/api'
 * ```
 *
 * When you are done with development, run the following command to use the real
 * ODRS server again:
 * ```
 * gsettings reset org.lingmo.appstore review-server
 * ```
 *
 * Since: 41
 */

#include "config.h"

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <lingmo-appstore.h>
#include <json-glib/json-glib.h>
#include <libsoup/soup.h>
#include <locale.h>
#include <math.h>
#include <string.h>

G_DEFINE_QUARK (gs-odrs-provider-error-quark, gs_odrs_provider_error)

/* Element in self->ratings, all allocated in one big block and sorted
 * alphabetically to reduce the number of allocations and fragmentation. */
typedef struct {
	gchar *app_id;  /* (owned) */
	guint32 n_star_ratings[6];
} GsOdrsRating;

static int
rating_compare (const GsOdrsRating *a, const GsOdrsRating *b)
{
	return g_strcmp0 (a->app_id, b->app_id);
}

static void
rating_clear (GsOdrsRating *rating)
{
	g_free (rating->app_id);
}

struct _GsOdrsProvider
{
	GObject		 parent_instance;

	gchar		*distro;  /* (not nullable) (owned) */
	gchar		*user_hash;  /* (not nullable) (owned) */
	gchar		*review_server;  /* (not nullable) (owned) */
	GArray		*ratings;  /* (element-type GsOdrsRating) (mutex ratings_mutex) (owned) (nullable) */
	GMutex		 ratings_mutex;
	guint64		 max_cache_age_secs;
	guint		 n_results_max;
	SoupSession	*session;  /* (owned) (not nullable) */
};

G_DEFINE_TYPE (GsOdrsProvider, gs_odrs_provider, G_TYPE_OBJECT)

typedef enum {
	PROP_REVIEW_SERVER = 1,
	PROP_USER_HASH,
	PROP_DISTRO,
	PROP_MAX_CACHE_AGE_SECS,
	PROP_N_RESULTS_MAX,
	PROP_SESSION,
} GsOdrsProviderProperty;

static GParamSpec *obj_props[PROP_SESSION + 1] = { NULL, };

static gboolean
gs_odrs_provider_load_ratings_for_app (JsonObject   *json_app,
                                       const gchar  *app_id,
                                       GsOdrsRating *rating_out)
{
	guint i;
	const gchar *names[] = { "star0", "star1", "star2", "star3",
				 "star4", "star5", NULL };

	for (i = 0; names[i] != NULL; i++) {
		if (!json_object_has_member (json_app, names[i]))
			return FALSE;
		rating_out->n_star_ratings[i] = (guint64) json_object_get_int_member (json_app, names[i]);
	}

	rating_out->app_id = g_strdup (app_id);

	return TRUE;
}

static gboolean
gs_odrs_provider_load_ratings (GsOdrsProvider  *self,
                               const gchar     *filename,
                               GError         **error)
{
	JsonNode *json_root;
	JsonObject *json_item;
	g_autoptr(JsonParser) json_parser = NULL;
	const gchar *app_id;
	JsonNode *json_app_node;
	JsonObjectIter iter;
	g_autoptr(GArray) new_ratings = NULL;
	g_autoptr(GMutexLocker) locker = NULL;
	g_autoptr(GError) local_error = NULL;

	/* parse the data and find the success */
	json_parser = json_parser_new_immutable ();
	if (!json_parser_load_from_mapped_file (json_parser, filename, &local_error)) {
		g_set_error (error,
			     GS_ODRS_PROVIDER_ERROR,
			     GS_ODRS_PROVIDER_ERROR_PARSING_DATA,
			     "Error parsing ODRS data: %s", local_error->message);
		return FALSE;
	}
	json_root = json_parser_get_root (json_parser);
	if (json_root == NULL) {
		g_set_error_literal (error,
				     GS_ODRS_PROVIDER_ERROR,
				     GS_ODRS_PROVIDER_ERROR_PARSING_DATA,
				     "no ratings root");
		return FALSE;
	}
	if (json_node_get_node_type (json_root) != JSON_NODE_OBJECT) {
		g_set_error_literal (error,
				     GS_ODRS_PROVIDER_ERROR,
				     GS_ODRS_PROVIDER_ERROR_PARSING_DATA,
				     "no ratings array");
		return FALSE;
	}

	json_item = json_node_get_object (json_root);

	new_ratings = g_array_sized_new (FALSE,  /* don’t zero-terminate */
					 FALSE,  /* don’t clear */
					 sizeof (GsOdrsRating),
					 json_object_get_size (json_item));
	g_array_set_clear_func (new_ratings, (GDestroyNotify) rating_clear);

	/* parse each app */
	json_object_iter_init (&iter, json_item);
	while (json_object_iter_next (&iter, &app_id, &json_app_node)) {
		GsOdrsRating rating;
		JsonObject *json_app;

		if (!JSON_NODE_HOLDS_OBJECT (json_app_node))
			continue;
		json_app = json_node_get_object (json_app_node);

		if (gs_odrs_provider_load_ratings_for_app (json_app, app_id, &rating))
			g_array_append_val (new_ratings, rating);
	}

	/* Allow for binary searches later. */
	g_array_sort (new_ratings, (GCompareFunc) rating_compare);

	/* Update the shared state */
	locker = g_mutex_locker_new (&self->ratings_mutex);
	g_clear_pointer (&self->ratings, g_array_unref);
	self->ratings = g_steal_pointer (&new_ratings);

	return TRUE;
}

static AsReview *
gs_odrs_provider_parse_review_object (JsonObject *item)
{
	AsReview *rev = as_review_new ();

	/* date */
	if (json_object_has_member (item, "date_created")) {
		gint64 timestamp;
		g_autoptr(GDateTime) dt = NULL;
		timestamp = json_object_get_int_member (item, "date_created");
		dt = g_date_time_new_from_unix_utc (timestamp);
		as_review_set_date (rev, dt);
	}

	/* assemble review */
	if (json_object_has_member (item, "rating"))
		as_review_set_rating (rev, (gint) json_object_get_int_member (item, "rating"));
	if (json_object_has_member (item, "score")) {
		as_review_set_priority (rev, (gint) json_object_get_int_member (item, "score"));
	} else if (json_object_has_member (item, "karma_up") &&
		   json_object_has_member (item, "karma_down")) {
		gdouble ku = (gdouble) json_object_get_int_member (item, "karma_up");
		gdouble kd = (gdouble) json_object_get_int_member (item, "karma_down");
		gdouble wilson = 0.f;

		/* from http://www.evanmiller.org/how-not-to-sort-by-average-rating.html */
		if (ku > 0 || kd > 0) {
			wilson = ((ku + 1.9208) / (ku + kd) -
				  1.96 * sqrt ((ku * kd) / (ku + kd) + 0.9604) /
				  (ku + kd)) / (1 + 3.8416 / (ku + kd));
			wilson *= 100.f;
		}
		as_review_set_priority (rev, (gint) wilson);
	}
	if (json_object_has_member (item, "user_hash"))
		as_review_set_reviewer_id (rev, json_object_get_string_member (item, "user_hash"));
	if (json_object_has_member (item, "user_display")) {
		g_autofree gchar *user_display = g_strdup (json_object_get_string_member (item, "user_display"));
		if (user_display)
			g_strstrip (user_display);
		as_review_set_reviewer_name (rev, user_display);
	}
	if (json_object_has_member (item, "summary")) {
		g_autofree gchar *summary = g_strdup (json_object_get_string_member (item, "summary"));
		if (summary)
			g_strstrip (summary);
		as_review_set_summary (rev, summary);
	}
	if (json_object_has_member (item, "description")) {
		g_autofree gchar *description = g_strdup (json_object_get_string_member (item, "description"));
		if (description)
			g_strstrip (description);
		as_review_set_description (rev, description);
	}
	if (json_object_has_member (item, "version"))
		as_review_set_version (rev, json_object_get_string_member (item, "version"));

	/* add extra metadata for the plugin */
	if (json_object_has_member (item, "user_skey")) {
		as_review_add_metadata (rev, "user_skey",
					json_object_get_string_member (item, "user_skey"));
	}
	if (json_object_has_member (item, "app_id")) {
		as_review_add_metadata (rev, "app_id",
					json_object_get_string_member (item, "app_id"));
	}
	if (json_object_has_member (item, "review_id")) {
		g_autofree gchar *review_id = NULL;
		review_id = g_strdup_printf ("%" G_GINT64_FORMAT,
					json_object_get_int_member (item, "review_id"));
		as_review_set_id (rev, review_id);
	}

	/* don't allow multiple votes */
	if (json_object_has_member (item, "vote_id"))
		as_review_add_flags (rev, AS_REVIEW_FLAG_VOTED);

	return rev;
}

/* json_parser_load*() must have been called on @json_parser before calling
 * this function. */
static GPtrArray *
gs_odrs_provider_parse_reviews (GsOdrsProvider  *self,
                                JsonParser      *json_parser,
                                GError         **error)
{
	JsonArray *json_reviews;
	JsonNode *json_root;
	guint i;
	g_autoptr(GHashTable) reviewer_ids = NULL;
	g_autoptr(GPtrArray) reviews = NULL;

	json_root = json_parser_get_root (json_parser);
	if (json_root == NULL) {
		g_set_error_literal (error,
				     GS_ODRS_PROVIDER_ERROR,
				     GS_ODRS_PROVIDER_ERROR_PARSING_DATA,
				     "no root");
		return NULL;
	}
	if (json_node_get_node_type (json_root) != JSON_NODE_ARRAY) {
		g_set_error_literal (error,
				     GS_ODRS_PROVIDER_ERROR,
				     GS_ODRS_PROVIDER_ERROR_PARSING_DATA,
				     "no array");
		return NULL;
	}

	/* parse each rating */
	reviews = g_ptr_array_new_with_free_func ((GDestroyNotify) g_object_unref);
	json_reviews = json_node_get_array (json_root);
	reviewer_ids = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	for (i = 0; i < json_array_get_length (json_reviews); i++) {
		JsonNode *json_review;
		JsonObject *json_item;
		const gchar *reviewer_id;
		g_autoptr(AsReview) review = NULL;

		/* extract the data */
		json_review = json_array_get_element (json_reviews, i);
		if (json_node_get_node_type (json_review) != JSON_NODE_OBJECT) {
			g_set_error_literal (error,
					     GS_ODRS_PROVIDER_ERROR,
					     GS_ODRS_PROVIDER_ERROR_PARSING_DATA,
					     "no object type");
			return NULL;
		}
		json_item = json_node_get_object (json_review);
		if (json_item == NULL) {
			g_set_error_literal (error,
					     GS_ODRS_PROVIDER_ERROR,
					     GS_ODRS_PROVIDER_ERROR_PARSING_DATA,
					     "no object");
			return NULL;
		}

		/* create review */
		review = gs_odrs_provider_parse_review_object (json_item);

		reviewer_id = as_review_get_reviewer_id (review);
		if (reviewer_id == NULL)
			continue;

		/* dedupe each on the user_hash */
		if (g_hash_table_lookup (reviewer_ids, reviewer_id) != NULL) {
			g_debug ("duplicate review %s, skipping", reviewer_id);
			continue;
		}
		g_hash_table_add (reviewer_ids, g_strdup (reviewer_id));
		g_ptr_array_add (reviews, g_object_ref (review));
	}
	return g_steal_pointer (&reviews);
}

static gboolean
gs_odrs_provider_parse_success (GInputStream  *input_stream,
                                GError       **error)
{
	JsonNode *json_root;
	JsonObject *json_item;
	const gchar *msg = NULL;
	g_autoptr(JsonParser) json_parser = NULL;
	g_autoptr(GError) local_error = NULL;

	/* parse the data and find the success
	 * FIXME: This should probably eventually be refactored and made async */
	json_parser = json_parser_new_immutable ();
	if (!json_parser_load_from_stream (json_parser, input_stream, NULL, &local_error)) {
		g_set_error (error,
			     GS_ODRS_PROVIDER_ERROR,
			     GS_ODRS_PROVIDER_ERROR_PARSING_DATA,
			     "Error parsing ODRS data: %s", local_error->message);
		return FALSE;
	}
	json_root = json_parser_get_root (json_parser);
	if (json_root == NULL) {
		g_set_error_literal (error,
				     GS_ODRS_PROVIDER_ERROR,
				     GS_ODRS_PROVIDER_ERROR_PARSING_DATA,
				     "no error root");
		return FALSE;
	}
	if (json_node_get_node_type (json_root) != JSON_NODE_OBJECT) {
		g_set_error_literal (error,
				     GS_ODRS_PROVIDER_ERROR,
				     GS_ODRS_PROVIDER_ERROR_PARSING_DATA,
				     "no error object");
		return FALSE;
	}
	json_item = json_node_get_object (json_root);
	if (json_item == NULL) {
		g_set_error_literal (error,
				     GS_ODRS_PROVIDER_ERROR,
				     GS_ODRS_PROVIDER_ERROR_PARSING_DATA,
				     "no error object");
		return FALSE;
	}

	/* failed? */
	if (json_object_has_member (json_item, "msg"))
		msg = json_object_get_string_member (json_item, "msg");
	if (!json_object_get_boolean_member (json_item, "success")) {
		g_set_error_literal (error,
				     GS_ODRS_PROVIDER_ERROR,
				     GS_ODRS_PROVIDER_ERROR_PARSING_DATA,
				     msg != NULL ? msg : "unknown failure");
		return FALSE;
	}

	/* just for the console */
	if (msg != NULL)
		g_debug ("success: %s", msg);
	return TRUE;
}

#if SOUP_CHECK_VERSION(3, 0, 0)
typedef struct {
	GInputStream *input_stream;
	gssize length;
	goffset read_from;
} MessageData;

static MessageData *
message_data_new (GInputStream *input_stream,
		  gssize length)
{
	MessageData *md;

	md = g_slice_new0 (MessageData);
	md->input_stream = g_object_ref (input_stream);
	md->length = length;

	if (G_IS_SEEKABLE (input_stream))
		md->read_from = g_seekable_tell (G_SEEKABLE (input_stream));

	return md;
}

static void
message_data_free (gpointer ptr,
		   GClosure *closure)
{
	MessageData *md = ptr;

	if (md) {
		g_object_unref (md->input_stream);
		g_slice_free (MessageData, md);
	}
}

static void
g_odrs_provider_message_restarted_cb (SoupMessage *message,
				      gpointer user_data)
{
	MessageData *md = user_data;

	if (G_IS_SEEKABLE (md->input_stream) && md->read_from != g_seekable_tell (G_SEEKABLE (md->input_stream)))
		g_seekable_seek (G_SEEKABLE (md->input_stream), md->read_from, G_SEEK_SET, NULL, NULL);

	soup_message_set_request_body (message, NULL, md->input_stream, md->length);
}

static void
g_odrs_provider_set_message_request_body (SoupMessage *message,
					  const gchar *content_type,
					  gconstpointer data,
					  gsize length)
{
	MessageData *md;
	GInputStream *input_stream;

	g_return_if_fail (SOUP_IS_MESSAGE (message));
	g_return_if_fail (data != NULL);

	input_stream = g_memory_input_stream_new_from_data (g_memdup2 (data, length), length, g_free);
	md = message_data_new (input_stream, length);

	g_signal_connect_data (message, "restarted",
		G_CALLBACK (g_odrs_provider_message_restarted_cb), md, message_data_free, 0);

	soup_message_set_request_body (message, content_type, input_stream, length);

	g_object_unref (input_stream);
}
#endif

static gboolean
gs_odrs_provider_json_post (SoupSession  *session,
                            const gchar  *uri,
                            const gchar  *data,
			    GCancellable *cancellable,
                            GError      **error)
{
	guint status_code;
	g_autoptr(SoupMessage) msg = NULL;
	gconstpointer downloaded_data;
	gsize downloaded_data_length;
	g_autoptr(GInputStream) input_stream = NULL;
#if SOUP_CHECK_VERSION(3, 0, 0)
	g_autoptr(GBytes) bytes = NULL;
#endif
	/* create the GET data */
	g_debug ("Sending ODRS request to %s: %s", uri, data);
	msg = soup_message_new (SOUP_METHOD_POST, uri);
#if SOUP_CHECK_VERSION(3, 0, 0)
	g_odrs_provider_set_message_request_body (msg, "application/json; charset=utf-8",
						  data, strlen (data));
	bytes = soup_session_send_and_read (session, msg, cancellable, error);
	if (bytes == NULL)
		return FALSE;

	downloaded_data = g_bytes_get_data (bytes, &downloaded_data_length);
	status_code = soup_message_get_status (msg);
#else
	soup_message_set_request (msg, "application/json; charset=utf-8",
				  SOUP_MEMORY_COPY, data, strlen (data));

	/* set sync request */
	status_code = soup_session_send_message (session, msg);
	downloaded_data = msg->response_body ? msg->response_body->data : NULL;
	downloaded_data_length = msg->response_body ? msg->response_body->length : 0;
#endif
	g_debug ("ODRS server returned status %u: %.*s", status_code, (gint) downloaded_data_length, (const gchar *) downloaded_data);
	if (status_code != SOUP_STATUS_OK) {
		g_warning ("Failed to set rating on ODRS: %s",
			   soup_status_get_phrase (status_code));
		g_set_error (error,
                             GS_ODRS_PROVIDER_ERROR,
                             GS_ODRS_PROVIDER_ERROR_SERVER_ERROR,
                             "Failed to submit review to ODRS: %s", soup_status_get_phrase (status_code));
		return FALSE;
	}

	/* process returned JSON */
	input_stream = g_memory_input_stream_new_from_data (downloaded_data, downloaded_data_length, NULL);
	return gs_odrs_provider_parse_success (input_stream, error);
}

static GPtrArray *
_gs_app_get_reviewable_ids (GsApp *app)
{
	GPtrArray *ids = g_ptr_array_new_with_free_func (g_free);
	GPtrArray *provided = gs_app_get_provided (app);

	/* add the main component id */
	g_ptr_array_add (ids, g_strdup (gs_app_get_id (app)));

	/* add any ID provides */
	for (guint i = 0; i < provided->len; i++) {
		GPtrArray *items;
		AsProvided *prov = g_ptr_array_index (provided, i);
		if (as_provided_get_kind (prov) != AS_PROVIDED_KIND_ID)
			continue;

		items = as_provided_get_items (prov);
		for (guint j = 0; j < items->len; j++) {
			const gchar *value = (const gchar *) g_ptr_array_index (items, j);
			guint k;
			if (value == NULL)
				continue;
			for (k = 0; k < ids->len; k++) {
				const gchar *existing_id = g_ptr_array_index (ids, k);
				if (g_strcmp0 (existing_id, value) == 0)
					break;
			}
			/* when `k` is less than `ids->len`, then a match was found, thus skip a duplicate */
			if (k == ids->len)
				g_ptr_array_add (ids, g_strdup (value));
		}
	}
	return ids;
}

static gboolean
gs_odrs_provider_refine_ratings (GsOdrsProvider  *self,
                                 GsApp           *app,
                                 GCancellable    *cancellable,
                                 GError         **error)
{
	gint rating;
	guint32 ratings_raw[6] = { 0, 0, 0, 0, 0, 0 };
	guint cnt = 0;
	g_autoptr(GArray) review_ratings = NULL;
	g_autoptr(GPtrArray) reviewable_ids = NULL;
	g_autoptr(GMutexLocker) locker = NULL;

	/* get ratings for each reviewable ID */
	reviewable_ids = _gs_app_get_reviewable_ids (app);

	locker = g_mutex_locker_new (&self->ratings_mutex);

	if (!self->ratings) {
		g_autofree gchar *cache_filename = NULL;

		g_clear_pointer (&locker, g_mutex_locker_free);

		/* Load from the local cache, if available, when in offline or
		   when refresh/download disabled on start */
		cache_filename = gs_utils_get_cache_filename ("odrs",
							      "ratings.json",
							      GS_UTILS_CACHE_FLAG_WRITEABLE |
							      GS_UTILS_CACHE_FLAG_CREATE_DIRECTORY,
							      error);

		if (!cache_filename)
			return TRUE;

		if (!gs_odrs_provider_load_ratings (self, cache_filename, NULL)) {
			g_autoptr(GFile) cache_file = g_file_new_for_path (cache_filename);
			g_debug ("Failed to load cache file ‘%s’, deleting it", cache_filename);
			g_file_delete (cache_file, NULL, NULL);
			return TRUE;
		}

		locker = g_mutex_locker_new (&self->ratings_mutex);

		if (!self->ratings)
			return TRUE;
	}

	for (guint i = 0; i < reviewable_ids->len; i++) {
		const gchar *id = g_ptr_array_index (reviewable_ids, i);
		const GsOdrsRating search_rating = { (gchar *) id, { 0, }};
		guint found_index;
		const GsOdrsRating *found_rating;

		if (!g_array_binary_search (self->ratings, &search_rating,
					    (GCompareFunc) rating_compare, &found_index))
			continue;

		found_rating = &g_array_index (self->ratings, GsOdrsRating, found_index);

		/* copy into accumulator array */
		for (guint j = 0; j < 6; j++)
			ratings_raw[j] += found_rating->n_star_ratings[j];
		cnt++;
	}
	if (cnt == 0)
		return TRUE;

	/* Done with self->ratings now */
	g_clear_pointer (&locker, g_mutex_locker_free);

	/* merge to accumulator array back to one GArray blob */
	review_ratings = g_array_sized_new (FALSE, TRUE, sizeof(guint32), 6);
	for (guint i = 0; i < 6; i++)
		g_array_append_val (review_ratings, ratings_raw[i]);
	gs_app_set_review_ratings (app, review_ratings);

	/* find the wilson rating */
	rating = gs_utils_get_wilson_rating (g_array_index (review_ratings, guint32, 1),
					     g_array_index (review_ratings, guint32, 2),
					     g_array_index (review_ratings, guint32, 3),
					     g_array_index (review_ratings, guint32, 4),
					     g_array_index (review_ratings, guint32, 5));
	if (rating > 0)
		gs_app_set_rating (app, rating);
	return TRUE;
}

static JsonNode *
gs_odrs_provider_get_compat_ids (GsApp *app)
{
	GPtrArray *provided = gs_app_get_provided (app);
	g_autoptr(GHashTable) ids = NULL;
	g_autoptr(JsonArray) json_array = json_array_new ();
	g_autoptr(JsonNode) json_node = json_node_new (JSON_NODE_ARRAY);

	ids = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, NULL);
	for (guint i = 0; i < provided->len; i++) {
		GPtrArray *items;
		AsProvided *prov = g_ptr_array_index (provided, i);

		if (as_provided_get_kind (prov) != AS_PROVIDED_KIND_ID)
			continue;

		items = as_provided_get_items (prov);
		for (guint j = 0; j < items->len; j++) {
			const gchar *value = g_ptr_array_index (items, j);
			if (value == NULL)
				continue;

			if (g_hash_table_add (ids, (gpointer) value))
				json_array_add_string_element (json_array, value);
		}
	}
	if (json_array_get_length (json_array) == 0)
		return NULL;
	json_node_set_array (json_node, json_array);
	return g_steal_pointer (&json_node);
}

static void open_input_stream_cb (GObject      *source_object,
                                  GAsyncResult *result,
                                  gpointer      user_data);
static void parse_reviews_cb (GObject      *source_object,
                              GAsyncResult *result,
                              gpointer      user_data);
static void set_reviews_on_app (GsOdrsProvider *self,
                                GsApp          *app,
                                GPtrArray      *reviews);

typedef struct {
	GsApp *app;  /* (not nullable) (owned) */
	gchar *cache_filename;  /* (not nullable) (owned) */
	SoupMessage *message;  /* (nullable) (owned) */
} FetchReviewsForAppData;

static void
fetch_reviews_for_app_data_free (FetchReviewsForAppData *data)
{
	g_clear_object (&data->app);
	g_free (data->cache_filename);
	g_clear_object (&data->message);

	g_free (data);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (FetchReviewsForAppData, fetch_reviews_for_app_data_free)

static void
gs_odrs_provider_fetch_reviews_for_app_async (GsOdrsProvider      *self,
                                              GsApp               *app,
                                              GCancellable        *cancellable,
                                              GAsyncReadyCallback  callback,
                                              gpointer             user_data)
{
	JsonNode *json_compat_ids;
	const gchar *version;
	g_autofree gchar *cachefn_basename = NULL;
	g_autofree gchar *cachefn = NULL;
	g_autofree gchar *request_body = NULL;
	g_autofree gchar *uri = NULL;
	g_autoptr(GFile) cachefn_file = NULL;
	g_autoptr(GPtrArray) reviews = NULL;
	g_autoptr(JsonBuilder) builder = NULL;
	g_autoptr(JsonParser) json_parser = NULL;
	g_autoptr(JsonGenerator) json_generator = NULL;
	g_autoptr(JsonNode) json_root = NULL;
	g_autoptr(SoupMessage) msg = NULL;
	g_autoptr(GTask) task = NULL;
	FetchReviewsForAppData *data;
	g_autoptr(FetchReviewsForAppData) data_owned = NULL;
	g_autoptr(GError) local_error = NULL;

	task = g_task_new (self, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_odrs_provider_fetch_reviews_for_app_async);

	data = data_owned = g_new0 (FetchReviewsForAppData, 1);
	data->app = g_object_ref (app);
	g_task_set_task_data (task, g_steal_pointer (&data_owned), (GDestroyNotify) fetch_reviews_for_app_data_free);

	/* look in the cache */
	cachefn_basename = g_strdup_printf ("%s.json", gs_app_get_id (app));
	cachefn = gs_utils_get_cache_filename ("odrs",
					       cachefn_basename,
					       GS_UTILS_CACHE_FLAG_WRITEABLE |
					       GS_UTILS_CACHE_FLAG_CREATE_DIRECTORY,
					       &local_error);
	if (cachefn == NULL) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	data->cache_filename = g_strdup (cachefn);
	cachefn_file = g_file_new_for_path (cachefn);
	if (gs_utils_get_file_age (cachefn_file) < self->max_cache_age_secs) {
		g_debug ("got review data for %s from %s",
			 gs_app_get_id (app), cachefn);

		/* parse the data and find the array of ratings */
		json_parser = json_parser_new_immutable ();
		if (!json_parser_load_from_mapped_file (json_parser, cachefn, &local_error)) {
			g_task_return_new_error (task,
						 GS_ODRS_PROVIDER_ERROR,
						 GS_ODRS_PROVIDER_ERROR_PARSING_DATA,
						 "Error parsing ODRS data: %s", local_error->message);
			return;
		}

		reviews = gs_odrs_provider_parse_reviews (self, json_parser, &local_error);
		if (reviews == NULL) {
			g_task_return_error (task, g_steal_pointer (&local_error));
		} else {
			set_reviews_on_app (self, app, reviews);
			g_task_return_boolean (task, TRUE);
		}

		return;
	}

	/* not always available */
	version = gs_app_get_version (app);
	if (version == NULL)
		version = "unknown";

	/* create object with review data */
	builder = json_builder_new ();
	json_builder_begin_object (builder);
	json_builder_set_member_name (builder, "user_hash");
	json_builder_add_string_value (builder, self->user_hash);
	json_builder_set_member_name (builder, "app_id");
	json_builder_add_string_value (builder, gs_app_get_id (app));
	json_builder_set_member_name (builder, "locale");
	json_builder_add_string_value (builder, setlocale (LC_MESSAGES, NULL));
	json_builder_set_member_name (builder, "distro");
	json_builder_add_string_value (builder, self->distro);
	json_builder_set_member_name (builder, "version");
	json_builder_add_string_value (builder, version);
	json_builder_set_member_name (builder, "limit");
	json_builder_add_int_value (builder, self->n_results_max);
	json_compat_ids = gs_odrs_provider_get_compat_ids (app);
	if (json_compat_ids != NULL) {
		json_builder_set_member_name (builder, "compat_ids");
		json_builder_add_value (builder, json_compat_ids);
	}
	json_builder_end_object (builder);

	/* export as a string */
	json_root = json_builder_get_root (builder);
	json_generator = json_generator_new ();
	json_generator_set_pretty (json_generator, TRUE);
	json_generator_set_root (json_generator, json_root);
	request_body = json_generator_to_data (json_generator, NULL);

	uri = g_strdup_printf ("%s/fetch", self->review_server);
	g_debug ("Updating ODRS cache for %s from %s to %s; request %s", gs_app_get_id (app),
		 uri, cachefn, request_body);
	msg = soup_message_new (SOUP_METHOD_POST, uri);
	data->message = g_object_ref (msg);

#if SOUP_CHECK_VERSION(3, 0, 0)
	g_odrs_provider_set_message_request_body (msg, "application/json; charset=utf-8",
						  request_body, strlen (request_body));
	soup_session_send_async (self->session, msg, G_PRIORITY_DEFAULT,
				 cancellable, open_input_stream_cb, g_steal_pointer (&task));
#else
	soup_message_set_request (msg, "application/json; charset=utf-8",
				  SOUP_MEMORY_COPY, request_body, strlen (request_body));
	soup_session_send_async (self->session, msg, cancellable,
				 open_input_stream_cb, g_steal_pointer (&task));
#endif
}

static void
open_input_stream_cb (GObject      *source_object,
                      GAsyncResult *result,
                      gpointer      user_data)
{
	SoupSession *soup_session = SOUP_SESSION (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	FetchReviewsForAppData *data = g_task_get_task_data (task);
	GCancellable *cancellable = g_task_get_cancellable (task);
	g_autoptr(GInputStream) input_stream = NULL;
	guint status_code;
	g_autoptr(JsonParser) json_parser = NULL;
	g_autoptr(GError) local_error = NULL;

#if SOUP_CHECK_VERSION(3, 0, 0)
	input_stream = soup_session_send_finish (soup_session, result, &local_error);
	status_code = soup_message_get_status (data->message);
#else
	input_stream = soup_session_send_finish (soup_session, result, &local_error);
	status_code = data->message->status_code;
#endif

	if (input_stream == NULL) {
		if (!g_network_monitor_get_network_available (g_network_monitor_get_default ()))
			g_task_return_new_error (task,
						 GS_ODRS_PROVIDER_ERROR,
						 GS_ODRS_PROVIDER_ERROR_NO_NETWORK,
						 "server couldn't be reached");
		else
			g_task_return_new_error (task,
						 GS_ODRS_PROVIDER_ERROR,
						 GS_ODRS_PROVIDER_ERROR_PARSING_DATA,
						 "server returned no data");
		return;
	}

	if (status_code != SOUP_STATUS_OK) {
		if (!gs_odrs_provider_parse_success (input_stream, &local_error)) {
			g_task_return_error (task, g_steal_pointer (&local_error));
			return;
		}

		/* not sure what to do here */
		g_task_return_new_error (task,
					 GS_ODRS_PROVIDER_ERROR,
					 GS_ODRS_PROVIDER_ERROR_DOWNLOADING,
					 "status code invalid");
		return;
	}

	/* parse the data and find the array of ratings */
	json_parser = json_parser_new_immutable ();
	json_parser_load_from_stream_async (json_parser, input_stream, cancellable, parse_reviews_cb, g_steal_pointer (&task));
}

static void
parse_reviews_cb (GObject      *source_object,
                  GAsyncResult *result,
                  gpointer      user_data)
{
	JsonParser *json_parser = JSON_PARSER (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	GsOdrsProvider *self = g_task_get_source_object (task);
	FetchReviewsForAppData *data = g_task_get_task_data (task);
	g_autoptr(GPtrArray) reviews = NULL;
	g_autoptr(JsonGenerator) cache_generator = NULL;
	g_autoptr(GError) local_error = NULL;

	if (!json_parser_load_from_stream_finish (json_parser, result, &local_error)) {
		g_task_return_new_error (task,
					 GS_ODRS_PROVIDER_ERROR,
					 GS_ODRS_PROVIDER_ERROR_PARSING_DATA,
					 "Error parsing ODRS data: %s", local_error->message);
		return;
	}

	reviews = gs_odrs_provider_parse_reviews (self, json_parser, &local_error);
	if (reviews == NULL) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	/* save to the cache */
	cache_generator = json_generator_new ();
	json_generator_set_pretty (cache_generator, FALSE);
	json_generator_set_root (cache_generator, json_parser_get_root (json_parser));

	if (!json_generator_to_file (cache_generator, data->cache_filename, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	set_reviews_on_app (self, data->app, reviews);

	/* success */
	g_task_return_boolean (task, TRUE);
}

static void
set_reviews_on_app (GsOdrsProvider *self,
                    GsApp          *app,
                    GPtrArray      *reviews)
{
	for (guint i = 0; i < reviews->len; i++) {
		AsReview *review = g_ptr_array_index (reviews, i);

		/* save this on the app object so we can use it for
		 * submitting a new review */
		if (i == 0) {
			gs_app_set_metadata (app, "ODRS::user_skey",
					     as_review_get_metadata_item (review, "user_skey"));
		}

		/* ignore invalid reviews */
		if (as_review_get_rating (review) == 0)
			continue;

		/* the user_hash matches, so mark this as our own review */
		if (g_strcmp0 (as_review_get_reviewer_id (review),
			       self->user_hash) == 0) {
			as_review_set_flags (review, AS_REVIEW_FLAG_SELF);
		}
		gs_app_add_review (app, review);
	}
}

static gboolean
gs_odrs_provider_fetch_reviews_for_app_finish (GsOdrsProvider  *self,
                                               GAsyncResult    *result,
                                               GError         **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static gchar *
gs_odrs_provider_trim_version (const gchar *version)
{
	gchar *str;
	gchar *tmp;

	/* nothing set */
	if (version == NULL)
		return g_strdup ("unknown");

	/* remove epoch */
	str = g_strrstr (version, ":");
	if (str != NULL)
		version = str + 1;

	/* remove release */
	tmp = g_strdup (version);
	g_strdelimit (tmp, "-", '\0');

	/* remove '+dfsg' suffix */
	str = g_strstr_len (tmp, -1, "+dfsg");
	if (str != NULL)
		*str = '\0';

	return tmp;
}

static gboolean
gs_odrs_provider_invalidate_cache (AsReview *review, GError **error)
{
	g_autofree gchar *cachefn_basename = NULL;
	g_autofree gchar *cachefn = NULL;
	g_autoptr(GFile) cachefn_file = NULL;

	/* look in the cache */
	cachefn_basename = g_strdup_printf ("%s.json",
					    as_review_get_metadata_item (review, "app_id"));
	cachefn = gs_utils_get_cache_filename ("odrs",
					       cachefn_basename,
					       GS_UTILS_CACHE_FLAG_WRITEABLE |
					       GS_UTILS_CACHE_FLAG_CREATE_DIRECTORY,
					       error);
	if (cachefn == NULL)
		return FALSE;
	cachefn_file = g_file_new_for_path (cachefn);
	if (!g_file_query_exists (cachefn_file, NULL))
		return TRUE;
	return g_file_delete (cachefn_file, NULL, error);
}

static gboolean
gs_odrs_provider_vote (GsOdrsProvider  *self,
                       AsReview        *review,
                       const gchar     *uri,
		       GCancellable    *cancellable,
                       GError         **error)
{
	const gchar *tmp;
	g_autofree gchar *data = NULL;
	g_autoptr(JsonBuilder) builder = NULL;
	g_autoptr(JsonGenerator) json_generator = NULL;
	g_autoptr(JsonNode) json_root = NULL;

	/* create object with vote data */
	builder = json_builder_new ();
	json_builder_begin_object (builder);

	json_builder_set_member_name (builder, "user_hash");
	json_builder_add_string_value (builder, self->user_hash);
	json_builder_set_member_name (builder, "user_skey");
	json_builder_add_string_value (builder,
				       as_review_get_metadata_item (review, "user_skey"));
	json_builder_set_member_name (builder, "app_id");
	json_builder_add_string_value (builder,
				       as_review_get_metadata_item (review, "app_id"));
	tmp = as_review_get_id (review);
	if (tmp != NULL) {
		gint64 review_id;
		json_builder_set_member_name (builder, "review_id");
		review_id = g_ascii_strtoll (tmp, NULL, 10);
		json_builder_add_int_value (builder, review_id);
	}
	json_builder_end_object (builder);

	/* export as a string */
	json_root = json_builder_get_root (builder);
	json_generator = json_generator_new ();
	json_generator_set_pretty (json_generator, TRUE);
	json_generator_set_root (json_generator, json_root);
	data = json_generator_to_data (json_generator, NULL);
	if (data == NULL)
		return FALSE;

	/* clear cache */
	if (!gs_odrs_provider_invalidate_cache (review, error))
		return FALSE;

	/* send to server */
	if (!gs_odrs_provider_json_post (self->session, uri, data, cancellable, error))
		return FALSE;

	/* mark as voted */
	as_review_add_flags (review, AS_REVIEW_FLAG_VOTED);

	/* success */
	return TRUE;
}

static GsApp *
gs_odrs_provider_create_app_dummy (const gchar *id)
{
	GsApp *app = gs_app_new (id);
	g_autoptr(GString) str = NULL;
	str = g_string_new (id);
	gs_utils_gstring_replace (str, ".desktop", "");
	g_string_prepend (str, "No description is available for ");
	gs_app_set_name (app, GS_APP_QUALITY_LOWEST, "Unknown App");
	gs_app_set_summary (app, GS_APP_QUALITY_LOWEST, "App not found");
	gs_app_set_description (app, GS_APP_QUALITY_LOWEST, str->str);
	return app;
}

static void
gs_odrs_provider_init (GsOdrsProvider *self)
{
	g_mutex_init (&self->ratings_mutex);
}

static void
gs_odrs_provider_constructed (GObject *object)
{
	GsOdrsProvider *self = GS_ODRS_PROVIDER (object);

	G_OBJECT_CLASS (gs_odrs_provider_parent_class)->constructed (object);

	/* Check all required properties have been set. */
	g_assert (self->review_server != NULL);
	g_assert (self->user_hash != NULL);
	g_assert (self->distro != NULL);
}

static void
gs_odrs_provider_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
	GsOdrsProvider *self = GS_ODRS_PROVIDER (object);

	switch ((GsOdrsProviderProperty) prop_id) {
	case PROP_REVIEW_SERVER:
		g_value_set_string (value, self->review_server);
		break;
	case PROP_USER_HASH:
		g_value_set_string (value, self->user_hash);
		break;
	case PROP_DISTRO:
		g_value_set_string (value, self->distro);
		break;
	case PROP_MAX_CACHE_AGE_SECS:
		g_value_set_uint64 (value, self->max_cache_age_secs);
		break;
	case PROP_N_RESULTS_MAX:
		g_value_set_uint (value, self->n_results_max);
		break;
	case PROP_SESSION:
		g_value_set_object (value, self->session);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_odrs_provider_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
	GsOdrsProvider *self = GS_ODRS_PROVIDER (object);

	switch ((GsOdrsProviderProperty) prop_id) {
	case PROP_REVIEW_SERVER:
		/* Construct-only */
		g_assert (self->review_server == NULL);
		self->review_server = g_value_dup_string (value);
		break;
	case PROP_USER_HASH:
		/* Construct-only */
		g_assert (self->user_hash == NULL);
		self->user_hash = g_value_dup_string (value);
		break;
	case PROP_DISTRO:
		/* Construct-only */
		g_assert (self->distro == NULL);
		self->distro = g_value_dup_string (value);
		break;
	case PROP_MAX_CACHE_AGE_SECS:
		/* Construct-only */
		g_assert (self->max_cache_age_secs == 0);
		self->max_cache_age_secs = g_value_get_uint64 (value);
		break;
	case PROP_N_RESULTS_MAX:
		/* Construct-only */
		g_assert (self->n_results_max == 0);
		self->n_results_max = g_value_get_uint (value);
		break;
	case PROP_SESSION:
		/* Construct-only */
		g_assert (self->session == NULL);
		self->session = g_value_dup_object (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_odrs_provider_dispose (GObject *object)
{
	GsOdrsProvider *self = GS_ODRS_PROVIDER (object);

	g_clear_object (&self->session);

	G_OBJECT_CLASS (gs_odrs_provider_parent_class)->dispose (object);
}

static void
gs_odrs_provider_finalize (GObject *object)
{
	GsOdrsProvider *self = GS_ODRS_PROVIDER (object);

	g_free (self->user_hash);
	g_free (self->distro);
	g_free (self->review_server);
	g_clear_pointer (&self->ratings, g_array_unref);
	g_mutex_clear (&self->ratings_mutex);

	G_OBJECT_CLASS (gs_odrs_provider_parent_class)->finalize (object);
}

static void
gs_odrs_provider_class_init (GsOdrsProviderClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->constructed = gs_odrs_provider_constructed;
	object_class->get_property = gs_odrs_provider_get_property;
	object_class->set_property = gs_odrs_provider_set_property;
	object_class->dispose = gs_odrs_provider_dispose;
	object_class->finalize = gs_odrs_provider_finalize;

	/**
	 * GsOdrsProvider:review-server: (not nullable)
	 *
	 * The URI of the ODRS review server to contact.
	 *
	 * Since: 41
	 */
	obj_props[PROP_REVIEW_SERVER] =
		g_param_spec_string ("review-server", NULL, NULL,
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

	/**
	 * GsOdrsProvider:user-hash: (not nullable)
	 *
	 * An opaque hash of the user identifier, used to identify the user on
	 * the server.
	 *
	 * Since: 41
	 */
	obj_props[PROP_USER_HASH] =
		g_param_spec_string ("user-hash", NULL, NULL,
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

	/**
	 * GsOdrsProvider:distro: (not nullable)
	 *
	 * A human readable string identifying the current distribution.
	 *
	 * Since: 41
	 */
	obj_props[PROP_DISTRO] =
		g_param_spec_string ("distro", NULL, NULL,
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

	/**
	 * GsOdrsProvider:max-cache-age-secs:
	 *
	 * The maximum age of the ODRS cache files, in seconds. Older files will
	 * be refreshed on demand.
	 *
	 * Since: 41
	 */
	obj_props[PROP_MAX_CACHE_AGE_SECS] =
		g_param_spec_uint64 ("max-cache-age-secs", NULL, NULL,
				     0, G_MAXUINT64, 0,
				     G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

	/**
	 * GsOdrsProvider:n-results-max:
	 *
	 * Maximum number of reviews or ratings to download. The default value
	 * of 0 means no limit is applied.
	 *
	 * Since: 41
	 */
	obj_props[PROP_N_RESULTS_MAX] =
		g_param_spec_uint ("n-results-max", NULL, NULL,
				   0, G_MAXUINT, 0,
				   G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

	/**
	 * GsOdrsProvider:session: (not nullable)
	 *
	 * #SoupSession to use for downloading things.
	 *
	 * Since: 41
	 */
	obj_props[PROP_SESSION] =
		g_param_spec_object ("session", NULL, NULL,
				     SOUP_TYPE_SESSION,
				     G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);
}

/**
 * gs_odrs_provider_new:
 * @review_server: (not nullable): value for #GsOdrsProvider:review-server
 * @user_hash: (not nullable): value for #GsOdrsProvider:user-hash
 * @distro: (not nullable): value for #GsOdrsProvider:distro
 * @max_cache_age_secs: value for #GsOdrsProvider:max-cache-age-secs
 * @n_results_max: value for #GsOdrsProvider:n-results-max
 * @session: value for #GsOdrsProvider:session
 *
 * Create a new #GsOdrsProvider. This does no network activity.
 *
 * Returns: (transfer full): a new #GsOdrsProvider
 * Since: 41
 */
GsOdrsProvider *
gs_odrs_provider_new (const gchar *review_server,
                      const gchar *user_hash,
                      const gchar *distro,
                      guint64      max_cache_age_secs,
                      guint        n_results_max,
                      SoupSession *session)
{
	g_return_val_if_fail (review_server != NULL && *review_server != '\0', NULL);
	g_return_val_if_fail (user_hash != NULL && *user_hash != '\0', NULL);
	g_return_val_if_fail (distro != NULL && *distro != '\0', NULL);
	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);

	return g_object_new (GS_TYPE_ODRS_PROVIDER,
			     "review-server", review_server,
			     "user-hash", user_hash,
			     "distro", distro,
			     "max-cache-age-secs", max_cache_age_secs,
			     "n-results-max", n_results_max,
			     "session", session,
			     NULL);
}

static void download_ratings_cb (GObject      *source_object,
                                 GAsyncResult *result,
                                 gpointer      user_data);

/**
 * gs_odrs_provider_refresh_ratings_async:
 * @self: a #GsOdrsProvider
 * @cache_age_secs: cache age, in seconds, as passed to #GsPluginClass.refresh_metadata_async()
 * @progress_callback: (nullable): callback to call with progress information
 * @progress_user_data: (nullable) (closure progress_callback): data to pass
 *   to @progress_callback
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @callback: function to call when the asynchronous operation is complete
 * @user_data: data to pass to @callback
 *
 * Refresh the cached ODRS ratings and re-load them asynchronously.
 *
 * Since: 42
 */
void
gs_odrs_provider_refresh_ratings_async (GsOdrsProvider             *self,
                                        guint64                     cache_age_secs,
                                        GsDownloadProgressCallback  progress_callback,
                                        gpointer                    progress_user_data,
                                        GCancellable               *cancellable,
                                        GAsyncReadyCallback         callback,
                                        gpointer                    user_data)
{
	g_autofree gchar *cache_filename = NULL;
	g_autoptr(GFile) cache_file = NULL;
	g_autofree gchar *uri = NULL;
	g_autoptr(GError) error_local = NULL;
	g_autoptr(GTask) task = NULL;

	task = g_task_new (self, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_odrs_provider_refresh_ratings_async);

	/* check cache age */
	cache_filename = gs_utils_get_cache_filename ("odrs",
						      "ratings.json",
						      GS_UTILS_CACHE_FLAG_WRITEABLE |
						      GS_UTILS_CACHE_FLAG_CREATE_DIRECTORY,
						      &error_local);
	if (cache_filename == NULL) {
		g_task_return_error (task, g_steal_pointer (&error_local));
		return;
	}

	cache_file = g_file_new_for_path (cache_filename);
	g_task_set_task_data (task, g_object_ref (cache_file), g_object_unref);

	if (cache_age_secs > 0) {
		guint64 tmp;

		tmp = gs_utils_get_file_age (cache_file);
		if (tmp < cache_age_secs) {
			g_debug ("%s is only %" G_GUINT64_FORMAT " seconds old, so ignoring refresh",
				 cache_filename, tmp);
			if (!gs_odrs_provider_load_ratings (self, cache_filename, &error_local)) {
				g_debug ("Failed to load cache file ‘%s’, deleting it", cache_filename);
				g_file_delete (cache_file, NULL, NULL);

				g_task_return_error (task, g_steal_pointer (&error_local));
			} else {
				g_task_return_boolean (task, TRUE);
			}
			return;
		}
	}

	/* download the complete file */
	uri = g_strdup_printf ("%s/ratings", self->review_server);
	g_debug ("Updating ODRS cache from %s to %s", uri, cache_filename);

	gs_download_file_async (self->session, uri, cache_file, G_PRIORITY_LOW,
				progress_callback, progress_user_data,
				cancellable, download_ratings_cb, g_steal_pointer (&task));
}

static void
download_ratings_cb (GObject      *source_object,
                     GAsyncResult *result,
                     gpointer      user_data)
{
	SoupSession *soup_session = SOUP_SESSION (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	GsOdrsProvider *self = g_task_get_source_object (task);
	GFile *cache_file = g_task_get_task_data (task);
	const gchar *cache_file_path = NULL;
	g_autoptr(GError) local_error = NULL;

	if (!gs_download_file_finish (soup_session, result, &local_error) &&
	    !g_error_matches (local_error, GS_DOWNLOAD_ERROR, GS_DOWNLOAD_ERROR_NOT_MODIFIED)) {
		g_task_return_new_error (task, GS_ODRS_PROVIDER_ERROR,
					 GS_ODRS_PROVIDER_ERROR_DOWNLOADING,
					 "%s", local_error->message);
		return;
	}

	g_clear_error (&local_error);

	cache_file_path = g_file_peek_path (cache_file);
	if (!gs_odrs_provider_load_ratings (self, cache_file_path, &local_error)) {
		g_debug ("Failed to load cache file ‘%s’, deleting it", cache_file_path);
		g_file_delete (cache_file, NULL, NULL);

		g_task_return_new_error (task, GS_ODRS_PROVIDER_ERROR,
					 GS_ODRS_PROVIDER_ERROR_PARSING_DATA,
					 "%s", local_error->message);
	} else {
		g_task_return_boolean (task, TRUE);
	}
}

/**
 * gs_odrs_provider_refresh_ratings_finish:
 * @self: a #GsOdrsProvider
 * @result: result of the asynchronous operation
 * @error: return location for a #GError, or %NULL
 *
 * Finish an asynchronous refresh operation started with
 * gs_odrs_provider_refresh_ratings_async().
 *
 * Returns: %TRUE on success, %FALSE otherwise
 * Since: 42
 */
gboolean
gs_odrs_provider_refresh_ratings_finish (GsOdrsProvider  *self,
                                         GAsyncResult    *result,
                                         GError         **error)
{
	g_return_val_if_fail (GS_IS_ODRS_PROVIDER (self), FALSE);
	g_return_val_if_fail (g_task_is_valid (result, self), FALSE);
	g_return_val_if_fail (g_task_get_source_tag (G_TASK (result)) == gs_odrs_provider_refresh_ratings_async, FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	return g_task_propagate_boolean (G_TASK (result), error);
}

static void refine_app_op (GsOdrsProvider            *self,
                           GTask                     *task,
                           GsApp                     *app,
                           GsOdrsProviderRefineFlags  flags,
                           GCancellable              *cancellable);
static void refine_reviews_cb (GObject      *source_object,
                               GAsyncResult *result,
                               gpointer      user_data);
static void finish_refine_op (GTask  *task,
                              GError *error);

typedef struct {
	/* Input data. */
	GsAppList *list;  /* (owned) (not nullable) */
	GsOdrsProviderRefineFlags flags;

	/* In-progress data. */
	guint n_pending_ops;
	GError *error;  /* (nullable) (owned) */
} RefineData;

static void
refine_data_free (RefineData *data)
{
	g_assert (data->n_pending_ops == 0);

	g_clear_object (&data->list);
	g_clear_error (&data->error);

	g_free (data);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (RefineData, refine_data_free)

/**
 * gs_odrs_provider_refine_async:
 * @self: a #GsOdrsProvider
 * @list: list of apps to refine
 * @flags: refine flags
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @callback: callback for asynchronous completion
 * @user_data: data to pass to @callback
 *
 * Asynchronously refine the given @list of apps to add ratings and review data
 * to them, as specified in @flags.
 *
 * Since: 42
 */
void
gs_odrs_provider_refine_async (GsOdrsProvider            *self,
                               GsAppList                 *list,
                               GsOdrsProviderRefineFlags  flags,
                               GCancellable              *cancellable,
                               GAsyncReadyCallback        callback,
                               gpointer                   user_data)
{
	g_autoptr(GTask) task = NULL;
	g_autoptr(RefineData) data = NULL;
	RefineData *data_unowned = NULL;

	task = g_task_new (self, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_odrs_provider_refine_async);

	data_unowned = data = g_new0 (RefineData, 1);
	data->list = g_object_ref (list);
	data->flags = flags;
	g_task_set_task_data (task, g_steal_pointer (&data), (GDestroyNotify) refine_data_free);

	if ((flags & (GS_ODRS_PROVIDER_REFINE_FLAGS_GET_RATINGS |
		      GS_ODRS_PROVIDER_REFINE_FLAGS_GET_REVIEWS)) == 0) {
		g_task_return_boolean (task, TRUE);
		return;
	}

	/* Mark one operation as pending while all the operations are started,
	 * so the overall operation can’t complete while things are still being
	 * started. */
	data_unowned->n_pending_ops++;

	for (guint i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app = gs_app_list_index (list, i);

		/* not valid */
		if (gs_app_get_kind (app) == AS_COMPONENT_KIND_ADDON)
			continue;
		if (gs_app_get_id (app) == NULL)
			continue;

		data_unowned->n_pending_ops++;
		refine_app_op (self, task, app, flags, cancellable);
	}

	finish_refine_op (task, NULL);
}

static void
refine_app_op (GsOdrsProvider            *self,
               GTask                     *task,
               GsApp                     *app,
               GsOdrsProviderRefineFlags  flags,
               GCancellable              *cancellable)
{
	g_autoptr(GError) local_error = NULL;

	/* add ratings if possible */
	if ((flags & GS_ODRS_PROVIDER_REFINE_FLAGS_GET_RATINGS) &&
	    gs_app_get_review_ratings (app) == NULL) {
		if (!gs_odrs_provider_refine_ratings (self, app, cancellable, &local_error)) {
			if (g_error_matches (local_error, GS_ODRS_PROVIDER_ERROR, GS_ODRS_PROVIDER_ERROR_NO_NETWORK)) {
				g_debug ("failed to refine app %s: %s",
					 gs_app_get_unique_id (app), local_error->message);
			} else {
				g_prefix_error (&local_error, "failed to refine app: ");
				finish_refine_op (task, g_steal_pointer (&local_error));
				return;
			}
		}
	}

	/* add reviews if possible */
	if ((flags & GS_ODRS_PROVIDER_REFINE_FLAGS_GET_REVIEWS) &&
	    gs_app_get_reviews (app)->len == 0) {
		/* get from server asynchronously */
		gs_odrs_provider_fetch_reviews_for_app_async (self, app, cancellable, refine_reviews_cb, g_object_ref (task));
	} else {
		finish_refine_op (task, NULL);
	}
}

static void
refine_reviews_cb (GObject      *source_object,
                   GAsyncResult *result,
                   gpointer      user_data)
{
	GsOdrsProvider *self = GS_ODRS_PROVIDER (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	g_autoptr(GError) local_error = NULL;

	if (!gs_odrs_provider_fetch_reviews_for_app_finish (self, result, &local_error)) {
		if (g_error_matches (local_error, GS_ODRS_PROVIDER_ERROR, GS_ODRS_PROVIDER_ERROR_NO_NETWORK)) {
			g_debug ("failed to refine app: %s", local_error->message);
		} else {
			g_prefix_error (&local_error, "failed to refine app: ");
			finish_refine_op (task, g_steal_pointer (&local_error));
			return;
		}
	}

	finish_refine_op (task, NULL);
}

/* @error is (transfer full) if non-NULL. */
static void
finish_refine_op (GTask  *task,
                  GError *error)
{
	RefineData *data = g_task_get_task_data (task);
	g_autoptr(GError) error_owned = g_steal_pointer (&error);

	if (data->error == NULL && error_owned != NULL)
		data->error = g_steal_pointer (&error_owned);
	else if (error_owned != NULL)
		g_debug ("Additional error while refining ODRS data: %s", error_owned->message);

	g_assert (data->n_pending_ops > 0);
	data->n_pending_ops--;

	if (data->n_pending_ops == 0) {
		if (data->error != NULL)
			g_task_return_error (task, g_steal_pointer (&data->error));
		else
			g_task_return_boolean (task, TRUE);
	}
}

/**
 * gs_odrs_provider_refine_finish:
 * @self: a #GsOdrsProvider
 * @result: result of the asynchronous operation
 * @error: return location for a #GError, or %NULL
 *
 * Finish an asynchronous refine operation started with
 * gs_odrs_provider_refine_finish().
 *
 * Returns: %TRUE on success, %FALSE otherwise
 * Since: 42
 */
gboolean
gs_odrs_provider_refine_finish (GsOdrsProvider  *self,
                                GAsyncResult    *result,
                                GError         **error)
{
	g_return_val_if_fail (GS_IS_ODRS_PROVIDER (self), FALSE);
	g_return_val_if_fail (g_task_is_valid (result, self), FALSE);
	g_return_val_if_fail (g_async_result_is_tagged (result, gs_odrs_provider_refine_async), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	return g_task_propagate_boolean (G_TASK (result), error);
}

/**
 * gs_odrs_provider_submit_review:
 * @self: a #GsOdrsProvider
 * @app: the app being reviewed
 * @review: the review
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @error: return location for a #GError
 *
 * Submit a new @review for @app.
 *
 * Returns: %TRUE on success, %FALSE otherwise
 * Since: 41
 */
gboolean
gs_odrs_provider_submit_review (GsOdrsProvider  *self,
                                GsApp           *app,
                                AsReview        *review,
                                GCancellable    *cancellable,
                                GError         **error)
{
	g_autofree gchar *data = NULL;
	g_autofree gchar *uri = NULL;
	g_autofree gchar *version = NULL;
	g_autoptr(JsonBuilder) builder = NULL;
	g_autoptr(JsonGenerator) json_generator = NULL;
	g_autoptr(JsonNode) json_root = NULL;

	/* save as we don't re-request the review from the server */
	as_review_add_flags (review, AS_REVIEW_FLAG_SELF);
	as_review_set_reviewer_name (review, g_get_real_name ());
	as_review_add_metadata (review, "app_id", gs_app_get_id (app));
	as_review_add_metadata (review, "user_skey",
				gs_app_get_metadata_item (app, "ODRS::user_skey"));

	/* create object with review data */
	builder = json_builder_new ();
	json_builder_begin_object (builder);
	json_builder_set_member_name (builder, "user_hash");
	json_builder_add_string_value (builder, self->user_hash);
	json_builder_set_member_name (builder, "user_skey");
	json_builder_add_string_value (builder,
				       as_review_get_metadata_item (review, "user_skey"));
	json_builder_set_member_name (builder, "app_id");
	json_builder_add_string_value (builder,
				       as_review_get_metadata_item (review, "app_id"));
	json_builder_set_member_name (builder, "locale");
	json_builder_add_string_value (builder, setlocale (LC_MESSAGES, NULL));
	json_builder_set_member_name (builder, "distro");
	json_builder_add_string_value (builder, self->distro);
	json_builder_set_member_name (builder, "version");
	version = gs_odrs_provider_trim_version (as_review_get_version (review));
	json_builder_add_string_value (builder, version);
	json_builder_set_member_name (builder, "user_display");
	json_builder_add_string_value (builder, as_review_get_reviewer_name (review));
	json_builder_set_member_name (builder, "summary");
	json_builder_add_string_value (builder, as_review_get_summary (review));
	json_builder_set_member_name (builder, "description");
	json_builder_add_string_value (builder, as_review_get_description (review));
	json_builder_set_member_name (builder, "rating");
	json_builder_add_int_value (builder, as_review_get_rating (review));
	json_builder_end_object (builder);

	/* export as a string */
	json_root = json_builder_get_root (builder);
	json_generator = json_generator_new ();
	json_generator_set_pretty (json_generator, TRUE);
	json_generator_set_root (json_generator, json_root);
	data = json_generator_to_data (json_generator, NULL);

	/* clear cache */
	if (!gs_odrs_provider_invalidate_cache (review, error))
		return FALSE;

	/* POST */
	uri = g_strdup_printf ("%s/submit", self->review_server);
	if (!gs_odrs_provider_json_post (self->session, uri, data, cancellable, error))
		return FALSE;

	/* modify the local app */
	gs_app_add_review (app, review);

	return TRUE;
}

/**
 * gs_odrs_provider_report_review:
 * @self: a #GsOdrsProvider
 * @app: the app whose review is being reported
 * @review: the review to report
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @error: return location for a #GError
 *
 * Report the given @review on @app for being incorrect or breaking the code of
 * conduct.
 *
 * Returns: %TRUE on success, %FALSE otherwise
 * Since: 41
 */
gboolean
gs_odrs_provider_report_review (GsOdrsProvider  *self,
                                GsApp           *app,
                                AsReview        *review,
                                GCancellable    *cancellable,
                                GError         **error)
{
	g_autofree gchar *uri = NULL;
	uri = g_strdup_printf ("%s/report", self->review_server);
	return gs_odrs_provider_vote (self, review, uri, cancellable, error);
}

/**
 * gs_odrs_provider_upvote_review:
 * @self: a #GsOdrsProvider
 * @app: the app whose review is being upvoted
 * @review: the review to upvote
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @error: return location for a #GError
 *
 * Add one vote to @review on @app.
 *
 * Returns: %TRUE on success, %FALSE otherwise
 * Since: 41
 */
gboolean
gs_odrs_provider_upvote_review (GsOdrsProvider  *self,
                                GsApp           *app,
                                AsReview        *review,
                                GCancellable    *cancellable,
                                GError         **error)
{
	g_autofree gchar *uri = NULL;
	uri = g_strdup_printf ("%s/upvote", self->review_server);
	return gs_odrs_provider_vote (self, review, uri, cancellable, error);
}

/**
 * gs_odrs_provider_downvote_review:
 * @self: a #GsOdrsProvider
 * @app: the app whose review is being downvoted
 * @review: the review to downvote
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @error: return location for a #GError
 *
 * Remove one vote from @review on @app.
 *
 * Returns: %TRUE on success, %FALSE otherwise
 * Since: 41
 */
gboolean
gs_odrs_provider_downvote_review (GsOdrsProvider  *self,
                                  GsApp           *app,
                                  AsReview        *review,
                                  GCancellable    *cancellable,
                                  GError         **error)
{
	g_autofree gchar *uri = NULL;
	uri = g_strdup_printf ("%s/downvote", self->review_server);
	return gs_odrs_provider_vote (self, review, uri, cancellable, error);
}

/**
 * gs_odrs_provider_dismiss_review:
 * @self: a #GsOdrsProvider
 * @app: the app whose review is being dismissed
 * @review: the review to dismiss
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @error: return location for a #GError
 *
 * Dismiss (ignore) @review on @app when moderating.
 *
 * Returns: %TRUE on success, %FALSE otherwise
 * Since: 41
 */
gboolean
gs_odrs_provider_dismiss_review (GsOdrsProvider  *self,
                                 GsApp           *app,
                                 AsReview        *review,
                                 GCancellable    *cancellable,
                                 GError         **error)
{
	g_autofree gchar *uri = NULL;
	uri = g_strdup_printf ("%s/dismiss", self->review_server);
	return gs_odrs_provider_vote (self, review, uri, cancellable, error);
}

/**
 * gs_odrs_provider_remove_review:
 * @self: a #GsOdrsProvider
 * @app: the app whose review is being removed
 * @review: the review to remove
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @error: return location for a #GError
 *
 * Remove a @review written by the user, from @app.
 *
 * Returns: %TRUE on success, %FALSE otherwise
 * Since: 41
 */
gboolean
gs_odrs_provider_remove_review (GsOdrsProvider  *self,
                                GsApp           *app,
                                AsReview        *review,
                                GCancellable    *cancellable,
                                GError         **error)
{
	g_autofree gchar *uri = NULL;
	uri = g_strdup_printf ("%s/remove", self->review_server);
	if (!gs_odrs_provider_vote (self, review, uri, cancellable, error))
		return FALSE;

	/* update the local app */
	gs_app_remove_review (app, review);

	return TRUE;
}

/**
 * gs_odrs_provider_add_unvoted_reviews:
 * @self: a #GsOdrsProvider
 * @list: list of apps to add unvoted reviews to
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @error: return location for a #GError
 *
 * Add the unmoderated reviews for each app in @list to the apps.
 *
 * Returns: %TRUE on success, %FALSE otherwise
 * Since: 41
 */
gboolean
gs_odrs_provider_add_unvoted_reviews (GsOdrsProvider  *self,
                                      GsAppList       *list,
                                      GCancellable    *cancellable,
                                      GError         **error)
{
	guint status_code;
	guint i;
	gconstpointer downloaded_data;
	gsize downloaded_data_length;
	g_autofree gchar *uri = NULL;
	g_autoptr(GHashTable) hash = NULL;
	g_autoptr(JsonParser) json_parser = NULL;
	g_autoptr(GPtrArray) reviews = NULL;
	g_autoptr(SoupMessage) msg = NULL;
#if SOUP_CHECK_VERSION(3, 0, 0)
	g_autoptr(GBytes) bytes = NULL;
#endif
	g_autoptr(GError) local_error = NULL;

	/* create the GET data *with* the machine hash so we can later
	 * review the app ourselves */
	uri = g_strdup_printf ("%s/moderate/%s/%s",
			       self->review_server,
			       self->user_hash,
			       setlocale (LC_MESSAGES, NULL));
	msg = soup_message_new (SOUP_METHOD_GET, uri);
#if SOUP_CHECK_VERSION(3, 0, 0)
	bytes = soup_session_send_and_read (self->session, msg, cancellable, error);
	if (bytes == NULL)
		return FALSE;

	downloaded_data = g_bytes_get_data (bytes, &downloaded_data_length);
	status_code = soup_message_get_status (msg);
#else
	status_code = soup_session_send_message (self->session, msg);
	downloaded_data = msg->response_body ? msg->response_body->data : NULL;
	downloaded_data_length = msg->response_body ? msg->response_body->length : 0;
#endif
	if (status_code != SOUP_STATUS_OK) {
		g_autoptr(GInputStream) input_stream = g_memory_input_stream_new_from_data (downloaded_data, downloaded_data_length, NULL);
		if (!gs_odrs_provider_parse_success (input_stream, error))
			return FALSE;
		/* not sure what to do here */
		g_set_error_literal (error,
				     GS_ODRS_PROVIDER_ERROR,
				     GS_ODRS_PROVIDER_ERROR_DOWNLOADING,
				     "status code invalid");
		return FALSE;
	}
	g_debug ("odrs returned: %.*s", (gint) downloaded_data_length, (const gchar *) downloaded_data);

	/* nothing */
	if (downloaded_data == NULL) {
		if (!g_network_monitor_get_network_available (g_network_monitor_get_default ()))
			g_set_error_literal (error,
					     GS_ODRS_PROVIDER_ERROR,
					     GS_ODRS_PROVIDER_ERROR_NO_NETWORK,
					     "server couldn't be reached");
		else
			g_set_error_literal (error,
					     GS_ODRS_PROVIDER_ERROR,
					     GS_ODRS_PROVIDER_ERROR_PARSING_DATA,
					     "server returned no data");
		return FALSE;
	}

	/* parse the data and find the array of ratings */
	json_parser = json_parser_new_immutable ();
	if (!json_parser_load_from_data (json_parser, downloaded_data, downloaded_data_length, &local_error)) {
		g_set_error (error,
			     GS_ODRS_PROVIDER_ERROR,
			     GS_ODRS_PROVIDER_ERROR_PARSING_DATA,
			     "Error parsing ODRS data: %s", local_error->message);
		return FALSE;
	}

	reviews = gs_odrs_provider_parse_reviews (self, json_parser, error);
	if (reviews == NULL)
		return FALSE;

	/* look at all the reviews; faking app objects */
	hash = g_hash_table_new_full (g_str_hash, g_str_equal,
				      g_free, g_object_unref);
	for (i = 0; i < reviews->len; i++) {
		GsApp *app;
		AsReview *review;
		const gchar *app_id;

		/* same app? */
		review = g_ptr_array_index (reviews, i);
		app_id = as_review_get_metadata_item (review, "app_id");
		app = g_hash_table_lookup (hash, app_id);
		if (app == NULL) {
			app = gs_odrs_provider_create_app_dummy (app_id);
			gs_app_list_add (list, app);
			g_hash_table_insert (hash, g_strdup (app_id), app);
		}
		gs_app_add_review (app, review);
	}

	return TRUE;
}
