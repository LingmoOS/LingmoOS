/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#include "mainprogram.h"

#include <QApplication>
#include <QtSingleApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QStyleFactory>
#include <QTextCodec>
#include <QDebug>
#include <QTranslator>
#include <QLibraryInfo>
#include <lingmo-log4qt.h>
#include "lingmosdk/lingmosdk-system/libkysysinfo.h"

#include "dbus/sysdbusinterface.h"

extern bool global_rightToleft;

int main(int argc, char *argv[])
{
    initLingmoUILog4qt("lingmo-bluetooth");

//    QApplication::setStyle(QStyleFactory::create("lingmo-default"));
    QApplication::setQuitOnLastWindowClosed(false);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ"){
       QGuiApplication::setLayoutDirection(Qt::RightToLeft);
       global_rightToleft = true;
       KyInfo() << "global_rightToleft set true";
    } else {
        QGuiApplication::setLayoutDirection(Qt::LeftToRight);
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QtSingleApplication app(argc, argv);

    if (app.isRunning()) {
        KyDebug() << "lingmo-bluetooth is Already running ! ! !";
        return EXIT_SUCCESS;
    }
    else
    {
        QTranslator *t = new QTranslator();
        t->load("/usr/share/libexplor-qt/libexplor-qt_"+QLocale::system().name() + ".qm");
        QApplication::installTranslator(t);

        //安装lingmo-bluetooth的中文翻译
        QTranslator * translator = new QTranslator();;
        translator->load("/usr/share/lingmo-bluetooth/translations/lingmo-bluetooth_" + QLocale::system().name() + ".qm");
        app.installTranslator(translator);

        //安装Qt中文翻译
        QTranslator * qt_translator = new QTranslator();
        QString qtTransPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
        if (qt_translator->load(QLocale(), "qt", "_", qtTransPath))
            app.installTranslator(qt_translator);

        auto ptr = new sysdbusinterface();

        MainProgram w(envPC);
        if(w.exit_flag){
            return 0;
        }
        return app.exec();
    }
}
