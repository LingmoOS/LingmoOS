// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKGLOW_P_P_H
#define DQUICKGLOW_P_P_H

#include "dquickglow_p.h"

#include <private/qquickitem_p.h>

DQUICK_BEGIN_NAMESPACE

class DQuickGlowPrivate : public QQuickItemPrivate
{
    Q_DECLARE_PUBLIC(DQuickGlow)

public:
    DQuickGlowPrivate()
        : QQuickItemPrivate()
        , fill(false)
    {

    }

    qreal glowRadius;
    QColor color;
    qreal spread;
    qreal relativeSizeX;
    qreal relativeSizeY;
    bool fill;
};

DQUICK_END_NAMESPACE

#endif // DQUICKGLOW_P_P_H
