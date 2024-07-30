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

#ifndef KCONFIGCOMMONSTRUCTS_H
#define KCONFIGCOMMONSTRUCTS_H

#include <QList>
#include <QString>

#include "KConfigParameters.h"

struct Param {
    QString name;
    QString type;
};

struct Signal {
    QString name;
    QString label;
    QList<Param> arguments;
    bool modify = false;
};

class CfgEntry
{
public:
    struct Choice {
        QString name;
        QString context;
        QString label;
        QString toolTip;
        QString whatsThis;
        QString val;

        QString value() const
        {
            return !val.isEmpty() ? val : name;
        }
    };
    class Choices
    {
    public:
        Choices()
        {
        }
        Choices(const QList<Choice> &d, const QString &n, const QString &p)
            : prefix(p)
            , choices(d)
            , mName(n)
        {
            int i = n.indexOf(QLatin1String("::"));
            if (i >= 0) {
                mExternalQual = n.left(i + 2);
            }
        }
        QString prefix;
        QList<Choice> choices;
        const QString &name() const
        {
            return mName;
        }
        const QString &externalQualifier() const
        {
            return mExternalQual;
        }
        bool external() const
        {
            return !mExternalQual.isEmpty();
        }

    private:
        QString mName;
        QString mExternalQual;
    };

public:
    QString group;
    QString parentGroup;
    QString type;
    QString key;
    QString name;
    QString labelContext;
    QString label;
    QString toolTipContext;
    QString toolTip;
    QString whatsThisContext;
    QString whatsThis;
    QString code;
    QString defaultValue;
    QString param;
    QString paramName;
    QString paramType;
    Choices choices;
    QList<Signal> signalList;
    QStringList paramValues;
    QStringList paramDefaultValues;
    int paramMax;
    bool hidden;
    QString min;
    QString max;
};

struct ParseResult {
    QString cfgFileName;
    bool cfgFileNameArg = false;
    bool cfgStateConfig = false;
    QList<Param> parameters;
    QList<Signal> signalList;
    QStringList includes;
    QList<CfgEntry *> entries;
    bool hasNonModifySignals = false;
};

// TODO: Move those methods
QString enumName(const QString &n);
QString enumName(const QString &n, const CfgEntry::Choices &c);
QString param(const QString &t);
QString cppType(const QString &t);
QString itemType(const QString &type);
QString changeSignalName(const QString &n);

QString enumType(const CfgEntry *e, bool globalEnums);

QString getDefaultFunction(const QString &n, const QString &className = QString());
QString setFunction(const QString &n, const QString &className = QString());
QString getFunction(const QString &n, const QString &className = QString());
QString immutableFunction(const QString &n, const QString &className = QString());

QString itemAccessorBody(const CfgEntry *e, const KConfigParameters &cfg);
QString signalEnumName(const QString &signalName);

bool isUnsigned(const QString &type);

// returns the name of an member variable
// use itemPath to know the full path
// like using d-> in case of dpointer
QString varName(const QString &n, const KConfigParameters &cfg);

QString varPath(const QString &n, const KConfigParameters &cfg);

// returns the name of an item variable
// use itemPath to know the full path
// like using d-> in case of dpointer
QString itemVar(const CfgEntry *e, const KConfigParameters &cfg);

// returns the name of the local inner item if there is one
// (before wrapping with KConfigCompilerSignallingItem)
// Otherwise return itemVar()
QString innerItemVar(const CfgEntry *e, const KConfigParameters &cfg);

QString itemPath(const CfgEntry *e, const KConfigParameters &cfg);

QString filenameOnly(const QString &path);

QString itemDeclaration(const CfgEntry *e, const KConfigParameters &cfg);

QString translatedString(const KConfigParameters &cfg,
                         const QString &string,
                         const QString &context = QString(),
                         const QString &param = QString(),
                         const QString &paramValue = QString());

// TODO: Sanitize those functions.
QString newItem(const CfgEntry *entry, const QString &key, const QString &defaultValue, const KConfigParameters &cfg, const QString &param = QString());

QString newInnerItem(const CfgEntry *entry, const QString &key, const QString &defaultValue, const KConfigParameters &cfg, const QString &param = QString());

QString userTextsFunctions(const CfgEntry *e, const KConfigParameters &cfg, QString itemVarStr = QString(), const QString &i = QString());

QString paramString(const QString &s, const CfgEntry *e, int i);
QString paramString(const QString &group, const QList<Param> &parameters);

QString defaultValue(const QString &t);
QString memberGetDefaultBody(const CfgEntry *e);
QString literalString(const QString &s);
QString enumTypeQualifier(const QString &n, const CfgEntry::Choices &c);

void addQuotes(QString &s);
void addDebugMethod(QTextStream &out, const KConfigParameters &cfg, const QString &n);

#endif
