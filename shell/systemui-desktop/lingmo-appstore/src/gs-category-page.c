/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
 * Copyright (C) 2014-2018 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <string.h>
#include <glib/gi18n.h>

#include "gs-app-list-private.h"
#include "gs-common.h"
#include "gs-featured-carousel.h"
#include "gs-summary-tile.h"
#include "gs-category-page.h"
#include "gs-utils.h"

struct _GsCategoryPage
{
	GsPage		 parent_instance;

	GsPluginLoader	*plugin_loader;
	GCancellable	*cancellable;
	GsCategory	*category;
	GsCategory	*subcategory;
	gboolean	 content_valid;

	GtkWidget	*top_carousel;
	GtkWidget	*other_heading;
	GtkWidget	*category_detail_box;
	GtkWidget	*scrolledwindow_category;
	GtkWidget	*featured_flow_box;
	GtkWidget	*recently_updated_flow_box;
	GtkWidget	*web_apps_flow_box;
};

G_DEFINE_TYPE (GsCategoryPage, gs_category_page, GS_TYPE_PAGE)

/* See https://github.com/LingmoOS/lingmo-appstore/-/issues/2053
 * for the rationale behind the numbers */
#define MAX_RECENT_APPS_TO_DISPLAY 12
#define MIN_RECENT_APPS_REQUIRED 50
#define MIN_SECTION_APPS 3

#define validate_app_buckets() \
	n_total_apps = n_carousel_apps + n_featured_apps + n_recently_updated_apps + n_web_apps + n_other_apps; \
	print_app_bucket_stats (self, n_carousel_apps, n_featured_apps, n_recently_updated_apps, n_web_apps, n_other_apps, n_category_apps); \
	\
	g_assert (n_total_apps == n_category_apps); \
	g_assert (n_featured_apps == 0 || n_featured_apps == featured_app_tiles->len); \
	g_assert (n_recently_updated_apps == 0 || n_recently_updated_apps == recently_updated_app_tiles->len); \
	g_assert (n_web_apps == 0 || n_web_apps == web_app_tiles->len); \
	g_assert (n_other_apps == 0 || n_other_apps == other_app_tiles->len); \

typedef enum {
	PROP_CATEGORY = 1,
	/* Override properties: */
	PROP_TITLE,
} GsCategoryPageProperty;

static GParamSpec *obj_props[PROP_CATEGORY + 1] = { NULL, };

typedef enum {
	SIGNAL_APP_CLICKED,
} GsCategoryPageSignal;

static guint obj_signals[SIGNAL_APP_CLICKED + 1] = { 0, };

static void
app_activated_cb (GsCategoryPage *self, GsAppTile *tile)
{
	GsApp *app;

	app = gs_app_tile_get_app (tile);

	if (!app)
		return;

	g_signal_emit (self, obj_signals[SIGNAL_APP_CLICKED], 0, app);
}

static void
top_carousel_app_clicked_cb (GsFeaturedCarousel *carousel,
                             GsApp              *app,
                             gpointer            user_data)
{
	GsCategoryPage *self = GS_CATEGORY_PAGE (user_data);

	g_signal_emit (self, obj_signals[SIGNAL_APP_CLICKED], 0, app);
}

static void
gs_category_page_add_placeholders (GsCategoryPage *self,
                                   GtkFlowBox     *flow_box,
                                   guint           n_placeholders)
{
	gs_widget_remove_all (GTK_WIDGET (flow_box), (GsRemoveFunc) gtk_flow_box_remove);

	for (guint i = 0; i < n_placeholders; ++i) {
		GtkWidget *tile = gs_summary_tile_new (NULL);
		gtk_flow_box_insert (flow_box, tile, -1);
	}

	gtk_widget_set_visible (GTK_WIDGET (flow_box), TRUE);
}

typedef struct {
	GsCategoryPage *page;  /* (owned) */
	GHashTable *featured_app_ids;  /* (owned) (nullable) (element-type utf8 utf8) */
	gboolean get_featured_apps_finished;
	GsAppList *apps;  /* (owned) (nullable) */
	gboolean get_main_apps_finished;
	gboolean cancelled;
} LoadCategoryData;

static void
load_category_data_free (LoadCategoryData *data)
{
	g_clear_object (&data->page);
	g_clear_pointer (&data->featured_app_ids, g_hash_table_unref);
	g_clear_object (&data->apps);
	g_free (data);
}

static void load_category_finish (LoadCategoryData *data);

