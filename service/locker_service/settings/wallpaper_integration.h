/*
SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#pragma once

#include <KConfigPropertyMap>
#include <KPackage/Package>
#include <KSharedConfig>

class KConfigLoader;

namespace ScreenLocker
{
class WallpaperIntegration : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString pluginName READ pluginName NOTIFY packageChanged)
    Q_PROPERTY(KConfigPropertyMap *configuration READ configuration NOTIFY configurationChanged)

public:
    explicit WallpaperIntegration(QObject *parent);
    ~WallpaperIntegration() override;

    void init();

    void setConfig(const KSharedConfig::Ptr &config)
    {
        m_config = config;
    }
    QString pluginName() const
    {
        return m_pluginName;
    }
    void setPluginName(const QString &name);

    KPackage::Package package() const
    {
        return m_package;
    }

    KConfigPropertyMap *configuration() const
    {
        return m_configuration;
    }

    KConfigLoader *configScheme();

Q_SIGNALS:
    void packageChanged();
    void configurationChanged();

    /**
     * This is to keep compatible with WallpaperInterface in plasma-framework.
     * It doesn't have any practical use.
     */
    void repaintNeeded();

private:
    QString m_pluginName;
    KPackage::Package m_package;
    KSharedConfig::Ptr m_config;
    KConfigLoader *m_configLoader = nullptr;
    KConfigPropertyMap *m_configuration = nullptr;
};

}
