// Copyright (C) 2021 ~ 2021 Lingmo Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 Lingmo Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "publisher/publisherdef.h"
#include "window/imwindow.h"
#include <DApplication>
#include <DWidgetUtil>
#include <DApplicationSettings>
#include <DMainWindow>
#include <QVBoxLayout>
#include <thread>
#include <QObject>
#include <QPushButton>
#include "widgets/settingsgroup.h"
#include "widgets/settingsitem.h"
#include <DLog>
#include <QTranslator>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
using namespace std;

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
    DApplication a(argc, argv);
    QTranslator translator;
    translator.load(QString("/usr/share/lingmo-fcitx5configtool-plugin/translations/lingmo-fcitx5configtool-plugin_%1.qm").arg(QLocale::system().name()));
    a.installTranslator(&translator);  //安装翻译器
    a.setOrganizationName("lingmo");
    a.setApplicationName("lingmo-fcitx5configtool-plugin");
    a.setApplicationVersion("1.0");
    a.setProductIcon(QIcon());
    a.setProductName("lingmo-fcitx5configtool-plugin");
    a.setApplicationDescription("This is a dtk lingmo-fcitx5configtool-plugin");
    a.loadTranslator();
    a.setApplicationDisplayName("lingmo-fcitx5configtool-plugin");
    DMainWindow m;
    IMWindow w(&m);
    m.resize(800, 600);
    m.setCentralWidget(&w);
    //w.setGeometry(m.geometry());
    m.show();
    //w.show();
    Dtk::Widget::moveToCenter(&w);

    return a.exec();
}