static void
gs_category_page_get_featured_apps_cb (GObject *source_object,
				       GAsyncResult *res,
				       gpointer user_data)
{
	LoadCategoryData *data = user_data;
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source_object);
	g_autoptr(GError) local_error = NULL;
	g_autoptr(GsAppList) list = NULL;
	g_autoptr(GHashTable) featured_app_ids = NULL;

	list = gs_plugin_loader_job_process_finish (plugin_loader,
						    res,
						    &local_error);
	if (list == NULL) {
		if (!g_error_matches (local_error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) &&
		    !g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			g_warning ("failed to get featured apps for category apps: %s", local_error->message);
		else
			data->cancelled = TRUE;
		data->get_featured_apps_finished = TRUE;
		load_category_finish (data);
		return;
	}

	featured_app_ids = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

	for (guint i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app = gs_app_list_index (list, i);

		g_hash_table_add (featured_app_ids, g_strdup (gs_app_get_id (app)));
	}

	data->featured_app_ids = g_steal_pointer (&featured_app_ids);
	data->get_featured_apps_finished = TRUE;
	load_category_finish (data);
}

static void
gs_category_page_get_apps_cb (GObject *source_object,
                              GAsyncResult *res,
                              gpointer user_data)
{
	LoadCategoryData *data = user_data;
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source_object);
	g_autoptr(GError) local_error = NULL;
	g_autoptr(GsAppList) list = NULL;

	list = gs_plugin_loader_job_process_finish (plugin_loader,
						    res,
						    &local_error);
	if (list == NULL) {
		if (!g_error_matches (local_error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) &&
		    !g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			g_warning ("failed to get apps for category apps: %s", local_error->message);
		else
			data->cancelled = TRUE;
		data->get_main_apps_finished = TRUE;
		load_category_finish (data);
		return;
	}

	data->apps = g_steal_pointer (&list);
	data->get_main_apps_finished = TRUE;
	load_category_finish (data);
}

static gboolean
app_has_hi_res_icon (GsCategoryPage *self,
		     GsApp *app)
{
	g_autoptr(GIcon) icon = NULL;

	/* This is the minimum icon size needed by `GsFeatureTile`. */
	icon = gs_app_get_icon_for_size (app,
					 128,
					 gtk_widget_get_scale_factor (GTK_WIDGET (self)),
					 NULL);

	/* Returning TRUE means to keep the app in the list */
	return (icon != NULL);
}

static GsAppList *
choose_top_carousel_apps (LoadCategoryData *data,
                          guint64           recently_updated_cutoff_secs)
{
	guint n_top_carousel_apps;
	g_autoptr(GPtrArray) candidates = g_ptr_array_new_with_free_func (NULL);
	g_autoptr(GsAppList) top_carousel_apps = gs_app_list_new ();
	guint top_carousel_seed;
	g_autoptr(GRand) top_carousel_rand = NULL;

	if (data->apps == NULL ||
	    gs_app_list_length (data->apps) < 20) {
		g_debug ("%u is not enough category apps, hiding top carousel", data->apps == NULL ? 0 : gs_app_list_length (data->apps));
		return g_steal_pointer (&top_carousel_apps);
	}

	/* See https://github.com/LingmoOS/lingmo-appstore/-/issues/2053 for design rationale */
	if (gs_app_list_length (data->apps) < 40)
		n_top_carousel_apps = 3;
	else
		n_top_carousel_apps = 5;

	/* The top carousel should contain @n_top_carousel_apps, taken from the
	 * set of featured or recently updated apps which have hi-res icons.
	 *
	 * The apps in the top carousel should be changed on a fixed schedule,
	 * once a week.
	 */
	top_carousel_seed = (g_get_real_time () / G_USEC_PER_SEC) / (7 * 24 * 60 * 60);
	top_carousel_rand = g_rand_new_with_seed (top_carousel_seed);
	g_debug ("Top carousel seed: %u", top_carousel_seed);

	for (guint i = 0; i < gs_app_list_length (data->apps); i++) {
		GsApp *app = gs_app_list_index (data->apps, i);
		gboolean is_featured, is_recently_updated, is_hi_res;

		is_featured = (data->featured_app_ids != NULL &&
			       g_hash_table_contains (data->featured_app_ids, gs_app_get_id (app)));
		is_recently_updated = (gs_app_get_release_date (app) > recently_updated_cutoff_secs);
		is_hi_res = app_has_hi_res_icon (data->page, app);

		if ((is_featured || is_recently_updated) && is_hi_res)
			g_ptr_array_add (candidates, app);
	}

	/* If there aren’t enough candidate apps to populate the top carousel,
	 * return an empty app list. */
	if (candidates->len < n_top_carousel_apps) {
		g_debug ("Only %u candidate apps for top carousel, needed at least %u; returning empty", candidates->len, n_top_carousel_apps);
		goto out;
	}

	/* Select @n_top_carousel_apps from @candidates uniformly randomly
	 * without replacement. */
	for (guint i = 0; i < n_top_carousel_apps; i++) {
		guint random_index = g_rand_int_range (top_carousel_rand, 0, candidates->len);
		GsApp *app = g_ptr_array_index (candidates, random_index);

		gs_app_list_add (top_carousel_apps, app);
		g_ptr_array_remove_index_fast (candidates, random_index);
	}

 out:
	g_assert (gs_app_list_length (top_carousel_apps) == 0 ||
		  gs_app_list_length (top_carousel_apps) == n_top_carousel_apps);

	return g_steal_pointer (&top_carousel_apps);
}

