// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <QObject>

QT_BEGIN_NAMESPACE
class QCommandLineParser;
class QCoreApplication;
class QCommandLineOption;
QT_END_NAMESPACE

class CommandParser : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CommandParser)

public:
    static CommandParser &instance();

    QStringList processCommand(const QString &name);
    void process();
    void process(const QStringList &arguments);

private:
    void initialize();
    void initOptions();
    void addOption(const QCommandLineOption &option);

private:
    explicit CommandParser(QObject *parent = nullptr);
    ~CommandParser();

private:
    QCommandLineParser *cmdParser;
};

#endif   // COMMANDPARSER_H
