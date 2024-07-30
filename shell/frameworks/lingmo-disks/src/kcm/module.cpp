// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include <KQuickConfigModule>

#include "devicemodel.h"
#include "servicerunner.h"
#include "version.h"
#include <KPluginFactory>

class Module : public KQuickConfigModule
{
    Q_OBJECT
public:
    explicit Module(QObject *parent, const KPluginMetaData &data)
        : KQuickConfigModule(parent, data)
    {
        // We have no help so remove the button from the buttons.
        setButtons(buttons() ^ Help ^ Default ^ Apply);

        qmlRegisterType<DeviceModel>("SMART", 1, 0, "DeviceModel");
        qmlRegisterType<ServiceRunner>("SMART", 1, 0, "ServiceRunner");
    }
};

K_PLUGIN_CLASS_WITH_JSON(Module, "kcm_disks.json")

#include "module.moc"