static gint
app_name_flowbox_sort_func (GtkFlowBoxChild *child1,
			    GtkFlowBoxChild *child2,
			    gpointer         user_data)
{
	GsAppTile *tile1 = GS_APP_TILE (child1);
	GsAppTile *tile2 = GS_APP_TILE (child2);
	GsApp *app1 = gs_app_tile_get_app (tile1);
	GsApp *app2 = gs_app_tile_get_app (tile2);

	/* Placeholder tiles have no app. */
	if (app1 == NULL && app2 == NULL)
		return 0;
	if (app1 == NULL)
		return 1;
	if (app2 == NULL)
		return -1;

	return gs_utils_app_sort_name (app1, app2, NULL);
}

static gint
release_date_sort_func (GsAppTile *tile1,
			GsAppTile *tile2)
{
	GsApp *app1 = gs_app_tile_get_app (tile1);
	GsApp *app2 = gs_app_tile_get_app (tile2);
	guint64 release_date1, release_date2;

	/* Placeholder tiles have no app. */
	if (app1 == NULL && app2 == NULL)
		return 0;
	if (app1 == NULL)
		return 1;
	if (app2 == NULL)
		return -1;

	release_date1 = gs_app_get_release_date (app1);
	release_date2 = gs_app_get_release_date (app2);

	if (release_date1 == release_date2)
		return gs_utils_app_sort_name (app1, app2, NULL);

	return release_date1 < release_date2 ? 1 : -1;
}

static gint
release_date_gptrarray_sort_func (gconstpointer tile1,
				  gconstpointer tile2)
{
	GsAppTile *tile_a = (*(GsAppTile **) tile1);
	GsAppTile *tile_b = (*(GsAppTile **) tile2);

	return release_date_sort_func (tile_a, tile_b);
}

static gint
release_date_flowbox_sort_func (GtkFlowBoxChild *child1,
				GtkFlowBoxChild *child2,
				gpointer         user_data)
{
	GsAppTile *tile1 = GS_APP_TILE (child1);
	GsAppTile *tile2 = GS_APP_TILE (child2);

	return release_date_sort_func (tile1, tile2);
}


/* Sort all flow boxes in this page, if 'enable' is TRUE, else disable
   sorting on all flow boxes. Sorting should always be enabled, except
   for cases when doing bulk additions, where sorting once after the
   bulk addition will offer better performance. */
static void
gs_category_page_set_sort (GsCategoryPage *self, gboolean enable)
{
	GtkFlowBoxSortFunc name_sort_func = NULL;
	GtkFlowBoxSortFunc recent_sort_func = NULL;

	if (enable) {
		name_sort_func = app_name_flowbox_sort_func;
		recent_sort_func = release_date_flowbox_sort_func;
	}

	/* Sort the featured apps flowbox by app name, when sorting is
	   enabled. */
	gtk_flow_box_set_sort_func (GTK_FLOW_BOX (self->featured_flow_box),
				    name_sort_func,
				    NULL,
				    NULL);

	/* Sort the recent apps flowbox by release date, when sorting
	   is enabled. Note that recent apps flowbox is already
	   sorted. This call is there to account for the case
	   (possibly in future), where app tiles are added to the
	   flowbox after the initial population in 'populate_flow_boxes ()'. */
	gtk_flow_box_set_sort_func (GTK_FLOW_BOX (self->recently_updated_flow_box),
				    recent_sort_func,
				    NULL,
				    NULL);

	/* Sort the web apps flowbox by app name, when sorting is enabled. */
	gtk_flow_box_set_sort_func (GTK_FLOW_BOX (self->web_apps_flow_box),
				    name_sort_func,
				    NULL,
				    NULL);

	/* Sort the other apps flowbox by app name, when sorting is enabled. */
	gtk_flow_box_set_sort_func (GTK_FLOW_BOX (self->category_detail_box),
				    name_sort_func,
				    NULL,
				    NULL);
}

