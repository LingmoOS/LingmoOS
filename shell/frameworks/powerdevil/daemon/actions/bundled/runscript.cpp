/*
 *   SPDX-FileCopyrightText: 2010 Dario Freddi <drf@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "runscript.h"

#include <PowerDevilProfileSettings.h>
#include <powerdevil_debug.h>

#include <KPluginFactory>
#include <QProcess>

K_PLUGIN_CLASS_WITH_JSON(PowerDevil::BundledActions::RunScript, "powerdevilrunscriptaction.json")

namespace PowerDevil::BundledActions
{
RunScript::RunScript(QObject *parent)
    : Action(parent)
{
    // TODO: Which policy should we enforce here? Let's go for the less restrictive one
    setRequiredPolicies(PowerDevil::PolicyAgent::ChangeScreenSettings);
}

RunScript::~RunScript()
{
}

void RunScript::onProfileUnload()
{
    if (!m_profileUnloadCommand.isEmpty()) {
        runCommand(m_profileUnloadCommand);
    }
}

void RunScript::onIdleTimeout(std::chrono::milliseconds timeout)
{
    Q_UNUSED(timeout);
    runCommand(m_idleTimeoutCommand);
}

void RunScript::onProfileLoad(const QString & /*previousProfile*/, const QString & /*newProfile*/)
{
    if (!m_profileLoadCommand.isEmpty()) {
        runCommand(m_profileLoadCommand);
    }
}

void RunScript::triggerImpl(const QVariantMap &args)
{
    Q_UNUSED(args);
}

bool RunScript::loadAction(const PowerDevil::ProfileSettings &profileSettings)
{
    m_profileLoadCommand = profileSettings.profileLoadCommand();
    m_profileUnloadCommand = profileSettings.profileUnloadCommand();
    m_idleTimeoutCommand = profileSettings.idleTimeoutCommand();

    if (m_profileLoadCommand.isEmpty() && m_profileUnloadCommand.isEmpty() && m_idleTimeoutCommand.isEmpty()) {
        return false;
    }

    if (!m_idleTimeoutCommand.isEmpty()) {
        registerIdleTimeout(std::chrono::seconds(profileSettings.runScriptIdleTimeoutSec()));
    }
    return true;
}

void RunScript::runCommand(const QString &command)
{
    bool success;

    QStringList args = QProcess::splitCommand(command);
    if (args.isEmpty()) {
        qCWarning(POWERDEVIL) << "Empty command?";
        return;
    }

    QProcess process;
    process.setProgram(args.takeFirst());
    process.setArguments(args);
    success = process.startDetached();

    if (!success) {
        qCWarning(POWERDEVIL) << "Failed to run" << command;
    }
}
}

#include "runscript.moc"

#include "moc_runscript.cpp"
