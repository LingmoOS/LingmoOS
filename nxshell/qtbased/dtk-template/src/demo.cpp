// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <librsvg/rsvg.h>

#include "demo_p.h"
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QDebug>

DDEMO_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DUnexpected;
using DCORE_NAMESPACE::emplace_tag;

DemoPrivate::DemoPrivate(Demo *parent)
    : q_ptr(parent)
    , m_inter(new DDemoInterface(this))
{
}

Demo::Demo(QObject *parent)
    : QObject(parent)
    , d_ptr(new DemoPrivate(this))
{
    Q_D(const Demo);
    connect(d->m_inter, &DDemoInterface::DockedChanged, this, &Demo::DockedChanged);
}

Demo::~Demo() = default;

bool Demo::docked()
{
    Q_D(const Demo);
    return d->m_inter->Docked();
}

DExpected<UserPathList> Demo::listUsers()
{
    Q_D(const Demo);
    auto reply = d->m_inter->ListUsers();
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    UserPathList ret;
    for (const auto &i : reply.value())
        ret.append(i);
    return ret;
}

int Demo::add(const int a, const int b)
{
    return a + b;
}

bool Demo::svg2png(const QString &svgfile, const QString &pngfile, QSize size)
{
    QImage outPutimage(size, QImage::Format_ARGB32);
    outPutimage.fill(Qt::transparent);
    QPainter outputPainter(&outPutimage);

    // librsvg
    {
        GError *error = nullptr;
        RsvgHandle *handle = rsvg_handle_new_from_file(svgfile.toUtf8().data(), &error);
        if (error) {
            qWarning("DSvgRenderer::load: %s", error->message);
            g_error_free(error);
            return false;
        }

        RsvgDimensionData rsvg_data;

        rsvg_handle_get_dimensions(handle, &rsvg_data);

        QSizeF defaultSize(rsvg_data.width, rsvg_data.height);
        QRectF viewBox(QPointF(0, 0), defaultSize);

        cairo_surface_t *surface = cairo_image_surface_create_for_data(
            outPutimage.bits(), CAIRO_FORMAT_ARGB32, outPutimage.width(), outPutimage.height(), outPutimage.bytesPerLine());
        cairo_t *cairo = cairo_create(surface);
        cairo_scale(cairo, outPutimage.width() / viewBox.width(), outPutimage.height() / viewBox.height());
        cairo_translate(cairo, -viewBox.x(), -viewBox.y());
        rsvg_handle_render_cairo(handle, cairo);

        cairo_destroy(cairo);
        cairo_surface_destroy(surface);
        g_object_unref(handle);
    }

    return outPutimage.save(pngfile);
}

DDEMO_END_NAMESPACE
