/*
    SPDX-FileCopyrightText: 2010-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_SCRIPTHELPERS_H
#define KATE_SCRIPTHELPERS_H

#include <QJSValue>
#include <QObject>
#include <ktexteditor_export.h>

class QJSEngine;

namespace Kate
{
/** Top-level script functions */
namespace Script
{
/** read complete file contents, helper */
KTEXTEDITOR_EXPORT bool readFile(const QString &sourceUrl, QString &sourceCode);

} // namespace Script

class KTEXTEDITOR_EXPORT ScriptHelper : public QObject
{
    Q_OBJECT
    QJSEngine *m_engine;

public:
    explicit ScriptHelper(QJSEngine *engine)
        : m_engine(engine)
    {
    }
    Q_INVOKABLE QString read(const QString &file);
    Q_INVOKABLE void require(const QString &file);
    Q_INVOKABLE void debug(const QString &msg);
    Q_INVOKABLE QString _i18n(const QString &msg);
    Q_INVOKABLE QString _i18nc(const QString &textContext, const QString &text);
    Q_INVOKABLE QString _i18np(const QString &trSingular, const QString &trPlural, int number);
    Q_INVOKABLE QString _i18ncp(const QString &trContext, const QString &trSingular, const QString &trPlural, int number = 0);
};

} // namespace Kate

#endif
