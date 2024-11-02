/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#include "lingmo-search-dir-manager-dbus.h"
#include "dir-watcher.h"
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>

LingmoUISearchDirManagerDbus::LingmoUISearchDirManagerDbus(int &argc, char *argv[], const QString &applicationName)
    : QtSingleApplication (applicationName, argc, argv)
{
    setQuitOnLastWindowClosed(false);
    setApplicationVersion(QString("v%1").arg(VERSION));

    if (!this->isRunning()) {
        qDebug() << "I'm in dir manager dbus rigister";
        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        if(!sessionBus.registerService("com.lingmo.search.fileindex.service")) {
            qCritical() << "QDbus register service failed reason:" << sessionBus.lastError();
        }
        if(!sessionBus.registerObject("/org/lingmo/search/privateDirWatcher", DirWatcher::getDirWatcher(), QDBusConnection::ExportNonScriptableSlots | QDBusConnection::ExportAllSignals)) {
            qCritical() << "lingmo-search-fileindex dbus register object failed reason:" << sessionBus.lastError();
        }

        connect(this, &QtSingleApplication::messageReceived, [=](QString msg) {
            this->parseCmd(msg, true);
        });
    } else {
        qWarning() << "LingmoUI search dir manager is running!";
    }
    //parse cmd
    qDebug()<<"parse cmd";
    auto message = this->arguments().join(' ').toUtf8();
    parseCmd(message, !isRunning());
}

void LingmoUISearchDirManagerDbus::parseCmd(QString msg, bool isPrimary)
{
    QCommandLineParser parser;

    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption quitOption(QStringList()<<"q"<<"quit", tr("Quit DBus service"));
    parser.addOption(quitOption);

     if (isPrimary) {
         const QStringList args = QString(msg).split(' ');
         parser.process(args);
         if (parser.isSet(quitOption)) {
             qApp->quit();
             return;
         }
     } else {
         if (arguments().count() < 2) {
             parser.showHelp();
         }
         parser.process(arguments());
         sendMessage(msg);
     }

}
