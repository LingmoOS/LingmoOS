// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <librsvg/rsvg.h>

// extern "C" {
RsvgHandle *rsvg_handle_new_from_file_stub(const gchar *file_name, GError **error);
void rsvg_handle_get_dimensions_stub(RsvgHandle *handle, RsvgDimensionData *dimension_data);
cairo_surface_t *
cairo_image_surface_create_for_data_stub(unsigned char *data, cairo_format_t format, int width, int height, int stride);
cairo_t *cairo_create_stub(cairo_surface_t *target);
void cairo_scale_stub(cairo_t *cr, double sx, double sy);
void cairo_translate_stub(cairo_t *cr, double tx, double ty);
gboolean rsvg_handle_render_cairo_stub(RsvgHandle *handle, cairo_t *cr);
void cairo_surface_destroy_stub(cairo_surface_t *surface);
void cairo_destroy_stub(cairo_t *cr);
void g_object_unref_stub(gpointer object);

// }
