/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <kcrash.h>
#ifdef Q_OS_UNIX
#include <cerrno>
#include <sys/resource.h> // setrlimit
#endif

QFile output;

void saveFunction(int)
{
    output.write("saveFunction called\n");
    output.flush();
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    KCrash::initialize();

    const QStringList args = app.arguments();
    QByteArray flag = args.count() > 1 ? args.at(1).toLatin1() : QByteArray();

    if (flag == "AR") { // auto restart
        KCrash::setFlags(KCrash::AutoRestart);
    } else if (flag == "ARD") { // auto restart, always directly
        KCrash::setFlags(KCrash::AutoRestart | KCrash::AlwaysDirectly);
    } else if (flag == "ES") { // emergency save
        KCrash::setEmergencySaveFunction(saveFunction);
    }

#ifdef Q_OS_UNIX
    // No core file
    struct rlimit rlp;
    rlp.rlim_cur = 0;
    rlp.rlim_max = 0;
    if (setrlimit(RLIMIT_CORE, &rlp) != 0) {
        qDebug() << strerror(errno);
    }
#endif

    output.setFileName(QStringLiteral("kcrashtest_log"));
    if (!output.open(QIODevice::WriteOnly | QIODevice::Append)) {
        return 1;
    }
    if (qEnvironmentVariableIsEmpty("KCRASH_AUTO_RESTARTED")) {
        output.write("starting ");
        output.write(flag);
        output.write("\n");
        output.flush();
        // CRASH!
        delete (char *)0xdead;
    } else {
        output.write("autorestarted ");
        output.write(flag);
        output.write("\n");
        output.close();
    }

    return 0;
}
