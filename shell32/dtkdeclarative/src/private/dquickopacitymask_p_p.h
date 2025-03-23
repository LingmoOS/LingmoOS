// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKOPACITYMASK_P_P_H
#define DQUICKOPACITYMASK_P_P_H

#include "dquickopacitymask_p.h"

#include <private/qquickitem_p.h>

DQUICK_BEGIN_NAMESPACE

class DQuickOpacityMaskPrivate : public QQuickItemPrivate
{
    Q_DECLARE_PUBLIC(DQuickOpacityMask)

public:
    DQuickOpacityMaskPrivate()
        : source(nullptr)
        , maskSource(nullptr)
        , invert(false)
        , sourceChanged(false)
    {}

    QQuickItem *source;
    QQuickItem *maskSource;
    bool invert;
    bool sourceChanged;
};

DQUICK_END_NAMESPACE

#endif // DQUICKOPACITYMASK_P_P_H
