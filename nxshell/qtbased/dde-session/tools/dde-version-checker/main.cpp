// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QSettings>
#include <QDebug>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QProcess>

bool isDeepinVersionChanged()
{
    // 查看系统当前版本
    QSettings osSettings("/etc/os-version", QSettings::IniFormat);
    osSettings.beginGroup("Version");
    const QString &minorVersion = osSettings.value("MinorVersion").toString();
    const QString &welcomePath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first()
            + QDir::separator()
            + "deepin"
            + QDir::separator()
            + "dde-version-checker"
            + QDir::separator();
    const QString &welcomeConf = "dde-welcome.conf";

    // 比较系统版本和配置文件中记录的是否一致
    QSettings welcomeSettings(welcomePath + welcomeConf, QSettings::IniFormat);
    const QString &welcomeVersion = welcomeSettings.value("MinorVersion").toString();
    // dde-welcome.conf中不存在version时，为第一次进入系统或进入当前用户，不显示此界面
    bool changed = (welcomeVersion.isEmpty() ? false : minorVersion != welcomeVersion);
    qDebug() << "os version: " << minorVersion
             << ", welcome version: " << welcomeVersion
             << ", version changed: " << changed;
    if (!welcomeSettings.isWritable()) {
        qWarning() << "fail to update welcome config";
        return changed;
    }

    if (changed)
        welcomeSettings.setValue("MinorVersion", minorVersion);

    return changed;
}

void maybeOpenWelcome()
{
    if (!isDeepinVersionChanged()) {
        return;
    }

    // TODO 如果当前正在回滚系统，不应该显示欢迎界面，以前是通过ABRecovery来完成的，V23废弃了这个项目，这里的逻辑待定

    qDebug() << "open the welcome";
    QProcess::startDetached("/usr/lib/deepin-daemon/dde-welcome");
}

int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    qDebug() << "deepin version checker init";
    maybeOpenWelcome();
    qDebug() << "deepin version checker init finished";

    return 0;
}
