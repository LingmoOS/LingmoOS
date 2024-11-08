/*
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
 */
#include "app-manager.h"
#include "basic-app-model.h"

#include <QDebug>
#include <QDBusReply>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QProcess>
#include <QSettings>

#define LINGMO_APP_MANAGER_NAME         "com.lingmo.ProcessManager"
#define LINGMO_APP_MANAGER_PATH         "/com/lingmo/ProcessManager/AppLauncher"
#define LINGMO_APP_MANAGER_INTERFACE    "com.lingmo.ProcessManager.AppLauncher"

namespace LingmoUIMenu {
AppManager *AppManager::instance()
{
    static AppManager instance(nullptr);
    return &instance;
}

AppManager::AppManager(QObject *parent) : QObject(parent)
{

}

void AppManager::launchApp(const QString &appid)
{
    if (appid.isEmpty()) {
        return;
    }

    Q_EMIT request(LingmoUIMenuApplication::Hide);

    QDBusMessage message = QDBusMessage::createMethodCall(LINGMO_APP_MANAGER_NAME, LINGMO_APP_MANAGER_PATH, LINGMO_APP_MANAGER_INTERFACE, "LaunchApp");
    message << appid;

    auto watcher = new QDBusPendingCallWatcher(QDBusPendingCall(QDBusConnection::sessionBus().asyncCall(message)), this);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [appid] (QDBusPendingCallWatcher *self) {
        if (self->isError()) {
            QStringList list = parseDesktopFile(appid);
            if (!list.isEmpty()) {
                QString cmd = list.takeFirst();
                QProcess::startDetached(cmd, list);
                qDebug() << "launch with QProcess:" << cmd << list;
            }
        }
        self->deleteLater();
    });

    BasicAppModel::instance()->databaseInterface()->updateApLaunchedState(appid, true);
}

void AppManager::launchBinaryApp(const QString &app, const QString &args)
{
    if (app.isEmpty()) {
        return;
    }

    QDBusMessage message = QDBusMessage::createMethodCall(LINGMO_APP_MANAGER_NAME, LINGMO_APP_MANAGER_PATH, LINGMO_APP_MANAGER_INTERFACE, "LaunchApp");
    message << app << args;

    auto watcher = new QDBusPendingCallWatcher(QDBusPendingCall(QDBusConnection::sessionBus().asyncCall(message)), this);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [app, args] (QDBusPendingCallWatcher *self) {
        if (self->isError()) {
            QProcess::startDetached(app, {args}, "");
        }
        self->deleteLater();
    });
}

void AppManager::launchAppWithArguments(const QString &appid, const QStringList &args, const QString &applicationName)
{
    if (appid.isEmpty()) {
        return;
    }

    Q_EMIT request(LingmoUIMenuApplication::Hide);

    QDBusMessage message = QDBusMessage::createMethodCall(LINGMO_APP_MANAGER_NAME,
                                                          LINGMO_APP_MANAGER_PATH,
                                                          LINGMO_APP_MANAGER_INTERFACE,
                                                          "LaunchAppWithArguments");
    message << appid << args;

    auto watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [applicationName, args] (QDBusPendingCallWatcher *self) {
        if (self) {
            QDBusPendingReply<QString, QByteArray> reply = *self;
            if (self->isError()) {
                qWarning() << reply.error().message();
                QProcess::startDetached(applicationName, args);
            }
            self->deleteLater();
        }
    });
}

void AppManager::changeFavoriteState(const QString &appid, bool isFavorite)
{
    BasicAppModel::instance()->databaseInterface()->fixAppToFavorite(appid, isFavorite ? 1 : 0);
}

QStringList AppManager::parseDesktopFile(const QString &appid)
{
    // TODO: try QSettings?
    QSettings file(appid, QSettings::IniFormat);

    file.beginGroup(QStringLiteral("Desktop Entry"));
    QStringList stringList = file.value(QStringLiteral("Exec")).toString().split(" ");
    file.endGroup();

    if (stringList.isEmpty()) {
        return {};
    }

    QStringList res;
    res << stringList.takeFirst();

    // TODO: freedesktop group
    for (const auto &str : stringList) {
        if (!str.startsWith("%")) {
            res.append(str);
        }
    }

    return res;
}

} // LingmoUIMenu
