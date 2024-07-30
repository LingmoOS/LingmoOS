/*
    This file is part of libkdbus

    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <kdbusservice.h>

// sigaction
#include <signal.h>
// close
#include <unistd.h>

// rlimit
#include <sys/resource.h>
#include <sys/time.h>

// USR1.
// Close all sockets and eventually ABRT. This mimics behavior during KCrash
// handling. Closing all sockets will effectively disconnect us from the bus
// and result in the daemon reclaiming all our service names and allowing
// other processes to register them instead.
void usr1_handler(int signum)
{
    qDebug() << "usr1" << signum << SIGSEGV;

    // Close all remaining file descriptors so we drop off of the bus.
    struct rlimit rlp;
    getrlimit(RLIMIT_NOFILE, &rlp);
    for (int i = 3; i < (int)rlp.rlim_cur; i++) {
        close(i);
    }

    // Sleep a bit for good measure. We could actually loop ad infinitum here
    // as after USR1 we are expected to get killed. In the interest of sane
    // behavior we'll simply exit on our own as well though.
    sleep(4);
    abort();
}

// Simple application under test.
// Closes all sockets on USR1 and aborts to simulate a kcrash shutdown behavior
// which can result in a service registration race.
int main(int argc, char *argv[])
{
    qDebug() << "hello there!";

    struct sigaction action;

    action.sa_handler = usr1_handler;
    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, SIGUSR1);
    action.sa_flags = SA_RESTART;

    if (sigaction(SIGUSR1, &action, nullptr) < 0) {
        qDebug() << "failed to register segv handler";
        return 1;
    }

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("kdbussimpleservice");
    QCoreApplication::setOrganizationDomain("kde.org");

    KDBusService service(KDBusService::Unique);
    if (!service.isRegistered()) {
        qDebug() << "service not registered => exiting";
        return 1;
    }
    qDebug() << "service registered";

    int ret = app.exec();
    qDebug() << "exiting deadservice";
    return ret;
}
