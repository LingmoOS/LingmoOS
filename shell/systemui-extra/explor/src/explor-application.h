/*
 * Peony-Qt
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef PEONYAPPLICATION_H
#define PEONYAPPLICATION_H

/**
 * @brief A single application use client/server desgin.
 * @details single application with c/s conmunication mode is a common desgin used for many file manager, like explor, caja, and nautilus.
 * <pre>
 * There are several benefits to this type application:
 *  * Reduce wastes of resources
 *  * Make application(s) easy to manage
 *  * Reduce IPC
 * </pre>
 *
 */

#include "singleapplication.h"
#include <QCommandLineParser>
#include <QCommandLineOption>

class PeonyApplication : public SingleApplication
{
    Q_OBJECT
public:
    explicit PeonyApplication(int &argc, char *argv[], const char *applicationName = "explor-qt");
    static void about();
    static void help();

    static bool userGuideDaemonRunning();
    static void showGuide(const QString &appName = "system-file-manager");

    static qint64 explor_start_time;

public Q_SLOTS:
    //unmount all ftp links
    void unmountAllFtpLinks();

private:
    QString getUriMessage (QStringList&);

protected Q_SLOTS:
    void parseCmd(quint32 id, QByteArray msg);

private:
    QCommandLineParser parser;
    QCommandLineOption quitOption = QCommandLineOption(QStringList()<<"q"<<"quit", tr("Close all explor-qt windows and quit"));
    QCommandLineOption showItemsOption = QCommandLineOption(QStringList()<<"i"<<"show-items", tr("Show items"));
    QCommandLineOption showFoldersOption = QCommandLineOption(QStringList()<<"f"<<"show-folders", tr("Show folders"));
    QCommandLineOption showPropertiesOption = QCommandLineOption(QStringList()<<"p"<<"show-properties", tr("Show properties"));

    bool m_first_parse = true;
};

#endif // PEONYAPPLICATION_H