static void
populate_flow_boxes (GsCategoryPage *self,
		     GPtrArray *featured_app_tiles,
		     GPtrArray *recently_updated_app_tiles,
		     GPtrArray *web_app_tiles,
		     GPtrArray *other_app_tiles)
{
	guint i;
	GtkWidget *tile;

	/* Disable sorting on all flowboxes so we don't sort on each
	   flowbox insertion which is not good for performance. */
	gs_category_page_set_sort (self, FALSE);

	/* Populate featured flowbox */
	if (featured_app_tiles) {
		for (i = 0; i < featured_app_tiles->len; i++) {
			tile = g_ptr_array_index (featured_app_tiles, i);
			gtk_flow_box_insert (GTK_FLOW_BOX (self->featured_flow_box), tile, -1);
		}
	}

	/* Populate recently updated flowbox */
	if (recently_updated_app_tiles) {
		for (i = 0; i < recently_updated_app_tiles->len; i++) {
			guint64 release_date;
			g_autofree gchar *release_date_tooltip = NULL;
			tile = g_ptr_array_index (recently_updated_app_tiles, i);

			/* Shows the latest release date of the app in
			   relative format (e.g. "10 days ago") on hover. */
			release_date = gs_app_get_release_date (gs_app_tile_get_app (GS_APP_TILE (tile)));
			release_date_tooltip = gs_utils_time_to_string (release_date);
			gtk_widget_set_tooltip_text (tile, release_date_tooltip);

			gtk_flow_box_insert (GTK_FLOW_BOX (self->recently_updated_flow_box), tile, -1);
		}
	}

	/* Populate web apps flowbox */
	if (web_app_tiles) {
		for (i = 0; i < web_app_tiles->len; i++) {
			tile = g_ptr_array_index (web_app_tiles, i);
			gtk_flow_box_insert (GTK_FLOW_BOX (self->web_apps_flow_box), tile, -1);
		}
	}

	/* Populate other apps flowbox */
	if (other_app_tiles) {
		for (i = 0; i < other_app_tiles->len; i++) {
			tile = g_ptr_array_index (other_app_tiles, i);
			gtk_flow_box_insert (GTK_FLOW_BOX (self->category_detail_box), tile, -1);
		}
	}

	/* Re-enable sorting on all flowboxes now that they are fully
	   populated */
	gs_category_page_set_sort (self, TRUE);
}

static void
print_app_bucket_stats (GsCategoryPage *self,
			guint64 n_carousel_apps,
			guint64 n_featured_apps,
			guint64 n_recently_updated_apps,
			guint64 n_web_apps,
			guint64 n_other_apps,
			guint64 n_total_apps)
{
	g_debug ("[%s] Carousel apps: %" G_GUINT64_FORMAT ", Featured apps: %" G_GUINT64_FORMAT ", Recent apps: %" G_GUINT64_FORMAT ", "
                 "Web apps: %" G_GUINT64_FORMAT ", Other apps: %" G_GUINT64_FORMAT ", Total apps: %" G_GUINT64_FORMAT,
                 gs_category_get_name (self->category),
                 n_carousel_apps, n_featured_apps, n_recently_updated_apps,
                 n_web_apps, n_other_apps, n_total_apps);
}

