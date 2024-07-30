/*
    SPDX-FileCopyrightText: 2009 Marco Martin <notmart@gmail.com>
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kwindoweffects_dummy_p.h"

#include <QList>

KWindowEffectsPrivateDummy::KWindowEffectsPrivateDummy()
{
}

KWindowEffectsPrivateDummy::~KWindowEffectsPrivateDummy()
{
}

bool KWindowEffectsPrivateDummy::isEffectAvailable(KWindowEffects::Effect effect)
{
    Q_UNUSED(effect)
    return false;
}

void KWindowEffectsPrivateDummy::slideWindow(QWindow *window, KWindowEffects::SlideFromLocation location, int offset)
{
    Q_UNUSED(window)
    Q_UNUSED(location)
    Q_UNUSED(offset)
}

void KWindowEffectsPrivateDummy::enableBlurBehind(QWindow *window, bool enable, const QRegion &region)
{
    Q_UNUSED(window)
    Q_UNUSED(enable)
    Q_UNUSED(region)
}

void KWindowEffectsPrivateDummy::enableBackgroundContrast(QWindow *window,
                                                          bool enable,
                                                          qreal contrast,
                                                          qreal intensity,
                                                          qreal saturation,
                                                          const QRegion &region)
{
    Q_UNUSED(window)
    Q_UNUSED(enable)
    Q_UNUSED(contrast)
    Q_UNUSED(intensity)
    Q_UNUSED(saturation)
    Q_UNUSED(region)
}
