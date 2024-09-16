// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PERFORMANCEMONITOR_H
#define PERFORMANCEMONITOR_H


#include <QTime>
#include <QDateTime>
#include <QDebug>

class PerformanceMonitor
{

public:
    explicit PerformanceMonitor();

    static void initializeAppStart();
    static void initializAppFinish();
    static void closeAppStart();
    static void closeAPPFinish();
    static void openFileStart();
    static void openFileFinish(const QString &strFileName, qint64 iFileSize);

private:
    Q_DISABLE_COPY(PerformanceMonitor)

    static qint64 initializeAppStartMs;
    static qint64 inittalizeApoFinishMs;
    static qint64 closeAppStartMs;
    static qint64 closeAppFinishMs;
    static qint64 openFileStartMs;
    static qint64 openFileFinishMs;
};

#endif // PERFORMANCEMONITOR_H
