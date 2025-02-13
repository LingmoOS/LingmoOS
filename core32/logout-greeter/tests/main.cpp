/*
    ksmserver - the KDE session management server

    SPDX-FileCopyrightText: 2016 Martin Graesslin <mgraesslin@kde.org>

    SPDX-License-Identifier: MIT
*/
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFutureWatcher>
#include <QProcess>
#include <QtConcurrentRun>

#include <unistd.h>

#include <config.h>

static void readFromPipe(int pipe)
{
    QFile readPipe;
    if (!readPipe.open(pipe, QIODevice::ReadOnly)) {
        QCoreApplication::exit(1);
        return;
    }
    QByteArray result = readPipe.readLine();
    qDebug() << "!!!! Result from helper process: " << result;
    if (result.isEmpty()) {
        qDebug() << "!!!! Error";
        QCoreApplication::exit(1);
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();

    QCommandLineOption shutdownAllowedOption(QStringLiteral("shutdown-allowed"), QStringLiteral("Whether the user is allowed to shut down the system."));
    parser.addOption(shutdownAllowedOption);

    QCommandLineOption chooseOption(QStringLiteral("choose"), QStringLiteral("Whether the user is offered the choices between logout, shutdown, etc."));
    parser.addOption(chooseOption);

    QCommandLineOption modeOption(QStringLiteral("mode"),
                                  QStringLiteral("The initial exit mode to offer to the user."),
                                  QStringLiteral("logout|shutdown|reboot"),
                                  QStringLiteral("logout"));
    parser.addOption(modeOption);

    parser.process(app);

    int pipeFds[2];
    if (pipe(pipeFds) != 0) {
        return 1;
    }

    QProcess p;
    p.setProgram(QStringLiteral(LOGOUT_GREETER_BIN));
    QStringList arguments;
    if (parser.isSet(shutdownAllowedOption)) {
        arguments << QStringLiteral("--shutdown-allowed");
    }
    if (parser.isSet(chooseOption)) {
        arguments << QStringLiteral("--choose");
    }
    if (parser.isSet(modeOption)) {
        arguments << QStringLiteral("--mode");
        arguments << parser.value(modeOption);
    }
    arguments << QStringLiteral("--mode-fd");
    arguments << QString::number(pipeFds[1]);
    p.setArguments(arguments);

    QObject::connect(&p, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred), &app, [] {
        QCoreApplication::exit(1);
    });

    const int resultPipe = pipeFds[0];
    QObject::connect(&p, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), &app, [resultPipe](int exitCode) {
        if (exitCode != 0) {
            qDebug() << "!!!! finished with exit code: " << exitCode;
            close(resultPipe);
            QCoreApplication::exit(1);
            return;
        }
        QFutureWatcher<void> *watcher = new QFutureWatcher<void>();
        QObject::connect(watcher, &QFutureWatcher<void>::finished, QCoreApplication::instance(), &QCoreApplication::quit, Qt::QueuedConnection);
        QObject::connect(watcher, &QFutureWatcher<void>::finished, watcher, &QFutureWatcher<void>::deleteLater, Qt::QueuedConnection);
        watcher->setFuture(QtConcurrent::run(readFromPipe, resultPipe));
    });

    p.start();
    close(pipeFds[1]);

    return app.exec();
}
