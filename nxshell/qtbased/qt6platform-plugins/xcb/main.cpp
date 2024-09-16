// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <qpa/qplatformintegrationplugin.h>
#include "dplatformintegration.h"

#include <QDebug>

DPP_USE_NAMESPACE

QT_BEGIN_NAMESPACE

class DPlatformIntegrationPlugin : public QPlatformIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QPlatformIntegrationFactoryInterface_iid FILE "dpp.json")

public:
    QPlatformIntegration *create(const QString&, const QStringList&, int &, char **) Q_DECL_OVERRIDE;
};

QPlatformIntegration* DPlatformIntegrationPlugin::create(const QString& system, const QStringList& parameters, int &argc, char **argv)
{
#ifdef Q_OS_LINUX
    bool loadDXcb = false;

    if (qEnvironmentVariableIsSet("D_DXCB_DISABLE")) {
        loadDXcb = false;
    } else if (system == "dxcb") {
        loadDXcb = true;
    } else if (QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin") ||
               (qgetenv("XDG_CURRENT_DESKTOP") == QByteArrayLiteral("DDE"))) {
        loadDXcb = true;
    }

    return loadDXcb ? new DPlatformIntegration(parameters, argc, argv)
                    : new DPlatformIntegrationParent(parameters, argc, argv);
#endif

    return nullptr;
}

QT_END_NAMESPACE

#include "main.moc"
