/*
 * Copyright (C) 2020 CuteOS.
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

#include <QFile>
#include <QGuiApplication>
#include <QIcon>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>

#include "calcengine.h"

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);
  app.setWindowIcon(QIcon::fromTheme("lingmo-calculator"));

  qmlRegisterType<CalcEngine>("Lingmo.Calculator", 1, 0, "CalcEngine");

  QQmlApplicationEngine engine;

#ifdef QT_DEBUG
  engine.rootContext()->setContextProperty(QStringLiteral("debug"), true);
#else
  engine.rootContext()->setContextProperty(QStringLiteral("debug"), false);
#endif

  // Translations
  QLocale locale;
  QString qmFilePath = QString("%1/%2.qm")
                           .arg("/usr/share/lingmo-calculator/translations/")
                           .arg(locale.name());
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
