/*
    SPDX-FileCopyrightText: 2021 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "xrendersurfacetexture_internal.h"

namespace KWin
{

XRenderSurfaceTextureInternal::XRenderSurfaceTextureInternal(XRenderBackend *backend, SurfacePixmapInternal *pixmap)
    : XRenderSurfaceTexture(backend)
    , m_pixmap(pixmap)
{
}

} // namespace KWin
