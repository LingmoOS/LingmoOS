// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <DLog>
#include "graphicsdriverinterface.h"
#include "graphicsdriveradaptor.h"
#include <Logger.h>
#include <ConsoleAppender.h>
#include <RollingFileAppender.h>

DCORE_USE_NAMESPACE

const QString GraphicMangerServiceName = "com.deepin.graphicmanger";
const QString GraphicMangerPath = "/com/deepin/graphicmanger";

int	main(int argc, char **argv)
{
    QString PATH = qgetenv("PATH");

    if (PATH.isEmpty()) {
        PATH = "/usr/bin";
    }
    PATH += ":/usr/sbin";
    PATH += ":/sbin";
    qputenv("PATH", PATH.toLatin1());

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    QCoreApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setApplicationName("deepin-graphics-driver-manager-server");

    //设置日志
    const QString logFormat = "%{time}{yyyyMMdd.HH:mm:ss.zzz}[%{type:1}][%{function:-40} %{line:-4} %{threadid:-8} ] %{message}\n";
    const QString log_file(QString("/var/log/deepin-graphics-driver-manager-server.log"));
    ConsoleAppender *consoleAppender = new ConsoleAppender;
    consoleAppender->setFormat(logFormat);
    RollingFileAppender *rollingFileAppender = new RollingFileAppender(log_file);
    rollingFileAppender->setFormat(logFormat);
    rollingFileAppender->setLogFilesLimit(5);
    rollingFileAppender->setDatePattern(RollingFileAppender::DailyRollover);

    logger->registerAppender(consoleAppender);
    logger->registerAppender(rollingFileAppender);

    QDBusConnection connection = QDBusConnection::systemBus();
    
    GraphicsDriverInterface interface;
    GraphicsDriverAdaptor adaptor(&interface);

    if (!connection.registerService(GraphicMangerServiceName))
    {
        qDebug() << connection.lastError().message();
        exit(1);
    }

    connection.registerObject(GraphicMangerPath, &interface, QDBusConnection::ExportAllContents);

    return app.exec();
}









