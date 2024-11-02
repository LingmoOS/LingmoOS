/* Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
*             2020 KylinSoft Co., Ltd.
* This file is part of Lingmo-Screenshot.
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
*/
#include "systemnotification.h"
#include "src/utils/confighandler.h"
#include <QApplication>
#include <QUrl>

#ifndef Q_OS_WIN
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusInterface>
#else
#endif
#include "src/core/controller.h"

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
SystemNotification::SystemNotification(QObject *parent) : QObject(parent)
{
    m_interface = new QDBusInterface(QStringLiteral("org.freedesktop.Notifications"),
                                     QStringLiteral("/org/freedesktop/Notifications"),
                                     QStringLiteral("org.freedesktop.Notifications"),
                                     QDBusConnection::sessionBus(),
                                     this);
}

#else
SystemNotification::SystemNotification(QObject *parent) : QObject(parent)
{
    m_interface = nullptr;
}

#endif

void SystemNotification::sendMessage(const QString &text, const QString &savePath)
{
    sendMessage(text, tr("ScreenShot Info"), savePath);
}

void SystemNotification::sendMessage(
    const QString &text, const QString &title, const QString &savePath, const int timeout)
{
    if (!ConfigHandler().desktopNotificationValue()) {
        return;
    }

#ifndef Q_OS_WIN
    QList<QVariant> args;
    // QVariantMap hintsMap;
    QMap<QString, QVariant> hintsMap;
    hintsMap.insert("sound-name","screen-capture"); //添加声音

// if (!savePath.isEmpty()) {
// QUrl fullPath = QUrl::fromLocalFile(savePath);
//// allows the notification to be dragged and dropped
// hintsMap[QStringLiteral("x-kde-urls")] = QStringList({fullPath.toString()});
// }
    args << tr("Screenshot")                 // appname
         << static_cast<unsigned int>(0) // id
         << "lingmo-screenshot"                  // icon
         << title                        // summary
         << text                         // body
         << QStringList()                // actions
         << hintsMap                     // hints
         << timeout;                     // timeout
    m_interface->callWithArgumentList(QDBus::AutoDetect, QStringLiteral("Notify"), args);
#else
    auto c = Controller::getInstance();
    c->sendTrayNotification(text, title, timeout);
#endif
}
