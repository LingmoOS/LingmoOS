/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LINGMOUILINGMOFACTORY_H
#define LINGMOUILINGMOFACTORY_H

#include <LingmoUI/Platform/PlatformPluginFactory>
#include <QObject>

class LingmoUILingmoFactory : public LingmoUI::Platform::PlatformPluginFactory
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID PlatformPluginFactory_iid FILE "lingmouilingmointegration.json")

    Q_INTERFACES(LingmoUI::Platform::PlatformPluginFactory)

public:
    explicit LingmoUILingmoFactory(QObject *parent = nullptr);
    ~LingmoUILingmoFactory() override;

    LingmoUI::Platform::PlatformTheme *createPlatformTheme(QObject *parent) override;
    LingmoUI::Platform::Units *createUnits(QObject *parent) override;
};

#endif // LINGMOUILINGMOFACTORY_H
