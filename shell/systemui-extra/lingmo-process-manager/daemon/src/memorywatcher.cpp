/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "memorywatcher.h"

#include <proc/sysinfo.h>

#include <QTimer>
#include <QDebug>

using namespace resource;

MemoryWatcher::MemoryWatcher(const std::map<Resource, ResourceUrgencyThreshold>& resourceThresholds,
                             QObject *parent)
    : ResourceWatcher{parent}
    , m_timer(new QTimer(this))
{
    initMemoryThreshold(resourceThresholds);

    connect(m_timer, &QTimer::timeout, this, &MemoryWatcher::checkMemoryRemaining);
}

void MemoryWatcher::initMemoryThreshold(const std::map<Resource, ResourceUrgencyThreshold>& resourceThresholds)
{
    using ResourceThreshold = std::pair<Resource, ResourceUrgencyThreshold>;
    for (const ResourceThreshold& resourceThreshold : resourceThresholds) {
        Resource resource = resourceThreshold.first;
        if (resource == Resource::Memory)
            m_memoryThreshold = resourceThreshold.second;
    }
    qDebug() << "Init memory thresholds:" << m_memoryThreshold[ResourceUrgency::Low]
             << m_memoryThreshold[ResourceUrgency::Medium] << m_memoryThreshold[ResourceUrgency::High];
}

void MemoryWatcher::checkMemoryRemaining()
{
    meminfo();
    uint mbMainAvailable =  kb_main_available / (double)1024;
    uint low = m_memoryThreshold[ResourceUrgency::Low];
    uint medium = m_memoryThreshold[ResourceUrgency::Medium];
    uint high = m_memoryThreshold[ResourceUrgency::High];
    uint warningLevel = 0;

    // The memory threshold is the amount of memory remaining,so it's safe
    // to go above the threshold, when less than the threshold will give a warning
    if (mbMainAvailable > low) {
        return;
    }

    if (mbMainAvailable <= low && mbMainAvailable > medium) {
        warningLevel = 1;
    } else if (mbMainAvailable <= medium && mbMainAvailable > high) {
        warningLevel = 2;
    } else if (mbMainAvailable <= high) {
        warningLevel = 3;
    }

    qDebug() << "Emit Memory resource warning level" << warningLevel
             << "memory remaining" << mbMainAvailable << "MB";
    Q_EMIT ResourceThresholdWarning("Memory", warningLevel);
}

void MemoryWatcher::stop()
{
    m_timer->stop();
}

void MemoryWatcher::start()
{
    qDebug() << "Start watch memory remaining";
    m_timer->start(1000);
}
