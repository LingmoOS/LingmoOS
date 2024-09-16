// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>

#include <QSettings>
#include <QTranslator>

#include "gltestwidget.h"
#include "gltestwindow.h"

#include <DLog>

DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    QApplication app(argc, argv);
    QSettings settings("/usr/lib/deepin-graphics-driver-manager/working-dir/config.conf", QSettings::IniFormat);
    const QString lang = settings.value("lang", "en_US").toString();
    QTranslator trans;
    trans.load(QString(TRANSLATIONS_DIR"/deepin-graphics-driver-manager_%1.qm").arg(lang));
    app.installTranslator(&trans);

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

    GLTestWindow w;
    w.show();

    return app.exec();

}


