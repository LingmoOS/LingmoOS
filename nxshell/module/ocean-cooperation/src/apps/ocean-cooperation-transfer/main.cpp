// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "singleton/singleapplication.h"
#include "singleton/commandparser.h"
#include "base/baseutils.h"
#include "config.h"

#include "transfer/transferplugin.h"

#include <QDir>
#include <QProcess>

#include <signal.h>

using namespace lingmo_cross;
using namespace cooperation_transfer;

const char *dependProc = "ocean-cooperation";

static void appExitHandler(int sig)
{
    qInfo() << "break with !SIGTERM! " << sig;
    qApp->quit();
}

int main(int argc, char *argv[])
{
    // qputenv("QT_LOGGING_RULES", "ocean-cooperation-transfer.debug=true");
    // qputenv("CUTEIPC_DEBUG", "1");
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    lingmo_cross::SingleApplication app(argc, argv);
    app.setProperty("onlyTransfer", true);

    bool cooRunning = app.checkProcess(dependProc);
    if (cooRunning) {
        qInfo() << "cooperation App launched!";
    } else {
        // run backend and set minimize
        QProcess::startDetached(dependProc, QStringList() << "-m");
    }

    {
        // 加载翻译
        auto appName = app.applicationName();
        app.setApplicationName(dependProc);
        app.loadTranslator();
        app.setApplicationName(appName);
    }

    bool isSingleInstance = app.setSingleInstance(app.applicationName());
    if (!isSingleInstance) {
        QStringList msgs = app.arguments().mid(1); //remove first arg: app name
        if (msgs.isEmpty()) {
            msgs << "top"; // top show
        }
        qWarning() << "new client: " << msgs;
        app.onDeliverMessage(app.applicationName(), msgs);
        return 0;
    } else {
        CommandParser::instance().process();
        auto sendfiles = CommandParser::instance().processCommand("s");
        if (!sendfiles.isEmpty()) {
            app.setProperty("sendFiles", QVariant::fromValue(sendfiles));
        }
    }

    if (lingmo_cross::BaseUtils::isWayland()) {
        // do something
    }
    TransferPlugin *plugin = new TransferPlugin();
    plugin->start();

    QObject::connect(&app, &lingmo_cross::SingleApplication::onArrivedCommands, [&] (const QStringList &args) {
        CommandParser::instance().process(args);
        auto sendfiles = CommandParser::instance().processCommand("s");
        if (!sendfiles.isEmpty()) {
            app.setProperty("sendFiles", QVariant::fromValue(sendfiles));
        }
    });

    signal(SIGINT, appExitHandler);
    signal(SIGTERM, appExitHandler);
    int ret = app.exec();

    plugin->stop();

    return ret;
}
