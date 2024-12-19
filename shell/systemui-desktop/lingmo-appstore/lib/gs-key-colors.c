/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2016 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
 * Copyright (C) 2014-2018 Kalev Lember <klember@redhat.com>
 * Copyright (C) 2021 Endless OS Foundation LLC
 *
 * Authors:
 *  - Richard Hughes <richard@hughsie.com>
 *  - Kalev Lember <klember@redhat.com>
 *  - Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-key-colors
 * @short_description: Helper functions for calculating key colors
 *
 * Key colors are RGB colors which represent an app, and they are derived from
 * the app’s icon, or manually specified as an override.
 *
 * Use gs_calculate_key_colors() to calculate the key colors from an app’s icon.
 *
 * Since: 40
 */

#include "config.h"

#include <glib.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "gs-key-colors.h"

/* Hard-code the number of clusters to split the icon color space into. This
 * gives the maximum number of key colors returned for an icon. This number has
 * been chosen by examining 1000 icons to subjectively see how many key colors
 * each has. The number of key colors ranged from 1 to 6, but the mode was
 * definitely 3. */
const guint n_clusters = 3;

/* Discard pixels with less than this level of alpha. Almost all icons have a
 * transparent background/border at 100% transparency, and a blending fringe
 * with some intermediate level of transparency which should be ignored for
 * choosing key colors. A number of icons have partially-transparent colored
 * sections in the main body of the icon, which should be used if they’re above
 * this threshold. About 1% of icons have no completely opaque pixels, so we
 * can’t discard non-opaque pixels entirely. */
const guint minimum_alpha = 0.5 * 255;

typedef struct {
	guint8 red;
	guint8 green;
	guint8 blue;
} Pixel8;

typedef struct {
	Pixel8 color;
	union {
		guint8 alpha;
		guint8 cluster;
	};
} ClusterPixel8;

typedef struct {
	guint red;
	guint green;
	guint blue;
	guint n_members;
} CentroidAccumulator;

static inline guint
color_distance (const Pixel8 *a,
                const Pixel8 *b)
{
	/* Take the absolute value rather than the square root to save some
	 * time, as the caller is comparing distances.
	 *
	 * The arithmetic here can’t overflow, as the R/G/B components have a
	 * maximum value of 255 but the arithmetic is done in (at least) 32-bit
	 * variables.*/
	gint dr = b->red - a->red;
	gint dg = b->green - a->green;
	gint db = b->blue - a->blue;

	return abs (dr * dr + dg * dg + db * db);
}

/* NOTE: This has to return stable results when more than one cluster is
 * equidistant from the @pixel, or the k_means() function may not terminate. */
static inline gsize
nearest_cluster (const Pixel8 *pixel,
                 const Pixel8 *cluster_centres,
                 gsize         n_cluster_centres)
{
	gsize nearest_cluster = 0;
	guint nearest_cluster_distance = color_distance (&cluster_centres[0], pixel);

	for (gsize i = 1; i < n_cluster_centres; i++) {
		guint distance = color_distance (&cluster_centres[i], pixel);
		if (distance < nearest_cluster_distance) {
			nearest_cluster = i;
			nearest_cluster_distance = distance;
		}
	}

	return nearest_cluster;
}

/* A variant of g_random_int_range() which chooses without replacement,
 * tracking the used integers in @used_ints and @n_used_ints.
 * Once all integers in 0..max_ints have been used once, it will choose
 * with replacement. */
static gint32
random_int_range_no_replacement (guint max_ints,
				 gboolean *used_ints,
				 guint *n_used_ints)
{
	gint32 random_value = g_random_int_range (0, (gint32) max_ints);

	if (*n_used_ints < max_ints) {
		while (used_ints[random_value])
			random_value = (random_value + 1) % max_ints;

		used_ints[random_value] = TRUE;
		*n_used_ints = *n_used_ints + 1;
	}

	return random_value;
}

