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
#include "mainwindow.h"
#include "dbus.h"
#include <QTranslator>
#include <QLocale>
#include "qt-single-application.h"
#include <QDebug>
#include <QDesktopWidget>
#include <QFile>
#include <KWindowSystem>
#include <lingmo-log4qt.h>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
#include "xatom-helper.h"
#endif

#define LOG_IDENT "lingmo_lingmo_nm"

const QString QT_TRANSLATE_FILE = "/usr/share/qt5/translations/qt_zh_CN.qm";

void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    QByteArray currentDateTime = QDateTime::currentDateTime().toString().toLocal8Bit();

    bool showDebug = true;
    QString logFilePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config/lingmo/lingmo-nm.log";
    //若不需要自动创建日志文件，请放开此注释
//    if (!QFile::exists(logFilePath)) {
//        showDebug = false;
//    }
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

QString displayFromPid(uint pid)
{
    QFile environFile(QStringLiteral("/proc/%1/environ").arg(QString::number(pid)));
    if (environFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QByteArray DISPLAY  = KWindowSystem::isPlatformWayland() ? QByteArrayLiteral("WAYLAND_DISPLAY") : QByteArrayLiteral("DISPLAY");
        const auto lines = environFile.readAll().split('\0');
        for (const QByteArray &line : lines) {
            const int equalsIdx = line.indexOf('=');
            if (equalsIdx <= 0) {
                continue;
            }
            const QByteArray key = line.left(equalsIdx);
            if (key == DISPLAY) {
                const QByteArray value = line.mid(equalsIdx + 1);
                return value;
            }
        }
    }
    return {};
}

int main(int argc, char *argv[])
{
    initLingmoUILog4qt("lingmo-nm");

#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

//    QApplication a(argc, argv);
    QString id = QString("lingmo-nm"+ QLatin1String(getenv("DISPLAY")));
    QtSingleApplication a(id, argc, argv);

    QApplication::setQuitOnLastWindowClosed(false);


    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "lingmonm"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption swOption(QStringLiteral("sw"),QCoreApplication::translate("main", "show lingmo-nm wifi page"));
    QCommandLineOption snOption(QStringLiteral("sn"),QCoreApplication::translate("main", "show lingmo-nm lan page"));

    parser.addOptions({swOption,snOption});
    parser.process(a);

    QString display;
    QString sessionType;
    if(QString(getenv("XDG_SESSION_TYPE")) == "wayland") {
        sessionType = "wayland";
        display = getenv("WAYLAND_DISPLAY");
    } else {
        sessionType = "x11";
        display = getenv("DISPLAY");
    }
    qDebug() << sessionType << display;
    qApp->setProperty("sessionType", sessionType);

    QDBusInterface interface("com.lingmo.network",
                             "/com/lingmo/network",
                             "com.lingmo.network",
                             QDBusConnection::sessionBus());

    if (a.isRunning()) {
        if(interface.isValid()) {
            if (parser.isSet(swOption)) {
                interface.call(QStringLiteral("showLingmoNM"), 1);
            } else if (parser.isSet(snOption)) {
                interface.call(QStringLiteral("showLingmoNM"), 0);
            } else {
                interface.call(QStringLiteral("showLingmoNM"), 2);
            }
        }
        return 0;
    }

    QThread *thread = new QThread();
    KyNetworkResourceManager *p_networkResource = KyNetworkResourceManager::getInstance();
    p_networkResource->moveToThread(thread);
    QObject::connect(thread, &QThread::started, p_networkResource, &KyNetworkResourceManager::onInitNetwork);
    QObject::connect(&a,&QtSingleApplication::aboutToQuit, thread, &QThread::quit);
    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();

    // Internationalization
    QString locale = QLocale::system().name();
    QTranslator trans_global;
    qDebug() << "QLocale " << QLocale();
    if (trans_global.load(QLocale(), "lingmo-nm", "_", "/usr/share/lingmo-nm/lingmo-nm/"))
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

    QTranslator sdkTranslator;
    if (sdkTranslator.load(QLocale(), "gui", "_", ":/translations/"))
    {
        a.installTranslator(&sdkTranslator);
        qDebug()<<"SDK Translations load success";
    } else {
        qWarning() << "SDK Translations load fail";
    }

    while (!p_networkResource->NetworkManagerIsInited()) {
        ::usleep(1000);
    }

    MainWindow w(display, nullptr);
    a.setActivationWindow(&w);
    w.setProperty("useStyleWindowManager", false); //禁用拖动
    a.setWindowIcon(QIcon::fromTheme("lingmo-network"));
    //设置窗口无边框，阴影

//    MotifWmHints window_hints;
//    window_hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
//    window_hints.functions = MWM_FUNC_ALL;
//    window_hints.decorations = MWM_DECOR_BORDER;
//    XAtomHelper::getInstance()->setWindowMotifHint(w.winId(), window_hints);

//    w.setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint /*| Qt::X11BypassWindowManagerHint*/);


    DbusAdaptor adaptor(display, &w);
    Q_UNUSED(adaptor);

    return a.exec();
}
