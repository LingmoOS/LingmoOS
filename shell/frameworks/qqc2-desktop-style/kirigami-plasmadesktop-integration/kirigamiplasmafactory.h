/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIRIGAMIPLASMAFACTORY_H
#define KIRIGAMIPLASMAFACTORY_H

#include <Kirigami/Platform/PlatformPluginFactory>

#include <QObject>

class KirigamiPlasmaFactory : public Kirigami::Platform::PlatformPluginFactory
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID PlatformPluginFactory_iid FILE "kirigamiplasmaintegration.json")

    Q_INTERFACES(Kirigami::Platform::PlatformPluginFactory)

public:
    explicit KirigamiPlasmaFactory(QObject *parent = nullptr);
    ~KirigamiPlasmaFactory() override;

    Kirigami::Platform::PlatformTheme *createPlatformTheme(QObject *parent) override;
    Kirigami::Platform::Units *createUnits(QObject *parent) override;
};

#endif // KIRIGAMIPLASMAFACTORY_H
