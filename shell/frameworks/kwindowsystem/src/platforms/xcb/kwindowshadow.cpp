/*
    SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kwindowshadow_p_x11.h"

#include <private/qtx11extras_p.h>

static const QByteArray s_atomName = QByteArrayLiteral("_KDE_NET_WM_SHADOW");

bool KWindowShadowTilePrivateX11::create()
{
    xcb_connection_t *connection = QX11Info::connection();
    xcb_window_t rootWindow = QX11Info::appRootWindow();

    const uint16_t width = uint16_t(image.width());
    const uint16_t height = uint16_t(image.height());
    const uint8_t depth = uint8_t(image.depth());

    pixmap = xcb_generate_id(connection);
    gc = xcb_generate_id(connection);

    xcb_create_pixmap(connection, depth, pixmap, rootWindow, width, height);
    xcb_create_gc(connection, gc, pixmap, 0, nullptr);

    xcb_put_image(connection, //
                  XCB_IMAGE_FORMAT_Z_PIXMAP,
                  pixmap,
                  gc,
                  width,
                  height,
                  0,
                  0,
                  0,
                  depth,
                  image.sizeInBytes(),
                  image.constBits());

    return true;
}

void KWindowShadowTilePrivateX11::destroy()
{
    xcb_connection_t *connection = QX11Info::connection();
    if (connection) {
        xcb_free_pixmap(connection, pixmap);
        xcb_free_gc(connection, gc);
    }
    pixmap = XCB_PIXMAP_NONE;
    gc = XCB_NONE;
}

KWindowShadowTilePrivateX11 *KWindowShadowTilePrivateX11::get(const KWindowShadowTile *tile)
{
    KWindowShadowTilePrivate *d = KWindowShadowTilePrivate::get(tile);
    return static_cast<KWindowShadowTilePrivateX11 *>(d);
}

static xcb_atom_t lookupAtom(const QByteArray &atomName)
{
    xcb_connection_t *connection = QX11Info::connection();
    if (!connection) {
        return XCB_ATOM_NONE;
    }

    xcb_intern_atom_cookie_t atomCookie = xcb_intern_atom_unchecked(connection, //
                                                                    false,
                                                                    atomName.size(),
                                                                    atomName.constData());
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, atomCookie, nullptr);

    if (!reply) {
        return XCB_ATOM_NONE;
    }

    xcb_atom_t atom = reply->atom;
    free(reply);

    return atom;
}

static xcb_pixmap_t nativeHandleForTile(const KWindowShadowTile::Ptr &tile)
{
    const auto d = KWindowShadowTilePrivateX11::get(tile.data());
    return d->pixmap;
}

bool KWindowShadowPrivateX11::create()
{
    xcb_connection_t *connection = QX11Info::connection();

    const xcb_atom_t atom = lookupAtom(s_atomName);
    if (atom == XCB_ATOM_NONE) {
        return false;
    }

    QList<quint32> data(12);
    int i = 0;

    // Unfortunately we cannot use handle of XCB_PIXMAP_NONE for missing shadow tiles because
    // KWin expects **all** shadow tile handles to be valid. Maybe we could address this small
    // inconvenience and then remove the empty tile stuff.

    if (topTile) {
        data[i++] = nativeHandleForTile(topTile);
    } else {
        data[i++] = nativeHandleForTile(getOrCreateEmptyTile());
    }

    if (topRightTile) {
        data[i++] = nativeHandleForTile(topRightTile);
    } else {
        data[i++] = nativeHandleForTile(getOrCreateEmptyTile());
    }

    if (rightTile) {
        data[i++] = nativeHandleForTile(rightTile);
    } else {
        data[i++] = nativeHandleForTile(getOrCreateEmptyTile());
    }

    if (bottomRightTile) {
        data[i++] = nativeHandleForTile(bottomRightTile);
    } else {
        data[i++] = nativeHandleForTile(getOrCreateEmptyTile());
    }

    if (bottomTile) {
        data[i++] = nativeHandleForTile(bottomTile);
    } else {
        data[i++] = nativeHandleForTile(getOrCreateEmptyTile());
    }

    if (bottomLeftTile) {
        data[i++] = nativeHandleForTile(bottomLeftTile);
    } else {
        data[i++] = nativeHandleForTile(getOrCreateEmptyTile());
    }

    if (leftTile) {
        data[i++] = nativeHandleForTile(leftTile);
    } else {
        data[i++] = nativeHandleForTile(getOrCreateEmptyTile());
    }

    if (topLeftTile) {
        data[i++] = nativeHandleForTile(topLeftTile);
    } else {
        data[i++] = nativeHandleForTile(getOrCreateEmptyTile());
    }

    if (topLeftTile || topTile || topRightTile) {
        data[i++] = uint32_t(padding.top());
    } else {
        data[i++] = 1;
    }

    if (topRightTile || rightTile || bottomRightTile) {
        data[i++] = uint32_t(padding.right());
    } else {
        data[i++] = 1;
    }

    if (bottomRightTile || bottomTile || bottomLeftTile) {
        data[i++] = uint32_t(padding.bottom());
    } else {
        data[i++] = 1;
    }

    if (bottomLeftTile || leftTile || topLeftTile) {
        data[i++] = uint32_t(padding.left());
    } else {
        data[i++] = 1;
    }

    xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window->winId(), atom, XCB_ATOM_CARDINAL, 32, data.size(), data.constData());
    xcb_flush(connection);

    return true;
}

void KWindowShadowPrivateX11::destroy()
{
    emptyTile = nullptr;

    // For some reason, QWindow changes visibility of QSurface::surfaceHandle().
    const QSurface *surface = window;

    // Attempting to uninstall the shadow after the platform window had been destroyed.
    if (!(surface && surface->surfaceHandle())) {
        return;
    }

    xcb_connection_t *connection = QX11Info::connection();

    const xcb_atom_t atom = lookupAtom(s_atomName);
    if (atom == XCB_ATOM_NONE) {
        return;
    }

    xcb_delete_property(connection, window->winId(), atom);
}

KWindowShadowTile::Ptr KWindowShadowPrivateX11::getOrCreateEmptyTile()
{
    if (!emptyTile) {
        QImage image(QSize(1, 1), QImage::Format_ARGB32);
        image.fill(Qt::transparent);

        emptyTile = KWindowShadowTile::Ptr::create();
        emptyTile->setImage(image);
        emptyTile->create();
    }

    return emptyTile;
}
