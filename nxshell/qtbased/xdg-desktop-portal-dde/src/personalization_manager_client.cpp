// Copyright (C) 2024 Wenhao Peng <pengwenhao@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "personalization_manager_client.h"

#include <QtWaylandClient/QWaylandClientExtension>

PersonalizationManager::PersonalizationManager()
    : QWaylandClientExtensionTemplate<PersonalizationManager>(1)
{
    connect(this,
            &PersonalizationManager::activeChanged,
            this,
            &PersonalizationManager::onActiveChanged);
}

PersonalizationManager::~PersonalizationManager()
{

}

void PersonalizationManager::onActiveChanged()
{
    if (!isActive())
        return;

    if (!m_wallpaperContext) {
        m_wallpaperContext = new PersonalizationWallpaper(get_wallpaper_context());
    }
}

PersonalizationWallpaper::PersonalizationWallpaper(
        struct ::personalization_wallpaper_context_v1 *object)
    : QWaylandClientExtensionTemplate<PersonalizationWallpaper>(1)
    , QtWayland::personalization_wallpaper_context_v1(object)
{
}

void PersonalizationWallpaper::personalization_wallpaper_context_v1_metadata(
        const QString &metadata)
{
    Q_EMIT metadataChanged(metadata);
}
