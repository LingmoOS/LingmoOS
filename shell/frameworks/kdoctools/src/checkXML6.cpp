/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2015-2016 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "loggingcategory.h"

#include <QCoreApplication>
#include <QProcess>
#include <QStandardPaths>

#include <stdio.h>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    const QStringList arguments = app.arguments();
    if (arguments.count() != 2) {
        qCCritical(KDocToolsLog) << "wrong argument count";
        return (1);
    }

    const QString exec = QStandardPaths::findExecutable(QStringLiteral("meinproc6"));
    if (exec.isEmpty()) {
        qCCritical(KDocToolsLog) << "Could not find meinproc6 executable in PATH";
        return 1;
    }

    QProcess meinproc;
    meinproc.start(exec, QStringList{QStringLiteral("--check"), QStringLiteral("--stdout"), arguments[1]});
    if (!meinproc.waitForStarted()) {
        return -2;
    }
    if (!meinproc.waitForFinished()) {
        return -1;
    }
    fprintf(stderr, "%s", meinproc.readAllStandardError().constData());
    return meinproc.exitCode();
}
