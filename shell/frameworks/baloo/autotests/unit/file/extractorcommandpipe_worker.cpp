/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2021 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QCoreApplication>
#include <QFile>
#include <QSocketNotifier>
#include <QTimer>

#include "baloodebug.h"
#include "extractor/commandpipe.h"

#include <unistd.h> //for STDIN_FILENO

int main(int argc, char* argv[])
{
    using Baloo::Private::WorkerPipe;

    QCoreApplication app(argc, argv);

    QFile input;
    input.open(STDIN_FILENO, QIODevice::ReadOnly | QIODevice::Unbuffered);
    QSocketNotifier inputNotifier(STDIN_FILENO, QSocketNotifier::Read);

    QFile output;
    output.open(STDOUT_FILENO, QIODevice::WriteOnly | QIODevice::Unbuffered);

    WorkerPipe worker(&input, &output);
    QObject::connect(&inputNotifier, &QSocketNotifier::activated,
                     &worker, &WorkerPipe::processIdData);

    QObject::connect(&worker, &WorkerPipe::inputEnd, []() {
        qCInfo(BALOO) << "Pipe closed, exiting";
        QCoreApplication::quit();
    });
    QObject::connect(&worker, &WorkerPipe::newDocumentIds,
        [&worker](const QVector<quint64>& ids) {
            QTimer::singleShot(0, [&worker, ids]() {
                qCInfo(BALOO) << "Processing ...";
                for(auto id : ids) {
                    worker.urlStarted(QString::number(id));
                    worker.urlFinished(QString::number(id));
                }
                worker.batchFinished();
                qCInfo(BALOO) << "Processing done";
            });
        });

    return app.exec();
}
