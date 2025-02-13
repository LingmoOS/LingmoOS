/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     Lingmo OS Team <team@lingmo.org>
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
#include <QQmlApplicationEngine>
#include <QDBusConnection>
#include <QQmlContext>
#include <QLocale>
#include <QTranslator>
#include <QFile>
#include <QIcon>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "version.h"
#include "appearance.h"
#include "background.h"
#include "setup.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);

    QQmlApplicationEngine engine;

    QCommandLineParser parser;
    parser.setApplicationDescription("Lingmo OS First Setup");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption noStartExec("d", "Exit");
    parser.addOption(noStartExec);

    parser.process(app);

    if (parser.isSet(noStartExec)) {
        return 0;
    }

    const char *uri = "Lingmo.Setup";
    
    qmlRegisterType<SysVersion>(uri, 1, 0, "SysVersion");
    qmlRegisterType<Appearance>(uri, 1, 0, "Appearance");
    qmlRegisterType<Background>(uri, 1, 0, "Background");
    qmlRegisterType<FirstSetupUI>(uri, 1, 0, "FirstSetupUI");

    // Translations
    QLocale locale;
    QString qmFilePath = QString("%1/%2.qm").arg("/usr/share/lingmo-welcome/translations/").arg(locale.name());
    if (QFile::exists(qmFilePath)) {
        QTranslator *translator = new QTranslator(QGuiApplication::instance());
        if (translator->load(qmFilePath)) {
            QGuiApplication::installTranslator(translator);
        } else {
            translator->deleteLater();
        }
    }

    engine.addImportPath(QStringLiteral("qrc:/"));
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    return app.exec();
}
