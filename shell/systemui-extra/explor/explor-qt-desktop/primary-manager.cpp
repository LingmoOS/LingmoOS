/*
 * Peony-Qt
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "primary-manager.h"
#include <QDebug>
#include <QTimer>

#define DBUS_NAME       "org.lingmo.SettingsDaemon"
#define DBUS_PATH       "/org/lingmo/SettingsDaemon/wayland"
#define DBUS_INTERFACE  "org.lingmo.SettingsDaemon.wayland"

PrimaryManager::PrimaryManager()
{
    QDBusConnection conn = QDBusConnection::sessionBus();
    if (!conn.isConnected()) {
        qCritical()<<"failed to init primary screen manager, can not connect to session dbus";
        return;
    }
    mDbusXrandInter = new QDBusInterface(DBUS_NAME,
                                         DBUS_PATH,
                                         DBUS_INTERFACE,
                                         QDBusConnection::sessionBus());
    connect(mDbusXrandInter, SIGNAL(screenPrimaryChanged(int,int,int,int)),
            this, SLOT(priScreenChanged(int,int,int,int)));

}

void PrimaryManager::start()
{
    int priX, priY, priWid, priHei;
    QString priName;
    priX = getScreenGeometry("x");
    priY = getScreenGeometry("y");
    priWid = getScreenGeometry("width");
    priHei = getScreenGeometry("height");
    priName = getScreenName ("priScreenName");

    qDebug("Start: Primary screen geometry is x=%d, y=%d, windth=%d, height=%d,",
           priX, priY, priWid, priHei);
    qDebug()<<"Primary screen name = " << priName;
}

int PrimaryManager::getScreenGeometry(QString methodName)
{
    int res = 0;
    QDBusMessage message = QDBusMessage::createMethodCall(DBUS_NAME,
                               DBUS_PATH,
                               DBUS_INTERFACE,
                               methodName);
    QDBusMessage response = QDBusConnection::sessionBus().call(message, QDBus::Block, 3000);
    if (response.type() == QDBusMessage::ReplyMessage)
    {
        if(response.arguments().isEmpty() == false) {
            int value = response.arguments().takeFirst().toInt();
            res = value;
            qDebug() << value;
        }
    } else {
        qCritical()<<methodName<<"called failed";
    }
    return res;
}

QString PrimaryManager::getScreenName(QString methodName)
{
    QString res = nullptr;
    QDBusMessage message = QDBusMessage::createMethodCall(DBUS_NAME,
                               DBUS_PATH,
                               DBUS_INTERFACE,
                               methodName);
    QDBusMessage response = QDBusConnection::sessionBus().call(message, QDBus::Block, 3000);
    if (response.type() == QDBusMessage::ReplyMessage)
    {
        if(response.arguments().isEmpty() == false) {
            QString value = response.arguments().takeFirst().toString();
            res = value;
            qDebug() << value;
        }
    } else {
        qCritical()<<methodName<<"called failed";
    }
    return res;
}

/* get primary screen changed */
void PrimaryManager::priScreenChanged(int x, int y, int width, int height)
{
    Q_EMIT this->priScreenChangedSignal(x, y, width, height);
    qDebug("primary screen  changed, geometry is  x=%d, y=%d, windth=%d, height=%d", x, y, width, height);
}
