/*
 * Copyright (C) 2021 CuteOS Team.
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

#include "application.h"
#include <QDBusConnection>
#include <QTranslator>
#include <QLocale>

int main(int argc, char *argv[])
{
    Application app(argc, argv);

    if (!QDBusConnection::sessionBus().registerService("com.lingmo.ScreenLocker")) {
        return -1;
    }

    if (!QDBusConnection::sessionBus().registerObject("/ScreenLocker", &app)) {
        return -1;
    }

    // Translations
    QLocale locale;
    QString qmFilePath = QString("%1/%2.qm").arg("/usr/share/lingmo-screenlocker/translations/").arg(locale.name());
    if (QFile::exists(qmFilePath)) {
        QTranslator *translator = new QTranslator(app.instance());
        if (translator->load(qmFilePath)) {
            app.installTranslator(translator);
        } else {
            translator->deleteLater();
        }
    }

    app.setQuitOnLastWindowClosed(false);
    app.initialViewSetup();
    return app.exec();
}
