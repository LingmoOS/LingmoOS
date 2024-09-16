// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "slideshowscreensaver.h"
#include "commandlinemanager.h"
#include "slideshowconfig.h"

#include <DApplication>

#include <QtDebug>
#include <QWindow>
#include <QMessageBox>
#include <QScreen>
#include <QStringList>
#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include <QTimer>
#include <QDateTime>

#include <X11/Xlib.h>

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    auto envType = qEnvironmentVariable("XDG_SESSION_TYPE");
    if (envType.contains("wayland")) {
        qInfo() << QDateTime::currentDateTime().toString() << "notes:change wayland to xcb for QT_QPA_PLATFORM.";
        qputenv("QT_QPA_PLATFORM", "xcb");
    }

    DApplication a(argc, argv);
    a.setOrganizationName("deepin");
    a.loadTranslator();
    a.setApplicationName(QObject::tr("deepin-screensaver"));
    a.setApplicationDisplayName(QObject::tr("deepin-screensaver"));
    CommandLineManager::instance()->process(a.arguments());

    if (CommandLineManager::instance()->isSet("window-id")) {

        QString windowId = CommandLineManager::instance()->value("window-id");
        WId windowHwnd = WId(windowId.toULongLong());
        QWindow *window = QWindow::fromWinId(windowHwnd);

        if (!window) {
            qDebug() << "Error:not found QWindow by window id:" << windowId;
            return -1;
        }

        SlideshowScreenSaver screensaver(true);
        screensaver.setProperty("_q_embedded_native_parent_handle", QVariant(windowHwnd));
        screensaver.winId();
        screensaver.windowHandle()->setParent(window);

        Display *display = XOpenDisplay(nullptr);
        if (!display) {
            qWarning() << "can not connect xservice.";
            return -1;
        }

        Window windowXID(windowHwnd);
        XWindowAttributes attr;
        XGetWindowAttributes(display, windowXID, &attr);

        screensaver.setGeometry(QRect(0, 0, attr.width, attr.height));
        screensaver.init();
        screensaver.show();

        return a.exec();

    } else if (CommandLineManager::instance()->isSet("d")) {

        QString path = CommandLineManager::instance()->value("d");
        SlideshowConfig::instance()->setSlideShowPath(path);

    } else if (CommandLineManager::instance()->isSet("t")) {

        int time = CommandLineManager::instance()->value("t").toInt();
        SlideshowConfig::instance()->setIntervalTime(time);

    } else if (CommandLineManager::instance()->isSet("m")) {

        QString shuffle = CommandLineManager::instance()->value("m");
        if (QStringLiteral("true") == shuffle)
            SlideshowConfig::instance()->setShuffle(true);
        else
            SlideshowConfig::instance()->setShuffle(false);

    } else if (CommandLineManager::instance()->isSet("config")) {
        qInfo() << "show dconfig dialog";
        if (SlideshowConfig::instance()->startCustomConfig())
            return a.exec();
        return 0;
    } else {
        SlideshowScreenSaver screensaver;
        screensaver.init();
        screensaver.show();

        return a.exec();
    }

    return 0;
}
