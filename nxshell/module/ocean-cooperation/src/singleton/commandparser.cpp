// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commandparser.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <QVariant>

CommandParser &CommandParser::instance()
{
    static CommandParser ins;
    return ins;
}

QStringList CommandParser::processCommand(const QString &name)
{
    QStringList args;
    if (cmdParser->isSet(name)) {
        args = cmdParser->positionalArguments();
    }
    return args;
}

void CommandParser::process()
{
    return process(qApp->arguments());
}

void CommandParser::process(const QStringList &arguments)
{
    qDebug() << "App start args: " << arguments;
    cmdParser->process(arguments);
}

void CommandParser::initialize()
{
    cmdParser->setApplicationDescription(QString("%1 helper").arg(QCoreApplication::applicationName()));
    initOptions();
    cmdParser->addHelpOption();
    cmdParser->addVersionOption();
}

void CommandParser::initOptions()
{
    QCommandLineOption sendFiles(QStringList() << "s"
                                               << "send-files",
                                 "send files");
    QCommandLineOption detail(QStringList() << "d"
                                            << "detail",
                              "Enable detail log");
    QCommandLineOption minimize(QStringList() << "m"
                                              << "minimize",
                                "Launch with minimize UI");
    QCommandLineOption forward(QStringList() << "f"
                                            << "forward",
                              "Forward files to target with IP and name");

    addOption(sendFiles);
    addOption(detail);
    addOption(minimize);
    addOption(forward);
}

void CommandParser::addOption(const QCommandLineOption &option)
{
    cmdParser->addOption(option);
}

CommandParser::CommandParser(QObject *parent)
    : QObject(parent),
      cmdParser(new QCommandLineParser)
{
    initialize();
}

CommandParser::~CommandParser()
{
}
