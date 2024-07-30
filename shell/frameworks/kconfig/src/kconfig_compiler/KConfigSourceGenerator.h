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

#ifndef KCONFIGSOURCEGENERATOR_H
#define KCONFIGSOURCEGENERATOR_H

#include "KConfigCodeGeneratorBase.h"
#include "KConfigCommonStructs.h"

#include <QList>
#include <QString>

class KConfigParameters;
class CfgEntry;
class QTextStream;
struct ParseResult;

class KConfigSourceGenerator : public KConfigCodeGeneratorBase
{
public:
    KConfigSourceGenerator(const QString &inputFile, const QString &baseDir, const KConfigParameters &parameters, ParseResult &parseResult);

    void start() override;

private:
    // Those are fairly self contained functions.
    void createHeaders();
    void createPrivateDPointerImplementation();
    void createSingletonImplementation();
    void createPreamble();
    void createDestructor();
    void createConstructorParameterList();
    void createParentConstructorCall();
    void createInitializerList();
    void createDefaultValueGetterSetter();
    void createNonModifyingSignalsHelper();
    void createSignalFlagsHandler();

    // Constructor related methods
    // the `do` methods have related helper functions that are only related
    // to it. So we can break the function into many smaller ones and create
    // logic inside of the `do` function.
    void doConstructor();
    void createEnums(const CfgEntry *entry);
    void createNormalEntry(const CfgEntry *entry, const QString &key);
    void createIndexedEntry(const CfgEntry *entry, const QString &key);
    void handleCurrentGroupChange(const CfgEntry *entry);

    void doGetterSetterDPointerMode();
    void createGetterDPointerMode(const CfgEntry *entry);
    void createImmutableGetterDPointerMode(const CfgEntry *entry);
    void createSetterDPointerMode(const CfgEntry *entry);
    void createItemGetterDPointerMode(const CfgEntry *entry);

private:
    QString mCurrentGroup;
    QStringList mConfigGroupList; // keeps track of generated KConfigGroup;
};

#endif
