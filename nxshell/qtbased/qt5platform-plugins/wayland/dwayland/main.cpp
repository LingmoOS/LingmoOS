// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "global.h"

#include "dwaylandintegration.h"

#include <qpa/qplatformintegrationplugin.h>

DPP_USE_NAMESPACE

QT_BEGIN_NAMESPACE

class DWaylandIntegrationPlugin : public QPlatformIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QPlatformIntegrationFactoryInterface_iid FILE "dwayland.json")
public:
    QPlatformIntegration *create(const QString&, const QStringList&) override;
};

QPlatformIntegration* DWaylandIntegrationPlugin::create(const QString& system, const QStringList& paramList)
{
#ifdef Q_OS_LINUX
    Q_UNUSED(system);
    Q_UNUSED(paramList);
    auto *integration = DWaylandIntegration::instance();

    if (integration->hasFailed()) {
        delete integration;
        integration = nullptr;
    }

    return integration;
#endif

    return nullptr;
}

QT_END_NAMESPACE

#include "main.moc"
