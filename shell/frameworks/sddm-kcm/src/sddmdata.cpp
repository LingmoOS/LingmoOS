/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
    SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "sddmdata.h"

#include "config.h"
#include "sddmsettings.h"

#include <KSharedConfig>

#include <QDir>

SddmData::SddmData(QObject *parent)
    : KCModuleData(parent)
{
    auto config = KSharedConfig::openConfig(QStringLiteral(SDDM_CONFIG_FILE), KConfig::CascadeConfig);
    QStringList configFiles = QDir(QStringLiteral(SDDM_CONFIG_DIR)).entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::LocaleAware);
    std::transform(configFiles.begin(), configFiles.end(), configFiles.begin(), [](const QString &filename) -> QString {
        return QStringLiteral(SDDM_CONFIG_DIR "/") + filename;
    });
    config->addConfigSources(configFiles);
    m_settings = new SddmSettings(config, this);
    autoRegisterSkeletons();
}

SddmSettings *SddmData::sddmSettings() const
{
    return m_settings;
}
