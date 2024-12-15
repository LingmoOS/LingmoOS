// SPDX-FileCopyrightText: 2021 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mainwindow.h"
#include "helper.hpp"

#include <DApplication>
#include <DWidgetUtil>

DWIDGET_USE_NAMESPACE


int main(int argc, char *argv[])
{
    DApplication a(argc, argv);
    a.setApplicationVersion(VERSION);
    a.setApplicationName("ocean-dconfig-editor");
    a.setOrganizationName("lingmo");
    a.loadTranslator();
    loadTranslation("ocean-dconfig-editor");
    a.setProductIcon(QIcon::fromTheme(APP_ICON));
    a.setWindowIcon(QIcon::fromTheme(APP_ICON));
    MainWindow window;
    window.show();

    Dtk::Widget::moveToCenter(&window);

    return a.exec();
}
