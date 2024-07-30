/*
    SPDX-FileCopyrightText: 2022 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <KConfigSkeleton>
#include <KConfigWatcher>

#pragma once

class GlobalConfigSkeleton : public KConfigSkeleton
{
public:
    explicit GlobalConfigSkeleton(const QString &configName = QStringLiteral("plasmaparc"), QObject *parent = nullptr)
        : KConfigSkeleton(configName, parent)
    {
        setupWatcher();
    };
    explicit GlobalConfigSkeleton(KSharedConfig::Ptr config, QObject *parent = nullptr)
        : KConfigSkeleton(config, parent)
    {
        setupWatcher();
    };
    ~GlobalConfigSkeleton() override{};

private:
    KConfigWatcher::Ptr m_configWatcher;
    void setupWatcher()
    {
        m_configWatcher = KConfigWatcher::create(sharedConfig());
        connect(m_configWatcher.data(), &KConfigWatcher::configChanged, this, [this]() {
            load();
        });
    }
};
