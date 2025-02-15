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

#ifndef KCONFIGHEADERGENERATOR_H
#define KCONFIGHEADERGENERATOR_H

#include "KConfigCodeGeneratorBase.h"
#include "KConfigCommonStructs.h"

#include <QList>
#include <QString>

class KConfigParameters;
class CfgEntry;
class QTextStream;
struct ParseResult;

class KConfigHeaderGenerator : public KConfigCodeGeneratorBase
{
public:
    KConfigHeaderGenerator(const QString &inputFile, const QString &baseDir, const KConfigParameters &parameters, ParseResult &parseResult);

    void start() override;

private:
    void startHeaderGuards();
    void endHeaderGuards();

    void implementEnums();
    void implementChoiceEnums(const CfgEntry *entry, const CfgEntry::Choices &choices);
    void implementValueEnums(const CfgEntry *entry, const QStringList &values);

    void doClassDefinition();
    void createHeaders();
    void createDPointer();
    void createNonDPointerHelpers();

    void createConstructor();
    void createDestructor();
    void createForwardDeclarations();
    void createSingleton();
    void createSignals();

    void createSetters(const CfgEntry *entry);
    void createItemAcessors(const CfgEntry *entry, const QString &returnType);
    void createGetters(const CfgEntry *entry, const QString &returnType);
    void createImmutableGetters(const CfgEntry *entry);
    void createProperties(const CfgEntry *entry, const QString &returnType);
    void createImmutableProperty(const CfgEntry *entry);
    void createDefaultValueMember(const CfgEntry *entry);
};

#endif
