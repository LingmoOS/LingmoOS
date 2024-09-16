// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CMDMANAGER_H
#define CMDMANAGER_H

#include <QObject>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QApplication>

class CMDManager : public QObject
{
    Q_OBJECT
public:
    static CMDManager* instance();
    void process(const QApplication &app);
    void init();
    bool isSet(const QString& name) const;
    QString getPath();
    QStringList positionalArguments() const;
    void showHelp(int exitCode = 0);
    int getWinId();

signals:

public slots:

private:
    explicit CMDManager(QObject *parent = 0);
    QCommandLineParser m_parser;
    QCommandLineOption m_modelModeOpt;
};

#endif // CMDMANAGER_H
