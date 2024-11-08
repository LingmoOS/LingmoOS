/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "filelauncher.h"

#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QSettings>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QDebug>

FileLauncher *SELF = nullptr;

FileLauncher *FileLauncher::self()
{
    if (SELF == nullptr)
        SELF = new FileLauncher;

    return SELF;
}

FileLauncher::FileLauncher(QObject *parent)
    : QObject(parent)
{

}

bool FileLauncher::launchApp(const QString &desktopFile, const QString &fileName)
{
    QSettings settings(desktopFile, QSettings::IniFormat);
    settings.beginGroup("Desktop Entry");

    QStringList list = settings.value("Exec").toString().split(' ');
    QStringList args;

    if (list.isEmpty() || list.size() < 0)
        return false;

    QString exec = list.first();
    list.removeOne(exec);

    for (const QString &arg : list) {
        QString newArg = arg;

        if (newArg.startsWith("%F", Qt::CaseInsensitive))
            newArg.replace("%F", fileName, Qt::CaseInsensitive);

        if (newArg.startsWith("%U", Qt::CaseInsensitive))
            newArg.replace("%U", fileName, Qt::CaseInsensitive);

        args.append(newArg);
    }

    qDebug() << "launchApp()" << exec << args;

    return startDetached(exec, args);
}

bool FileLauncher::launchExecutable(const QString &fileName)
{
    return startDetached(fileName);
}

bool FileLauncher::startDetached(const QString &exec, QStringList args)
{
    QDBusInterface iface("com.lingmo.Session",
                         "/Session",
                         "com.lingmo.Session", QDBusConnection::sessionBus());

    if (iface.isValid()) {
        iface.asyncCall("launch", exec, args).waitForFinished();
    } else {
        QProcess::startDetached(exec, args);
    }

    return true;
}

bool FileLauncher::startDetached(const QString &exec, const QString &workingDir, QStringList args)
{
    QDBusInterface iface("com.lingmo.Session",
                         "/Session",
                         "com.lingmo.Session", QDBusConnection::sessionBus());

    if (iface.isValid()) {
        iface.asyncCall("launch", exec, workingDir, args).waitForFinished();
    }

    return true;
}
