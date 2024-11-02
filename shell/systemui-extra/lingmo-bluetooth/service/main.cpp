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

#include <QCoreApplication>
#include <QtSingleCoreApplication>
#include <QDBusConnection>
#include <QCommandLineParser>
#include <QDebug>
#include <QFile>
#include <QTranslator>
#include <QLibraryInfo>
#include <lingmo-log4qt.h>

#include "daemon.h"
#include "sessiondbusregister.h"
#include "systemadapter.h"
#include "app.h"
#include "adapter.h"
#include "device.h"
#include "common.h"
#include "filesess.h"
#include "leavelock.h"


extern "C" {
    #include <string.h>
    #include <signal.h>
}

#include "lingmosdk/lingmosdk-system/libkysysinfo.h"


void signalHandlerFunc(int signal) {
    fprintf(stderr,"---------------------- %d",signal);
}


static QString executeLinuxCmd(QString strCmd)
{
    QProcess p;
    p.start("bash", QStringList() <<"-c" << strCmd);
    p.waitForFinished();
    QString strResult = p.readAllStandardOutput();
    return strResult.toLower();
}

static void setEnvPCValue()
{
    unsigned int productFeatures = 0;
    char * projectName = "";
    char * projectSubName = "";
    productFeatures = kdk_system_get_productFeatures();
    projectName = kdk_system_get_projectName();
    QString qs_projectName = QString(projectName);
    projectSubName = kdk_system_get_projectSubName();
    QString qs_projectSubName = QString(projectSubName);

    envPC = Environment::NOMAL;

    QString str = executeLinuxCmd("cat /proc/cpuinfo | grep -i hardware");
    if(str.length() == 0)
    {
        goto FuncEnd;
    }

    if(str.indexOf("huawei") != -1 || str.indexOf("pangu") != -1 ||
            str.indexOf("kirin") != -1)
    {
        QString str1 = executeLinuxCmd("dmidecode |grep \"String 4\"");
        //M900公版无需采用CBG方案，可跟随主线方案
        if(str.indexOf("m900") != -1 && (str1.isEmpty() || str1.indexOf("pwc30") == -1))
        {
            envPC = Environment::NOMAL;
        }
        else
        {
            envPC = Environment::HUAWEI;
        }

    }

FuncEnd:
    if (QFile::exists("/etc/apt/ota_version"))
        envPC = Environment::LAIKA;
    else if(qs_projectName.contains("V10SP1-edu",Qt::CaseInsensitive) &&
            qs_projectSubName.contains("mavis",Qt::CaseInsensitive))
    {
        envPC = Environment::MAVIS;
    }

    KyInfo () << envPC;

}

static bool InterfaceAlreadyExists()
{
    QDBusConnection conn = QDBusConnection::systemBus();
    if (!conn.isConnected())
        return 0;

    QDBusReply<QString> reply = conn.interface()->call("GetNameOwner", "com.lingmo.bluetooth");
    return reply.value() == "";
}


static int init(void)
{
    if (InterfaceAlreadyExists())
    {

        CONFIG::instance();
        CONFIG::getInstance()->init();

        QDBusConnection conn = QDBusConnection::systemBus();
        if (!conn.registerService("com.lingmo.bluetooth")) {
            KyWarning() << "QDbus register service failed reason:" << conn.lastError();
            return  -1;
        }

        SYSDBUSMNG::instance();
        SYSDBUSMNG::getInstance()->start();

        if(!conn.registerObject("/com/lingmo/bluetooth", "com.lingmo.bluetooth",
             SYSDBUSMNG::getInstance()->get_sys_adapter(),
             QDBusConnection::ExportAllSlots|QDBusConnection::ExportAllSignals))
        {
            KyWarning() << "QDbus register object failed reason:" << conn.lastError();
            return -1;
        }

        BPDMNG::instance();

        APPMNG::instance();

        ADAPTERMNG::instance();

        Q_EMIT SYSDBUSMNG::getInstance()->updateClient();

        DAEMON::instance();
        DAEMON::getInstance()->start();

        LEAVELOCK::instance();

        if(envPC != HUAWEI)
        {
            FILESESSMNG::instance();
            FILESESSMNG::getInstance()->init();
        }
    }
    else
    {
        KyInfo() << "already running";
        return -1;
    }

    return 0;
}

static int fini(void)
{
    return 0;
}


int main(int argc, char *argv[])
{
    fprintf(stdout,"Program running.....\n");
    initLingmoUILog4qt(QString("bluetoothserver"));

    setEnvPCValue();
    KyDebug() << "envPC: "<< envPC;

    QCoreApplication a(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({{{"o","obex"},QCoreApplication::translate("main","Load the Bluetooth file transfer agent")}});
    parser.setApplicationDescription(QCoreApplication::translate("main","LINGMO bluetooth daemon"));
    parser.process(a);

    QTranslator *t = new QTranslator();
    t->load("/usr/share/libexplor-qt/libexplor-qt_"+QLocale::system().name() + ".qm");
    QCoreApplication::installTranslator(t);

    int ret = init();
    if(0 != ret)
    {
        return ret;
    }

    KyInfo() << "QCoreApplication exec";
    int quitValue = a.exec();

    fini();

    KyInfo() << "Daemon program exit!!!!";

    return quitValue;
}
