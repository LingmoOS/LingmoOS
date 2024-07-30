/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2003 Zack Rusin <zack@kde.org>
    SPDX-FileCopyrightText: 2006 Michaël Larouche <michael.larouche@kdemail.net>
    SPDX-FileCopyrightText: 2008 Allen Winter <winter@kde.org>
    SPDX-FileCopyrightText: 2020 Tomaz Cananbrava <tcanabrava@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGXTPARAMETERS_H
#define KCONFIGXTPARAMETERS_H

#include <QSettings>
#include <QString>
#include <QStringList>

/**
   Configuration Compiler Configuration
*/
class KConfigParameters
{
public:
    KConfigParameters(const QString &codegenFilename);

public:
    enum TranslationSystem {
        QtTranslation,
        KdeTranslation,
    };

    // These are read from the .kcfgc configuration file
    QString nameSpace; // The namespace for the class to be generated
    QString className; // The class name to be generated
    QString inherits; // The class the generated class inherits (if empty, from KConfigSkeleton)
    QString visibility;
    bool parentInConstructor; // The class has the optional parent parameter in its constructor
    bool forceStringFilename;
    bool singleton; // The class will be a singleton
    bool staticAccessors; // provide or not static accessors
    bool customAddons;
    QString memberVariables;
    QStringList headerIncludes;
    QStringList sourceIncludes;
    QStringList mutators;
    QStringList defaultGetters;
    QStringList notifiers;
    QString qCategoryLoggingName;
    QString headerExtension;
    bool allMutators;
    bool setUserTexts;
    bool allDefaultGetters;
    bool dpointer;
    bool globalEnums;
    bool useEnumTypes;
    bool itemAccessors;
    bool allNotifiers;
    TranslationSystem translationSystem;
    QString translationDomain;
    bool generateProperties;
    QString baseName;
    bool qmlRegistration;
};

#endif
