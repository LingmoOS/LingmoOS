/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2023 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <KCrash>
#include <QCoreApplication>
#include <QFile>
#include <QSocketNotifier>
#include <QTimer>

#include "baloodebug.h"
#include "extractor/commandpipe.h"

#include <signal.h>
#include <sys/resource.h>
#include <unistd.h> //for STDIN_FILENO

int main(int argc, char* argv[])
{
    using Baloo::Private::WorkerPipe;

    // Disable writing core dumps, we may crash on purpose
    struct rlimit corelimit{0, 0};
    setrlimit(RLIMIT_CORE, &corelimit);
    // DrKonqi blocks the signal and causes timeout of the QSignalSpy
    KCrash::setDrKonqiEnabled(false);
    QCoreApplication app(argc, argv);

    QFile input;
    input.open(STDIN_FILENO, QIODevice::ReadOnly | QIODevice::Unbuffered);
    QSocketNotifier inputNotifier(STDIN_FILENO, QSocketNotifier::Read);

    QFile output;
    output.open(STDOUT_FILENO, QIODevice::WriteOnly | QIODevice::Unbuffered);

    WorkerPipe worker(&input, &output);
    QObject::connect(&inputNotifier, &QSocketNotifier::activated,
                     &worker, &WorkerPipe::processIdData);

    QObject::connect(&worker, &WorkerPipe::inputEnd, &QCoreApplication::quit);
    QObject::connect(&worker, &WorkerPipe::newDocumentIds,
        [&worker](const QVector<quint64>& ids) {
            QTimer::singleShot(0, [&worker, ids]() {
                qCInfo(BALOO) << "Processing ...";

                for(auto id : ids) {
                    worker.urlStarted(QString::number(id));

		    if (id == 0) {
		      raise(SIGSEGV);
		    } else if (id == 1) {
		      exit(1);
		    } else if (id == 2) {
		      exit(2);
		    } else if (id < 100) {
                        worker.urlFailed(QString::number(id));
		    } else {
                        worker.urlFinished(QString::number(id));
		    }
                }
                worker.batchFinished();
                qCInfo(BALOO) << "Processing done";
            });
        });

    return app.exec();
}
