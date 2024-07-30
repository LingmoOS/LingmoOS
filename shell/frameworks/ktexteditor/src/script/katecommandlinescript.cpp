/*
    SPDX-FileCopyrightText: 2009-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katecommandlinescript.h"

#include <QJSEngine>
#include <QJSValue>

#include <KLocalizedString>
#include <KShell>

#include "katecmd.h"
#include "katedocument.h"
#include "katepartdebug.h"
#include "kateview.h"

KateCommandLineScript::KateCommandLineScript(const QString &url, const KateCommandLineScriptHeader &header)
    : KateScript(url)
    , KTextEditor::Command(header.functions())
    , m_commandHeader(header)
{
}

const KateCommandLineScriptHeader &KateCommandLineScript::commandHeader()
{
    return m_commandHeader;
}

bool KateCommandLineScript::callFunction(const QString &cmd, const QStringList &args, QString &errorMessage)
{
    clearExceptions();
    QJSValue command = function(cmd);
    if (!command.isCallable()) {
        errorMessage = i18n("Function '%1' not found in script: %2", cmd, url());
        return false;
    }

    // add the arguments that we are going to pass to the function
    QJSValueList arguments;
    arguments.reserve(args.size());
    for (const QString &arg : args) {
        arguments << QJSValue(arg);
    }

    QJSValue result = command.call(arguments);
    // error during the calling?
    if (result.isError()) {
        errorMessage = backtrace(result, i18n("Error calling %1", cmd));
        return false;
    }

    return true;
}

bool KateCommandLineScript::exec(KTextEditor::View *view, const QString &cmd, QString &msg, const KTextEditor::Range &range)
{
    if (range.isValid()) {
        view->setSelection(range);
    }

    KShell::Errors errorCode;
    QStringList args(KShell::splitArgs(cmd, KShell::NoOptions, &errorCode));

    if (errorCode != KShell::NoError) {
        msg = i18n("Bad quoting in call: %1. Please escape single quotes with a backslash.", cmd);
        return false;
    }

    QString _cmd(args.first());
    args.removeFirst();

    if (!view) {
        msg = i18n("Could not access view");
        return false;
    }

    if (setView(qobject_cast<KTextEditor::ViewPrivate *>(view))) {
        // setView fails if the script cannot be loaded
        // balance edit stack in any case!
        qobject_cast<KTextEditor::ViewPrivate *>(view)->doc()->pushEditState();
        bool success = callFunction(_cmd, args, msg);
        qobject_cast<KTextEditor::ViewPrivate *>(view)->doc()->popEditState();
        return success;
    }

    return false;
}

bool KateCommandLineScript::supportsRange(const QString &)
{
    return true;
}

bool KateCommandLineScript::help(KTextEditor::View *view, const QString &cmd, QString &msg)
{
    if (!setView(qobject_cast<KTextEditor::ViewPrivate *>(view))) {
        // setView fails, if the script cannot be loaded
        return false;
    }

    clearExceptions();
    QJSValue helpFunction = function(QStringLiteral("help"));
    if (!helpFunction.isCallable()) {
        return false;
    }

    // add the arguments that we are going to pass to the function
    QJSValueList arguments;
    arguments << QJSValue(cmd);

    QJSValue result = helpFunction.call(arguments);

    // error during the calling?
    if (result.isError()) {
        msg = backtrace(result, i18n("Error calling 'help %1'", cmd));
        return false;
    }

    if (result.isUndefined() || !result.isString()) {
        qCDebug(LOG_KTE) << i18n("No help specified for command '%1' in script %2", cmd, url());
        return false;
    }
    msg = result.toString();

    return !msg.isEmpty();
}
