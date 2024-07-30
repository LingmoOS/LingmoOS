/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2014 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTemporaryDir>
#include <QTest>

#include <ThreadWeaver/ThreadWeaver>

#include "Benchmark.h"
#include "Model.h"

using namespace std;
using namespace ThreadWeaver;

void Benchmark::processThumbNailsAsBenchmarkInLoop()
{
    const QFileInfoList files = images();
    { // create a block to avoid adding the time needed to remove the temporary
        // directory from the file system to the measured time:
        QTemporaryDir temp;
        QBENCHMARK {
            Model model;
            model.prepareConversions(files, temp.path());
            QVERIFY(model.computeThumbNailsBlockingInLoop());
        }
    }
}

void Benchmark::processThumbNailsAsBenchmarkWithThreadWeaver()
{
    const QFileInfoList files = images();
    { // create a block to avoid adding the time needed to remove the temporary
        // directory from the file system to the measured time:
        QTemporaryDir temp;
        QBENCHMARK {
            Model model;
            model.prepareConversions(files, temp.path());
            QVERIFY(model.computeThumbNailsBlockingConcurrent());
        }
    }
}

const QFileInfoList Benchmark::images()
{
    const QDir dir = QDir(QLatin1String("/usr/share/backgrounds"));
    return dir.entryInfoList(QStringList() << QLatin1String("*.jpg"));
}

#include "moc_Benchmark.cpp"
