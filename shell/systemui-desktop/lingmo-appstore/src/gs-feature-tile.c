/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
 * Copyright (C) 2019 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "gs-feature-tile.h"
#include "gs-layout-manager.h"
#include "gs-common.h"
#include "gs-css.h"

#define GS_TYPE_FEATURE_TILE_LAYOUT (gs_feature_tile_layout_get_type ())
G_DECLARE_FINAL_TYPE (GsFeatureTileLayout, gs_feature_tile_layout, GS, FEATURE_TILE_LAYOUT, GsLayoutManager)

struct _GsFeatureTileLayout
{
	GsLayoutManager parent_instance;

	gboolean	narrow_mode;
};

G_DEFINE_TYPE (GsFeatureTileLayout, gs_feature_tile_layout, GS_TYPE_LAYOUT_MANAGER)

enum {
	SIGNAL_NARROW_MODE_CHANGED,
	SIGNAL_LAST
};

static guint signals [SIGNAL_LAST] = { 0 };

static void
gs_feature_tile_layout_allocate (GtkLayoutManager *layout_manager,
				 GtkWidget        *widget,
				 gint              width,
				 gint              height,
				 gint              baseline)
{
	GsFeatureTileLayout *self = GS_FEATURE_TILE_LAYOUT (layout_manager);
	gboolean narrow_mode;

	GTK_LAYOUT_MANAGER_CLASS (gs_feature_tile_layout_parent_class)->allocate (layout_manager,
		widget, width, height, baseline);

	/* Engage ‘narrow mode’ if the allocation becomes too narrow. The exact
	 * choice of width is arbitrary here. */
	narrow_mode = (width < 600);
	if (self->narrow_mode != narrow_mode) {
		self->narrow_mode = narrow_mode;
		g_signal_emit (self, signals[SIGNAL_NARROW_MODE_CHANGED], 0, self->narrow_mode);
	}
}

