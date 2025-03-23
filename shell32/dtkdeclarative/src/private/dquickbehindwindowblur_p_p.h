// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKBEHINDWINDOWBLUR_P_P_H
#define DQUICKBEHINDWINDOWBLUR_P_P_H

#include "dquickbehindwindowblur_p.h"
#include "dquickwindow.h"

#include <private/qquickitem_p.h>

DQUICK_BEGIN_NAMESPACE

class DQuickBehindWindowBlurPrivate : public QQuickItemPrivate
{
    Q_DECLARE_PUBLIC(DQuickBehindWindowBlur)

public:
    DQuickBehindWindowBlurPrivate()
        : windowAttach(nullptr)
        , blendColor(Qt::transparent)
    {

    }
    ~DQuickBehindWindowBlurPrivate();

    void _q_onHasBlurWindowChanged();
    void _q_updateBlurArea();
    bool updateBlurEnable();

    inline bool isValidBlur() const {
        Q_ASSERT(effectiveBlurEnabled);
        if (!blurPath.isEmpty())
            return true;
        return blurArea.width > 0 && blurArea.height > 0;
    }
    inline void makeToInvalidBlur() {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        blurPath.clear();
#else
        blurPath = QPainterPath();
#endif
        blurArea.width = blurArea.height = -1;
    }

    DQuickWindowAttached *windowAttach = nullptr;

    qreal radius = 0;
    QColor blendColor;
    DPlatformHandle::WMBlurArea blurArea;
    QPainterPath blurPath;
    bool blurEnabled = true;
    bool effectiveBlurEnabled = false;
};

DQUICK_END_NAMESPACE

#endif // DQUICKBEHINDWINDOWBLUR_H_H
