#include "wallpaper.h"

Wallpaper::Wallpaper(QObject *parent)
    : QObject(parent)
    , m_interface("com.lingmo.Settings",
                  "/Theme", "com.lingmo.Theme",
                  QDBusConnection::sessionBus(), this)
{
    if (m_interface.isValid()) {
        connect(&m_interface, SIGNAL(wallpaperChanged(QString)), this, SLOT(onPathChanged(QString)));
        connect(&m_interface, SIGNAL(darkModeDimsWallpaerChanged()), this, SIGNAL(dimsWallpaperChanged()));
        connect(&m_interface, SIGNAL(backgroundTypeChanged()), this, SIGNAL(typeChanged()));
        connect(&m_interface, SIGNAL(backgroundColorChanged()), this, SIGNAL(colorChanged()));
    }
}

int Wallpaper::type() const
{
    return m_interface.property("backgroundType").toInt();
}

QString Wallpaper::path() const
{
    return m_interface.property("wallpaper").toString();
}

bool Wallpaper::dimsWallpaper() const
{
    return m_interface.property("darkModeDimsWallpaer").toBool();
}

QString Wallpaper::color() const
{
    return m_interface.property("backgroundColor").toString();
}

void Wallpaper::onPathChanged(QString path)
{
    Q_UNUSED(path);

    emit pathChanged();
}
