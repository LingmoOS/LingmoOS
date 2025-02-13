/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "layouttemplate.h"

#include <KLocalizedString>

void LayoutTemplatePackage::initPackage(KPackage::Package *package)
{
    package->setDefaultPackageRoot(QStringLiteral("plasma/layout-templates/"));
    package->addFileDefinition("mainscript", QStringLiteral("layout.js"), i18n("Main Script File"));
    package->setRequired("mainscript", true);
}

K_PLUGIN_CLASS_WITH_JSON(LayoutTemplatePackage, "lingmo-packagestructure-layouttemplate.json")

#include "layouttemplate.moc"
