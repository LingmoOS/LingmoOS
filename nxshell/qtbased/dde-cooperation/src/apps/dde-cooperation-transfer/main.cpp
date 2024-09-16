// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "singleton/singleapplication.h"
#include "singleton/commandparser.h"
#include "base/baseutils.h"
#include "config.h"

#include "transfer/transferplugin.h"

#include <QDir>
#include <QProcess>

using namespace cooperation_transfer;

const char *dependProc = "dde-cooperation";

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    deepin_cross::SingleApplication app(argc, argv);
    app.setProperty("onlyTransfer", true);

    bool cooRunning = app.checkProcess(dependProc);
    if (cooRunning) {
        qInfo() << "cooperation App launched!";
    } else {
        // run dde-cooperation and set minimize
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

    if (deepin_cross::BaseUtils::isWayland()) {
        // do something
    }
    TransferPlugin *plugin = new TransferPlugin();
    plugin->start();

    QObject::connect(&app, &deepin_cross::SingleApplication::onArrivedCommands, [&] (const QStringList &args) {
        CommandParser::instance().process(args);
        auto sendfiles = CommandParser::instance().processCommand("s");
        if (!sendfiles.isEmpty()) {
            app.setProperty("sendFiles", QVariant::fromValue(sendfiles));
        }
    });

    int ret = app.exec();

    plugin->stop();

    return ret;
}
