// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "singleton/singleapplication.h"
#include "base/baseutils.h"
#include "config.h"
#include "core/datatransfercoreplugin.h"

#include <QDir>
#include <QIcon>
#include <QTranslator>
#include <QDebug>
#include <QProcess>

#include <signal.h>

using namespace deepin_cross;
using namespace data_transfer_core;

const char *compatProc = "cooperation-daemon";

static void appExitHandler(int sig)
{
    qInfo() << "break with !SIGTERM! " << sig;
    qApp->quit();
}

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    deepin_cross::SingleApplication app(argc, argv);

#ifdef linux
    app.loadTranslator();
    app.setApplicationName("deepin-data-transfer");
    app.setApplicationDisplayName(app.translate("Application", "UOS data transfer"));
    app.setApplicationVersion(APP_VERSION);
    QIcon icon(":/icon/icon_256.svg");
    app.setProductIcon(icon);
    app.setApplicationAcknowledgementPage("https://www.deepin.org/acknowledgments/" );
    app.setApplicationDescription(app.translate("Application", "UOS transfer tool enables one click migration of your files, personal data, and applications to UOS, helping you seamlessly replace your system."));
#endif

    bool compatDaemonRun = app.checkProcess(compatProc);
    if (compatDaemonRun) {
        qInfo() << "compat backend launched!";
    } else {
        QString procPath = QString(COMPAT_DAEMON_DIR);
        if (procPath.isEmpty()) {
            procPath = QCoreApplication::applicationDirPath();
        }
        procPath.append("/").append(compatProc);
#if defined(_WIN32) || defined(_WIN64)
        procPath.append(".exe");
#endif
        qWarning() << procPath;
        QFile procexe = QFile(procPath);
        if (procexe.exists()) {
            // run compat daemon backend
            QProcess::startDetached(procPath, QStringList());
        } else {
            qWarning() << "compat backend is not exist!";
        }
    }

    bool canSetSingle = app.setSingleInstance(app.applicationName());
    if (!canSetSingle) {
        qCritical() << app.applicationName() << "is already running.";
        return 0;
    }

    if (deepin_cross::BaseUtils::isWayland()) {
        // do something
    }
    DataTransferCorePlugin *core = new DataTransferCorePlugin();
    core->start();

    // 安全退出
#ifndef _WIN32
    signal(SIGQUIT, appExitHandler);
#endif
    signal(SIGINT, appExitHandler);
    signal(SIGTERM, appExitHandler);
    int ret = app.exec();

    core->stop();

    return ret;
}
