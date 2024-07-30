/*
    SPDX-FileCopyrightText: 2010-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// Undefine this because we don't want our i18n*() method names to be turned
// into i18nd*(). This means any translated string in this file should use
// i18nd("ktexteditor6", "foo") instead of i18n("foo")
#undef TRANSLATION_DOMAIN

#include "katescripthelpers.h"
#include "katepartdebug.h"

#include <iostream>

#include <QFile>
#include <QJSEngine>
#include <QStandardPaths>

#include <QDebug>

#include <KLocalizedString>

namespace Kate
{
namespace Script
{
bool readFile(const QString &sourceUrl, QString &sourceCode)
{
    sourceCode = QString();

    QFile file(sourceUrl);
    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(LOG_KTE) << QStringLiteral("Unable to find '%1'").arg(sourceUrl);
        return false;
    } else {
        QTextStream stream(&file);
        sourceCode = stream.readAll();
        file.close();
    }
    return true;
}

} // namespace Script

QString ScriptHelper::read(const QString &name)
{
    // get full name of file
    // skip on errors
    QString content;
    QString fullName = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("katepart5/script/files/") + name);
    if (fullName.isEmpty()) {
        // retry with resource
        fullName = QLatin1String(":/ktexteditor/script/files/") + name;
        if (!QFile::exists(fullName)) {
            return content;
        }
    }

    // try to read complete file
    // skip non-existing files
    Script::readFile(fullName, content);
    return content;
}

void ScriptHelper::require(const QString &name)
{
    // get full name of file
    // skip on errors
    QString fullName = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("katepart5/script/libraries/") + name);
    if (fullName.isEmpty()) {
        // retry with resource
        fullName = QLatin1String(":/ktexteditor/script/libraries/") + name;
        if (!QFile::exists(fullName)) {
            return;
        }
    }

    // check include guard
    QJSValue require_guard = m_engine->globalObject().property(QStringLiteral("require_guard"));
    if (require_guard.property(fullName).toBool()) {
        return;
    }

    // try to read complete file
    // skip non-existing files
    QString code;
    if (!Script::readFile(fullName, code)) {
        return;
    }

    // eval in current script engine
    const QJSValue val = m_engine->evaluate(code, fullName);
    if (val.isError()) {
        qCWarning(LOG_KTE) << "error evaluating" << fullName << val.toString() << ", at line" << val.property(QStringLiteral("lineNumber")).toInt();
    }

    // set include guard
    require_guard.setProperty(fullName, QJSValue(true));
}

void ScriptHelper::debug(const QString &message)
{
    // debug in blue to distance from other debug output if necessary
    std::cerr << "\033[31m" << qPrintable(message) << "\033[0m\n";
}

// BEGIN code adapted from kdelibs/kross/modules/translation.cpp
/// helper function to do the substitution from QtScript > QVariant > real values
// KLocalizedString substituteArguments(const KLocalizedString &kls, const QVariantList &arguments, int max = 99)
//{
//    KLocalizedString ls = kls;
//    int cnt = qMin(arguments.count(), max);   // QString supports max 99
//    for (int i = 0; i < cnt; ++i) {
//        QVariant arg = arguments[i];
//        switch (arg.type()) {
//        case QVariant::Int: ls = ls.subs(arg.toInt()); break;
//        case QVariant::UInt: ls = ls.subs(arg.toUInt()); break;
//        case QVariant::LongLong: ls = ls.subs(arg.toLongLong()); break;
//        case QVariant::ULongLong: ls = ls.subs(arg.toULongLong()); break;
//        case QVariant::Double: ls = ls.subs(arg.toDouble()); break;
//        default: ls = ls.subs(arg.toString()); break;
//        }
//    }
//    return ls;
//}

/// i18n("text", arguments [optional])
QString ScriptHelper::_i18n(const QString &text)
{
    KLocalizedString ls = ki18n(text.toUtf8().constData());
    return ls.toString(); // substituteArguments(ls, args).toString();
}

/// i18nc("context", "text", arguments [optional])
QString ScriptHelper::_i18nc(const QString &textContext, const QString &text)
{
    KLocalizedString ls = ki18nc(textContext.toUtf8().constData(), text.toUtf8().constData());
    return ls.toString(); // substituteArguments(ls, args).toString();
}

/// i18np("singular", "plural", number, arguments [optional])
QString ScriptHelper::_i18np(const QString &trSingular, const QString &trPlural, int number)
{
    KLocalizedString ls = ki18np(trSingular.toUtf8().constData(), trPlural.toUtf8().constData()).subs(number);
    return ls.toString(); // substituteArguments(ls, args, 98).toString();
}

/// i18ncp("context", "singular", "plural", number, arguments [optional])
QString ScriptHelper::_i18ncp(const QString &trContext, const QString &trSingular, const QString &trPlural, int number)
{
    KLocalizedString ls = ki18ncp(trContext.toUtf8().data(), trSingular.toUtf8().data(), trPlural.toUtf8().data()).subs(number);
    return ls.toString(); // substituteArguments(ls, args, 98).toString();
}

// END code adapted from kdelibs/kross/modules/translation.cpp

} // namespace kate

#include "moc_katescripthelpers.cpp"
