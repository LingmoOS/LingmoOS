/*
    SPDX-FileCopyrightText: 2022 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rendertarget.h"
#include "kwinglutils.h"

#include <QDesktopWidget>

namespace KWin
{

RenderTarget::RenderTarget()
{
}

RenderTarget::RenderTarget(GLFramebuffer *fbo)
    : m_nativeHandle(fbo)
{
}

RenderTarget::RenderTarget(QImage *image)
    : m_nativeHandle(image)
{
}

RenderTarget::RenderTarget(xcb_render_picture_t *renderPicture)
    : m_nativeHandle(renderPicture)
{
}


QSize RenderTarget::size() const
{
    if (auto fbo = std::get_if<GLFramebuffer *>(&m_nativeHandle)) {
        return (*fbo)->size();
    } else if (auto image = std::get_if<QImage *>(&m_nativeHandle)) {
        return (*image)->size();
    }  else if (auto renderPicture = std::get_if<xcb_render_picture_t *>(&m_nativeHandle)) {
        QDesktopWidget desktopWidget;
        QRect screenRect = desktopWidget.screenGeometry();
        return QSize(screenRect.width(), screenRect.height());
    }  else {
        Q_UNREACHABLE();
    }
}

RenderTarget::NativeHandle RenderTarget::nativeHandle() const
{
    return m_nativeHandle;
}

void RenderTarget::setDevicePixelRatio(qreal ratio)
{
    m_devicePixelRatio = ratio;
}

qreal RenderTarget::devicePixelRatio() const
{
    return m_devicePixelRatio;
}

} // namespace KWin
