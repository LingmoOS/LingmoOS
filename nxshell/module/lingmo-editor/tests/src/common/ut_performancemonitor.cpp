// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_performancemonitor.h"
#include "../../src/common/performancemonitor.h"

test_performanceMonitor::test_performanceMonitor()
{

}

//explicit PerformanceMonitor(QObject *parent = nullptr);
TEST_F(test_performanceMonitor, PerformanceMonitor)
{
    PerformanceMonitor PerformanceMonitor;
    
}

//static void initializeAppStart();
TEST_F(test_performanceMonitor, initializeAppStart)
{
    PerformanceMonitor p;
    p.initializeAppStart();
    EXPECT_NE(p.initializeAppStartMs,0);
    
}

//static void initializAppFinish();
TEST_F(test_performanceMonitor, initializAppFinish)
{
    PerformanceMonitor p;
    p.initializAppFinish();

    EXPECT_NE(p.inittalizeApoFinishMs,0);
    
}

//static void closeAppStart();
TEST_F(test_performanceMonitor, closeAppStart)
{
    PerformanceMonitor p;
    p.closeAppStart();

    EXPECT_NE(p.closeAppStartMs,0);
    
}

//static void closeAPPFinish();
TEST_F(test_performanceMonitor, closeAPPFinish)
{
    PerformanceMonitor p;
    p.closeAPPFinish();

    EXPECT_NE(p.closeAppFinishMs,0);
    
}

//static void openFileStart();
TEST_F(test_performanceMonitor, openFileStart)
{
     PerformanceMonitor p;
     p.openFileStart();
    
     EXPECT_NE(p.openFileStartMs,0);
}

//static void openFileFinish(const QString &strFileName, qint64 iFileSize);
TEST_F(test_performanceMonitor, openFileFinish)
{
    PerformanceMonitor p;
    p.openFileFinish("aa",1);

    EXPECT_NE(p.openFileFinishMs,0);
    
}
