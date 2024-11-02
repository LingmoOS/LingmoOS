/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */
#include <QApplication>
#include <QDebug>
#include <QShortcut>
#include <QTranslator>
#include <QtSingleApplication>
#include <QFile>

#include "ukmedia_main_widget.h"
#include <QObject>
#include <QDateTime>
#include <QMutex>
#include <QFile>
#include <QDesktopWidget>
#include <QDir>
#include <KWindowEffects>
#include <QMessageBox>
#include <QLibraryInfo>
#include <QStandardPaths>
#include <X11/Xlib.h>
#include <lingmo-log4qt.h>
#include "xatom/xatom-helper.h"
#include "windowmanager/windowmanager.h"

int main(int argc, char *argv[])
{
    initLingmoUILog4qt("lingmo-media");
    Display *display = XOpenDisplay(NULL);
    Screen *scrn = DefaultScreenOfDisplay(display);
    if (scrn == nullptr) {
        return 0;
    }

    #if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    #endif
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    #endif

    QString id = QString("lingmo-volume-control-applet-qt-" + QLatin1String(getenv("DISPLAY")));
    QtSingleApplication app(id, argc, argv);
    qDebug() << " DISPLAY id:" << id;
    if (app.isRunning()) {

        QDBusInterface interface("org.lingmo.media",
                                 "/org/lingmo/media",
                                 "org.lingmo.media",
                                 QDBusConnection::sessionBus());
        if (interface.isValid()){
            interface.call("advancedWidgetShow");
        }

        app.sendMessage(QApplication::arguments().length() > 1 ? QApplication::arguments().at(1) : app.applicationFilePath());
        qDebug() << QObject::tr("lingmo-volume-control-applet-qt is already running!");
        return EXIT_SUCCESS;

    }

#ifndef QT_NO_TRANSLATION
    QString translatorFileName = QLatin1String("qt_");
    translatorFileName += QLocale::system().name();
    QTranslator *pTranslator = new QTranslator();
    if (pTranslator->load(translatorFileName, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        app.installTranslator(pTranslator);
#endif

    //加载qm文件
    QTranslator translator;
    translator.load("/usr/share/lingmo-media/translations/" + QLocale::system().name());
    app.installTranslator(&translator);

    //加载qss文件
    QFile qss(":/data/qss/lingmomedia.qss");
    qss.open(QFile::ReadOnly);

    qss.close();
    UkmediaMainWidget w;
    app.setActivationWindow(&w);
    w.setProperty("useStyleWindowManager", false); //禁用拖动

    QString platform = QGuiApplication::platformName();
    if(!platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive)){
        MotifWmHints window_hints;
        window_hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
        window_hints.functions = MWM_FUNC_ALL;
        window_hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(w.winId(), window_hints);
    }
    else {
        w.setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    }

    XCloseDisplay(display);
    return app.exec();
}

