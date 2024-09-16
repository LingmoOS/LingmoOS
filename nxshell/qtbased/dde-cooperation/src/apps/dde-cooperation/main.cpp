// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "singleton/singleapplication.h"
#include "singleton/commandparser.h"
#include "base/baseutils.h"
#include "config.h"

#include "core/cooperationcoreplugin.h"

#include <QDir>
#include <QIcon>
#include <QDebug>
#include <QProcess>

#include <signal.h>

#if defined(_WIN32) || defined(_WIN64)
// Fix no OPENSSL_Applink crash issue.
extern "C"
{
#include <openssl/applink.c>
}
#endif

using namespace deepin_cross;
using namespace cooperation_core;

const char *compatProc = "cooperation-daemon";

static void appExitHandler(int sig)
{
    qInfo() << "break with !SIGTERM! " << sig;
    qApp->quit();
}

int main(int argc, char *argv[])
{
    // qputenv("QT_LOGGING_RULES", "dde-cooperation.debug=true");
    // qputenv("CUTEIPC_DEBUG", "1");
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    deepin_cross::SingleApplication app(argc, argv);

#ifdef linux
    app.loadTranslator();
    app.setApplicationDisplayName(app.translate("Application", "Cooperation"));
    app.setApplicationVersion(APP_VERSION);
    app.setProductIcon(QIcon::fromTheme("dde-cooperation"));
    app.setApplicationAcknowledgementPage("https://www.deepin.org/acknowledgments/");
    app.setApplicationDescription(app.translate("Application", "Cooperation is a powerful cross-terminal "
                                                               "office tool that helps you deliver files, "
                                                               "share keys and mice, and share clipboards "
                                                               "between different devices."));
#endif

    //Dconfig: should check the compatibility mode
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

    CooperaionCorePlugin *core = new CooperaionCorePlugin();
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
