/*
    SPDX-FileCopyrightText: 2022 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "deepin-kwin_export.h"

#include <QImage>

#include <variant>

#include <xcb/render.h>
namespace KWin
{

class GLFramebuffer;

class KWIN_EXPORT RenderTarget
{
public:
    RenderTarget();
    explicit RenderTarget(GLFramebuffer *fbo);
    explicit RenderTarget(QImage *image);
    explicit RenderTarget(xcb_render_picture_t *renderPicture);

    QSize size() const;

    using NativeHandle = std::variant<GLFramebuffer *, QImage *, xcb_render_picture_t *>;
    NativeHandle nativeHandle() const;

    void setDevicePixelRatio(qreal ratio);
    qreal devicePixelRatio() const;

private:
    NativeHandle m_nativeHandle;
    qreal m_devicePixelRatio = 1;
};

} // namespace KWin
