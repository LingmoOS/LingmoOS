/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2014 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <iostream>

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QFileInfoList>
#include <QTest>

#include "Benchmark.h"
#include "MainWindow.h"
#include "Model.h"

using namespace std;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName(QLatin1String("ThumbNailer"));
    app.setOrganizationDomain(QLatin1String("kde.org"));
    QCommandLineParser parser;
    parser.setApplicationDescription(app.translate("main", "ThreadWeaver ThumbNailer Example"));
    parser.addHelpOption();
    parser.addPositionalArgument(QLatin1String("mode"), QLatin1String("Benchmark or demo mode"));
    parser.process(app);
    const QStringList positionals = parser.positionalArguments();
    const QString mode = positionals.isEmpty() ? QLatin1String("demo") : positionals[0];
    if (mode == QLatin1String("benchmark")) {
        Benchmark benchmark;
        const QStringList arguments = app.arguments().mid(1); // remove mode selection
        return QTest::qExec(&benchmark, arguments);
    } else if (mode == QLatin1String("demo")) {
        // demo mode
        MainWindow mainWindow;
        mainWindow.show();
        return app.exec();
    } else {
        wcerr << "Unknown mode " << mode.toStdWString() << endl << endl;
        parser.showHelp();
        Q_UNREACHABLE();
    }
    return 0;
}
