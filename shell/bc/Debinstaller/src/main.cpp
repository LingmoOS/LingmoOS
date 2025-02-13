/*
 * Copyright (C) 2021 Lingmo Technology Co., Ltd.
 *
 * Author:     Reion Wong <reion@lingmoos.com>
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
#include <QCommandLineParser>
#include <QFile>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>

#include "debinstaller.h"
#include "aptcli.h"

int main(int argc, char *argv[]) {
  // Init Components for apt cli
  InitSignals();
  InitOutput();

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

  QApplication app(argc, argv);
  QApplication::setQuitOnLastWindowClosed(true);

  qmlRegisterUncreatableType<DebInstaller>("Lingmo.DebInstaller", 1, 0, "DebInstaller", "error");

  // Translations
  QLocale locale;
  QString qmFilePath = QString("%1/%2.qm").arg("/usr/share/lingmo-debinstaller/translations/").arg(locale.name());
  if (QFile::exists(qmFilePath)) {
    QTranslator *translator = new QTranslator(app.instance());
    if (translator->load(qmFilePath)) {
      app.installTranslator(translator);
    }
    else {
      translator->deleteLater();
    }
  }

  qmlRegisterUncreatableType<DebInstaller>("Lingmo.DebInstaller", 1, 0, "DebInstaller", "error");

  QCommandLineParser parser;
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("file", ".deb file");
  parser.process(app);

  QString fileName;
  if (!parser.positionalArguments().isEmpty()) {
    fileName = parser.positionalArguments().first();
  }

  QQmlApplicationEngine engine;
  const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
  QObject::connect(
    &engine, &QQmlApplicationEngine::objectCreated, &app,
    [url](QObject *obj, const QUrl &objUrl) {
      if (!obj && url == objUrl)
        QCoreApplication::exit(-1);
    },
    Qt::QueuedConnection);

  DebInstaller *debInstaller = new DebInstaller;
  engine.rootContext()->setContextProperty("Installer", debInstaller);
  engine.load(url);
  debInstaller->setFileName(fileName);

  return QApplication::exec();
}
