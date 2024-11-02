/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <QtWidgets/QApplication>
#include <gio/gio.h>
#include <unistd.h>

#include <QDebug>

#include "../app/samba-config-interface.h"
#include <QDebug>

// 用d-feet测试，这个例子还有点问题
int main (int argc, char* argv[])
{
    Q_UNUSED(argc)
    Q_UNUSED(argv)

    QApplication app(argc, argv);

    SambaConfigInterface si(DBUS_NAME, DBUS_PATH, QDBusConnection::systemBus());

    qDebug() << g_get_user_name() << "  " << getpid() << "  " << getuid();
    sleep(1);

    qDebug() << si.init(g_get_user_name(), getpid(), getuid());

    sleep(1);
    qDebug() << si.hasPasswd();

    sleep(1);
    qDebug() << si.setPasswd("qwer1234");

    si.finished();

    return app.exec();
}
