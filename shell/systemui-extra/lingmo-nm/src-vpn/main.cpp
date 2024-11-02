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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

//#include "mainwindow.h"
//#include "dbusadaptor.h"
#include "vpndbusadaptor.h"
#include <QTranslator>
#include <QLocale>
#include "qt-single-application.h"
#include <QDebug>
#include <QDesktopWidget>
#include <QFile>
#include <lingmo-log4qt.h>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
#include "xatom-helper.h"
#endif

#include "vpnobject.h"

#define LOG_IDENT "lingmo_vpn"

const QString QT_TRANSLATE_FILE = "/usr/share/qt5/translations/qt_zh_CN.qm";

void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    QByteArray currentDateTime = QDateTime::currentDateTime().toString().toLocal8Bit();

    bool showDebug = true;
    QString logFilePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config/lingmo/lingmo-nm.log";
    FILE *log_file = nullptr;

    if (showDebug) {
        log_file = fopen(logFilePath.toLocal8Bit().constData(), "a+");
    }

    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    switch (type) {
    case QtDebugMsg:
        if (!log_file) {
            break;
        }
        fprintf(log_file, "Debug: %s: %s (%s:%u, %s)\n", currentDateTime.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtInfoMsg:
        fprintf(log_file? log_file: stdout, "Info: %s: %s (%s:%u, %s)\n", currentDateTime.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtWarningMsg:
        fprintf(log_file? log_file: stderr, "Warning: %s: %s (%s:%u, %s)\n", currentDateTime.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtCriticalMsg:
        fprintf(log_file? log_file: stderr, "Critical: %s: %s (%s:%u, %s)\n", currentDateTime.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtFatalMsg:
        fprintf(log_file? log_file: stderr, "Fatal: %s: %s (%s:%u, %s)\n", currentDateTime.constData(), localMsg.constData(), file, context.line, function);
        break;
    }

    if (log_file)
        fclose(log_file);
}

int main(int argc, char *argv[])
{
    initLingmoUILog4qt("lingmo-vpn");

#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QString id = QString("lingmo-vpn"+ QLatin1String(getenv("DISPLAY")));
    QtSingleApplication a(id, argc, argv);

    QApplication::setQuitOnLastWindowClosed(false);

    QDBusInterface interface("com.lingmo.lingmovpn",
                                                   "/com/lingmo/lingmovpn",
                                                   "com.lingmo.lingmovpn",
                                                   QDBusConnection::sessionBus());
    if(interface.isValid()) {
        return 0;
    }

    QThread *thread = new QThread();
    KyNetworkResourceManager *p_networkResource = KyNetworkResourceManager::getInstance();
    p_networkResource->moveToThread(thread);
    QObject::connect(thread, &QThread::started, p_networkResource, &KyNetworkResourceManager::onInitNetwork);
    QObject::connect(&a,&QtSingleApplication::aboutToQuit, thread, &QThread::quit);
    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    QObject::connect(thread, &QThread::finished, [=](){
        qDebug() << "release" ;
        p_networkResource->Release();
    });
    thread->start();

    // Internationalization
    QString locale = QLocale::system().name();
    QTranslator trans_global;
    qDebug() << "QLocale " << QLocale();
    if (trans_global.load(QLocale(), "lingmo-vpn", "_", "/usr/share/lingmo-nm/lingmo-vpn/"))
    {
        a.installTranslator(&trans_global);
        qDebug()<<"Translations load success";
    } else {
        qWarning() << "Translations load fail";
    }

    QTranslator qtBaseTranslator;
    if (qtBaseTranslator.load(QLocale(), "qt", "_", "/usr/share/qt5/translations/"))
    {
        a.installTranslator(&qtBaseTranslator);
        qDebug()<<"QtBase Translations load success";
    } else {
        qWarning() << "QtBase Translations load fail";
    }

    while (!p_networkResource->NetworkManagerIsInited()) {
        ::usleep(1000);
    }

    vpnObject vpnobject;
    a.setActivationWindow(&vpnobject);
    vpnobject.setProperty("useStyleWindowManager", false); //禁用拖动
    a.setWindowIcon(QIcon::fromTheme("lingmo-vpn-symbolic"));

    VpnDbusAdaptor vpnAdaptor(&vpnobject);
    Q_UNUSED(vpnAdaptor);

    auto connection = QDBusConnection::sessionBus();
    if (!connection.registerService("com.lingmo.lingmovpn")
        || !connection.registerObject("/com/lingmo/lingmovpn", &vpnobject)) {
        qCritical() << "QDbus register service failed reason:" << connection.lastError();
    }

    return a.exec();
}
