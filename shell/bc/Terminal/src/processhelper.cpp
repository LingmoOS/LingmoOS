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

#include "processhelper.h"
#include <QDesktopServices>
#include <QDBusInterface>
#include <QApplication>
#include <QProcess>
#include <QUrl>

#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusPendingReply>

ProcessHelper *SELF = nullptr;

ProcessHelper *ProcessHelper::self()
{
    if (!SELF)
        SELF = new ProcessHelper;

    return SELF;
}

ProcessHelper::ProcessHelper(QObject *parent)
    : QObject(parent)
{

}

bool ProcessHelper::startDetached(const QString &program, const QStringList &arguments)
{
    return QProcess::startDetached(program, arguments);
}

bool ProcessHelper::openUrl(const QString &url)
{
    QUrl _url = QUrl::fromUserInput(url);

    if (url.isNull())
        return false;

    return QDesktopServices::openUrl(_url);
}

bool ProcessHelper::openFileManager(const QString &url)
{
    QDBusInterface iface(QStringLiteral("org.freedesktop.FileManager1"),
                         QStringLiteral("/org/freedesktop/FileManager1"),
                         QStringLiteral("org.freedesktop.FileManager1"));

    if (iface.lastError().isValid())
        return false;

    iface.call("ShowFolders",
               QStringList() << QUrl::fromLocalFile(url).toString(),
               QString::number(QApplication::applicationPid()));

    return true;
}
