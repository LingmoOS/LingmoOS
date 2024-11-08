/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
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

#include <QDBusConnection>
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QIcon>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>

#include "application.h"

int main(int argc, char *argv[]) {
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
  QCoreApplication::setAttribute(Qt::AA_UseOpenGLES, true);

  Application app(argc, argv);

  app.setWindowIcon(QIcon::fromTheme("lingmo-settings"));

  // QQmlApplicationEngine engine;
  // const QUrl url(QStringLiteral("qrc:/main.qml"));
  // QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
  //                  &app, [url](QObject *obj, const QUrl &objUrl) {
  //     if (!obj && url == objUrl)
  //         QCoreApplication::exit(-1);
  // }, Qt::QueuedConnection);
  // engine.rootContext()->setContextProperty("upgradeableModel",
  // UpgradeableModel::self()); engine.load(url);

  return 0;
}
