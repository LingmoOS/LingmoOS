// Copyright (C) 2021 ~ 2021 Lingmo Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "window/immodel/immodel.h"
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
DWIDGET_USE_NAMESPACE
using namespace std;
using namespace oceanui_fcitx_configtool::widgets;

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    DApplication a(argc, argv);
    a.setOrganizationName("lingmo");
    a.setApplicationName("lingmo-fcitxconfigtool-plugin");
    a.setApplicationVersion("1.0");
    a.setProductIcon(QIcon());
    a.setProductName("lingmo-fcitxconfigtool-plugin");
    a.setApplicationDescription("This is a dtk lingmo-fcitxconfigtool-plugin");
    a.loadTranslator();
    a.setApplicationDisplayName("lingmo-fcitxconfigtool-plugin");
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
