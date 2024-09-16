// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "environmentsmanager.h"
#include "utils/utils.h"
#include "org_freedesktop_systemd1_Manager.h"
#include "org_freedesktop_DBus.h"

#include <QProcess>
#include <QDebug>
#include <QDBusInterface>

EnvironmentsManager::EnvironmentsManager()
{
    createGeneralEnvironments();
    createKeyringEnvironments();
    createDBusEnvironments();
}

void EnvironmentsManager::init()
{
    qInfo() << "environments manager init";

    // clear env
    QByteArray sessionType = qgetenv("XDG_SESSION_TYPE");
    bool ret = false;
    if (sessionType == "x11") {
        // 从wayland切换到x11后，此环境变量还存在
        // 部分应用以此环境变量是否存在来判断当前是否是wayland环境, 为避免导致的一系列问题，启动后清除它
        ret = unsetEnv("WAYLAND_DISPLAY");
        ret = unsetEnv("QT_WAYLAND_SHELL_INTEGRATION");

    } else if (sessionType == "wayland") {
        ret = true;
    }
    qDebug() << "clear env, result: " << ret;

    // setup general envs
    for (auto it : m_envMap.keys()) {
        qputenv(it.toStdString().data(), m_envMap[it].toStdString().data());
    }

    {
        QStringList envs;
        for (auto it : m_envMap.keys()) {
            envs.append(it + "=" + m_envMap[it]);
        }

        // systemd1
        org::freedesktop::systemd1::Manager systemd1("org.freedesktop.systemd1", "/org/freedesktop/systemd1", QDBusConnection::sessionBus());
        QDBusPendingReply<void> replySystemd1 = systemd1.SetEnvironment(envs);
        replySystemd1.waitForFinished();
        if (replySystemd1.isError()) {
            qWarning() << "failed to set systemd1 envs: " << envs;
        }

        // dbus
        org::freedesktop::DBus dbus("org.freedesktop.DBus", "/org/freedesktop/DBus", QDBusConnection::sessionBus());
        EnvInfoList envInfos;
        for (auto it : m_envMap.keys()) {
            envInfos.insert(it, m_envMap[it]);
        }

        QDBusPendingReply<void> replyDbus = dbus.UpdateActivationEnvironment(envInfos);
        replyDbus.waitForFinished();
        if (replyDbus.isError()) {
            qWarning() << "failed to update dbus envs:" << envInfos;
        }
    }

    qInfo() << "environments manager init finished";
}

void EnvironmentsManager::createGeneralEnvironments()
{
    double scaleFactor = Utils::SettingValue("com.deepin.xsettings", QByteArray(), "scale-factor", 1.0).toDouble();
    auto envs = QProcessEnvironment::systemEnvironment();
    auto keys = envs.keys();

    for (const auto& key : keys) {
        m_envMap.insert(key, envs.value(key));
    }

    m_envMap.insert("GNOME_DESKTOP_SESSION_ID", "this-is-deprecated");
    m_envMap.insert("XDG_CURRENT_DESKTOP", "DDE");
    m_envMap.insert("QT_DBL_CLICK_DIST", QString::number(15 * scaleFactor));
}

void EnvironmentsManager::createKeyringEnvironments()
{
    // man gnome-keyring-daemon:
    // The daemon will print out various environment variables which should be set
    // in the user's environment, in order to interact with the daemon.
    EXEC_COMMAND("/usr/bin/gnome-keyring-daemon", QStringList() << "--start" << "--components=secrets,pkcs11,ssh");
    const QByteArray &output = p.readAllStandardOutput();
    QByteArrayList envList = output.split('\n');
    for (auto env : envList) {
        auto list = env.split('=');
        if (list.size() != 2)
            continue;

        m_envMap.insert(list.at(0), list.at(1));
    }
}

void EnvironmentsManager::createDBusEnvironments()
{
    QByteArrayList additionalEnvs = {"LANG", "LANGUAGE"};
    for (auto env : additionalEnvs) {
        m_envMap.insert(env, qgetenv(env));
    }
}

bool EnvironmentsManager::unsetEnv(QString env)
{
    // dbus
    org::freedesktop::DBus dbus("org.freedesktop.DBus", "/org/freedesktop/DBus", QDBusConnection::sessionBus());
    EnvInfoList envs;
    envs.insert(env, "");
    QDBusPendingReply<void> replyDbus = dbus.UpdateActivationEnvironment(envs);
    replyDbus.waitForFinished();
    if (replyDbus.isError()) {
        qWarning() << "unset dbus env failed:" << env;
        return false;
    }

    // systemd1
    org::freedesktop::systemd1::Manager systemd1("org.freedesktop.systemd1", "/org/freedesktop/systemd1", QDBusConnection::sessionBus());
    QDBusPendingReply<void> replySystemd1 = systemd1.UnsetEnvironment(QStringList(env));
    replySystemd1.waitForFinished();
    if (replySystemd1.isError()) {
        qWarning() << "unset systemd1 env failed:" << env;
        return false;
    }

    return true;
}
