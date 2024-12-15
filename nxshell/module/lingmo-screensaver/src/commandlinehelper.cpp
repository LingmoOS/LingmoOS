// SPDX-FileCopyrightText: 2017 ~ 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commandlinehelper.h"

#include <QCommandLineParser>
#include <QCommandLineOption>

CommandLineHelper::CommandLineHelper()
    : m_commandParser(new QCommandLineParser)
{
    initOptions();
}

CommandLineHelper *CommandLineHelper::instance()
{
    static CommandLineHelper instance;
    return &instance;
}

void CommandLineHelper::initOptions()
{
    QCommandLineOption optionDbus({ "d", "dbus" }, "Register DBus service.");
    QCommandLineOption optionStart({ "s", "start" }, "Start screen saver.");
    QCommandLineOption optionConfig({ "c", "config" }, "Start config dialog of screen saver.", "screensaer-name", "");

    m_commandParser->addOption(optionDbus);
    m_commandParser->addOption(optionStart);
    m_commandParser->addOption(optionConfig);
    m_commandParser->addPositionalArgument("screensaer-name", "Use the screensaver application.", "[name]");
    m_commandParser->addHelpOption();
    m_commandParser->addVersionOption();
}

bool CommandLineHelper::isSet(const QStringList &namelst) const
{
    if (namelst.isEmpty())
        return false;

    for (const QString &name : namelst) {
        if (m_commandParser->isSet(name))
            return true;
    }

    return false;
}

bool CommandLineHelper::isSet(const QCommandLineOption &option) const
{
    return m_commandParser->isSet(option);
}

const QString CommandLineHelper::getOptionValue(const QString &name)
{
    return m_commandParser->value(name);
}

const QString CommandLineHelper::getOptionValue(const QCommandLineOption &option)
{
    return m_commandParser->value(option);
}

QStringList CommandLineHelper::positionalArguments()
{
    return m_commandParser->positionalArguments();
}

void CommandLineHelper::process(const QStringList &arguments)
{
    m_commandParser->process(arguments);
}

void CommandLineHelper::addOption(const QCommandLineOption &option)
{
    m_commandParser->addOption(option);
}

void CommandLineHelper::addOptions(const QList<QCommandLineOption> &options)
{
    for (const QCommandLineOption &option : options)
        addOption(option);
}
