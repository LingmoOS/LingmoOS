// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later

#include "preloadplugin.h"

#include <QGuiApplication>
#include <QLocale>
#include <QTranslator>
#include <QUrl>

DQUICK_USE_NAMESPACE

PreloadPlugin::PreloadPlugin(QObject *parent)
    : QObject(parent)
{

}

PreloadPlugin::~PreloadPlugin()
{

}

QGuiApplication *PreloadPlugin::creatApplication(int &argc, char **argv) {

  QGuiApplication* app = new QGuiApplication(argc, argv);

  QTranslator* translator = new QTranslator();
  if (translator->load(QLocale::system().name(), ":/resources/translations/")) {
      app->installTranslator(translator);
  }

  app->setApplicationName("deepin-tweak");
  app->setApplicationVersion("1.0.0");
  app->setApplicationDisplayName(tr("Deepin Tweak"));

  return app;
}

QUrl PreloadPlugin::preloadComponentPath() const
{
    // 预览组件的 qml 路径
    return QUrl("qrc:///Preload.qml");
}
