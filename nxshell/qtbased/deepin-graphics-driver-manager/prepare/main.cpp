// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "environments.h"
#include "mainwindow.h"

#include <QTranslator>
#include <DApplication>
#include <QLocale>
#include <DLog>
#include <DApplicationSettings>
#include <DWidgetUtil>


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
    app.setAutoActivateWindows(true);
    app.installTranslator(&trans);

    QProcess *process = new QProcess;
    process->setProcessChannelMode(QProcess::MergedChannels);
    QStringList args{"-c", "ps aux | grep /usr/lib/deepin-graphics-driver-manager |grep -v grep"};
    process->start("/bin/bash", args);
    process->waitForFinished(-1);
    if (!process->exitCode()){
        qInfo() << "The graphics driver manager is already working";
        return -1;
    }

    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setAttribute(Qt::AA_EnableHighDpiScaling);
    app.setOrganizationName("deepin");
    app.setApplicationName("deepin-graphics-driver-manager");
    app.setApplicationVersion(VERSION);
    app.setApplicationAcknowledgementPage("https://www.deepin.org/acknowledgments/deepin-graphics-driver-manager/");
    app.setProductIcon(QIcon(":/resources/icons/deepin-graphics-driver-manager-64px.svg"));
    app.setProductName(QApplication::translate("main", "Graphics Driver Manager"));
    app.setApplicationDescription(QApplication::translate("main", "Graphics Driver Manager is a compact and easy to use graphics driver management tool. It includes graphics card hardware detection, graphics driver installation, graphics driver solution switching,  graphics driver automatic recovery and other functions."));

    //主题自动保存
    DApplicationSettings as;

    //设置日志
    const QString logFormat = "%{time}{yyyyMMdd.HH:mm:ss.zzz}[%{type:1}][%{function:-40} %{line:-4} %{threadid:-8} ] %{message}\n";

    QString log_file(QString("/home/%1/.cache/deepin/deepin-graphics-driver-manager/").arg(getUserName()));
    QDir dir(log_file);
    if (!dir.exists()) {
        dir.mkpath(log_file);
    }
    log_file += "deepin-graphics-driver-manager.log";
    ConsoleAppender *consoleAppender = new ConsoleAppender;
    consoleAppender->setFormat(logFormat);
    RollingFileAppender *rollingFileAppender = new RollingFileAppender(log_file);
    rollingFileAppender->setFormat(logFormat);
    rollingFileAppender->setLogFilesLimit(5);
    rollingFileAppender->setDatePattern(RollingFileAppender::DailyRollover);

    logger->registerAppender(consoleAppender);
    logger->registerAppender(rollingFileAppender);
    qInfo() << "VERSION: " << VERSION;
    qputenv("DTK_USE_SEMAPHORE_SINGLEINSTANCE", "1");
    if (!app.setSingleInstance(app.applicationName(), DApplication::UserScope)) {
        exit(0);
    }
    MainWindow w;
    w.show();

    return app.exec();
}
