/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     rekols <revenmartin@gmail.com>
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

#include "utils.h"
#include "systemappmonitor.h"
#include "systemappitem.h"

#include <QFile>
#include <QFileInfo>
#include <QUrlQuery>
#include <QSettings>
#include <QDebug>

#include <KWindowSystem>

static Utils *INSTANCE = nullptr;

Utils *Utils::instance()
{
    if (!INSTANCE)
        INSTANCE = new Utils;

    return INSTANCE;
}

Utils::Utils(QObject *parent)
    : QObject(parent)
    , m_sysAppMonitor(SystemAppMonitor::self())
{

}

QStringList Utils::commandFromPid(quint32 pid)
{
    QFile file(QString("/proc/%1/cmdline").arg(pid));

    if (file.open(QIODevice::ReadOnly)) {
        QByteArray cmd = file.readAll();

        // ref: https://github.com/KDE/kcoreaddons/blob/230c98aa7e01f9e36a9c2776f3633182e6778002/src/lib/util/kprocesslist_unix.cpp#L137
        if (!cmd.isEmpty()) {
            // extract non-truncated name from cmdline
            int zeroIndex = cmd.indexOf('\0');
            int processNameStart = cmd.lastIndexOf('/', zeroIndex);
            if (processNameStart == -1) {
                processNameStart = 0;
            } else {
                processNameStart++;
            }

            QString name = QString::fromLocal8Bit(cmd.mid(processNameStart, zeroIndex - processNameStart));

            // reion: Remove parameters
            name = name.split(' ').first();

            cmd.replace('\0', ' ');
            QString command = QString::fromLocal8Bit(cmd).trimmed();

            // There may be parameters.
            if (command.split(' ').size() > 1) {
                command = command.split(' ').first();
            }

            return { command, name };
        }
    }

    return QStringList();
}

QString Utils::desktopPathFromMetadata(const QString &appId, quint32 pid, const QString &xWindowWMClassName)
{
    QStringList commands = commandFromPid(pid);

    // The value returned from the commandFromPid() may be empty.
    // Calling first() and last() below will cause the statusbar to crash.
    if (commands.isEmpty() || xWindowWMClassName.isEmpty())
        return "";

    QString command = commands.first();
    QString commandName = commands.last();

    if (command.isEmpty())
        return "";

    QString result;

    if (!appId.isEmpty() && !xWindowWMClassName.isEmpty()) {
        for (SystemAppItem *item : m_sysAppMonitor->applications()) {
            // Start search.
            const QFileInfo desktopFileInfo(item->path);

            bool isExecPath = QFile::exists(item->exec);
            bool founded = false;

            if (item->exec == command || item->exec == commandName) {
                founded = true;
            }

            // StartupWMClass=STRING
            // If true, it is KNOWN that the application will map at least one
            // window with the given string as its WM class or WM name hint.
            // ref: https://specifications.freedesktop.org/startup-notification-spec/startup-notification-0.1.txt
            if (item->startupWMClass.startsWith(appId, Qt::CaseInsensitive) ||
                item->startupWMClass.startsWith(xWindowWMClassName, Qt::CaseInsensitive))
                founded = true;

            if (!founded && item->iconName.startsWith(xWindowWMClassName, Qt::CaseInsensitive))
                founded = true;

            // Icon name and cmdline.
            if (!founded && (item->iconName == command || item->iconName == commandName))
                founded = true;

            // Exec name and cmdline.
            if (!founded && (item->exec == command || item->exec == commandName))
                founded = true;

            // Try matching mapped name against 'Name'.
            if (!founded && item->name.startsWith(xWindowWMClassName, Qt::CaseInsensitive))
                founded = true;

            // exec
            if (!founded && item->exec.startsWith(xWindowWMClassName, Qt::CaseInsensitive))
                founded = true;

            if (!founded && desktopFileInfo.baseName().startsWith(xWindowWMClassName, Qt::CaseInsensitive))
                founded = true;

            // For exec path.
            if (isExecPath && !founded && (command.contains(item->exec) || commandName.contains(item->exec))) {
                founded = true;
            }

            if (founded) {
                result = item->path;
                break;
            }
        }
    }

    return result;
}

QMap<QString, QString> Utils::readInfoFromDesktop(const QString &desktopFile)
{
    QMap<QString, QString> info;
    for (SystemAppItem *item : m_sysAppMonitor->applications()) {
        if (item->path == desktopFile) {
            info.insert("Icon", item->iconName);
            info.insert("Name", item->name);
            info.insert("Exec", item->exec);
        }
    }

    return info;
}
