// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
//#include "../utils/environments.h"
#include "environments.h"
#include <QTranslator>
#include <DApplication>
#include <QLocale>
#include <DLog>
#include <DApplicationSettings>


DCORE_USE_NAMESPACE

QString getUserName()
{
    QString userPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    return userPath.section("/", -1, -1);
}

int main(int argc, char* argv[])
{
    DApplication app(argc, argv);
    app.loadTranslator();
    QString loc = QLocale::system().name();

    QString loc_tr_file = QString(TRANSLATIONS_DIR"/deepin-graphics-driver-manager_%1.qm");

    QTranslator trans;
    trans.load(loc_tr_file.arg(loc));
    app.installTranslator(&trans);

    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setAttribute(Qt::AA_EnableHighDpiScaling);
    app.setOrganizationName("deepin");
    app.setApplicationName("dgradvrmgr");
    app.setApplicationVersion(VERSION);
    app.setApplicationAcknowledgementPage("https://www.deepin.org/acknowledgments/deepin-graphics-driver-manager/");
    app.setProductIcon(QIcon(":/resources/icons/deepin-graphics-driver-manager-64px.svg"));
    app.setProductName(QApplication::translate("main", "Graphics Driver Manager"));
    app.setAutoActivateWindows(true);
    app.setApplicationDescription(QApplication::translate("main", "Graphics Driver Manager is a compact and easy to use graphics driver management tool. It includes graphics card hardware detection, graphics driver installation, graphics driver solution switching,  graphics driver automatic recovery and other functions."));
    DApplicationSettings as;

    if (!app.setSingleInstance("dgradvrmgr"))
        return -1;
    //设置日志
    const QString logFormat = "%{time}{yyyyMMdd.HH:mm:ss.zzz}[%{type:1}][%{function:-40} %{line:-4} %{threadid:-8} ] %{message}\n";

    QString log_file(QString("/home/%1/.cache/deepin/deepin-graphics-driver-manager/").arg(getUserName()));
    QDir dir(log_file);
    if (!dir.exists()) {
        dir.mkpath(log_file);
    }
    log_file += "test-installer.log";
    ConsoleAppender *consoleAppender = new ConsoleAppender;
    consoleAppender->setFormat(logFormat);
    RollingFileAppender *rollingFileAppender = new RollingFileAppender(log_file);
    rollingFileAppender->setFormat(logFormat);
    rollingFileAppender->setLogFilesLimit(5);
    rollingFileAppender->setDatePattern(RollingFileAppender::DailyRollover);

    logger->registerAppender(consoleAppender);
    logger->registerAppender(rollingFileAppender);
    MainWindow w;
    w.show();

    return app.exec();
}
