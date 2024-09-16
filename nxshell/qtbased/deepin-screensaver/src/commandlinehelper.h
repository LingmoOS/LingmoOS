// SPDX-FileCopyrightText: 2017 ~ 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMANDLINEHELPER_H
#define COMMANDLINEHELPER_H

#include <QObject>

QT_BEGIN_NAMESPACE
class QCommandLineParser;
class QCommandLineOption;
QT_END_NAMESPACE

class CommandLineHelper
{
public:
    static CommandLineHelper *instance();
    void initOptions();
    bool isSet(const QStringList &namelst) const;
    bool isSet(const QCommandLineOption &option) const;
    const QString getOptionValue(const QString &name);
    const QString getOptionValue(const QCommandLineOption &option);
    QStringList positionalArguments();
    void process(const QStringList &arguments);

    void addOption(const QCommandLineOption &option);
    void addOptions(const QList<QCommandLineOption> &options);

private:
    explicit CommandLineHelper();
    CommandLineHelper(CommandLineHelper &) = delete;
    CommandLineHelper &operator=(CommandLineHelper &) = delete;

private:
    QCommandLineParser *m_commandParser { nullptr };
};

#endif   // COMMANDLINEHELPER_H
