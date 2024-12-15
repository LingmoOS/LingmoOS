// Copyright (C) 2024 Wenhao Peng <pengwenhao@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwayland-treeland-personalization-manager-v1.h"

#include <QScreen>
#include <QtWaylandClient/QWaylandClientExtension>

class PersonalizationWallpaper;
class PersonalizationManager : public QWaylandClientExtensionTemplate<PersonalizationManager>,
                               public QtWayland::treeland_personalization_manager_v1
{
    Q_OBJECT
public:
    explicit PersonalizationManager();
    ~PersonalizationManager();
    void onActiveChanged();

    PersonalizationWallpaper *wallpaper() { return m_wallpaperContext; }

private:
    PersonalizationWallpaper *m_wallpaperContext = nullptr;
};

class PersonalizationWindow : public QWaylandClientExtensionTemplate<PersonalizationWindow>,
                              public QtWayland::personalization_window_context_v1
{
    Q_OBJECT
public:
    explicit PersonalizationWindow(struct ::personalization_window_context_v1 *object);
};

class PersonalizationWallpaper : public QWaylandClientExtensionTemplate<PersonalizationWallpaper>,
                                 public QtWayland::personalization_wallpaper_context_v1
{
    Q_OBJECT
public:
    explicit PersonalizationWallpaper(struct ::personalization_wallpaper_context_v1 *object);

signals:
    void metadataChanged(const QString &meta);

protected:
    void personalization_wallpaper_context_v1_metadata(const QString &metadata) override;
};
