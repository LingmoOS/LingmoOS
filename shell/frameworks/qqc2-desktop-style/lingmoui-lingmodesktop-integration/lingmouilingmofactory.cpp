/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "lingmouilingmofactory.h"

#include "lingmodesktoptheme.h"
#include "lingmodesktopunits.h"

LingmoUILingmoFactory::LingmoUILingmoFactory(QObject *parent)
    : LingmoUI::Platform::PlatformPluginFactory(parent)
{
}

LingmoUILingmoFactory::~LingmoUILingmoFactory() = default;

LingmoUI::Platform::PlatformTheme *LingmoUILingmoFactory::createPlatformTheme(QObject *parent)
{
    Q_ASSERT(parent);
    return new LingmoDesktopTheme(parent);
}

LingmoUI::Platform::Units *LingmoUILingmoFactory::createUnits(QObject *parent)
{
    Q_ASSERT(parent);
    return new LingmoDesktopUnits(parent);
}

#include "moc_lingmouilingmofactory.cpp"
