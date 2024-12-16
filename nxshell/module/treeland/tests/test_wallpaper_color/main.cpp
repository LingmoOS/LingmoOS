// Copyright (C) 2024 rewine <luhongxu@lingmo.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwayland-treeland-wallpaper-color-v1.h"

#include <private/qwaylandwindow_p.h>

#include <qwindow.h>

#include <QGuiApplication>
#include <QScreen>
#include <QtWaylandClient/QWaylandClientExtension>

class WallpaperColorManager : public QWaylandClientExtensionTemplate<WallpaperColorManager>,
                              public QtWayland::wallpaper_color_manager_v1
{
    Q_OBJECT
public:
    explicit WallpaperColorManager();

    void wallpaper_color_manager_v1_output_color(const QString &output, uint32_t isdark)
    {
        qInfo() << "Output(" << output << ") wallpaper is dark type: " << isdark;
    }
};

WallpaperColorManager::WallpaperColorManager()
    : QWaylandClientExtensionTemplate<WallpaperColorManager>(1)
{
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "wayland");
    QGuiApplication app(argc, argv);
    WallpaperColorManager manager;

    QObject::connect(&manager, &WallpaperColorManager::activeChanged, &manager, [&manager] {
        qDebug() << "personalzation manager init: " << manager.isActive();
        for (auto *screen : QGuiApplication::screens()) {
            qDebug() << "watch: " << screen->name();
            manager.watch(screen->name());
        }
    });

    return app.exec();
}

#include "main.moc"
