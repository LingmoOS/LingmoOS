// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#include <QApplication>
#include <QDebug>
#include <QFileInfo>

#include "widgetgallery.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(styles);

//    qputenv("QT_DEBUG_PLUGINS", "1");
    QStringList paths = QApplication::libraryPaths();
    paths.prepend(QFileInfo(QString::fromUtf8(argv[0])).path() + "/plugins");
    QApplication::setLibraryPaths(paths);

    QApplication app(argc, argv);

    WidgetGallery gallery;
    gallery.show();

    return app.exec();
}
