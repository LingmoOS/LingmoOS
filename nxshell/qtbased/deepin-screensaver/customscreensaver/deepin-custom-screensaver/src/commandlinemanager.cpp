// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commandlinemanager.h"

#include <QCommandLineOption>

class CommandLineManagerGlobal : public CommandLineManager {};
Q_GLOBAL_STATIC(CommandLineManagerGlobal, commandLineManagerGlobal);

CommandLineManager::CommandLineManager()
    : m_commandParser(new QCommandLineParser)
{
    // 必须设置为长选项模式，否则解析时 -window-id 将识别错误
    m_commandParser->setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

    initOptions();
}

CommandLineManager *CommandLineManager::instance()
{
    return commandLineManagerGlobal;
}

CommandLineManager::~CommandLineManager()
{

}

void CommandLineManager::initOptions()
{
    QCommandLineOption subWindowOption("window-id", "displays as a child of the given ID window.\nNOTE:is '-window-id',not '--window-id'", "ID");
    QCommandLineOption pathOption(QStringList() << "d" << "directory", "set the path to find pictures.", "PATH");
    QCommandLineOption intervalTimeOption(QStringList() << "t" << "interval-time", "set the number of seconds slideshow pictures."
                                                                                   "\n 0:1 minutes\n 1:3 minutes\n 2:5 minutes\n 3:10 minutes"
                                                                                   "\n 4:30 minutes\n 5:60 minutes", "TIME");
    QCommandLineOption intervalModeOption(QStringList() << "m" << "interval-mode", "set whether to paly randomly.\n  true or false", "MODE");
    QCommandLineOption config(QStringList() << "config", "show config user Interface");

    m_commandParser->addOption(subWindowOption);
    m_commandParser->addOption(pathOption);
    m_commandParser->addOption(intervalTimeOption);
    m_commandParser->addOption(intervalModeOption);
    m_commandParser->addOption(config);
}

void CommandLineManager::process(const QStringList &arguments)
{
    m_commandParser->process(arguments);
}

bool CommandLineManager::isSet(const QString &name) const
{
    return m_commandParser->isSet(name);
}

QString CommandLineManager::value(const QString &name) const
{
    return m_commandParser->value(name);
}

QStringList CommandLineManager::positionalArguments()
{
    return m_commandParser->positionalArguments();
}

QStringList CommandLineManager::unknownOptionNames()
{
    return m_commandParser->unknownOptionNames();
}

