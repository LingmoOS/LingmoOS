/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2003 Zack Rusin <zack@kde.org>
    SPDX-FileCopyrightText: 2006 Michaël Larouche <michael.larouche@kdemail.net>
    SPDX-FileCopyrightText: 2008 Allen Winter <winter@kde.org>
    SPDX-FileCopyrightText: 2020 Tomaz Cananbrava <tcanabrava@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGXMLPARSER_H
#define KCONFIGXMLPARSER_H

#include <QDomDocument>
#include <QRegularExpression>
#include <QString>

#include "KConfigCommonStructs.h"
#include "KConfigParameters.h"

/* This parses the contents of a Xml file into a ParseResult Structure,
 * It also fails hard:
 * If start() succeeds, you can use the result,
 * if start() fails, the program aborts with an error message so there's
 * no possibility of generating incorrect code information.
 */
class KConfigXmlParser
{
public:
    KConfigXmlParser(const KConfigParameters &cfg, const QString &inputFileName);

    // Start the parser and reads the contents of the inputFileName into the ParseResult Structure
    void start();

    // Get the result of the parse
    ParseResult getParseResult() const;

private:
    // creates a `somethingChanged` signal for every property
    void createChangedSignal(CfgEntry &readEntry);

    void validateNameAndKey(CfgEntry &readEntry, const QDomElement &element);

    // TODO: Use std::optional and CfgEntry (without heap allocation) for this function
    // *or* fail hard if the parse fails.
    CfgEntry *parseEntry(const QString &group, const QString &parentGroup, const QDomElement &element);

    // Steps
    void readIncludeTag(const QDomElement &element);
    void readGroupTag(const QDomElement &element);
    void readKcfgfileTag(const QDomElement &element);
    void readSignalTag(const QDomElement &element);

    // Those are the Entries in the Xml, that represent a parameter within the <group> </group> tag.
    void readParameterFromEntry(CfgEntry &entry, const QDomElement &element);
    bool hasDefaultCode(CfgEntry &entry, const QDomElement &element);
    void readChoicesFromEntry(CfgEntry &entry, const QDomElement &element);
    void readGroupElements(CfgEntry &entry, const QDomElement &element);
    void readParamDefaultValues(CfgEntry &entry, const QDomElement &element);

private:
    ParseResult mParseResult;
    KConfigParameters cfg;
    QString mInputFileName;
    QStringList mAllNames;
    QRegularExpression mValidNameRegexp;
};

#endif
