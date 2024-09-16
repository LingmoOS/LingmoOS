// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DebugTimeManager.h"
#include <QDateTime>
#include <QLoggingCategory>
#include "DDLog.h"

using namespace DDLog;

#include <sys/time.h>

DebugTimeManager    *DebugTimeManager::s_Instance = nullptr;

DebugTimeManager::DebugTimeManager()
{

}

void DebugTimeManager::clear()
{
    m_MapPoint.clear();
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

void DebugTimeManager::endPointLinux(const QString &point, const QString &sub)
{
    if (m_MapPoint.find(point) != m_MapPoint.end()) {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        m_MapPoint[point].time = tv.tv_sec * 1000 + tv.tv_usec / 1000 - m_MapPoint[point].time;

        if (m_MapPoint.find(sub) != m_MapPoint.end()) {
            m_MapPoint[point].time = m_MapPoint[point].time - m_MapPoint[sub].time;
        }

        qCInfo(appLog) << QString("[GRABPOINT] %1 %2 time=%3ms").arg(point).arg(m_MapPoint[point].desc).arg(m_MapPoint[point].time);
    }
}
