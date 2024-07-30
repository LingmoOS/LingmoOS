/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kwindoweffects_p.h"
#include "pluginwrapper_p.h"
#include <QWindow>

KWindowEffectsPrivate::KWindowEffectsPrivate()
{
}

KWindowEffectsPrivate::~KWindowEffectsPrivate()
{
}

namespace KWindowEffects
{
bool isEffectAvailable(Effect effect)
{
    return KWindowSystemPluginWrapper::self().effects()->isEffectAvailable(effect);
}

void enableBlurBehind(QWindow *window, bool enable, const QRegion &region)
{
    KWindowSystemPluginWrapper::self().effects()->enableBlurBehind(window, enable, region);
}

void enableBackgroundContrast(QWindow *window, bool enable, qreal contrast, qreal intensity, qreal saturation, const QRegion &region)
{
    KWindowSystemPluginWrapper::self().effects()->enableBackgroundContrast(window, enable, contrast, intensity, saturation, region);
}

void slideWindow(QWindow *window, SlideFromLocation location, int offset)
{
    KWindowSystemPluginWrapper::self().effects()->slideWindow(window, location, offset);
}
}
