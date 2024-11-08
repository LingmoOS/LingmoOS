/*
 * SPDX-FileCopyrightText: 2021 Reion Wong <reion@cuteos.com>
 * SPDX-FileCopyrightText: 2020 George Florea Bănuș <georgefb899@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QAbstractItemModel>
#include <QQmlApplicationEngine>
#include <QApplication>
#include <QObject>
#include <QAction>

class Application : public QObject
{
    Q_OBJECT

public:
    Application(int &argc, char **argv, const QString &applicationName);

    int run();

    Q_INVOKABLE QString mimeType(const QString &file);
    Q_INVOKABLE static QString formatTime(const double time);
    Q_INVOKABLE QString argument(int key);
    Q_INVOKABLE void addArgument(int key, const QString &value);

    Q_INVOKABLE static void hideCursor();
    Q_INVOKABLE static void showCursor();

private:
    void setupCommandLineParser();
    void setupQmlContextProperties();
    void registerQmlTypes();

private:
    QApplication *m_app;
    QQmlApplicationEngine *m_engine;
    QMap<int, QString> m_args;
};

#endif // APPLICATION_H
