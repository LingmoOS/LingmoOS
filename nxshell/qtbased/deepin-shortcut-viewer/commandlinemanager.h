// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMANDLINEMANAGER_H
#define COMMANDLINEMANAGER_H
#include <QCommandLineParser>
#include <QApplication>
#include <QPoint>

class CommandLineManager
{
public:
    CommandLineManager();
    void process(const QCoreApplication &app);
    void process(const QStringList &list);
    QPoint pos();
    QString jsonData();

    bool enableBypassWindowManagerHint() const;

private:
    QCommandLineParser m_commandLineParser;
    QCommandLineOption m_posOption;
    QCommandLineOption m_jsonDataOption;
};

#endif   // COMMANDLINEMANAGER_H
