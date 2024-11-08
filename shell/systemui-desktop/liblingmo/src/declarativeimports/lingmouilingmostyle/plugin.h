/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLUGIN_H
#define PLUGIN_H

#include <LingmoUI/Platform/PlatformPluginFactory>
#include <QObject>

class Plugin : public LingmoUI::Platform::PlatformPluginFactory
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID PlatformPluginFactory_iid FILE "plugin.json")

    Q_INTERFACES(LingmoUI::Platform::PlatformPluginFactory)

public:
    explicit Plugin(QObject *parent = nullptr);
    ~Plugin() override;

    LingmoUI::Platform::PlatformTheme *createPlatformTheme(QObject *parent) override;
    LingmoUI::Platform::Units *createUnits(QObject *parent) override;
};

#endif // PLUGIN_H
