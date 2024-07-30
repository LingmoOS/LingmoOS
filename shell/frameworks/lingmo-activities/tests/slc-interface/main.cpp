/*
    SPDX-FileCopyrightText: 2015 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "window.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Window w;
    w.show();

    // ResultSet results(UsedResources | Agent{"gvim"});
    //
    // int count = 20;
    // for (const auto& result: results) {
    //     qDebug() << "Result:" << result.title << result.resource;
    //     if (count -- == 0) break;
    // }
    //
    // ResultModel model(UsedResources | Agent{"gvim"});
    // model.setItemCountLimit(50);
    //
    // QListView view;
    // view.setModel(&model);
    //
    // view.show();

    return app.exec();
}
