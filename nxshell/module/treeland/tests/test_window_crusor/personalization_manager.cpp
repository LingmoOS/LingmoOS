// Copyright (C) 2024 rewine <luhongxu@lingmo.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "personalization_manager.h"
#include <QtGui/QGuiApplication>
#include <QtGui/QWindow>
#include <QtGui/QPlatformSurfaceEvent>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QDebug>

QT_BEGIN_NAMESPACE

PersonalizationManager::PersonalizationManager()
    : QWaylandClientExtensionTemplate<PersonalizationManager>(1)
{

}

PersonalizationWindow::PersonalizationWindow(struct ::personalization_window_context_v1 *object)
    : QWaylandClientExtensionTemplate<PersonalizationWindow>(1)
    , QtWayland::personalization_window_context_v1(object)
{

}

PersonalizationWallpaper::PersonalizationWallpaper(struct ::personalization_wallpaper_context_v1 *object)
    : QWaylandClientExtensionTemplate<PersonalizationWallpaper>(1)
    , QtWayland::personalization_wallpaper_context_v1(object)
{

}

void PersonalizationWallpaper::personalization_wallpaper_context_v1_metadata(const QString &metadata)
{
    qDebug() << "=========================================== " << metadata;
}

PersonalizationCursor::PersonalizationCursor(struct ::personalization_cursor_context_v1 *object)
    : QWaylandClientExtensionTemplate<PersonalizationCursor>(1)
    , QtWayland::personalization_cursor_context_v1(object)
{
}

void PersonalizationCursor::personalization_cursor_context_v1_verfity(int32_t success)
{
    qDebug() << "=========================================== " << success;
}

void PersonalizationCursor::personalization_cursor_context_v1_theme(const QString &name)
{
    qDebug() << "=========================================== " << name;
}

void PersonalizationCursor::personalization_cursor_context_v1_size(uint32_t size)
{
    qDebug() << "=========================================== " << size;
}

QT_END_NAMESPACE
