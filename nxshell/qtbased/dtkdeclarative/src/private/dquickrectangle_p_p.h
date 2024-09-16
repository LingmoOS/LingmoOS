// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKRECTANGLE_P_P_H
#define DQUICKRECTANGLE_P_P_H

#include "dquickrectangle_p.h"
#include "dquickitemviewport_p.h"

#include <private/qquickitem_p.h>

DQUICK_BEGIN_NAMESPACE

class Q_DECL_HIDDEN DQuickRectanglePrivate : public QQuickItemPrivate
{
    Q_DECLARE_PUBLIC(DQuickRectangle)

public:
    DQuickRectanglePrivate()
        : QQuickItemPrivate ()
        , radius(0.0)
        , color(Qt::white)
    {

    }

    inline QSGTexture *textureForRadiusMask()
    {
        if (!maskTexture && radius > 0) {
            QQuickItemPrivate *d = QQuickItemPrivate::get(q_func());
            maskTexture = MaskTextureCache::instance()->getTexture(d->sceneGraphRenderContext(),
                                                                   qRound(radius * d->window->effectiveDevicePixelRatio()), true);
        }
        return maskTexture->texture;
    }

    qreal radius;
    QColor color;
    DQuickRectangle::Corners corners;
    MaskTextureCache::TextureData maskTexture;
};

DQUICK_END_NAMESPACE

#endif // DQUICKRECTANGLE_P_P_H
