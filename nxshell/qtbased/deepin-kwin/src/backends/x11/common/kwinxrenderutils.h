/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2008 Lubos Lunak <l.lunak@kde.org>
    SPDX-FileCopyrightText: 2022 MBition GmbH
    SPDX-FileContributor: Kai Uwe Broulik <kai_uwe.broulik@mbition.io>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

// KWin
#include <deepin-kwin_export.h>
// Qt
#include <QExplicitlySharedDataPointer>
#include <QRegion>
// XCB
#include <xcb/render.h>
#include <xcb/xfixes.h>

#include <memory>

class QImage;

/** @addtogroup kwineffects */
/** @{ */

namespace KWin
{

/** @internal */
class KWIN_EXPORT XRenderPictureData
    : public QSharedData
{
public:
    explicit XRenderPictureData(xcb_render_picture_t pic = XCB_RENDER_PICTURE_NONE);
    ~XRenderPictureData();
    xcb_render_picture_t value();

private:
    xcb_render_picture_t picture;
    Q_DISABLE_COPY(XRenderPictureData)
};

/**
 * @short Wrapper around XRender Picture.
 *
 * This class wraps XRender's Picture, providing proper initialization,
 * convenience constructors and freeing of resources.
 * It should otherwise act exactly like the Picture type.
 */
class KWIN_EXPORT XRenderPicture
{
public:
    explicit XRenderPicture(xcb_render_picture_t pic = XCB_RENDER_PICTURE_NONE);
    explicit XRenderPicture(const QImage &img);
    XRenderPicture(xcb_pixmap_t pix, int depth);
    operator xcb_render_picture_t();

private:
    void fromImage(const QImage &img);
    QExplicitlySharedDataPointer< XRenderPictureData > d;
};

class KWIN_EXPORT XFixesRegion
{
public:
    explicit XFixesRegion(const QRegion &region);
    virtual ~XFixesRegion();

    operator xcb_xfixes_region_t();
private:
    xcb_xfixes_region_t m_region;
};

inline XRenderPictureData::XRenderPictureData(xcb_render_picture_t pic)
    : picture(pic)
{
}

inline xcb_render_picture_t XRenderPictureData::value()
{
    return picture;
}

inline XRenderPicture::XRenderPicture(xcb_render_picture_t pic)
    : d(new XRenderPictureData(pic))
{
}

inline XRenderPicture::operator xcb_render_picture_t()
{
    return d->value();
}

namespace XRenderUtils
{
/**
 * @internal
 */
KWIN_EXPORT void init(xcb_connection_t *connection, xcb_window_t rootWindow);

/**
 * Returns the Xrender format that corresponds to the given visual ID.
 */
KWIN_EXPORT xcb_render_pictformat_t findPictFormat(xcb_visualid_t visual);

/**
 * Returns the xcb_render_directformat_t for the given Xrender format.
 */
KWIN_EXPORT const xcb_render_directformat_t *findPictFormatInfo(xcb_render_pictformat_t format);

KWIN_EXPORT XRenderPicture xRenderFill(const xcb_render_color_t &c);
KWIN_EXPORT XRenderPicture xRenderBlendPicture(double opacity);
KWIN_EXPORT xcb_render_color_t preMultiply(const QColor &c, float opacity = 1.0);

/**
 * @internal
 */
KWIN_EXPORT void cleanup();

} // namespace XRenderUtils

} // namespace KWin

/** @} */
