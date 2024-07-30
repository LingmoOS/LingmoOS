/*
SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "wallpaper_integration.h"

#include <KConfig>
#include <KConfigGroup>
#include <KConfigLoader>
#include <KPackage/PackageLoader>

#include <QFile>

namespace ScreenLocker
{
WallpaperIntegration::WallpaperIntegration(QQuickItem *parent)
    : QQuickItem(parent)
    , m_package(KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Wallpaper")))
{
    qRegisterMetaType<KConfigPropertyMap *>();
}

WallpaperIntegration::~WallpaperIntegration() = default;

void WallpaperIntegration::init()
{
    if (!m_package.isValid()) {
        return;
    }
    if (auto config = configScheme()) {
        m_configuration = new KConfigPropertyMap(config, this);
        // potd (picture of the day) is using a kded to monitor changes and
        // cache data for the lockscreen. Let's notify it.
        m_configuration->setNotify(true);
    }
}

void WallpaperIntegration::setPluginName(const QString &name)
{
    if (m_pluginName == name) {
        return;
    }
    m_pluginName = name;
    m_package.setPath(name);
    Q_EMIT packageChanged();
}

KConfigLoader *WallpaperIntegration::configScheme()
{
    if (!m_configLoader) {
        const QString xmlPath = m_package.filePath(QByteArrayLiteral("config"), QStringLiteral("main.xml"));

        const KConfigGroup cfg = m_config->group("Greeter").group("Wallpaper").group(m_pluginName);

        if (xmlPath.isEmpty()) {
            m_configLoader = new KConfigLoader(cfg, nullptr, this);
        } else {
            QFile file(xmlPath);
            m_configLoader = new KConfigLoader(cfg, &file, this);
        }
    }
    return m_configLoader;
}

QColor WallpaperIntegration::accentColor() const
{
    return QColor();
}

void WallpaperIntegration::setAccentColor(const QColor &)
{
}
}

#include "moc_wallpaper_integration.cpp"