static void
gs_feature_tile_layout_class_init (GsFeatureTileLayoutClass *klass)
{
	GtkLayoutManagerClass *layout_manager_class = GTK_LAYOUT_MANAGER_CLASS (klass);
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	layout_manager_class->allocate = gs_feature_tile_layout_allocate;

	signals [SIGNAL_NARROW_MODE_CHANGED] =
		g_signal_new ("narrow-mode-changed",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL, g_cclosure_marshal_generic,
			      G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
}

static void
gs_feature_tile_layout_init (GsFeatureTileLayout *self)
{
}

/* ********************************************************************* */

struct _GsFeatureTile
{
	GtkButton	 parent_instance;
	GtkWidget	*stack;
	GtkWidget	*image;
	GtkWidget	*title;
	GtkWidget	*subtitle;
	const gchar	*markup_cache;  /* (unowned) (nullable) */
	GtkCssProvider	*tile_provider;  /* (owned) (nullable) */
	GtkCssProvider	*title_provider;  /* (owned) (nullable) */
	GtkCssProvider	*subtitle_provider;  /* (owned) (nullable) */
	GArray		*key_colors_cache;  /* (unowned) (nullable) */
	gboolean	 narrow_mode;
	GsApp		*app;
	guint		 refresh_id;
};

typedef enum {
	PROP_APP = 1,
} GsFeatureTileProperty;

static GParamSpec *obj_props[PROP_APP + 1] = { NULL, };

static void gs_feature_tile_refresh (GsFeatureTile *self);

static gboolean
gs_feature_tile_refresh_idle_cb (gpointer user_data)
{
	GsFeatureTile *tile = user_data;

	tile->refresh_id = 0;

	gs_feature_tile_refresh (tile);

	return G_SOURCE_REMOVE;
}

static void
schedule_refresh (GsFeatureTile *self)
{
	/* Already pending */
	if (self->refresh_id != 0)
		return;

	self->refresh_id = g_idle_add (gs_feature_tile_refresh_idle_cb, self);
}

static void
gs_feature_tile_layout_narrow_mode_changed_cb (GtkLayoutManager *layout_manager,
					       gboolean          narrow_mode,
					       gpointer          user_data)
{
	GsFeatureTile *self = GS_FEATURE_TILE (user_data);

	if (self->narrow_mode != narrow_mode) {
		self->narrow_mode = narrow_mode;
		schedule_refresh (self);
	}
}

/* A colour represented in hue, saturation, brightness form; with an additional
 * field for its contrast calculated with respect to some external colour.
 *
 * See https://en.wikipedia.org/wiki/HSL_and_HSV */
typedef struct
{
	gfloat hue;  /* [0.0, 1.0] */
	gfloat saturation;  /* [0.0, 1.0] */
	gfloat brightness;  /* [0.0, 1.0]; also known as lightness (HSL) or value (HSV) */
	gfloat contrast;  /* (0.047, 21] */
} GsHSBC;

G_DEFINE_TYPE (GsFeatureTile, gs_feature_tile, GTK_TYPE_BUTTON)

static void
gs_feature_tile_dispose (GObject *object)
{
	GsFeatureTile *tile = GS_FEATURE_TILE (object);

	gs_feature_tile_set_app (tile, NULL);

	g_clear_object (&tile->tile_provider);
	g_clear_object (&tile->title_provider);
	g_clear_object (&tile->subtitle_provider);

	G_OBJECT_CLASS (gs_feature_tile_parent_class)->dispose (object);
}

static void
gs_feature_tile_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	GsFeatureTile *self = GS_FEATURE_TILE (object);

	switch ((GsFeatureTileProperty) prop_id) {
	case PROP_APP:
		g_value_set_object (value, self->app);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_feature_tile_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	GsFeatureTile *self = GS_FEATURE_TILE (object);

	switch ((GsFeatureTileProperty) prop_id) {
	case PROP_APP:
		gs_feature_tile_set_app (self, g_value_get_object (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

/* These are subjectively chosen. See below. */
static const gfloat min_valid_saturation = 0.5;
static const gfloat max_valid_saturation = 0.85;

/* The minimum absolute contrast ratio between the foreground and background
 * colours, from WCAG:
 * https://www.w3.org/TR/UNDERSTANDING-WCAG20/visual-audio-contrast-contrast.html */
static const gfloat min_abs_contrast = 4.5;

/* Sort two candidate background colours for the feature tile, ranking them by
 * suitability for being chosen as the background colour, with the most suitable
 * first.
 *
 * There are several criteria being used here:
 *  1. First, colours are sorted by whether their saturation is in the range
 *     [0.5, 0.85], which is a subjectively-chosen range of ‘light, but not too
 *     saturated’ colours.
 *  2. Colours with saturation in that valid range are then sorted by contrast,
 *     with higher contrast being preferred. The contrast is calculated against
 *     an external colour by the caller.
 *  3. Colours with saturation outside that valid range are sorted by their
 *     absolute distance from the range, so that colours which are nearer to
 *     having a valid saturation are preferred. This is useful in the case where
 *     none of the key colours in this array have valid saturations; the caller
 *     will want the one which is closest to being valid.
 */
static gboolean
saturation_is_valid (const GsHSBC *hsbc,
                     gfloat       *distance_from_valid_range)
{
	*distance_from_valid_range = (hsbc->saturation > max_valid_saturation) ? hsbc->saturation - max_valid_saturation : min_valid_saturation - hsbc->saturation;
	return (hsbc->saturation >= min_valid_saturation && hsbc->saturation <= max_valid_saturation);
}

static gint
colors_sort_cb (gconstpointer a,
		gconstpointer b)
{
	const GsHSBC *hsbc_a = a;
	const GsHSBC *hsbc_b = b;
	gfloat hsbc_a_distance_from_range, hsbc_b_distance_from_range;
	gboolean hsbc_a_saturation_in_range = saturation_is_valid (hsbc_a, &hsbc_a_distance_from_range);
	gboolean hsbc_b_saturation_in_range = saturation_is_valid (hsbc_b, &hsbc_b_distance_from_range);

	if (hsbc_a_saturation_in_range && !hsbc_b_saturation_in_range)
		return -1;
	else if (!hsbc_a_saturation_in_range && hsbc_b_saturation_in_range)
		return 1;
	else if (!hsbc_a_saturation_in_range && !hsbc_b_saturation_in_range)
		return hsbc_a_distance_from_range - hsbc_b_distance_from_range;
	else
		return ABS (hsbc_b->contrast) - ABS (hsbc_a->contrast);
}

static gint
colors_sort_contrast_cb (gconstpointer a,
                         gconstpointer b)
{
	const GsHSBC *hsbc_a = a;
	const GsHSBC *hsbc_b = b;

	return hsbc_b->contrast - hsbc_a->contrast;
}

/* Calculate the relative luminance of @colour. This is [0.0, 1.0], where 0.0 is
 * the darkest black, and 1.0 is the lightest white.
 *
 * https://www.w3.org/TR/2008/REC-WCAG20-20081211/#relativeluminancedef */
static gfloat
relative_luminance (const GsHSBC *colour)
{
	gfloat red, green, blue;
	gfloat r, g, b;
	gfloat luminance;

	/* Convert to sRGB */
	gtk_hsv_to_rgb (colour->hue, colour->saturation, colour->brightness,
			&red, &green, &blue);

	r = (red <= 0.03928) ? red / 12.92 : pow ((red + 0.055) / 1.055, 2.4);
	g = (green <= 0.03928) ? green / 12.92 : pow ((green + 0.055) / 1.055, 2.4);
	b = (blue <= 0.03928) ? blue / 12.92 : pow ((blue + 0.055) / 1.055, 2.4);

	luminance = 0.2126 * r + 0.7152 * g + 0.0722 * b;
	g_assert (luminance >= 0.0 && luminance <= 1.0);
	return luminance;
}

/* Calculate the WCAG contrast ratio between the two colours. The returned ratio
 * is in the range (0.047, 21].
 *
 * https://www.w3.org/TR/UNDERSTANDING-WCAG20/visual-audio-contrast-contrast.html#contrast-ratiodef */
static gfloat
wcag_contrast (const GsHSBC *foreground,
               const GsHSBC *background)
{
	const GsHSBC *lighter, *darker;
	gfloat ratio;

	if (foreground->brightness >= background->brightness) {
		lighter = foreground;
		darker = background;
	} else {
		lighter = background;
		darker = foreground;
	}

	ratio = (relative_luminance (lighter) + 0.05) / (relative_luminance (darker) + 0.05);
	g_assert (ratio > 0.047 && ratio <= 21);
	return ratio;
}

/* Calculate a new brightness value for @background which improves its contrast
 * (as calculated using wcag_contrast()) with @foreground to at least
 * @desired_contrast.
 *
 * The return value is in the range [0.0, 1.0].
 */
static gfloat
wcag_contrast_find_brightness (const GsHSBC *foreground,
                               const GsHSBC *background,
                               gfloat        desired_contrast)
{
	GsHSBC modified_background;

	g_assert (desired_contrast > 0.047 && desired_contrast <= 21);

	/* This is an optimisation problem of modifying @background until
	 * the WCAG contrast is at least @desired_contrast. There might be a
	 * closed-form solution to this but I can’t be bothered to work it out
	 * right now. An optimisation loop should work.
	 *
	 * wcag_contrast() compares the lightest and darkest of the two colours,
	 * so ensure the background brightness is modified in the correct
	 * direction (increased or decreased) depending on whether the
	 * foreground colour is originally the brighter. This gives the largest
	 * search space for the background colour brightness, and ensures the
	 * optimisation works with dark and light themes. */
	for (modified_background = *background;
	     modified_background.brightness >= 0.0 &&
	     modified_background.brightness <= 1.0 &&
	     wcag_contrast (foreground, &modified_background) < desired_contrast;
	     modified_background.brightness += ((foreground->brightness > 0.5) ? -0.1 : 0.1)) {
		/* Nothing to do here */
	}

	return CLAMP (modified_background.brightness, 0.0, 1.0);
}

static void
gs_feature_tile_refresh (GsFeatureTile *tile)
{
	GsApp *app = tile->app;
	const gchar *markup = NULL;
	g_autofree gchar *name = NULL;
	g_autoptr(GIcon) icon = NULL;
	guint icon_size;

	if (app == NULL)
		return;

	gtk_stack_set_visible_child_name (GTK_STACK (tile->stack), "content");

	/* Set the narrow mode. */
	if (tile->narrow_mode)
		gtk_widget_add_css_class (GTK_WIDGET (tile), "narrow");
	else
		gtk_widget_remove_css_class (GTK_WIDGET (tile), "narrow");

	/* Update the icon. Try a 160px version if not in narrow mode, and it’s
	 * available; otherwise use 128px. */
	if (!tile->narrow_mode) {
		icon = gs_app_get_icon_for_size (app,
						 160,
						 gtk_widget_get_scale_factor (tile->image),
						 NULL);
		icon_size = 160;
	}
	if (icon == NULL) {
		icon = gs_app_get_icon_for_size (app,
						 128,
						 gtk_widget_get_scale_factor (tile->image),
						 NULL);
		icon_size = 128;
	}

	if (icon != NULL) {
		gtk_image_set_from_gicon (GTK_IMAGE (tile->image), icon);
		gtk_image_set_pixel_size (GTK_IMAGE (tile->image), icon_size);
		gtk_widget_set_visible (tile->image, TRUE);
	} else {
		gtk_widget_set_visible (tile->image, FALSE);
	}

	/* Update text and let it wrap if the widget is narrow. */
	gtk_label_set_label (GTK_LABEL (tile->title), gs_app_get_name (app));
	gtk_label_set_label (GTK_LABEL (tile->subtitle), gs_app_get_summary (app));

	gtk_label_set_wrap (GTK_LABEL (tile->subtitle), tile->narrow_mode);
	gtk_label_set_lines (GTK_LABEL (tile->subtitle), tile->narrow_mode ? 2 : 1);

	/* perhaps set custom css; cache it so that images don’t get reloaded
	 * unnecessarily. The custom CSS is direction-dependent, and will be
	 * reloaded when the direction changes. If RTL CSS isn’t set, fall back
	 * to the LTR CSS. */
	if (gtk_widget_get_direction (GTK_WIDGET (tile)) == GTK_TEXT_DIR_RTL)
		markup = gs_app_get_metadata_item (app, "LingmoAppstore::FeatureTile-css-rtl");
	if (markup == NULL)
		markup = gs_app_get_metadata_item (app, "LingmoAppstore::FeatureTile-css");

	if (tile->markup_cache != markup && markup != NULL) {
		g_autoptr(GsCss) css = gs_css_new ();
		g_autofree gchar *modified_markup = gs_utils_set_key_colors_in_css (markup, app);
		if (modified_markup != NULL)
			gs_css_parse (css, modified_markup, NULL);
		gs_utils_widget_set_css (GTK_WIDGET (tile), &tile->tile_provider,
					 gs_css_get_markup_for_id (css, "tile"));
		gs_utils_widget_set_css (tile->title, &tile->title_provider,
					 gs_css_get_markup_for_id (css, "name"));
		gs_utils_widget_set_css (tile->subtitle, &tile->subtitle_provider,
					 gs_css_get_markup_for_id (css, "summary"));
		tile->markup_cache = markup;
	} else if (markup == NULL) {
		GArray *key_colors = gs_app_get_key_colors (app);
		g_autofree gchar *css = NULL;

		/* If there is no override CSS for the app, default to a solid
		 * background colour based on the app’s key colors.
		 *
		 * Choose an arbitrary key color from the app’s key colors, and
		 * ensure that it’s:
		 *  - a light, not too saturated version of the dominant color
		 *    of the icon
		 *  - always light enough that grey text is visible on it
		 *
		 * Cache the result until the app’s key colours change, as the
		 * amount of calculation going on here is not entirely trivial.
		 */
		if (key_colors != tile->key_colors_cache) {
			g_autoptr(GArray) colors = NULL;
			GdkRGBA fg_rgba;
			GsHSBC fg_hsbc;
			const GsHSBC *chosen_hsbc;
			GsHSBC chosen_hsbc_modified;
			gboolean use_chosen_hsbc = FALSE;

			/* Look up the foreground colour for the feature tile,
			 * which is the colour of the text. This should always
			 * be provided as a named colour by the theme.
			 *
			 * Knowing the foreground colour allows calculation of
			 * the contrast between candidate background colours and
			 * the foreground which will be rendered on top of them.
			 *
			 * We want to choose a background colour with at least
			 * @min_abs_contrast contrast with the foreground, so
			 * that the text is legible.
			 */
			gtk_widget_get_color (GTK_WIDGET (tile), &fg_rgba);

			gtk_rgb_to_hsv (fg_rgba.red, fg_rgba.green, fg_rgba.blue,
					&fg_hsbc.hue, &fg_hsbc.saturation, &fg_hsbc.brightness);

			g_debug ("FG color: RGB: (%f, %f, %f), HSB: (%f, %f, %f)",
				 fg_rgba.red, fg_rgba.green, fg_rgba.blue,
				 fg_hsbc.hue, fg_hsbc.saturation, fg_hsbc.brightness);

			/* Convert all the RGBA key colours to HSB, and
			 * calculate their contrast against the foreground
			 * colour.
			 *
			 * The contrast is calculated as the Weber contrast,
			 * which is valid for small amounts of foreground colour
			 * (i.e. text) against larger background areas. Contrast
			 * is strictly calculated using luminance, but it’s OK
			 * to subjectively calculate it using brightness, as
			 * brightness is the subjective impression of luminance.
			 */
			if (key_colors != NULL)
				colors = g_array_sized_new (FALSE, FALSE, sizeof (GsHSBC), key_colors->len);

			g_debug ("Candidate background colors for %s:", gs_app_get_id (app));
			for (guint i = 0; key_colors != NULL && i < key_colors->len; i++) {
				const GdkRGBA *rgba = &g_array_index (key_colors, GdkRGBA, i);
				GsHSBC hsbc;

				gtk_rgb_to_hsv (rgba->red, rgba->green, rgba->blue,
						&hsbc.hue, &hsbc.saturation, &hsbc.brightness);
				hsbc.contrast = wcag_contrast (&fg_hsbc, &hsbc);
				g_array_append_val (colors, hsbc);

				g_debug (" • RGB: (%f, %f, %f), HSB: (%f, %f, %f), contrast: %f",
					 rgba->red, rgba->green, rgba->blue,
					 hsbc.hue, hsbc.saturation, hsbc.brightness,
					 hsbc.contrast);
			}

			/* Sort the candidate background colours to find the
			 * most appropriate one. */
			g_array_sort (colors, colors_sort_cb);

			/* If the developer/distro has provided override colours,
			 * use them. If there’s more than one override colour,
			 * use the one with the highest contrast with the
			 * foreground colour, unmodified. If there’s only one,
			 * modify it as below.
			 *
			 * If there are no override colours, take the top colour
			 * after sorting above. If it’s not good enough, modify
			 * its brightness to improve the contrast, and clamp its
			 * saturation to the valid range.
			 *
			 * If there are no colours, fall through and leave @css
			 * as %NULL. */
			if (gs_app_get_user_key_colors (app) &&
			    colors != NULL &&
			    colors->len > 1) {
				g_array_sort (colors, colors_sort_contrast_cb);

				chosen_hsbc = &g_array_index (colors, GsHSBC, 0);
				chosen_hsbc_modified = *chosen_hsbc;

				use_chosen_hsbc = TRUE;
			} else if (colors != NULL && colors->len > 0) {
				chosen_hsbc = &g_array_index (colors, GsHSBC, 0);
				chosen_hsbc_modified = *chosen_hsbc;

				chosen_hsbc_modified.saturation = CLAMP (chosen_hsbc->saturation, min_valid_saturation, max_valid_saturation);

				if (chosen_hsbc->contrast >= -min_abs_contrast &&
				    chosen_hsbc->contrast <= min_abs_contrast)
					chosen_hsbc_modified.brightness = wcag_contrast_find_brightness (&fg_hsbc, &chosen_hsbc_modified, min_abs_contrast);

				use_chosen_hsbc = TRUE;
			}

			if (use_chosen_hsbc) {
				GdkRGBA chosen_rgba;

				gtk_hsv_to_rgb (chosen_hsbc_modified.hue,
						chosen_hsbc_modified.saturation,
						chosen_hsbc_modified.brightness,
						&chosen_rgba.red, &chosen_rgba.green, &chosen_rgba.blue);

				g_debug ("Chosen background colour for %s (saturation %s, brightness %s): RGB: (%f, %f, %f), HSB: (%f, %f, %f)",
					 gs_app_get_id (app),
					 (chosen_hsbc_modified.saturation == chosen_hsbc->saturation) ? "not modified" : "modified",
					 (chosen_hsbc_modified.brightness == chosen_hsbc->brightness) ? "not modified" : "modified",
					 chosen_rgba.red, chosen_rgba.green, chosen_rgba.blue,
					 chosen_hsbc_modified.hue, chosen_hsbc_modified.saturation, chosen_hsbc_modified.brightness);

				css = g_strdup_printf ("background-color: rgb(%.0f,%.0f,%.0f);",
						       chosen_rgba.red * 255.f,
						       chosen_rgba.green * 255.f,
						       chosen_rgba.blue * 255.f);
			}

			gs_utils_widget_set_css (GTK_WIDGET (tile), &tile->tile_provider, css);
			gs_utils_widget_set_css (tile->title, &tile->title_provider, NULL);
			gs_utils_widget_set_css (tile->subtitle, &tile->subtitle_provider, NULL);

			tile->key_colors_cache = key_colors;
		}
	}

	switch (gs_app_get_state (app)) {
	case GS_APP_STATE_INSTALLED:
	case GS_APP_STATE_REMOVING:
	case GS_APP_STATE_UPDATABLE:
	case GS_APP_STATE_UPDATABLE_LIVE:
		name = g_strdup_printf ("%s (%s)",
					gs_app_get_name (app),
					C_("Single app", "Installed"));
		break;
	case GS_APP_STATE_AVAILABLE:
	case GS_APP_STATE_INSTALLING:
	case GS_APP_STATE_DOWNLOADING:
	default:
		name = g_strdup (gs_app_get_name (app));
		break;
	}

	if (name != NULL) {
		gtk_accessible_update_property (GTK_ACCESSIBLE (tile),
						GTK_ACCESSIBLE_PROPERTY_LABEL, name,
						GTK_ACCESSIBLE_PROPERTY_DESCRIPTION, gs_app_get_summary (app),
						-1);
	}
}

static void
gs_feature_tile_direction_changed (GtkWidget *widget, GtkTextDirection previous_direction)
{
	GsFeatureTile *tile = GS_FEATURE_TILE (widget);

	gs_feature_tile_refresh (tile);
}

static void
gs_feature_tile_css_changed (GtkWidget         *widget,
                             GtkCssStyleChange *css_change)
{
	GsFeatureTile *tile = GS_FEATURE_TILE (widget);

	/* Clear the key colours cache, as the tile background colour will
	 * potentially need recalculating if the widget’s foreground colour has
	 * changed. */
	tile->key_colors_cache = NULL;

	gs_feature_tile_refresh (tile);

	GTK_WIDGET_CLASS (gs_feature_tile_parent_class)->css_changed (widget, css_change);
}

static void
gs_feature_tile_init (GsFeatureTile *tile)
{
	GtkLayoutManager *layout_manager;

	gtk_widget_init_template (GTK_WIDGET (tile));

	layout_manager = gtk_widget_get_layout_manager (GTK_WIDGET (tile));
	g_warn_if_fail (layout_manager != NULL);
	g_signal_connect_object (layout_manager, "narrow-mode-changed",
		G_CALLBACK (gs_feature_tile_layout_narrow_mode_changed_cb), tile, 0);
}

static void
gs_feature_tile_class_init (GsFeatureTileClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->dispose = gs_feature_tile_dispose;
	object_class->get_property = gs_feature_tile_get_property;
	object_class->set_property = gs_feature_tile_set_property;

	widget_class->css_changed = gs_feature_tile_css_changed;
	widget_class->direction_changed = gs_feature_tile_direction_changed;

	/**
	 * GsFeatureTile:app:
	 *
	 * The app to display in this tile.
	 *
	 * Set this to %NULL to display a loading/empty tile.
	 *
	 * Since: 45
	 */
	obj_props[PROP_APP] =
		g_param_spec_object ("app", "App",
				     "The app to display in this tile.",
				     GS_TYPE_APP,
				     G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-feature-tile.ui");

	gtk_widget_class_bind_template_child (widget_class, GsFeatureTile, stack);
	gtk_widget_class_bind_template_child (widget_class, GsFeatureTile, image);
	gtk_widget_class_bind_template_child (widget_class, GsFeatureTile, title);
	gtk_widget_class_bind_template_child (widget_class, GsFeatureTile, subtitle);

	gtk_widget_class_set_css_name (widget_class, "feature-tile");
	gtk_widget_class_set_layout_manager_type (widget_class, GS_TYPE_FEATURE_TILE_LAYOUT);
}

GtkWidget *
gs_feature_tile_new (GsApp *app)
{
	return g_object_new (GS_TYPE_FEATURE_TILE,
			     "vexpand", FALSE,
			     "app", app,
			     NULL);
}

/**
 * gs_feature_tile_get_app:
 * @self: a #GsFeatureTile
 *
 * Get the value of #GsFeatureTile:app.
 *
 * Returns: (nullable) (transfer none): the #GsFeatureTile:app property
 *
 * Since: 45
 */
GsApp *
gs_feature_tile_get_app (GsFeatureTile *self)
{
	g_return_val_if_fail (GS_IS_FEATURE_TILE (self), NULL);
	return self->app;
}

/**
 * gs_feature_tile_set_app:
 * @self: a #GsFeatureTile
 * @app: (transfer none) (nullable): the new value for #GsFeatureTile:app
 *
 * Set the value of #GsFeatureTile:app.
 *
 * Since: 45
 */
void
gs_feature_tile_set_app (GsFeatureTile *self, GsApp *app)
{
	g_return_if_fail (GS_IS_FEATURE_TILE (self));
	g_return_if_fail (!app || GS_IS_APP (app));

	/* cancel pending refresh */
	g_clear_handle_id (&self->refresh_id, g_source_remove);

	/* disconnect old app */
	if (self->app != NULL)
		g_signal_handlers_disconnect_by_func (self->app, schedule_refresh, self);

	g_set_object (&self->app, app);

	if (self->app != NULL) {
		g_signal_connect_swapped (app, "notify",
					  G_CALLBACK (schedule_refresh), self);
		schedule_refresh (self);
	}

	g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_APP]);
}
