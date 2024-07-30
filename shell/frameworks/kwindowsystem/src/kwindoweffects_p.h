/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KWINDOWEFFECTS_P_H
#define KWINDOWEFFECTS_P_H
#include "kwindoweffects.h"

class KWINDOWSYSTEM_EXPORT KWindowEffectsPrivate
{
public:
    virtual ~KWindowEffectsPrivate();
    virtual bool isEffectAvailable(KWindowEffects::Effect effect) = 0;
    virtual void slideWindow(QWindow *window, KWindowEffects::SlideFromLocation location, int offset) = 0;
    virtual void enableBlurBehind(QWindow *window, bool enable = true, const QRegion &region = QRegion()) = 0;
    virtual void enableBackgroundContrast(QWindow *window,
                                          bool enable = true,
                                          qreal contrast = 1,
                                          qreal intensity = 1,
                                          qreal saturation = 1,
                                          const QRegion &region = QRegion()) = 0;

protected:
    KWindowEffectsPrivate();
};
#endif
