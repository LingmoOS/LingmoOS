/*
SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>
SPDX-FileCopyrightText: 2017 David Edmundson <davidedmundson@kde.org>

SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "lnf_integration.h"

#include <KConfig>
#include <KConfigGroup>
#include <KConfigLoader>

#include <QFile>

namespace ScreenLocker
{
LnFIntegration::LnFIntegration(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<KConfigPropertyMap *>();
}

LnFIntegration::~LnFIntegration() = default;

void LnFIntegration::init()
{
    if (!m_package.isValid()) {
        return;
    }
    if (auto config = configScheme()) {
        m_configuration = new KConfigPropertyMap(config, this);
    }
}

KConfigLoader *LnFIntegration::configScheme()
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
