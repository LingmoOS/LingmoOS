/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <QCoreApplication>
#include <QCommandLineParser>
#include "processmanagerservice.h"
#include "processmanager.h"

bool parseCommandLine(const QCoreApplication &app)
{
    QCommandLineParser parser;
    parser.setApplicationDescription("Lingmo Process Manager Daemon");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption removeOption(QStringList() << "r" << "remove", "Remove specified <cgroup path>.", "cgroup path");
    parser.addOption(removeOption);
    parser.process(app);
    if (parser.isSet(removeOption)) {
        QString groupPath = parser.value(removeOption);
        if (groupPath.isEmpty()) {
            qCritical() << "Error: No cgroup path specified.";
            return true;
        }
        CGroupManager cgroupManager;
        cgroupManager.removeProcessCGroup(groupPath);
        return true;
    }

    return false;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (parseCommandLine(a)) {
        return 0;
    }

    ProcessManager processManager;
    QDBusConnection connection = QDBusConnection::systemBus();
    if (!connection.registerService("com.lingmo.ProcessManagerDaemon") ||
            !connection.registerObject("/com/lingmo/ProcessManagerDaemon", &processManager)) {
        qWarning() << "register dbus service failed" << connection.lastError();
        return 0;
    }

    ProcessManagerService processManagerService(&processManager);

    return a.exec();
}
