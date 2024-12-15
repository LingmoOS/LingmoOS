// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "performancemonitor.h"
#include <QDateTime>
#include <QDebug>

static qint64 initializeAppStartMs = 0;
static qint64 initializeAppFinishMs = 0;

/**
 * @brief PerformanceMonitor::initializeAppStart
 * 记录程序初始化开始时间
 */
void PerformanceMonitor::initializeAppStart()
{
    QDateTime current = QDateTime::currentDateTime();
    initializeAppStartMs = current.toMSecsSinceEpoch();
}

/**
 * @brief PerformanceMonitor::initializeAppFinish
 *  记录程序初始化结束时间
 */
void PerformanceMonitor::initializeAppFinish()
{
    QDateTime current = QDateTime::currentDateTime();
    initializeAppFinishMs = current.toMSecsSinceEpoch();
    qint64 time = initializeAppFinishMs - initializeAppStartMs;
    qInfo() << QString("[GRABPOINT] POINT-01 startduration=%1ms").arg(time);
}
