/*
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntulingmo.com/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QApplication>
#include <QTranslator>
#include <QObject>
#include <QDesktopWidget>
#include <QtSingleApplication>
#include <KWindowEffects>
#include <lingmo-log4qt.h>

#include "xatom-helper.h"
#include "mainwindow.h"

#include <execinfo.h>
#include <unistd.h>
#define BUFF_SIZE  1024

static void crashHandler(int sig)
{
    signal(sig, SIG_IGN);
    size_t size = 0;
    char **strings = NULL;
    size_t i = 0;

    char path[BUFF_SIZE] = {0};
    static char *homePath = getenv("HOME");
    snprintf(path, BUFF_SIZE, "%s/.log", homePath);
    strcat(path,"/lingmo_sysmon_crash.log");
    FILE *fp = fopen(path,"a+");
    
    if (fp) {
        void *array[20];
        size = backtrace (array, 20);
        strings = (char **)backtrace_symbols (array, size);

        char logStr[BUFF_SIZE] = {0};
        sprintf(logStr,"!!!--- [%s]pid:%d received signal: %d=%s ---!!!\n version = %s, Stack trace\n",
            QDateTime::currentDateTime().toString().toStdString().c_str(),getpid(),sig,strsignal(sig), "2.0.8");
        fwrite(logStr,sizeof(char),sizeof(logStr),fp);
        for (i = 0; i < size; i++)
        {
            memset(logStr,'\0',BUFF_SIZE);
            snprintf(logStr, BUFF_SIZE, "#%lu\t%s \n",i,strings[i]);
            fwrite(logStr,sizeof(char),sizeof(logStr),fp);
        }

        fflush(fp);
        fclose(fp);
        free (strings);
    }
    exit(128 + sig);
}

static void registerSignals()
{
    if(signal(SIGCHLD,SIG_IGN)==SIG_ERR)//忽略子进程已经停止或退出
    {
        //注册SIGCHLD信号失败
        perror("signal error");
    }
    if(signal(SIGSEGV,crashHandler)==SIG_ERR)//无效内存段访问
    {
        //注册SIGSEGV信号失败
        perror("signal error");
    }
    if(signal(SIGILL,crashHandler)==SIG_ERR)//非法指令
    {
        //注册SIGILL信号失败
        perror("signal error");
    }
    if(signal(SIGTERM,crashHandler)==SIG_ERR)//终止
    {
        //注册SIGTERM信号失败
        perror("signal error");
    }
    if(signal(SIGHUP,crashHandler)==SIG_ERR)//系统挂断
    {
        //注册SIGHUP信号失败
        perror("signal error");
    }
    if(signal(SIGABRT,crashHandler)==SIG_ERR)//进程停止运行
    {
        //注册SIGABRT信号失败
        perror("signal error");
    }
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORMTHEME",QByteArray("lingmo"));

    registerSignals();
    initLingmoUILog4qt("lingmo-system-monitor");

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QString id = QString("lingmo-system-monitor"+QLatin1String(getenv("DISPLAY")));
    QtSingleApplication app(id,argc,argv);
    if(app.isRunning())
    {
        app.sendMessage(QApplication::arguments().length() > 1 ? QApplication::arguments().at(1) : app.applicationFilePath());
        qInfo() << QObject::tr("lingmo-system-monitor is already running!");
        return EXIT_SUCCESS;
    } else {
        QApplication::setApplicationName("Lingmo System Monitor");
        QApplication::setApplicationVersion("0.0.0.0001");

        QString locale = QLocale::system().name();
        QTranslator translator;
        QTranslator sdktranslator;
        QTranslator qttranslator;
        qDebug()<<"QLocale::system().name():"<<locale;
        #if 0
        if(!translator.load("lingmo-system-monitor_" + locale + ".qm",
                            ":/translation/")) {
        #else
        if(!translator.load("lingmo-system-monitor_" + locale + ".qm",
                            "/usr/share/lingmo-system-monitor/translations/")) {
        #endif
            qDebug() << "Load translation file："<< "lingmo-system-monitor_" + locale + ".qm" << " failed!";
        } else {
            app.installTranslator(&translator);
            qDebug()<<"load success";
        }

        if(!sdktranslator.load(QString(":/translations/gui_%1.qm").arg(locale))) {
            qDebug() << "Load sdk translation file："<< QString(":/translations/gui_%1.qm").arg(locale) << " failed!";
        } else {
            app.installTranslator(&sdktranslator);
            qDebug()<<"load sdk translation file success";
        }

        // 加载 qt 翻译
        if(!qttranslator.load(QLocale(), "qt", "_", "/usr/share/qt5/translations/")) {
            qDebug() << "Load qt translation file："<< QString("usr/share/qt5/translations/%1.qm").arg(locale) << " failed!";
        } else {
            app.installTranslator(&qttranslator);
            qDebug()<<"load qt translation file success";
        }

        MainWindow monitor;

        app.setWindowIcon(QIcon::fromTheme("lingmo-system-monitor"));
        QObject::connect(&app, SIGNAL(messageReceived(const QString&)),&monitor, SLOT(handleMessage(const QString&)));
        monitor.windowShow();
#ifdef ENBALE_TRAY
        /* 保证程序在最后窗口退出后才退出 */
        app.setQuitOnLastWindowClosed(false);
#endif

        return app.exec();
    }
}