static void
load_category_finish (LoadCategoryData *data)
{
	GsCategoryPage *self = data->page;
	guint64 recently_updated_cutoff_secs;
	guint64 n_recently_updated_apps = 0;
	guint64 n_featured_apps = 0;
	guint64 n_web_apps = 0;
	guint64 n_carousel_apps = 0;
	guint64 n_other_apps = 0;
	guint64 n_category_apps = 0;
	guint64 n_total_apps;
	g_autoptr(GPtrArray) featured_app_tiles = g_ptr_array_new ();
	g_autoptr(GPtrArray) recently_updated_app_tiles = g_ptr_array_new ();
	g_autoptr(GPtrArray) web_app_tiles = g_ptr_array_new ();
	g_autoptr(GPtrArray) other_app_tiles = NULL;
	g_autoptr(GsAppList) top_carousel_apps = NULL;
	GtkWidget *tile;

	if (!data->get_featured_apps_finished ||
	    !data->get_main_apps_finished)
		return;

	if (data->cancelled) {
		load_category_data_free (data);
		return;
	}

	/* Remove the loading tiles. */
	gs_widget_remove_all (self->featured_flow_box, (GsRemoveFunc) gtk_flow_box_remove);
	gs_widget_remove_all (self->recently_updated_flow_box, (GsRemoveFunc) gtk_flow_box_remove);
	gs_widget_remove_all (self->web_apps_flow_box, (GsRemoveFunc) gtk_flow_box_remove);
	gs_widget_remove_all (self->category_detail_box, (GsRemoveFunc) gtk_flow_box_remove);

	/* Last 30 days */
	recently_updated_cutoff_secs = g_get_real_time () / G_USEC_PER_SEC - 30 * 24 * 60 * 60;

	if (data->apps)
		n_category_apps = gs_app_list_length (data->apps);

	/* High probability that all apps could land here */
	other_app_tiles = g_ptr_array_sized_new (n_category_apps);

	/* Apps to go in the top carousel */
	top_carousel_apps = choose_top_carousel_apps (data, recently_updated_cutoff_secs);

	for (guint i = 0; data->apps != NULL && i < gs_app_list_length (data->apps); i++) {
		GsApp *app = gs_app_list_index (data->apps, i);
		gboolean is_featured, is_recently_updated;
		guint64 release_date;

		/* To be listed in the top carousel? */
		if (gs_app_list_lookup (top_carousel_apps, gs_app_get_unique_id (app)) != NULL) {
			n_carousel_apps++;
			continue;
		}

		release_date = gs_app_get_release_date (app);
		is_featured = (data->featured_app_ids != NULL &&
			       g_hash_table_contains (data->featured_app_ids, gs_app_get_id (app)));
		is_recently_updated = (release_date > recently_updated_cutoff_secs);

		tile = gs_summary_tile_new (app);

		if (is_featured) {
			n_featured_apps++;
			g_ptr_array_add (featured_app_tiles, tile);
		} else if (is_recently_updated) {
			n_recently_updated_apps++;
			g_ptr_array_add (recently_updated_app_tiles, tile);
		} else if (gs_app_get_kind (app) == AS_COMPONENT_KIND_WEB_APP) {
			n_web_apps++;
			g_ptr_array_add (web_app_tiles, tile);
		} else {
			n_other_apps++;
			g_ptr_array_add (other_app_tiles, tile);
		}
	}

	validate_app_buckets ();

	/* If these sections end up being too empty (which looks odd), merge them into the main section.
	 * See https://github.com/LingmoOS/lingmo-appstore/-/issues/2053 */
	if (n_featured_apps > 0 && n_featured_apps < MIN_SECTION_APPS) {
		g_debug ("\tOnly %" G_GUINT64_FORMAT " featured apps, needed at least %d; moving apps to others",
			 n_featured_apps, MIN_SECTION_APPS);
		g_ptr_array_extend_and_steal (other_app_tiles, g_steal_pointer (&featured_app_tiles));
		n_other_apps += n_featured_apps;
		n_featured_apps = 0;
	}


	if (n_web_apps > 0 && n_web_apps < MIN_SECTION_APPS) {
		g_debug ("\tOnly %" G_GUINT64_FORMAT " web apps, needed at least %d; moving apps to others",
			 n_web_apps, MIN_SECTION_APPS);
		g_ptr_array_extend_and_steal (other_app_tiles, g_steal_pointer (&web_app_tiles));
		n_other_apps += n_web_apps;
		n_web_apps = 0;
	}

	/* Show 'New & Updated' section only if there had been enough of them recognized.
	 * See https://github.com/LingmoOS/lingmo-appstore/-/issues/2053 */
	if (n_recently_updated_apps > 0) {
		if (n_recently_updated_apps < MIN_RECENT_APPS_REQUIRED) {
			g_debug ("\tOnly %" G_GUINT64_FORMAT " recent apps, needed at least %d; moving apps to others",
				 n_recently_updated_apps, MIN_RECENT_APPS_REQUIRED);
			g_ptr_array_extend_and_steal (other_app_tiles, g_steal_pointer (&recently_updated_app_tiles));
			n_other_apps += n_recently_updated_apps;
			n_recently_updated_apps = 0;
		} else {
			guint n_apps_to_move;

			/* Defer sorting till we're sure that sorted
			   data will be actually useful */
			g_ptr_array_sort (recently_updated_app_tiles, release_date_gptrarray_sort_func);

			g_assert (n_recently_updated_apps >= MAX_RECENT_APPS_TO_DISPLAY);
			n_apps_to_move = n_recently_updated_apps - MAX_RECENT_APPS_TO_DISPLAY;

			if (n_apps_to_move > 0) {
				g_debug ("\tAlready %" G_GUINT64_FORMAT " recent apps, needed at most %d; moving %u apps to others",
					 n_recently_updated_apps, MAX_RECENT_APPS_TO_DISPLAY, n_apps_to_move);

				for (guint j = 0; j < n_apps_to_move; j++) {
					/* keep removing at the same index */
					tile = g_ptr_array_steal_index_fast (recently_updated_app_tiles, MAX_RECENT_APPS_TO_DISPLAY);
					g_ptr_array_add (other_app_tiles, tile);
				}

				n_recently_updated_apps -= n_apps_to_move;
				n_other_apps += n_apps_to_move;
			}
		}
	}

	validate_app_buckets ();

	/* Populate all flowboxes. */
	populate_flow_boxes (self, featured_app_tiles, recently_updated_app_tiles, web_app_tiles, other_app_tiles);

	/* Show carousel only if it has apps */
	gtk_widget_set_visible (self->top_carousel, gs_app_list_length (top_carousel_apps) > 0);
	gs_featured_carousel_set_apps (GS_FEATURED_CAROUSEL (self->top_carousel), top_carousel_apps);

	/* Show each of the flow boxes only if they have apps. */
	gtk_widget_set_visible (self->featured_flow_box, n_featured_apps > 0);
	gtk_widget_set_visible (self->recently_updated_flow_box, n_recently_updated_apps > 0);
	gtk_widget_set_visible (self->web_apps_flow_box, n_web_apps > 0);
	gtk_widget_set_visible (self->category_detail_box, n_other_apps > 0);

	/* Don't show "Other Appstore" heading if it's the only heading  */
	gtk_widget_set_visible (self->other_heading, gtk_widget_get_visible (self->category_detail_box) && (
				gtk_widget_get_visible (self->featured_flow_box) ||
				gtk_widget_get_visible (self->recently_updated_flow_box) ||
				gtk_widget_get_visible (self->web_apps_flow_box)));

	self->content_valid = data->apps != NULL;

	load_category_data_free (data);
}

