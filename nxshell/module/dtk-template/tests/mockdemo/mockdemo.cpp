// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

// #include <librsvg/rsvg.h>
#include "mockdemo.h"
#include <QDebug>
#include <qglobal.h>
#include <qnumeric.h>

// extern "C"{

////// mock rsvg_handle_new_from_file
RsvgHandle *rsvg_handle_new_from_file_stub(const gchar *file_name, GError **error)
{
    Q_UNUSED(file_name);
    Q_UNUSED(error);
    qInfo() << __func__ << "mocked";
   // magic ptr...烫烫烫
   return reinterpret_cast<RsvgHandle *>(0xcccc);
}

////// mock rsvg_handle_get_dimensions
void rsvg_handle_get_dimensions_stub(RsvgHandle *handle, RsvgDimensionData *dimension_data)
{
    Q_UNUSED(handle);
    Q_UNUSED(dimension_data);
    qInfo() << __func__ << "mocked";
    return;
}

//// mock cairo_image_surface_create_for_data
cairo_surface_t *cairo_image_surface_create_for_data_stub(unsigned char *data,
          cairo_format_t format, int width, int height, int stride)
{
    Q_UNUSED(data);
    Q_UNUSED(format);
    Q_UNUSED(width);
    Q_UNUSED(height);
    Q_UNUSED(stride);
    qInfo() << __func__ << "mocked";

   return reinterpret_cast<cairo_surface_t *>(0xcccc); // magic ptr...烫烫烫
}

//// mock cairo_create
cairo_t *cairo_create_stub(cairo_surface_t *target)
{
    Q_UNUSED(target);
    qInfo() << __func__ << "mocked";

   return reinterpret_cast<cairo_t *>(0xcccc);// magic ptr...烫烫烫
}

//// mock cairo_scale
void cairo_scale_stub(cairo_t *cr, double sx, double sy)
{
    Q_UNUSED(cr);
    Q_UNUSED(sx);
    Q_UNUSED(sy);
    qInfo() << __func__ << "mocked";
    return;
}

//// mock cairo_translate
void cairo_translate_stub(cairo_t *cr, double tx, double ty)
{
    Q_UNUSED(cr);
    Q_UNUSED(tx);
    Q_UNUSED(ty);
    qInfo() << __func__ << "mocked";
    return;
}

//// mock rsvg_handle_render_cairo
gboolean rsvg_handle_render_cairo_stub(RsvgHandle *handle, cairo_t *cr)
{
    Q_UNUSED(handle);
    Q_UNUSED(cr);
    qInfo() << __func__ << "mocked";
    return true;
}

//// mock cairo_surface_destroy
void cairo_surface_destroy_stub(cairo_surface_t *surface)
{
    Q_UNUSED(surface);
    qInfo() << __func__ << "mocked";
    return;
}

//// mock cairo_destroy
void cairo_destroy_stub(cairo_t *cr)
{
    Q_UNUSED(cr);
    qInfo() << __func__ << "mocked";
    return;
}

//// mock g_object_unref
/* ld --wrap=symbol
*  Use a wrapper function for symbol.  Any undefined reference to
*  symbol will be resolved to "__wrap_symbol".  Any undefined
*  reference to "__real_symbol" will be resolved to symbol.
*  call __real_g_object_unref for real g_object_unref function
*/
void g_object_unref_stub (gpointer object)
{
    Q_UNUSED(object);
    qInfo() << __func__ << "mocked";
    return;
}

// }
