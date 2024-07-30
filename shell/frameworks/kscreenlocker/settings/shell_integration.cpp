/*
SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>
SPDX-FileCopyrightText: 2017 David Edmundson <davidedmundson@kde.org>

SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "shell_integration.h"

#include <KConfig>
#include <KConfigGroup>
#include <KConfigLoader>

#include <QFile>

namespace ScreenLocker
{
ShellIntegration::ShellIntegration(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<KConfigPropertyMap *>();
}

ShellIntegration::~ShellIntegration() = default;

void ShellIntegration::init()
{
    if (!m_package.isValid()) {
        return;
    }
    if (auto config = configScheme()) {
        m_configuration = new KConfigPropertyMap(config, this);
    }
}

QString ShellIntegration::defaultShell() const
{
    KSharedConfig::Ptr startupConf = KSharedConfig::openConfig(QStringLiteral("plasmashellrc"));
    KConfigGroup startupConfGroup(startupConf, QStringLiteral("Shell"));
    const QString defaultValue = qEnvironmentVariable("PLASMA_DEFAULT_SHELL", "org.kde.plasma.desktop");
    QString value = startupConfGroup.readEntry("ShellPackage", defaultValue);

    // In the global theme an empty value was written, make sure we still return a shell package
    return value.isEmpty() ? defaultValue : value;
}

KConfigLoader *ShellIntegration::configScheme()
{
    if (!m_configLoader) {
        const QString xmlPath = m_package.filePath(QByteArrayLiteral("lockscreen"), QStringLiteral("config.xml"));

        const KConfigGroup cfg = m_config->group("Greeter").group("LnF");

        if (xmlPath.isEmpty()) {
            m_configLoader = new KConfigLoader(cfg, nullptr, this);
        } else {
            QFile file(xmlPath);
            m_configLoader = new KConfigLoader(cfg, &file, this);
        }
    }
    return m_configLoader;
}

}

#include "moc_shell_integration.cpp"
