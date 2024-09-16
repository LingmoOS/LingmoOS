/*
    SPDX-FileCopyrightText: 2021 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "xrendersurfacetexture.h"

namespace KWin
{

class SurfacePixmapInternal;

class KWIN_EXPORT XRenderSurfaceTextureInternal : public XRenderSurfaceTexture
{
public:
    XRenderSurfaceTextureInternal(XRenderBackend *backend, SurfacePixmapInternal *pixmap);

protected:
    SurfacePixmapInternal *m_pixmap;
};

} // namespace KWin
