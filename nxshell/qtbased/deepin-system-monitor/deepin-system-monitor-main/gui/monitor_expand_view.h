// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MONITOR_EXPAND_VIEW_H
#define MONITOR_EXPAND_VIEW_H

#include <DFrame>

DWIDGET_USE_NAMESPACE

class CpuMonitor;
class MemoryMonitor;
class NetworkMonitor;

/**
 * @brief Expand monitor view frame
 */
class MonitorExpandView : public DFrame
{
    Q_OBJECT

public:
    /**
     * @brief Default constructor
     * @param parent Parent object
     */
    explicit MonitorExpandView(QWidget *parent = nullptr);
    // Default destructor
    ~MonitorExpandView() = default;

public:
    void setDetailButtonVisible(bool visible);

signals:
    void signalDetailInfoClicked();

    void signalDetailInfoByDbus(QString);


private:
    // Cpu monitor view
    CpuMonitor *m_cpuMonitor {};
    // Memory monitor view
    MemoryMonitor *m_memoryMonitor {};
    // Network monitor view
    NetworkMonitor *m_networkMonitor {};
};

#endif  // MONITOR_EXPAND_VIEW_H
