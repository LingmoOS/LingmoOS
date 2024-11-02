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

#include <QApplication>
#include <QTranslator>
#include "processmanager.h"
#include "processmanagerservice.h"

bool isWaylandPlatform()
{
    QByteArray xdgSessionTypeValue = qgetenv("XDG_SESSION_TYPE");
    return xdgSessionTypeValue.compare("wayland", Qt::CaseInsensitive) == 0;
}

void setWaylandPlatformEnvValue()
{
   qputenv("QT_QPA_PLATFORM", "wayland");
}

void setLauncherEnvironment()
{
    const char *gtkUsePortalEnvKey = "GTK_USE_PORTAL";
    const char *glibForceUsePortalEnvKey = "GLIB_FORCE_USE_PORTAL";
    const char *glibUseLingmoProcessManagerEnvKey = "GLIB_USE_LINGMO_PROCESS_MANAGER";

    QByteArray gtkUsePortalEnvValue = qgetenv(gtkUsePortalEnvKey);
    QByteArray glibForceUsePortalEnvValue = qgetenv(glibForceUsePortalEnvKey);
    QByteArray glibUseLingmoProcessManagerEnvValue = qgetenv(glibUseLingmoProcessManagerEnvKey);

    // 初始化glib
    qunsetenv(gtkUsePortalEnvKey);
    qunsetenv(glibForceUsePortalEnvKey);
    qunsetenv(glibUseLingmoProcessManagerEnvKey);

    // 需要通过一些接口调用到glib_should_use_portal完成glib的初始化
    GFile *tmp_file = g_file_new_for_uri("file:///nofile");
    g_file_trash(tmp_file, 0, 0);
    g_object_unref(tmp_file);

    qputenv(gtkUsePortalEnvKey, gtkUsePortalEnvValue);
    qputenv(glibForceUsePortalEnvKey, glibForceUsePortalEnvValue);
    qputenv(glibUseLingmoProcessManagerEnvKey, glibUseLingmoProcessManagerEnvValue);

    qputenv("XDG_CURRENT_DESKTOP", "LINGMO");
    qputenv("QT_QPA_PLATFORMTHEME", "lingmo");
}

void initEnvironment()
{
    // 在Xwayland或者wayland平台上以wayland方式启动lingmo-process-manager进程
    // 可兼容x与wayland窗口处理事件
    if (isWaylandPlatform()) {
        setWaylandPlatformEnvValue();
    }

    setLauncherEnvironment();
}

int main(int argc, char *argv[])
{
    initEnvironment();

    QApplication a(argc, argv);

    QTranslator translator;
    translator.load(QLocale::system().name(), "/usr/share/lingmo-process-manager/");
    a.installTranslator(&translator);

    ProcessManager processManager;
    ProcessManagerService processManagerService(&processManager);
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService("com.lingmo.ProcessManager") ||
        !connection.registerObject("/com/lingmo/ProcessManager", &processManager)) {
        qWarning() << "Register process manager dbus service failed:" << connection.lastError();
        return 0;
    }

    return a.exec();
}
