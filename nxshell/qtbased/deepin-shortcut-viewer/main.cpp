// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <DLog>
//#include "widget.h"
#include "./view/mainwidget.h"
#include <QRect>
#include <QPoint>
#include <QFile>
#include <QLocale>
#include "singleapplication.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    //Singlentan process
    SingleApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    app.setOrganizationName("deepin");
    app.setApplicationName(QObject::tr("Deepin Shortcut Viewer"));
    app.setApplicationVersion("v1.0");
    app.setTheme("dark");

    //Logger handle
    DLogManager::registerConsoleAppender();

    QString uniqueKey = app.applicationName();
    bool isSingleApplication = app.setSingleInstance(uniqueKey);

    //Handle singlelentan process communications;
    if (isSingleApplication) {
        SingleApplication::processArgs(app.arguments());

        return app.exec();
    } else {
        QByteArray array;

        for (const QString &arg : app.arguments())
            array.append(arg.toLocal8Bit()).append('\0');

        app.newClientProcess(uniqueKey, array);

        return 0;
    }
}
