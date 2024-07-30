/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2014 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <QFileInfoList>
#include <QObject>

class Benchmark : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void processThumbNailsAsBenchmarkInLoop();
    void processThumbNailsAsBenchmarkWithThreadWeaver();

private:
    const QFileInfoList images();
};

#endif // BENCHMARK_H
