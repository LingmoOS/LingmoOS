/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Endless OS Foundation LLC
 *
 * Authors:
 *  - Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdk.h>
#include <locale.h>
#include <math.h>

#include "gs-key-colors.h"

/* Test program which can be used to check the output and performance of the
 * gs_calculate_key_colors() function. It is linked against liblingmosoftware, so
 * will use the function implementation from there. It outputs a HTML page which
 * lists each icon from the flathub appstream data in your home directory, along
 * with its extracted key colors and how long extraction took. */

static void
print_colours (GString *html_output,
               GArray  *colours)
{
	g_string_append_printf (html_output, "<table class='colour-swatch'><tr>");
	for (guint i = 0; i < colours->len; i++) {
		GdkRGBA *rgba = &g_array_index (colours, GdkRGBA, i);

		g_string_append_printf (html_output,
					"<td style='background-color: rgb(%u, %u, %u)'></td>",
					(guint) (rgba->red * 255),
					(guint) (rgba->green * 255),
					(guint) (rgba->blue * 255));

		if (i % 3 == 2)
			g_string_append (html_output, "</tr><tr>");
	}
	g_string_append_printf (html_output, "</tr></table>");
}

static void
print_summary_statistics (GString *html_output,
                          GArray  *durations  /* (element-type gint64) */)
{
	gint64 sum = 0, min = G_MAXINT64, max = G_MININT64;
	guint n_measurements = durations->len;
	gint64 mean, stddev;
	gint64 sum_of_square_deviations = 0;

	for (guint i = 0; i < durations->len; i++) {
		gint64 duration = g_array_index (durations, gint64, i);
		sum += duration;
		min = MIN (min, duration);
		max = MAX (max, duration);
	}

	mean = sum / n_measurements;

	for (guint i = 0; i < durations->len; i++) {
		gint64 duration = g_array_index (durations, gint64, i);
		gint64 diff = duration - mean;
		sum_of_square_deviations += diff * diff;
	}

	stddev = sqrt (sum_of_square_deviations / n_measurements);

	g_string_append_printf (html_output,
				"[%" G_GINT64_FORMAT ", %" G_GINT64_FORMAT "]μs, mean %" G_GINT64_FORMAT "±%" G_GINT64_FORMAT "μs, n = %u",
				min, max, mean, stddev, n_measurements);
}

int
main (void)
{
	const gchar *icons_subdir = ".local/share/flatpak/appstream/flathub/x86_64/active/icons/128x128";
	g_autofree gchar *icons_dir = g_build_filename (g_get_home_dir (), icons_subdir, NULL);
	g_autoptr(GDir) dir = NULL;
	const gchar *entry;
	g_autoptr(GPtrArray) filenames = g_ptr_array_new_with_free_func (g_free);
	g_autoptr(GPtrArray) pixbufs = g_ptr_array_new_with_free_func (g_object_unref);
	g_autoptr(GString) html_output = g_string_new ("");
	g_autoptr(GArray) durations = g_array_new (FALSE, FALSE, sizeof (gint64));

	setlocale (LC_ALL, "");

	/* Load pixbufs from the icons directory. */
	dir = g_dir_open (icons_dir, 0, NULL);
	if (dir == NULL)
		return 1;

	while ((entry = g_dir_read_name (dir)) != NULL) {
		g_autofree gchar *filename = g_build_filename (icons_dir, entry, NULL);
		g_autoptr(GdkPixbuf) pixbuf = gdk_pixbuf_new_from_file (filename, NULL);

		if (pixbuf == NULL)
			continue;

		g_ptr_array_add (filenames, g_steal_pointer (&filename));
		g_ptr_array_add (pixbufs, g_steal_pointer (&pixbuf));
	}

	if (!pixbufs->len)
		return 2;

	/* Set up an output page */
	g_string_append (html_output,
			 "<!DOCTYPE html>\n"
			 "<html>\n"
			 "  <head>\n"
			 "    <meta charset='UTF-8'>\n"
			 "    <style>\n"
			 "      #main-table, #main-table th, #main-table td { border: 1px solid black; border-collapse: collapse }\n"
			 "      #main-table th, #main-table td { padding: 4px }\n"
			 "      td.number { text-align: right }\n"
			 "      table.colour-swatch td { width: 30px; height: 30px }\n"
			 "      .faster { background-color: rgb(190, 236, 57) }\n"
			 "      .slower { background-color: red }\n"
			 "    </style>\n"
			 "  </head>\n"
			 "  <body>\n"
			 "    <table id='main-table'>\n"
			 "      <thead>\n"
			 "        <tr>\n"
			 "          <td>Filename</td>\n"
			 "          <td>Icon</td>\n"
			 "          <td>Code duration (μs)</td>\n"
			 "          <td>Code colours</td>\n"
			 "        </tr>\n"
			 "      </thead>\n");

	/* For each pixbuf, run both algorithms. */
	for (guint i = 0; i < pixbufs->len; i++) {
		GdkPixbuf *pixbuf = pixbufs->pdata[i];
		const gchar *filename = filenames->pdata[i];
		g_autofree gchar *basename = g_path_get_basename (filename);
		g_autoptr(GArray) colours = NULL;
		gint64 start_time, duration;

		g_message ("Processing %u of %u, %s", i + 1, pixbufs->len, filename);

		start_time = g_get_real_time ();
		colours = gs_calculate_key_colors (pixbuf);
		duration = g_get_real_time () - start_time;

		g_string_append_printf (html_output,
					"<tr>\n"
					"<th>%s</th>\n"
					"<td><img src='file:%s'></td>\n"
					"<td class='number'>%" G_GINT64_FORMAT "</td>\n"
					"<td>",
					basename, filename, duration);
		print_colours (html_output, colours);
		g_string_append (html_output,
				 "</td>\n"
				 "</tr>\n");

		g_array_append_val (durations, duration);
	}

	/* Summary statistics for the timings. */
	g_string_append (html_output, "<tfoot><tr><td></td><td></td><td>");
	print_summary_statistics (html_output, durations);
	g_string_append (html_output, "</td><td></td></tr></tfoot>");

	g_string_append (html_output, "</table></body></html>");

	g_print ("%s\n", html_output->str);

	return 0;
}
