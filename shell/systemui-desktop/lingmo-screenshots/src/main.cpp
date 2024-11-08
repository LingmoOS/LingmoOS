/*
 * Copyright (C) 2021 LingmoOS Team.
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

#include <QApplication>
#include <QDBusConnection>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QTranslator>
#include <QLocale>

#include "screenshotview.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCommandLineOption delayOption(QStringList() << "d" << "delay", "Delay Screenshot", "NUM");
    QCommandLineParser parser;
    parser.setApplicationDescription("Lingmo Screenshot");
    parser.addHelpOption();
    parser.addOption(delayOption);
    parser.process(app);

    if (!QDBusConnection::sessionBus().registerService("com.lingmo.Screenshot")) {
        app.exit();
        return 0;
    }

    QString qmFilePath = QString("%1/%2.qm").arg("/usr/share/lingmo-screenshot/translations/").arg(QLocale::system().name());
    if (QFile::exists(qmFilePath)) {
        QTranslator *translator = new QTranslator(QApplication::instance());
        if (translator->load(qmFilePath)) {
            QApplication::installTranslator(translator);
        } else {
            translator->deleteLater();
        }
    }

    ScreenshotView view;
    if (parser.isSet(delayOption)) {
        view.delay(parser.value(delayOption).toInt());
    } else {
        view.start();
    }

    return app.exec();
}