static void
gs_category_page_load_category (GsCategoryPage *self)
{
	GsCategory *featured_subcat = NULL;
	g_autoptr(GsPluginJob) featured_plugin_job = NULL;
	g_autoptr(GsAppQuery) main_query = NULL;
	g_autoptr(GsPluginJob) main_plugin_job = NULL;
	LoadCategoryData *load_data = NULL;

	g_assert (self->subcategory != NULL);

	if (!gs_page_is_active (GS_PAGE (self)))
		return;

	featured_subcat = gs_category_find_child (self->category, "featured");

	g_cancellable_cancel (self->cancellable);
	g_clear_object (&self->cancellable);
	self->cancellable = g_cancellable_new ();

	g_debug ("search using %s/%s",
	         gs_category_get_id (self->category),
	         gs_category_get_id (self->subcategory));

	/* Add placeholders only when the content is not valid */
	if (!self->content_valid) {
		gs_featured_carousel_set_apps (GS_FEATURED_CAROUSEL (self->top_carousel), NULL);
		gs_category_page_add_placeholders (self, GTK_FLOW_BOX (self->category_detail_box),
						   MIN (30, gs_category_get_size (self->subcategory)));
		gs_category_page_add_placeholders (self, GTK_FLOW_BOX (self->recently_updated_flow_box), MAX_RECENT_APPS_TO_DISPLAY);
		gtk_widget_set_visible (self->top_carousel, TRUE);
		gtk_widget_set_visible (self->category_detail_box, TRUE);
		gtk_widget_set_visible (self->recently_updated_flow_box, TRUE);

		if (gs_plugin_loader_get_enabled (self->plugin_loader, "epiphany"))
			gs_category_page_add_placeholders (self, GTK_FLOW_BOX (self->web_apps_flow_box), 12);

		if (featured_subcat != NULL) {
			/* set up the placeholders as having the featured category is a good
			 * indicator that there will be featured apps */
			gs_category_page_add_placeholders (self, GTK_FLOW_BOX (self->featured_flow_box), 6);
			gtk_widget_set_visible (self->featured_flow_box, TRUE);
		} else {
			gs_widget_remove_all (self->featured_flow_box, (GsRemoveFunc) gtk_flow_box_remove);
			gtk_widget_set_visible (self->featured_flow_box, FALSE);
			gtk_widget_set_visible (self->top_carousel, FALSE);
		}
	}

	/* Load the list of apps in the category, and also the list of all
	 * featured apps, in parallel.
	 *
	 * The list of featured apps has to be loaded separately (we can’t just
	 * query each app for its featured status) since it’s provided by a
	 * separate appstream file (org.lingmo.Appstore.Featured.xml) and hence
	 * produces separate `GsApp` instances with stub data. In particular,
	 * they don’t have enough category data to match the main category
	 * query.
	 *
	 * Once both queries have returned, turn the list of featured apps into
	 * a filter, and split the main list in four:
	 *  - Featured apps
	 *  - Recently updated apps
	 *  - Web apps
	 *  - Everything else
	 * Then populate the UI.
	 *
	 * The `featured_subcat` can be `NULL` when loading the special ‘addons’
	 * category.
	 */
	load_data = g_new0 (LoadCategoryData, 1);
	load_data->page = g_object_ref (self);

	if (featured_subcat != NULL) {
		g_autoptr(GsAppQuery) featured_query = NULL;

		featured_query = gs_app_query_new ("category", featured_subcat,
						   "refine-flags", GS_PLUGIN_REFINE_FLAGS_REQUIRE_KUDOS,
						   "license-type", gs_page_get_query_license_type (GS_PAGE (self)),
						   "developer-verified-type", gs_page_get_query_developer_verified_type (GS_PAGE (self)),
						   NULL);
		featured_plugin_job = gs_plugin_job_list_apps_new (featured_query,
								   GS_PLUGIN_LIST_APPS_FLAGS_INTERACTIVE);
		gs_plugin_loader_job_process_async (self->plugin_loader,
						    featured_plugin_job,
						    self->cancellable,
						    gs_category_page_get_featured_apps_cb,
						    load_data);
	} else {
		/* Skip it */
		load_data->get_featured_apps_finished = TRUE;
	}

	main_query = gs_app_query_new ("category", self->subcategory,
				       "refine-flags", GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON |
						       GS_PLUGIN_REFINE_FLAGS_REQUIRE_RATING |
						       GS_PLUGIN_REFINE_FLAGS_REQUIRE_KUDOS,
				       "dedupe-flags", GS_APP_LIST_FILTER_FLAG_PREFER_INSTALLED |
						       GS_APP_LIST_FILTER_FLAG_KEY_ID_PROVIDES,
				       "license-type", gs_page_get_query_license_type (GS_PAGE (self)),
				       "developer-verified-type", gs_page_get_query_developer_verified_type (GS_PAGE (self)),
				       NULL);
	main_plugin_job = gs_plugin_job_list_apps_new (main_query,
						       GS_PLUGIN_LIST_APPS_FLAGS_INTERACTIVE);
	gs_plugin_loader_job_process_async (self->plugin_loader,
					    main_plugin_job,
					    self->cancellable,
					    gs_category_page_get_apps_cb,
					    load_data);
}

