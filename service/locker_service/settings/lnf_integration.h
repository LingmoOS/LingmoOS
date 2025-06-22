/*
SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>
SPDX-FileCopyrightText: 2017 David Edmundson <davidedmundson@kde.org>

SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#pragma once

#include <KConfigPropertyMap>
#include <KPackage/Package>
#include <KSharedConfig>

class KConfigLoader;

namespace ScreenLocker
{
class LnFIntegration : public QObject
{
    Q_OBJECT

    Q_PROPERTY(KConfigPropertyMap *configuration READ configuration NOTIFY configurationChanged)

public:
    explicit LnFIntegration(QObject *parent);
    ~LnFIntegration() override;

    void init();

    void setConfig(const KSharedConfig::Ptr &config)
    {
        m_config = config;
    }

    void setPackage(const KPackage::Package &package)
    {
        m_package = package;
    }

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

private:
    KPackage::Package m_package;
    KSharedConfig::Ptr m_config;
    KConfigLoader *m_configLoader = nullptr;
    KConfigPropertyMap *m_configuration = nullptr;
};

}
