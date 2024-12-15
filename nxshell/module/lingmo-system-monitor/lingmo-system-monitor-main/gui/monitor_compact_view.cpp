// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "monitor_compact_view.h"

#include "compact_cpu_monitor.h"
#include "compact_disk_monitor.h"
#include "compact_memory_monitor.h"
#include "compact_network_monitor.h"
#include "detailwidgetmanager.h"

#include <DApplicationHelper>
#include <DPalette>
#include <DStyle>

#include <QVBoxLayout>

// constructor
MonitorCompactView::MonitorCompactView(QWidget *parent)
    : DFrame(parent)
{
    // disable auto fill frame background
    setAutoFillBackground(false);
    // set frame background role
    setBackgroundRole(DPalette::Window);
    // set no frame borders
    setFrameStyle(DFrame::NoFrame);

    // cpu monitor view instance
    m_cpuMonitor = new CompactCpuMonitor(this);
    // memory monitor view instance
    m_memoryMonitor = new CompactMemoryMonitor(this);
    // network monitor view instance
    m_networkMonitor = new CompactNetworkMonitor(this);
    // disk monitor view instance
    m_diskMonitor = new CompactDiskMonitor(this);

    // vertical layout to hold monitor instances
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(10);
    layout->addStretch(1);
    layout->addWidget(m_cpuMonitor, 0, Qt::AlignHCenter);
    layout->addStretch(2);
    layout->addWidget(m_memoryMonitor, 0, Qt::AlignHCenter);
    layout->addStretch(2);
    layout->addWidget(m_networkMonitor, 0, Qt::AlignHCenter);
    layout->addStretch(2);
    layout->addWidget(m_diskMonitor, 0, Qt::AlignHCenter);
    layout->addStretch(1);

    // set frame layout
    setLayout(layout);

    connect(m_cpuMonitor, &CompactCpuMonitor::signalDetailInfoClicked, this, &MonitorCompactView::signalDetailInfoClicked);
    // 连接dbus发来的信号
    connect(&DetailWidgetManager::getInstance(), &DetailWidgetManager::sigJumpToDetailWidget, this, [=](QString msgCode) {
        emit signalDetailInfoByDbus(msgCode);
    });

    //点击左侧区域触发跳转
    connect(m_cpuMonitor, &CompactCpuMonitor::clicked, &DetailWidgetManager::getInstance(), &DetailWidgetManager::jumpDetailWidget);
    connect(m_memoryMonitor, &CompactMemoryMonitor::clicked, &DetailWidgetManager::getInstance(), &DetailWidgetManager::jumpDetailWidget);
    connect(m_networkMonitor, &CompactNetworkMonitor::clicked, &DetailWidgetManager::getInstance(), &DetailWidgetManager::jumpDetailWidget);
    connect(m_diskMonitor, &CompactDiskMonitor::clicked, &DetailWidgetManager::getInstance(), &DetailWidgetManager::jumpDetailWidget);
}

void MonitorCompactView::setDetailButtonVisible(bool visible)
{
    m_cpuMonitor->setDetailButtonVisible(visible);
}