static void
gs_category_page_reload (GsPage *page)
{
	GsCategoryPage *self = GS_CATEGORY_PAGE (page);

	if (self->subcategory == NULL)
		return;

	gs_category_page_load_category (self);
}

void
gs_category_page_set_category (GsCategoryPage *self, GsCategory *category)
{
	GsCategory *all_subcat = NULL;

	/* this means we've come from the app-view -> back */
	if (self->category == category)
		return;

	/* set the category */
	all_subcat = gs_category_find_child (category, "all");

	g_set_object (&self->category, category);
	g_set_object (&self->subcategory, all_subcat);

	/* load the apps from it */
	if (all_subcat != NULL) {
		GtkAdjustment *adj = NULL;

		/* scroll the list of apps to the beginning, otherwise it will show
		 * with the previous scroll value, for the previous category */
		adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (self->scrolledwindow_category));
		gtk_adjustment_set_value (adj, gtk_adjustment_get_lower (adj));

		self->content_valid = FALSE;

		gs_category_page_load_category (self);
	}

	/* notify of the updates — the category’s title will have changed too */
	g_object_notify (G_OBJECT (self), "category");
	g_object_notify (G_OBJECT (self), "title");
}

GsCategory *
gs_category_page_get_category (GsCategoryPage *self)
{
	return self->category;
}

