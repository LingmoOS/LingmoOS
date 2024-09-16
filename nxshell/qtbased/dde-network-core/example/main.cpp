// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "networkhandler.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QTranslator>
#include <DGuiApplicationHelper>

#include "dccplugintestwidget.h"

using namespace std;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator t;
    if (t.load(":/qm/network_cn_qm"))
        app.installTranslator(&t);
    Dtk::Gui::DGuiApplicationHelper::loadTranslator("dcc-network-plugin",
                                                    {app.applicationDirPath() + "/../dcc-network-plugin"},
                                                    {QLocale::system()});

    if (QString(argv[1]) == "dccPlug") {
        DccPluginTestWidget testPluginWidget;
        testPluginWidget.resize(1024, 720);
        QDesktopWidget *deskdop = QApplication::desktop();
        testPluginWidget.move((deskdop->width() - testPluginWidget.width()) / 2, (deskdop->height() - testPluginWidget.height()) / 2);

        testPluginWidget.show();
        return app.exec();
    }

    NetworkHandler h;
    app.installEventFilter(&h);

    return app.exec();
}
