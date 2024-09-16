// SPDX-FileCopyrightText: 2023 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <qpa/qplatformintegrationplugin.h>
#include "qminimalintegration.h"

class MinimalIntegrationPlugin : public QPlatformIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QPlatformIntegrationFactoryInterface_iid FILE "minimal.json")
public:
    QPlatformIntegration *create(const QString&, const QStringList&) override;
};

QPlatformIntegration *MinimalIntegrationPlugin::create(const QString& system, const QStringList& paramList)
{
    if (!system.compare("minimal", Qt::CaseInsensitive))
        return new MinimalIntegration(paramList);

    return nullptr;
}

#include "main.moc"