/* Extract the key colors from @pb by clustering the pixels in RGB space.
 * Clustering is done using k-means, with initialisation using a
 * Random Partition.
 *
 * This approach can be thought of as plotting every pixel in @pb in a
 * three-dimensional color space, with red, green and blue axes (alpha is
 * clipped to 0 (pixel is ignored) or 1 (pixel is used)). The key colors for
 * the image are the ones where a large number of pixels are plotted in a group
 * in the color space — either a lot of pixels with an identical color
 * (repeated use of exactly the same color in the image) or a lot of pixels in
 * a rough group (use of a lot of similar shades of the same color in the
 * image).
 *
 * By transforming to a color space, information about the X and Y positions of
 * each color is ignored, so a thin outline in the image of a single color
 * will appear in the color space as a cluster, just as a contiguous block of
 * one color would.
 *
 * The k-means clustering algorithm is then used to find these clusters. k-means
 * is used, rather than (say) principal component analysis, because it
 * inherently calculates the centroid for each cluster. In a color space, the
 * centroid is itself a color, which can then be used as the key color to
 * return.
 *
 * The number of clusters is limited to @n_clusters, as a subjective survey of
 * 1000 icons found that they commonly used this number of key colors.
 *
 * Various other shortcuts have been taken which make this approach quite
 * specific to key color extraction from icons, with the aim of making it
 * faster. That’s fine — it doesn’t matter if the results this function produces
 * are optimal, only that they’re good enough. */
static void
k_means (GArray    *colors,
         GdkPixbuf *pb)
{
	gint rowstride, n_channels;
	gint width, height;
	guint8 *raw_pixels;
	ClusterPixel8 *pixels;
	const ClusterPixel8 *pixels_end;
	Pixel8 cluster_centres[n_clusters];
	CentroidAccumulator cluster_accumulators[n_clusters];
	gboolean used_clusters[n_clusters];
	guint n_used_clusters = 0;
	guint n_assignments_changed;
	guint n_iterations = 0;
	guint assignments_termination_limit;

	n_channels = gdk_pixbuf_get_n_channels (pb);
	rowstride = gdk_pixbuf_get_rowstride (pb);
	raw_pixels = gdk_pixbuf_get_pixels (pb);
	width = gdk_pixbuf_get_width (pb);
	height = gdk_pixbuf_get_height (pb);

	/* The pointer arithmetic over pixels can be simplified if we can assume
	 * there are no gaps in the @raw_pixels data. Since the caller is
	 * downsizing the #GdkPixbuf, this is a reasonable assumption. */
	g_assert (rowstride == width * n_channels);
	g_assert (n_channels == 4);

	pixels = (ClusterPixel8 *) raw_pixels;
	pixels_end = &pixels[height * width];

	memset (cluster_centres, 0, sizeof (cluster_centres));
	memset (used_clusters, 0, sizeof (used_clusters));

	/* Initialise the clusters using the Random Partition method: randomly
	 * assign a starting cluster to each pixel.
	 *
	 * The Forgy method (choosing random pixels as the starting cluster
	 * centroids) is not appropriate as the checks required to make sure
	 * they aren’t transparent or duplicated colors mean that the
	 * initialisation step may never complete. Consider the case of an icon
	 * which is a block of solid color. */
	for (ClusterPixel8 *p = pixels; p < pixels_end; p++) {
		if (p->alpha < minimum_alpha)
			p->cluster = G_N_ELEMENTS (cluster_centres);
		else
			p->cluster = random_int_range_no_replacement (G_N_ELEMENTS (cluster_centres), used_clusters, &n_used_clusters);
	}

	/* Iterate until every cluster is relatively settled. This is determined
	 * by the number of pixels whose assignment to a cluster changes in
	 * each iteration — if the number of pixels is less than 1% of the image
	 * then subsequent iterations are not going to significantly affect the
	 * results.
	 *
	 * As we’re choosing key colors, finding the optimal result is not
	 * needed. We just need to find one which is good enough, quickly.
	 *
	 * A second termination condition is set on the number of iterations, to
	 * avoid a potential infinite loop. This termination condition is never
	 * normally expected to be hit — typically an icon will require 5–10
	 * iterations to terminate based on @n_assignments_changed. */
	assignments_termination_limit = width * height * 0.01;
	n_iterations = 0;
	do {
		/* Update step. Re-calculate the centroid of each cluster from
		 * the colors which are in it. */
		memset (cluster_accumulators, 0, sizeof (cluster_accumulators));

		for (const ClusterPixel8 *p = pixels; p < pixels_end; p++) {
			if (p->cluster >= G_N_ELEMENTS (cluster_centres))
				continue;

			cluster_accumulators[p->cluster].red += p->color.red;
			cluster_accumulators[p->cluster].green += p->color.green;
			cluster_accumulators[p->cluster].blue += p->color.blue;
			cluster_accumulators[p->cluster].n_members++;
		}

		for (gsize i = 0; i < G_N_ELEMENTS (cluster_centres); i++) {
			if (cluster_accumulators[i].n_members == 0)
				continue;

			cluster_centres[i].red = cluster_accumulators[i].red / cluster_accumulators[i].n_members;
			cluster_centres[i].green = cluster_accumulators[i].green / cluster_accumulators[i].n_members;
			cluster_centres[i].blue = cluster_accumulators[i].blue / cluster_accumulators[i].n_members;
		}

		/* Update assignments of colors to clusters. */
		n_assignments_changed = 0;
		for (ClusterPixel8 *p = pixels; p < pixels_end; p++) {
			gsize new_cluster;

			if (p->cluster >= G_N_ELEMENTS (cluster_centres))
				continue;

			new_cluster = nearest_cluster (&p->color, cluster_centres, G_N_ELEMENTS (cluster_centres));
			if (new_cluster != p->cluster)
				n_assignments_changed++;
			p->cluster = new_cluster;
		}

		n_iterations++;
	} while (n_assignments_changed > assignments_termination_limit && n_iterations < 50);

	/* Output the cluster centres: these are the icon’s key colors. */
	for (gsize i = 0; i < G_N_ELEMENTS (cluster_centres); i++) {
		GdkRGBA color;

		if (cluster_accumulators[i].n_members == 0)
			continue;

		color.red = (gdouble) cluster_centres[i].red / 255.0;
		color.green = (gdouble) cluster_centres[i].green / 255.0;
		color.blue = (gdouble) cluster_centres[i].blue / 255.0;
		color.alpha = 1.0;
		g_array_append_val (colors, color);
	}
}

