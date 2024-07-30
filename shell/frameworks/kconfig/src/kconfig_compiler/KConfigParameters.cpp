/*
    This file is part of the KDE libraries.

    SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2003 Zack Rusin <zack@kde.org>
    SPDX-FileCopyrightText: 2006 Michaël Larouche <michael.larouche@kdemail.net>
    SPDX-FileCopyrightText: 2008 Allen Winter <winter@kde.org>
    SPDX-FileCopyrightText: 2020 Tomaz Cananbrava <tcanabrava@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KConfigParameters.h"

#include <QDebug>
#include <QFileInfo>

#include <iostream>

KConfigParameters::KConfigParameters(const QString &codegenFilename)
{
    if (!codegenFilename.endsWith(QLatin1String(".kcfgc"))) {
        std::cerr << "Codegen options file must have extension .kcfgc" << std::endl;
        exit(1);
    }

    baseName = QFileInfo(codegenFilename).fileName();
    baseName = baseName.left(baseName.length() - 6);

    // Configure the compiler with some settings
    QSettings codegenConfig(codegenFilename, QSettings::IniFormat);

    nameSpace = codegenConfig.value(QStringLiteral("NameSpace")).toString();
    className = codegenConfig.value(QStringLiteral("ClassName")).toString();
    if (className.isEmpty()) {
        std::cerr << "Class name missing" << std::endl;
        exit(1);
    }
    inherits = codegenConfig.value(QStringLiteral("Inherits")).toString();
    if (inherits.isEmpty()) {
        inherits = QStringLiteral("KConfigSkeleton");
    }
    visibility = codegenConfig.value(QStringLiteral("Visibility")).toString();
    if (!visibility.isEmpty()) {
        visibility += QLatin1Char(' ');
    }
    parentInConstructor = codegenConfig.value(QStringLiteral("ParentInConstructor"), false).toBool();
    forceStringFilename = codegenConfig.value(QStringLiteral("ForceStringFilename"), false).toBool();
    singleton = codegenConfig.value(QStringLiteral("Singleton"), false).toBool();
    staticAccessors = singleton;
    customAddons = codegenConfig.value(QStringLiteral("CustomAdditions"), false).toBool();
    memberVariables = codegenConfig.value(QStringLiteral("MemberVariables")).toString();
    dpointer = (memberVariables == QLatin1String("dpointer"));
    headerIncludes = codegenConfig.value(QStringLiteral("IncludeFiles"), QStringList()).toStringList();
    sourceIncludes = codegenConfig.value(QStringLiteral("SourceIncludeFiles"), QStringList()).toStringList();
    mutators = codegenConfig.value(QStringLiteral("Mutators"), QStringList()).toStringList();
    allMutators = ((mutators.count() == 1) && (mutators.at(0).toLower() == QLatin1String("true")));
    itemAccessors = codegenConfig.value(QStringLiteral("ItemAccessors"), false).toBool();
    setUserTexts = codegenConfig.value(QStringLiteral("SetUserTexts"), false).toBool();
    defaultGetters = codegenConfig.value(QStringLiteral("DefaultValueGetters"), QStringList()).toStringList();
    allDefaultGetters = (defaultGetters.count() == 1) && (defaultGetters.at(0).toLower() == QLatin1String("true"));
    notifiers = codegenConfig.value(QStringLiteral("Notifiers"), QStringList()).toStringList();
    allNotifiers = ((notifiers.count() == 1) && (notifiers.at(0).toLower() == QLatin1String("true")));
    globalEnums = codegenConfig.value(QStringLiteral("GlobalEnums"), false).toBool();
    useEnumTypes = codegenConfig.value(QStringLiteral("UseEnumTypes"), false).toBool();
    const QString trString = codegenConfig.value(QStringLiteral("TranslationSystem")).toString().toLower();
    generateProperties = codegenConfig.value(QStringLiteral("GenerateProperties"), false).toBool();
    if (trString == QLatin1String("kde")) {
        translationSystem = KdeTranslation;
        translationDomain = codegenConfig.value(QStringLiteral("TranslationDomain")).toString();
    } else {
        if (!trString.isEmpty() && trString != QLatin1String("qt")) {
            std::cerr << "Unknown translation system, falling back to Qt tr()" << std::endl;
        }
        translationSystem = QtTranslation;
    }
    qCategoryLoggingName = codegenConfig.value(QStringLiteral("CategoryLoggingName"), QString()).toString();
    headerExtension = codegenConfig.value(QStringLiteral("HeaderExtension"), QStringLiteral("h")).toString();
    qmlRegistration = codegenConfig.value(QStringLiteral("QmlRegistration")).toBool();
}
