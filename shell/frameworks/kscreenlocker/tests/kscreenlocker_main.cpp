/*
SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "../ksldapp.h"
#include <QCommandLineParser>
#include <QGuiApplication>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("This test application starts the screen locker immediately and\n"
                       "exits once the screen got successfully unlocked. The purpose is\n"
                       "to test changes in KSLD without having to restart KSMServer.\n"
                       "Thus it's a good way to verify grabbing of keyboard/pointer and\n"
                       "the communication with kscreenlocker_greet. If the lock is not\n"
                       "working properly the test application can be killed and the\n"
                       "screen is unlocked again. If one just wants to test the greeter\n"
                       "it's better to start just kscreenlocker_greet."));

    parser.addHelpOption();

    parser.process(app);

    ScreenLocker::KSldApp locker(&app);
    locker.initialize();
    QObject::connect(&locker, &ScreenLocker::KSldApp::unlocked, &app, &QGuiApplication::quit);
    locker.lock(ScreenLocker::EstablishLock::Immediate);

    return app.exec();
}