static void
gs_category_page_init (GsCategoryPage *self)
{
	g_type_ensure (GS_TYPE_FEATURED_CAROUSEL);

	gtk_widget_init_template (GTK_WIDGET (self));

	/* Enable sorting on all flowboxes by default. */
	gs_category_page_set_sort (self, TRUE);

	gs_featured_carousel_set_apps (GS_FEATURED_CAROUSEL (self->top_carousel), NULL);
}

static void
gs_category_page_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
	GsCategoryPage *self = GS_CATEGORY_PAGE (object);

	switch ((GsCategoryPageProperty) prop_id) {
	case PROP_TITLE:
		if (self->category != NULL)
			g_value_set_string (value, gs_category_get_name (self->category));
		else
			g_value_set_string (value, NULL);
		break;
	case PROP_CATEGORY:
		g_value_set_object (value, self->category);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_category_page_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
	GsCategoryPage *self = GS_CATEGORY_PAGE (object);

	switch ((GsCategoryPageProperty) prop_id) {
	case PROP_TITLE:
		/* Read only */
		g_assert_not_reached ();
		break;
	case PROP_CATEGORY:
		gs_category_page_set_category (self, g_value_get_object (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_category_page_dispose (GObject *object)
{
	GsCategoryPage *self = GS_CATEGORY_PAGE (object);

	g_cancellable_cancel (self->cancellable);
	g_clear_object (&self->cancellable);

	g_clear_object (&self->category);
	g_clear_object (&self->subcategory);
	g_clear_object (&self->plugin_loader);

	G_OBJECT_CLASS (gs_category_page_parent_class)->dispose (object);
}

static gboolean
gs_category_page_setup (GsPage *page,
                        GsShell *shell,
                        GsPluginLoader *plugin_loader,
                        GCancellable *cancellable,
                        GError **error)
{
	GsCategoryPage *self = GS_CATEGORY_PAGE (page);

	self->plugin_loader = g_object_ref (plugin_loader);

	return TRUE;
}

static void
gs_category_page_class_init (GsCategoryPageClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPageClass *page_class = GS_PAGE_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->get_property = gs_category_page_get_property;
	object_class->set_property = gs_category_page_set_property;
	object_class->dispose = gs_category_page_dispose;

	page_class->reload = gs_category_page_reload;
	page_class->setup = gs_category_page_setup;

	/**
	 * GsCategoryPage:category: (nullable)
	 *
	 * The category to display the apps from.
	 *
	 * This may be %NULL if no category is selected. If so, the behaviour
	 * of the widget will be safe, but undefined.
	 *
	 * Since: 41
	 */
	obj_props[PROP_CATEGORY] =
		g_param_spec_object ("category", NULL, NULL,
				     GS_TYPE_CATEGORY,
				     G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);

	g_object_class_override_property (object_class, PROP_TITLE, "title");

	/**
	 * GsCategoryPage::app-clicked:
	 * @app: the #GsApp which was clicked on
	 *
	 * Emitted when one of the app tiles is clicked. Typically the caller
	 * should display the details of the given app in the callback.
	 *
	 * Since: 41
	 */
	obj_signals[SIGNAL_APP_CLICKED] =
		g_signal_new ("app-clicked",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      0, NULL, NULL, g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE, 1, GS_TYPE_APP);

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-category-page.ui");

	gtk_widget_class_bind_template_child (widget_class, GsCategoryPage, top_carousel);
	gtk_widget_class_bind_template_child (widget_class, GsCategoryPage, other_heading);
	gtk_widget_class_bind_template_child (widget_class, GsCategoryPage, category_detail_box);
	gtk_widget_class_bind_template_child (widget_class, GsCategoryPage, scrolledwindow_category);
	gtk_widget_class_bind_template_child (widget_class, GsCategoryPage, featured_flow_box);
	gtk_widget_class_bind_template_child (widget_class, GsCategoryPage, recently_updated_flow_box);
	gtk_widget_class_bind_template_child (widget_class, GsCategoryPage, web_apps_flow_box);

	gtk_widget_class_bind_template_callback (widget_class, top_carousel_app_clicked_cb);
	gtk_widget_class_bind_template_callback (widget_class, app_activated_cb);
}

GsCategoryPage *
gs_category_page_new (void)
{
	return g_object_new (GS_TYPE_CATEGORY_PAGE, NULL);
}