/**
 * gs_calculate_key_colors:
 * @pixbuf: an app icon to calculate key colors from
 *
 * Calculate the set of key colors present in @pixbuf. These are the colors
 * which stand out the most, and they are subjective. This function does not
 * guarantee to return perfect results, but should return workable results for
 * most icons.
 *
 * @pixbuf will be scaled down to 32×32 pixels, so if it can be provided at
 * that resolution by the caller, this function will return faster.
 *
 * Returns: (transfer full) (element-type GdkRGBA): key colors for @pixbuf
 * Since: 40
 */
GArray *
gs_calculate_key_colors (GdkPixbuf *pixbuf)
{
	g_autoptr(GdkPixbuf) pb_small = NULL;
	g_autoptr(GArray) colors = g_array_new (FALSE, FALSE, sizeof (GdkRGBA));

	/* people almost always use BILINEAR scaling with pixbufs, but we can
	 * use NEAREST here since we only care about the rough colour data, not
	 * whether the edges in the image are smooth and visually appealing;
	 * NEAREST is twice as fast as BILINEAR */
	pb_small = gdk_pixbuf_scale_simple (pixbuf, 32, 32, GDK_INTERP_NEAREST);

	/* require an alpha channel for storing temporary values; most images
	 * have one already, about 2% don’t */
	if (gdk_pixbuf_get_n_channels (pixbuf) != 4) {
		g_autoptr(GdkPixbuf) temp = g_steal_pointer (&pb_small);
		pb_small = gdk_pixbuf_add_alpha (temp, FALSE, 0, 0, 0);
	}

	/* get a list of key colors */
	k_means (colors, pb_small);

	return g_steal_pointer (&colors);
}
