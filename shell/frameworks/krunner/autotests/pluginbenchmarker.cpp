/*
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "runnermanager.h"
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTimer>

#include <iostream>

static void runQuery(const QString &runnerId, const QString &query, int iterations)
{
    QEventLoop loop;
    KRunner::RunnerManager manager;
    manager.setAllowedRunners({runnerId});
    manager.launchQuery("test");
    qWarning() << "Following runners are loaded" << manager.runners();
    QObject::connect(&manager, &KRunner::RunnerManager::queryFinished, &loop, &QEventLoop::quit);
    for (int i = 0; i < iterations; ++i) {
        // Setup of match session is done for first query automatically
        for (int j = 1; j <= query.size(); ++j) {
            manager.launchQuery(query.mid(0, j));
            loop.exec();
        }
        manager.matchSessionComplete();
    }
}

int main(int argv, char **argc)
{
    QCoreApplication app(argv, argc);
    QCommandLineOption iterationsOption("iterations", "Number of iterations where the query will be run", "iterations");

    QCommandLineParser parser;
    parser.addOption(iterationsOption);
    parser.addPositionalArgument("runner", "The runnerId you want to load");
    parser.addPositionalArgument("query", "The query to run, each letter is launched as it's own query");
    parser.process(app);

    const QStringList positionalArguments = parser.positionalArguments();
    if (positionalArguments.size() != 2) {
        qWarning() << "The runnerId and query must be specified as the only positional arguments";
        exit(1);
    }
    const QString runner = positionalArguments.at(0);
    const QString query = positionalArguments.at(1);
    int iterations = parser.isSet(iterationsOption) ? parser.value(iterationsOption).toInt() : 1;
    if (iterations < 1) {
        qWarning() << "invalid iterations value set, it must be more than 1" << iterations;
    }

    QTimer::singleShot(0, &app, [&app, runner, query, iterations]() {
        runQuery(runner, query, iterations);
        QTimer::singleShot(10, &app, &QCoreApplication::quit);
        std::cout << "Finished running queries" << std::endl;
    });

    return app.exec();
}
