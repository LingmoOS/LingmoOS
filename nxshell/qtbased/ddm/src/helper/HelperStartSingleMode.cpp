// Copyright (C) 2023 Dingyuan Zhang <lxz@mkacg.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <unistd.h>
#include <QCoreApplication>
#include <QTextStream>
#include <QDebug>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "singlewaylandhelper.h"
#include "MessageHandler.h"
#include <signal.h>
#include "Auth.h"
#include "SignalHandler.h"

void WaylandHelperMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    DDM::messageHandler(type, context, QStringLiteral("WaylandHelper: "), msg);
}

int main(int argc, char** argv)
{
    qInstallMessageHandler(WaylandHelperMessageHandler);
    QCoreApplication app(argc, argv);
    using namespace DDM;
    DDM::SignalHandler s;

    //Q_ASSERT(::getuid() != 0);
    if (argc < 3) {
        QTextStream(stderr) << "Wrong number of arguments\n";
        return Auth::HELPER_OTHER_ERROR;
    }

    SingleWaylandHelper helper;
    QObject::connect(&s, &DDM::SignalHandler::sigtermReceived, &app, [] {
        QCoreApplication::exit(0);
    });

    if (!helper.start(app.arguments()[1], app.arguments()[2])) {
        qWarning() << "DDM was unable to start" << app.arguments()[1];
        return Auth::HELPER_DISPLAYSERVER_ERROR;
    }

    return app.exec();
}
