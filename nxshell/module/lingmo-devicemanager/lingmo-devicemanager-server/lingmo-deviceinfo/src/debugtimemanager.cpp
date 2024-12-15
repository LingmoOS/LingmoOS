// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "debugtimemanager.h"
#include "DDLog.h"

#include <QDateTime>
#include <QLoggingCategory>

#include <sys/time.h>

using namespace DDLog;

DebugTimeManager    *DebugTimeManager::s_Instance = nullptr;

DebugTimeManager::DebugTimeManager()
{

}

void DebugTimeManager::clear()
{
    m_MapPoint.clear();
}

void DebugTimeManager::beginPointQt(const QString &point, const QString &status)
{
    PointInfo info;
    info.desc = status;
    info.time = QDateTime::currentMSecsSinceEpoch();
    m_MapPoint.insert(point, info);
}

void DebugTimeManager::endPointQt(const QString &point)
{
    if (m_MapPoint.find(point) != m_MapPoint.end()) {
        m_MapPoint[point].time = QDateTime::currentMSecsSinceEpoch() - m_MapPoint[point].time;
        qCInfo(appLog) << QString("[GRABPOINT] %1 %2 time=%3ms").arg(point).arg(m_MapPoint[point].desc).arg(m_MapPoint[point].time);
    }
}

void DebugTimeManager::beginPointLinux(const QString &point, const QString &status)
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);

    PointInfo info;
    info.desc = status;
    info.time = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    m_MapPoint.insert(point, info);
}

void DebugTimeManager::endPointLinux(const QString &point)
{
    if (m_MapPoint.find(point) != m_MapPoint.end()) {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        m_MapPoint[point].time = tv.tv_sec * 1000 + tv.tv_usec / 1000 - m_MapPoint[point].time;
        qCInfo(appLog) << QString("[GRABPOINT] %1 %2 time=%3ms").arg(point).arg(m_MapPoint[point].desc).arg(m_MapPoint[point].time);
    }
}
