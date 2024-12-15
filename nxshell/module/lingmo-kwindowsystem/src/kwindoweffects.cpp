/*
 * Copyright 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "kwindoweffects_p.h"
#include "pluginwrapper_p.h"
#include <QGuiApplication>

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

void enableBlurBehind(WId window, bool enable, const QRegion &region)
{
    KWindowSystemPluginWrapper::self().effects()->enableBlurBehind(window, enable, region);
}

void enableBackgroundContrast(WId window, bool enable, qreal contrast, qreal intensity, qreal saturation, const QRegion &region)
{
    KWindowSystemPluginWrapper::self().effects()->enableBackgroundContrast(window, enable, contrast, intensity, saturation, region);
}

void highlightWindows(WId controller, const QList< WId > &ids)
{
    KWindowSystemPluginWrapper::self().effects()->highlightWindows(controller, ids);
}

void markAsDashboard(WId window)
{
    KWindowSystemPluginWrapper::self().effects()->markAsDashboard(window);
}

void presentWindows(WId controller, const QList< WId > &ids)
{
    KWindowSystemPluginWrapper::self().effects()->presentWindows(controller, ids);
}

void presentWindows(WId controller, int desktop)
{
    KWindowSystemPluginWrapper::self().effects()->presentWindows(controller, desktop);
}

void slideWindow(WId id, SlideFromLocation location, int offset)
{
    KWindowSystemPluginWrapper::self().effects()->slideWindow(id, location, offset);
}

void slideWindow(QWidget *widget, SlideFromLocation location)
{
    KWindowSystemPluginWrapper::self().effects()->slideWindow(widget, location);
}

QList< QSize > windowSizes(const QList< WId > &ids)
{
    return KWindowSystemPluginWrapper::self().effects()->windowSizes(ids);
}

}
