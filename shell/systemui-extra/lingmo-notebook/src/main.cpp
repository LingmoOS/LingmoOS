/*
* Copyright (C) 2020 Tianjin LINGMO Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "widget.h"
#include <singleapplication.h>

#include "utils/utils.h"

#include <QApplication>

#include "log.h"
#include "information_collector.h"
#include "windowmanage.hpp"
#include <lingmosdk/diagnosetest/libkydatacollect.h>

void addTrans()
{
    QString locale = QLocale::system().name();
    QString qtTranslationsPath;
    qtTranslationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);// /usr/share/qt5/translationsQString locale = QLocale::system().name();
    static QTranslator trans_global, trans_menu, trans_sdk;
    if(!trans_global.load("lingmo-notebook_" + locale + ".qm", QLatin1String("/usr/share/lingmo-notebook/translations")))
        qDebug() << "Load translations file" <<QLocale() << "failed!";
    else
        QApplication::installTranslator(&trans_global);

    if(!trans_menu.load("qt_" + locale + ".qm", qtTranslationsPath))
        qDebug() << "Load translations file" <<QLocale() << "failed!";
    else
        QApplication::installTranslator(&trans_menu);

    if(!trans_sdk.load(":/translations/gui_" + locale + ".qm"))
        qDebug() << "Load translations file" <<QLocale() << "failed!";
    else
        QApplication::installTranslator(&trans_sdk);
}
/*!
 * \brief main
 */
int main(int argc, char *argv[])
{
    //在程序启动时调用，不需要传入参数
    kdk_dia_upload_program_startup();
#ifndef TEST_DEBUG
    qInstallMessageHandler(logOutput);
#else
    qInstallMessageHandler(myMessageOutput);
#endif
    InformationCollector::getInstance().addMessage("便签应用启动!");

    //高清屏幕自适应,适配990高DPI
    kabase::WindowManage::setScalingProperties();

    kdk::QtSingleApplication a(argc, argv);

    if (a.isRunning()) {
        qDebug() << "is running";
        a.sendMessage(kdk::QtSingleApplication::arguments().join(" ") , 4000);
        return 0;
    }

    addTrans();

    a.setWindowIcon(QIcon::fromTheme("lingmo-notebook"));
    a.setApplicationVersion ("1.0.0");

    QCommandLineParser parser;
    Utils::setCLIName(parser);
    /* 处理命令行参数。
       除了解析选项（如parse（））外，此函数还处理内置选项并处理错误。
       如果调用了addVersionOption，则内置选项为--version，如果调用了addHelpOption，则为--help --help-all。
       当调用这些选项之一时，或者当发生错误（例如，传递了未知选项）时，当前进程将使用exit（）函数停止。
    */
    parser.process(a);
    Widget w;
    QObject::connect(&a, &kdk::QtSingleApplication::messageReceived, [&w](const QString &message) {
        qDebug() << message;
        auto arguments = message.split(" ");
        if (QApplication::arguments().length() > 1) {
            if (QApplication::arguments().at(1) == "--show") {
                QString arg = QApplication::arguments().at(2);
                qDebug() << "main" << arg.toInt();
                w.openMemoWithId(arg.toInt());
            }
        } else {
            w.createNewNoteIfEmpty();
        }
    });


    return a.exec();
}
