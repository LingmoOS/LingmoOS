/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2024 zhang yu <zhangyud@uniontech.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "debugpixmap.h"
#include "window.h"
#include "x11window.h"
#include "unmanaged.h"
#include "scene/surfaceitem.h"
#include "scene/surfaceitem_x11.h"
#include "platformsupport/scenes/opengl/openglsurfacetexture.h"
#include "workspace.h"

namespace KWin
{

void DebugPixmap::saveImageFromTexture(xcb_window_t winid, Window *w)
{
    if (auto item = w->surfaceItem()) {
        if (SurfacePixmap *surfacePixmap = item->pixmap()) {
            if (GLTexture *texture = static_cast<OpenGLSurfaceTexture *>(surfacePixmap->texture())->texture()) {
                texture->toImage().save(QString("/tmp/%1-fromTexture.png").arg(winid), "PNG", 100);
            }
        }
    }
}

void DebugPixmap::saveImageFromPixmap(xcb_window_t winid, Window *w)
{
    if ((qobject_cast<KWin::X11Window *>(w) == nullptr) && (qobject_cast<KWin::Unmanaged *>(w) == nullptr)) {
        return;
    }
    if (auto item = w->surfaceItem()) {
        if (SurfacePixmap *surfacePixmap = item->pixmap()) {
            xcb_connection_t *connection = kwinApp()->x11Connection();
            xcb_window_t req_win_id = w->frameId();
            xcb_pixmap_t win_pixmap = qobject_cast<SurfacePixmapX11 *>(surfacePixmap)->pixmap();
            xcb_composite_name_window_pixmap(connection, req_win_id, win_pixmap);
            QRectF bufferGeometry = w->bufferGeometry();
            xcb_generic_error_t *err = NULL;
            QImage img;
            // get the image
            xcb_get_image_reply_t *m_gi_reply {nullptr};
            xcb_get_image_cookie_t gi_cookie = xcb_get_image(connection, XCB_IMAGE_FORMAT_Z_PIXMAP, win_pixmap, 0, 0, bufferGeometry.width(), bufferGeometry.height(), (uint32_t)(~0UL));
            m_gi_reply = xcb_get_image_reply(connection, gi_cookie, &err);

            if (m_gi_reply) {
                int data_len = xcb_get_image_data_length(m_gi_reply);
                qCritical() << " data_len = " << data_len << " visual = " << m_gi_reply->visual << "depth = " << m_gi_reply->depth
                        << "size = " << bufferGeometry.width() << " x " << bufferGeometry.height();

                img = QImage(xcb_get_image_data(m_gi_reply), bufferGeometry.width(), bufferGeometry.height(), QImage::Format_RGB32);
            } else if (err) {
                qCritical() << "XCB error "<< err->error_code;
                free(err);
                err = NULL;
                return;
            }
            img.save(QString("/tmp/%1-fromPixmap.png").arg(winid), "PNG", 100);
            free(m_gi_reply);
        }
    }
}

void DebugPixmap::saveImageFromXorg(xcb_window_t winid)
{
    xcb_window_t req_win_id = winid;
    xcb_connection_t *connection = xcb_connect(NULL, NULL);
    xcb_generic_error_t *err = NULL;

    QImage img;
    // request redirection of window
    xcb_composite_redirect_window(connection, req_win_id, XCB_COMPOSITE_REDIRECT_AUTOMATIC);
    int win_h, win_w, win_d;
    xcb_get_geometry_cookie_t gg_cookie = xcb_get_geometry(connection, req_win_id);
    xcb_get_geometry_reply_t *gg_reply = xcb_get_geometry_reply(connection, gg_cookie, &err);
    if (gg_reply) {
        win_w = gg_reply->width;
        win_h = gg_reply->height;
        win_d = gg_reply->depth;
        free(gg_reply);
    } else if (err) {
        qCritical() << "get geometry: XCB error "<< err->error_code;
        free(err);
        err = NULL;
        return;
    }

    // create a pixmap
    xcb_pixmap_t win_pixmap = xcb_generate_id(connection);
    xcb_composite_name_window_pixmap(connection, req_win_id, win_pixmap);

    // get the image
    xcb_get_image_reply_t *m_gi_reply {nullptr};
    xcb_get_image_cookie_t gi_cookie = xcb_get_image(connection, XCB_IMAGE_FORMAT_Z_PIXMAP, win_pixmap, 0, 0, win_w, win_h, (uint32_t)(~0UL));
    m_gi_reply = xcb_get_image_reply(connection, gi_cookie, &err);

    if (m_gi_reply) {
        int data_len = xcb_get_image_data_length(m_gi_reply);
        qCritical() << " data_len = " << data_len << " visual = " << m_gi_reply->visual << "depth = " << m_gi_reply->depth
                 << "size = " << win_w << " x " << win_h;

        img = QImage(xcb_get_image_data(m_gi_reply), win_w, win_h, QImage::Format_RGB32);
    } else if (err) {
        qCritical() << "XCB error "<< err->error_code;
        free(err);
        err = NULL;
        return;
    }
    img.save(QString("/tmp/%1-fromXorg.png").arg(winid), "PNG", 100);
    free(m_gi_reply);
    xcb_disconnect(connection);
}

void DebugPixmap::saveCompositePixmap()
{
    QRect geometry = workspace()->geometry();
    QImage image = QImage(geometry.width(), geometry.height(), QImage::Format_RGBA8888);
    glReadPixels(geometry.x(), geometry.y(), geometry.width(), geometry.height(), GL_RGBA,
                         GL_UNSIGNED_BYTE, static_cast<GLvoid *>(image.bits()));
    image.save("/tmp/composite_screen.png", "PNG", 100);
}

}
