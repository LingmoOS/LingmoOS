// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMANDLINEMANAGER_H
#define COMMANDLINEMANAGER_H

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QScopedPointer>

class CommandLineManager
{
public:
    static CommandLineManager *instance();

    ~CommandLineManager();

    void process(const QStringList &arguments);

    bool isSet(const QString &name) const;
    QString value(const QString &name) const;

    QStringList positionalArguments();
    QStringList unknownOptionNames();

protected:
    explicit CommandLineManager();

private:
    void initOptions();

private:
    CommandLineManager(CommandLineManager &) = delete;
    CommandLineManager &operator=(CommandLineManager &) = delete;

private:
    QScopedPointer<QCommandLineParser> m_commandParser;
};

#endif // COMMANDLINEMANAGER_H
