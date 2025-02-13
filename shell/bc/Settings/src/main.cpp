/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 * Author:     Elysia <c.elysia@foxmail.com>
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

#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QIcon>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>
#include <QDBusInterface>
#include <qguiapplication.h>

#include <memory>
#include <qqmlapplicationengine.h>

#include "application.h"

int main(int argc, char* argv[])
{

    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES, true);

    QGuiApplication app(argc, argv);

    app.setWindowIcon(QIcon::fromTheme("lingmo-settings"));
    app.setOrganizationName("Lingmo Community");

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Lingmo Settings"));
    parser.addHelpOption();

    QCommandLineOption moduleOption("m", "Switch to module", "module");
    parser.addOption(moduleOption);
    parser.process(app);

    const QString module = parser.value(moduleOption);

    if (!QDBusConnection::sessionBus().registerService("com.lingmo.SettingsUI")) {
        QDBusInterface iface("com.lingmo.SettingsUI", "/SettingsUI",
            "com.lingmo.SettingsUI",
            QDBusConnection::sessionBus());
        if (iface.isValid())
            iface.call("switchToPage", module);

        return 0;
    }

    auto engine = std::make_shared<QQmlApplicationEngine>();

    Application My_App(engine);

    if (!module.isEmpty()) {
        My_App.switchToPage(module);
    }

    return QGuiApplication::exec();
}
